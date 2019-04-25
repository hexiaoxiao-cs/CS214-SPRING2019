#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include<pwd.h>
extern int    errno;

typedef struct procinfo {
    int UID;
    char *USER;
    int PID;
    float pCPU; // %CPU
    float pMEM; // %MEM
    long VSZ; // virtual Memory in KB
    long RSS; // Process Resident Size
    int TTY;// TTY
    char *TTY_NAME; // TTY NAME
    char *STAT;// STAT
    long long START_TIME;//START DATE OR TIME
    long long UTIME;
    long long STIME;
    long long CUTIME;
    long long CSTIME;
    //double pCPU;
    long uptime;
    long total_time;
    char *COMMAND;
} procinfo;

//only need to read a portion of the file since the important information is all at the beginning of the proc file

void getTTY_Name(procinfo *haha) {
    int maj, min;
    haha->TTY_NAME = (char *) malloc(sizeof(char) * 1000);
    if (haha->TTY == 0) {
        asprintf(&(haha->TTY_NAME), "?");
        return;
    }
    else {
        maj = major(haha->TTY);
        min = minor(haha->TTY);
        if (maj == 136) { asprintf(&(haha->TTY_NAME), "pts/%d", min); }
        if (maj == 4) { asprintf(&(haha->TTY_NAME), "tty%d", min); }
    }
    return;
}

static int readFile(const char *filename, char *ret, int cap) {
    int fd, num_read;
    fd = open(filename, O_RDONLY, 0);
    if (fd == -1) return -1;
    num_read = read(fd, ret, cap - 1);
    close(fd);
    if (num_read <= 0 || errno != 0) return -1;
    ret[num_read] = '\0';
    return num_read;
}

void check_integrety(procinfo *process_c) {
    if (process_c->START_TIME == 0 ||
        strlen(process_c->STAT) == 0 ||
        strlen(process_c->COMMAND) == 0) {
        process_c->PID = -1;
    }
    return;
}

char *getUserFromUID(int uid) {
    char  *username;
    int current = 0;
    struct passwd *pwd;
    uid_t id = uid;
    username = (char *) malloc(sizeof(char) * 500);
    //other=(char*) malloc(sizeof(char)*1000);

    //username=pwd->pw_name;
    strcpy(username,getpwuid(uid)->pw_name);
    //system(command);
    //free(command);
    return username;
}

//need this for getting UID and other things that stat does not have
void parsestatus(char *status_data, procinfo *process_c) {
    char *curr;
    char *temp;
    int count = 0;
    //temp = (char*) malloc(sizeof(char*)*100);
//    sscanf(status_data,"%*s %*s"
//                       "%*s %*d"
//                       "%*s %c %*s"
//                       "%*s %*d"
//                       "%*s %*d"
//                       "%*s %d"
//                       "%*s %*d"
//                       "%*s %d %*d %*d %*d"
//                       "%*s %*d %*d %*d %*d"
//                       "%*s %*d"
//                       "%*s %*d")

    curr = strchr(status_data, ':');//skip first colon
    curr += 2;
    temp = (char *) malloc(sizeof(char *) * 1000);
    //sscanf(curr,"%s\n",temp);
    count = 0;
    while (!iscntrl(curr[0])) {
        temp[count] = curr[0];
        curr++;
        count++;
    }
    //printf("%s\n",temp);
    if (process_c->COMMAND == "\0") {
        asprintf(&(process_c->COMMAND), "[%s]", temp);
    }
    curr = strchr(curr + 1, ':');//skip second colon
    curr = strchr(curr + 1, ':');//locate third colon
    process_c->STAT = (char *) malloc(sizeof(char) * 10);
    strncpy(process_c->STAT, curr + 2, 1);//Copy Status Code
    process_c->STAT[1] = '\0';
    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr += 2;//start with the digit
    count = 0;
//    temp = (char *) malloc(sizeof(char *) * 100);
//    while (isdigit(curr[0]) != 0)//get digits
//    {
//        temp[count] = curr[0];
//        curr++;
//        count++;
//    }
//    temp[count] = '\0';
    //    printf("PID: %s\n",temp);
    //process_c->PID=atoi(temp);

    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr += 2;
    count = 0;
    temp = (char *) malloc(sizeof(char *) * 100);
    while (isdigit(curr[0]) != 0)//get digits
    {
        temp[count] = curr[0];
        curr++;
        count++;
    }
    temp[count] = '\0';
    //    printf("UID: %s\n",temp);
    process_c->UID = atoi(temp);
    process_c->USER = getUserFromUID(process_c->UID);

    curr = strchr(curr, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    while (isdigit(curr[0]) == 0) {
        curr++;
    }//find digit location
    count = 0;
    temp = (char *) malloc(sizeof(char *) * 100);
    while (isdigit(curr[0]) != 0)//get digits
    {
        temp[count] = curr[0];
        curr++;
        count++;
    }
    temp[count] = '\0';
    //    printf("VMS: %s\n",temp);
    process_c->VSZ=atol(temp); //Virtual Memory Size
    curr = strchr(curr, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    curr = strchr(curr + 1, ':');//skip next colon
    while (isdigit(curr[0]) == 0) {
        curr++;
    }//find digit location
    count = 0;
    temp = (char *) malloc(sizeof(char *) * 100);
    while (isdigit(curr[0]) != 0)//get digits
    {
        temp[count] = curr[0];
        curr++;
        count++;
    }
    temp[count] = '\0';
    //    printf("RSS: %s\n",temp);
    //process_c->RSS=atol(temp); //Vm RSS
    //Free Everything else
    free(temp);
    //free(curr);
}

void parserstat(char *stat_data, procinfo *process_c) {
    process_c->RSS = 0;
    process_c->VSZ = 0;
    process_c->START_TIME = 0;
    process_c->STAT = (char *) malloc(sizeof(char) * 10);
    process_c->UTIME = 0;
    process_c->STIME = 0;
    process_c->CUTIME = 0;
    process_c->CSTIME = 0;
    process_c->PID = 0;
    process_c->UID = 0;
    process_c->TTY = 0;
    process_c->COMMAND = (char *) malloc(sizeof(char) * 1000);
    sscanf(stat_data,
           "%d %*s %c "
           "%*d %*d %*d %d %*d "
           "%*lu %*lu %*lu %*lu %*lu "
           "%Lu %Lu %Lu %Lu "
           "%*ld %*ld "
           "%*d "
           "%*ld "
           "%Lu "
           "%lu "
           "%ld ",
           &(process_c->PID),
           process_c->STAT,
           &(process_c->TTY),
           &(process_c->UTIME),
           &(process_c->STIME),
           &(process_c->CUTIME),
           &(process_c->CSTIME),
           &(process_c->START_TIME),
           &(process_c->VSZ),
           &(process_c->RSS)
    );
    process_c->STAT[1] = '\0';
}


void readstatus(char *procnum, procinfo *process_c) {
    char *currfile, *bf;
    char *data = (char *) malloc(sizeof(char) * 2048);
    size_t size;
    int stat;
    asprintf(&currfile, "/proc/%s/status", procnum);
    //printf("%s\n",currfile);
    size = readFile(currfile, data, 2048);
    //printf("%d:\n%s\n",size,data);
    //printf("%s",data);
    if (size <= 0 || errno != 0) {
        process_c->PID = -1;
        return;
    }
    asprintf(&bf, "/proc/%s/cmdline", procnum);
    //printf("%s\n",bf);
    stat = readFile(bf, (process_c->COMMAND), 1000);
    if (stat == -1) { process_c->COMMAND = "\0"; }
    //printf("%s\n",process_c->COMMAND);
    //parsestatus(data, process_c);
    free(data);
}

void readstat(char *procnum, procinfo *process_c) {
    char *currfile;
    char *data = (char *) malloc(sizeof(char) * 2048);
    size_t size;
    asprintf(&currfile, "/proc/%s/stat", procnum);
    //printf("%s\n",currfile);
    size = readFile(currfile, data, 2048);
    //printf("%d:\n%s\n",size,data);
    //printf("%s",data);
    if (size <= 0 || errno != 0) {
        process_c->PID = -1;
        return;
    }
    parserstat(data, process_c);
    readstatus(procnum, process_c);
//    check_security(process_c);
    free(data);
}

void printprocess_info(procinfo **process_info) {
    printf("USER\tPID\t%CPU\t%MEM\tVSZ\tRSS\tTTT\tSTAT\tSTART\tTIME\tCOMMAND");

}


int main() {
    char **listproc = (char **) malloc(sizeof(char *));
    DIR *proc;
    procinfo **process_info;
    struct dirent *de;
    char *bf;
    int manyproc = 0, stat;
    struct sysinfo info;
    long uptime;
    long tick = sysconf(_SC_CLK_TCK);
    int fd, num_read;
    char *buffer = (char *) malloc(sizeof(char) * 2000);
    int temp = 0;
    unsigned long total_mem;
    float seconds;
    sysinfo(&info);

    total_mem=info.totalram;
    printf("%f\n",(float)total_mem);
    proc = opendir("/proc");
    if (proc == NULL) {
        printf("Fatal Error: Unable to open /proc.\nProgram will exit.");
        return -1;
    }
    while ((de = readdir(proc)) != NULL) {
        if (isdigit((de->d_name)[0]) != 0) {
            manyproc++;
            listproc = (char **) realloc(listproc, manyproc * sizeof(char *));
            listproc[manyproc - 1] = (char *) malloc(sizeof(char) * 5000);
            strcpy(listproc[manyproc - 1], de->d_name);
            //printf("%s\n",listproc[manyproc-1]);
        }
    }
    //printf("%s\n",getUserFromUID(85600));
    //process_info = (procinfo *) malloc(sizeof(procinfo));
    //readstat("11809",process_info);
    //printf("%s, %d, %d, %s, %d, %lld, %lld, %ld, %ld\n", process_info->STAT, process_info->PID, process_info->UID, process_info->USER,process_info->TTY,process_info->TIME,process_info->START_TIME, process_info->VSZ,process_info->RSS);
    fd = open("/proc/uptime", O_RDONLY, 0);
    if (fd == -1) {
        printf("UPTIME FILE NOT AVAILABLE\n");
        return -1;
    }
    process_info = (procinfo **) malloc(sizeof(procinfo) * manyproc);

    for (temp = 0; temp < manyproc; temp++) {
        //printf("%s\n",listproc[temp]);
        process_info[temp] = (procinfo *) malloc(sizeof(procinfo));
        readstat(listproc[temp], process_info[temp]);

        if (process_info[temp]->PID == -1) { continue; }
        //else{printf("%s\n",process_info[temp]->COMMAND);}
        process_info[temp]->total_time = process_info[temp]->UTIME + process_info[temp]->STIME;
        num_read = read(fd, buffer, 1999);
        lseek(fd, 0, SEEK_SET);
        if (num_read <= 0) {
            printf("Reading UPTIMEFILE ERROR\n");
            return -1;
        }
        buffer[num_read] = '\0';
        sscanf(buffer, "%ld", &(process_info[temp]->uptime));
        seconds = (float)process_info[temp]->uptime - (float)process_info[temp]->START_TIME / (float) tick;
        if (seconds == 0) { process_info[temp]->pCPU = 0; }
        else {
            process_info[temp]->pCPU = ((float)process_info[temp]->total_time/ (float)tick) / (float)seconds * 100.0;
        }

        process_info[temp]->pMEM=((float)process_info[temp]->RSS)/((float)total_mem);
        //check_integrety(process_info[temp]);
        if (process_info[temp]->PID == -1) { continue; }
        //printf("%ld",process_info[temp]->uptime);
        getTTY_Name(process_info[temp]);
        printf("%d %d %.1f %.1f ", major(process_info[temp]->TTY), minor(process_info[temp]->TTY),
               process_info[temp]->pCPU,process_info[temp]->pMEM);
        printf("%s, %d, %d, %s, %d, %lld, %lld, %ld, %ld, %s, %s\n", process_info[temp]->STAT, process_info[temp]->PID,
               process_info[temp]->UID, process_info[temp]->USER, process_info[temp]->TTY, process_info[temp]->UTIME,
               process_info[temp]->START_TIME, process_info[temp]->VSZ, process_info[temp]->RSS,
               process_info[temp]->TTY_NAME, process_info[temp]->COMMAND);
    }
    return 0;
}