#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <dirent.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sysinfo.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXPATH 64

typedef struct {
    uint64_t pid;
    float cpu_percentage;
    float memory_percentage;
    uint64_t vsz;
    uint64_t rss;
    char tty[11];
    char state[8];
    char start_time[6];
    char runtime[8];
    char cmdline[4 * 1024 + 2];
    char* user; //this is a static space owned by passwd
} proc ;

//0 failure
//>0 success & file size
size_t readfile(const char* path, char* data, size_t file_size) {
    int handler = open(path, O_RDONLY);
    if (handler < 0) {
        //Unable to open specific files
        return 0;
    }
    //Blocking and readAll
    size_t tmp;
    ssize_t ret;
    tmp = 0;
    while (tmp < file_size) {
        ret = read(handler, data + tmp, file_size - tmp - 1);
        if (ret < 0) {
            return 0;
        } else if (ret == 0) {
            break;
        } else {
            //Positive interger
            tmp += ret;
        }
    }
    close(handler);    //Close file
    data[tmp] = 0;  //null terminate
    return tmp;
}

//0 failure
//1 success
int readproc(const char* pid, proc* p) {
    char path_buffer[MAXPATH];  // /proc/$PID/XXX
    char* appender = path_buffer + 6 + strlen(pid) + 1; //skip $pid/
    char stat_buffer[4096];
    char comm_buffer[4096];
    char status_buffer[4096];
    char* file_data;
    unsigned long int utime, stime;
    int session, pgrp, tpgid, stateend;
    long nice, num_threads;
    unsigned long long starttime;
    struct sysinfo systeminfo;
    struct stat filestat;
    struct tm* local_tm;
    time_t tmp_time;
    size_t file_size, i;
    uint64_t Tgid = 0, VmLck = 0;
    int tty = 0;
    uint32_t now_year = 0, now_day_of_year = 0, total_cpu_time = 0;

    // initialize path buffer
    strcpy(path_buffer, "/proc/");
    strcat(path_buffer, pid);
    strcat(path_buffer, "/");

    // parse pid
    p->pid = atoll(pid);

    // read Tgid and vm_lock
    strcpy(appender, "status");
    file_size = readfile(path_buffer, status_buffer, 4096);
    if (file_size < 0)
        return 0;
    file_data = status_buffer;
    for (;;) {
        char* colon;
        colon = strchr(file_data, ':');
        if (colon - file_data == 4 && memcmp(file_data, "Tgid", 4) == 0) {
            // have Tgid
            file_data += 4; //skip field name
            file_data += 2; //skip :\t
            Tgid = strtol(file_data, &file_data, 10);
            if (Tgid != p->pid)
                return 0;   //this is a thread folder
        } else if (colon - file_data == 5 && memcmp(file_data, "VmLck", 5) == 0) {
            // have VmLck
            file_data += 5; //skip field name
            file_data += 2; //skip :\t
            VmLck = strtol(file_data, &file_data, 10);
        }
        file_data = strchr(file_data, '\n');
        if (file_data == 0)
            break;  //no new line
        file_data++;
        if (*file_data == 0)
            break;  //last line
    }

    // read command line
    strcpy(appender, "cmdline");
    file_size = readfile(path_buffer, p->cmdline, 4096);
    if (file_size < 0)
        return 0;   //unable to read command line
    for (i = 0;i < file_size;i++) {
        if (p->cmdline[i] == 0) {
            p->cmdline[i] = ' ';    //sanitize cmdline
        }
    }

    if (strlen(p->cmdline) == 0) {
        //probably a system process
        strcpy(appender, "comm");
        file_size = readfile(path_buffer, comm_buffer, 4096);
        if (file_size < 0)
            return 0;
        strcat(p->cmdline, "[");
        strcat(p->cmdline, comm_buffer);
        p->cmdline[strlen(p->cmdline) - 1] = ']';   //comm is new line terminated
    }

    // read stat file
    strcpy(appender, "stat");
    file_size = readfile(path_buffer, stat_buffer, 4096);
    if (file_size < 0)
        return 0;
    sscanf(stat_buffer, "%*d %*s %c %*d %d %d %d %d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %ld %ld %*ld %llu %lu %ld", p->state, &pgrp, &session, &tty, &tpgid, &utime, &stime, &nice, &num_threads, &starttime, &p->vsz, &p->rss);

    // fill in minor code of state
    stateend = 1;
    if (nice < 0)
        p->state[stateend++] = '<';
    if (nice > 0)
        p->state[stateend++] = 'N';
    if (VmLck > 0)
        p->state[stateend++] = 'L';
    if (session == Tgid)
        p->state[stateend++] = 's';
    if (num_threads > 1)
        p->state[stateend++] = 'l';
    if (pgrp == tpgid)
        p->state[stateend++] = '+';
    p->state[stateend] = 0;


    // convert some format
    p->vsz /= 1024; //convert VSZ to kilobytes
    p->rss *= getpagesize();    //multiply with pagesize
    p->rss /= 1024; //convert RSS to kilobytes

    // read uptime & calculate percentages
    sysinfo(&systeminfo);
    if (systeminfo.uptime - (float)starttime / sysconf(_SC_CLK_TCK) == 0.0)
        p->cpu_percentage = 0.0;
    else
        p->cpu_percentage = ((float)(utime + stime) / sysconf(_SC_CLK_TCK)) / (systeminfo.uptime - (float)starttime / sysconf(_SC_CLK_TCK)) * 100;
    p->memory_percentage = (float)p->rss / ((float)systeminfo.totalram / 1024) * 100;

    // get username
    stat(path_buffer, &filestat);   //stat the stat file
    p->user = getpwuid(filestat.st_uid)->pw_name;

    // calculate start time
    tmp_time = time(0);
    local_tm = localtime(&tmp_time);
    now_year = local_tm->tm_year;
    now_day_of_year = local_tm->tm_yday;

    tmp_time = time(0) - (systeminfo.uptime - starttime / sysconf(_SC_CLK_TCK));
    local_tm = localtime(&tmp_time);

    if(local_tm->tm_year != now_year)
        strftime(p->start_time, 6, "%Y", local_tm);     //not the same year
    else if (local_tm->tm_yday != now_day_of_year)      //the same year, but not the same day
        strftime(p->start_time, 6, "%b%d", local_tm);
    else
        strftime(p->start_time, 6, "%H:%M", local_tm);  //the same day

    // calculate runtime
    total_cpu_time = (utime + stime) / sysconf(_SC_CLK_TCK);
    sprintf(p->runtime, "%d:%02d", total_cpu_time / 60, total_cpu_time % 60);

    // adjust TTY name
    p->tty[0] = 0;

    if (major(tty) == 136)
        strcpy(p->tty, "pts/");
    else if(major(tty) == 4)
        strcpy(p->tty, "tty");
    else {
        strcpy(p->tty, "?");
        return 1;
    }
    sprintf(p->tty + strlen(p->tty), "%d", minor(tty));
    return 1;
}

void displayproc(proc* p) {
    printf("%-10s %7ld %6.1lf %4.1lf %10lu %8lu %-11s %-4s %8s %10s %s\n", p->user, p->pid, p->cpu_percentage, p->memory_percentage, p->vsz, p->rss, p->tty, p->state, p->start_time, p->runtime, p->cmdline);
}

int main() {
    DIR* testdir = opendir("/proc/");
    struct dirent* dir;
    proc p;
    printf("%-10s %7s %6s %4s %10s %8s %-11s %-4s %8s %10s %s\n", "USER", "PID", "%CPU", "%MEM", "VSZ", "RSS", "TT", "STAT", "STARTED", "TIME", "COMMAND");
    while ((dir = readdir(testdir)) != NULL) {
        if (dir->d_type == DT_DIR && dir->d_name[0] > '0' && dir->d_name[0] <= '9') {
            if (readproc(dir->d_name, &p)) {
                displayproc(&p);
            }
        }
    }
    closedir(testdir);
    return 0;
}