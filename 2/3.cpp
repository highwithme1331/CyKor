#include <stdio.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#define MAX_ARGS 128
#include <errno.h>     

int builtin(char **argv) {
    if(strcmp(argv[0], "cd")==0) {
        const char *dest = argv[1]?argv[1]:getenv("HOME");
        if(chdir(dest)==-1)
            perror("cd");
		
        return 0;
    }

    if(strcmp(argv[0], "pwd")==0) {
        char cwd[PATH_MAX];
        if(getcwd(cwd, sizeof(cwd)))
            puts(cwd);
        else
            perror("pwd");
        return 0;
		
    }

    if(strcmp(argv[0], "exit")==0) {
        exit(0);
    }

    return -1;
}


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

        if(builtin(argv)==1)
			continue;

        pid_t pid=fork();
        if(pid==0) {
            execvp(argv[0], argv);
            perror("execvp");
            exit(127);
        } 

        else if{pid>0)
            int status;
            waitpid(pid, &status, 0);
        }
		
		else
			perror("fork");
    }

    free(line);
    return 0;
}