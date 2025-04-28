#include <stdio.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#define MAX_ARGS 128

int main() {
    char cwd[PATH_MAX];
    char *line=NULL; 
    size_t len=0;

    while(1) {
        getcwd(cwd, sizeof(cwd));
        printf("[%s]$ ", basename(cwd));
        fflush(stdout);

        if(getline(&line, &len, stdin)==-1) 
            break;

        if(strspn(line," \t\r\n")==strlen(line)) 
            continue;

        char *argv[MAX_ARGS]; 
        int argc=0;
        char *tok=strtok(line, " \t\n");
        while(tok&&argc<MAX_ARGS-1) {
            argv[argc++]=tok;
            tok=strtok(NULL, " \t\n");
        }
        argv[argc]=NULL;
        if(argc==0) 
            continue;

        pid_t pid=fork();
        if(pid==0) {
            execvp(argv[0], argv);
            perror("execvp");
            exit(127);
        } 

        else {
            int status;
            waitpid(pid, &status, 0);
        }
    }

    free(line);
    return 0;
}