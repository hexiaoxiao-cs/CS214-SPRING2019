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

#define MAXPATH 64

typedef struct {
    uint64_t pid;
    float cpu_percentage;
    float memory_percentage;
    uint64_t vsz;
    uint64_t rss;
    uint32_t tty;
    char state[8];
    char start_time[6];
    uint64_t runtime;
    char cmdline[4 * 1024 + 2];
    char* user; //this is a static space owned by passwd
} proc ;

//0 failure
//1 success
int readfilea(const char* path, char** data, size_t* size) {
    int handler = open(path, O_RDONLY);
    if (handler < 0) {
        //Unable to open specific files
        return 0;
    }
    //Blocking and readAll
    size_t tmp;
    ssize_t ret;
    size_t file_size = (size_t) lseek(handler, 0, SEEK_END);
    lseek(handler, 0, SEEK_SET);
    char *huge_shit = (char *) malloc(file_size + 1);
    huge_shit[file_size] = 0;       //Zero terminated
    tmp = 0;
    while (tmp < file_size) {
        ret = read(handler, huge_shit + tmp, file_size - tmp);
        if (ret < 0) {
            return 0;
        } else if (ret == 0) {
            break;
        } else {
            //Positive interger
            tmp += ret;
        }
    }
    *data = huge_shit;
    *size = file_size;
    close(handler);    //Close file
    return 1;
}

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
    //TODO: minor status code, starttime, runtime
    char path_buffer[MAXPATH];  // /proc/$PID/XXX
    char* appender = path_buffer + 6 + strlen(pid) + 1; //skip $pid/
    char stat_buffer[4096];
    char comm_buffer[4096];
    char* file_data;
    unsigned long int utime, stime;
    unsigned long long starttime;
    struct sysinfo systeminfo;
    struct stat filestat;
    size_t file_size, i;

    // initialize path buffer
    strcpy(path_buffer, "/proc/");
    strcat(path_buffer, pid);
    strcat(path_buffer, "/");


    // parse pid
    p->pid = atoll(pid);


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
    sscanf(stat_buffer, "%*d %*s %c %*d %*d %*d %d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %*ld %*ld %llu %lu %ld", p->state, &p->tty, &utime, &stime, &starttime, &p->vsz, &p->rss);
    p->vsz /= 1024; //convert VSZ to kilobytes
    p->rss *= getpagesize();    //multiply with pagesize
    p->rss /= 1024; //convert RSS to KB

    // read uptime & calculate percentages
    sysinfo(&systeminfo);
    p->cpu_percentage = ((float)(utime + stime) / sysconf(_SC_CLK_TCK)) / (systeminfo.uptime - (float)starttime / sysconf(_SC_CLK_TCK)) * 100;
    p->memory_percentage = (float)p->rss / ((float)systeminfo.totalram / 1024) * 100;

    // get username
    stat(path_buffer, &filestat);   //stat the stat file
    p->user = getpwuid(filestat.st_uid)->pw_name;
    return 1;
}

void displayproc(proc* p) {
    printf("%s %ld %0.1lf %0.1lf %lu %lu %d %s %s %s\n", p->user, p->pid, p->cpu_percentage, p->memory_percentage, p->vsz, p->rss, p->tty, p->state, "", p->cmdline);
}

int main() {
    DIR* testdir = opendir("/proc/");
    struct dirent* dir;
    proc p;
    while ((dir = readdir(testdir)) != NULL) {
        if (dir->d_type == DT_DIR && dir->d_name[0] > '0' && dir->d_name[0] <= '9') {
            if (readproc(dir->d_name, &p)) {
                displayproc(&p);
            }
        }
    }
    closedir(testdir);
}