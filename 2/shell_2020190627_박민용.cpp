#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#define MAX_ARGS 128
#define MAX_HISTORY 1024
#define MAX_LINE 4096
#define MAX_PIPES 32
char *history[MAX_HISTORY];
int history_count=0;

static void remove_zombies() {
    int status;
	
    while(waitpid(-1, &status, WNOHANG)>0) 
		{}
}

typedef int (*builtin_func)(char **);

typedef struct {
    const char *name;
    builtin_func func;
} bulitincmd;

int cmd_cd(char **argv) {
    const char *dest=argv[1] ? argv[1] : getenv("HOME");
    
	if(chdir(dest)==-1)
        perror("cd");
	
    return 0;
}

int cmd_pwd(char **argv) {
    char cwd[PATH_MAX];
	
    if(getcwd(cwd, sizeof(cwd)))
        puts(cwd);
	
    else
        perror("pwd");
	
    return 0;
}

int cmd_exit(char **argv) {
    exit(0);
}

int cmd_echo(char **argv) {
    int i=1;
    int newline=1;
	
    if(argv[1]&&strcmp(argv[1], "-n")==0) {
        newline=0;
        i=2;
    }

    for(; argv[i]; i++) {
        printf("%s", argv[i]);
		
        if(argv[i+1])
            putchar(' ');
    }
	
    if(newline)
        putchar('\n');

    return 0;
}

int cmd_history(char **argv) {
    for(int i=0; i<history_count; i++)
        printf("%4d  %s", i+1, history[i]);
	
    return 0;
}

int cmd_help(char **argv) {
    puts("Supported built-in commands:");
    puts("cd [dir] Change directory");
    puts("pwd Print working directory");
    puts("exit Exit the shell");
    puts("echo [-n] Print strings");
    puts("history Show command history");
    puts("help Show this help message");
	
    return 0;
}

bulitincmd builtincmds[]={
    {"cd", cmd_cd},
    {"pwd", cmd_pwd},
    {"exit", cmd_exit},
    {"echo", cmd_echo},
    {"history", cmd_history},
    {"help", cmd_help},
    {NULL, NULL}
};

int builtin(char **argv) {
    if(argv[0]==NULL) 
		return -1;

    for(int i=0; builtincmds[i].name!=NULL; i++)
        if(strcmp(argv[0], builtincmds[i].name)==0)
            return builtincmds[i].func(argv);

    return -1;
}

int run_pipeline(char *cmdline, int bg) {
	char *seg_save;
	char *segment=strtok_r(cmdline, "|", &seg_save);
	int input_fd=STDIN_FILENO;
	pid_t pids[MAX_PIPES];
	int pid_count=0;
	int last_status=0;

	while(segment) {
		char *next=strtok_r(NULL, "|", &seg_save);
		int fds[2]; 
		int output_fd=STDOUT_FILENO;

		if(next) {
			if(pipe(fds)==-1) { 
				perror("pipe"); 
				return 1; 
			}
				
			output_fd=fds[1];
		}

		char *args[MAX_ARGS]; 
		int arg_count=0;
		int overflow=0;
		char *tok=strtok(segment," \t\n");
		
		while(tok) {
			if(arg_count>=MAX_ARGS-1) {
				overflow=1; 
				break;
			}
		
			args[arg_count++]=tok;
			tok=strtok(NULL, " \t\n");
		}

		if(overflow) {
			fprintf(stderr, "myshell:overflow (max %d)\n", MAX_ARGS-1);
			return 1;
		}
		
		args[arg_count]=NULL;
		
		if(arg_count==0) { 
			segment=next; 
			continue; 
		}

		if(builtin(args)==0) {
			if(next||input_fd!=STDIN_FILENO)
				fprintf(stderr, "myshell:cannot be piped\n");
			
			last_status=0;
		}
		
		else {
			pid_t pid=fork();
			
			if(pid==0) {
				if(input_fd!=STDIN_FILENO) 
					dup2(input_fd,STDIN_FILENO);
				
				if(output_fd!=STDOUT_FILENO) 
					dup2(output_fd,STDOUT_FILENO);
				
				if(next) 
					close(fds[0]);
				
				execvp(args[0],args);
				perror("execvp"); 
				_exit(127);
			}
			
			else if(pid>0) 
				pids[pid_count++]=pid;
			
			else {
				perror("fork");
				break;
			}
		}

		if(input_fd!=STDIN_FILENO)
			close(input_fd);
		
		if(output_fd!=STDOUT_FILENO) 
			close(output_fd);
		
		input_fd=next?fds[0]:STDIN_FILENO;
		
		segment=next;
	}
	
	if(!bg) {
        for(int i=0; i<pid_count; i++) {
            int st; 
			waitpid(pids[i], &st, 0);
            
			if(i==pid_count-1) 
				last_status=WIFEXITED(st)?WEXITSTATUS(st):1;
        }
    }
	
	else 
        printf("[bg pid %d]\n", pids[pid_count-1]);

	return last_status;
}

int exec_line(char *line) {
	int last_status=0;
	enum{NONE,AND,OR} prev=NONE;
	char *p=line;
	
	while(*p) {
		int bg=0;
		
		while(*p==' '||*p=='\t') 
			++p;
		
		if(!*p||*p=='\n') 
			break;

		char *cmd_start=p;
		enum{END,SC,SA,SO} sep=END;

		while(*p && *p!='\n') {
			if(p[0]==';') { 
				sep=SC; 
				break; 
			}
			
			if(p[0]=='&'&&p[1]=='&') { 
				sep=SA; 
				break; 
			}
			
			if(p[0]=='|'&&p[1]=='|') { 
				sep=SO; 
				break; 
			}
			
			++p;
		}
		
		char *cmd_end=p;
		
		if(*p) {
			*cmd_end='\0';
			
			if(sep==SA||sep==SO) 
				p+=2; 
			
			else 
				p+=1;
		}

		for(char *q=cmd_end-1; q>=cmd_start && (*q==' '||*q=='\t'); --q)
			q='\0';
		
		size_t len=strlen(cmd_start);
		
		if(len && cmd_start[len-1]=='&') {
			bg=1;
		
			cmd_start[len-1]='\0';

			while(len>1 && (cmd_start[len-2]==' '||cmd_start[len-2]=='\t'))
				cmd_start[--len-1]='\0';
		}

		int exec=1;
		
		if(prev==AND && last_status!=0) 
			exec=0;
		
		if(prev==OR  && last_status==0) 
			exec=0;

		if(exec) 
			last_status=run_pipeline(cmd_start, bg);

		prev=(sep==SA)?AND:(sep==SO)?OR:NONE;
	}

	return last_status;
}

int main() {
	char cwd[PATH_MAX];
	char *line=NULL; 
	size_t len=0;
	
	while(1) {
		remove_zombies();
		if(getcwd(cwd,sizeof(cwd)))
			printf("[%s]$ ", basename(cwd));
		else
			perror("getcwd");
		fflush(stdout);

		if(getline(&line, &len, stdin)==-1) 
			break;
		
		if(strlen(line)>MAX_LINE) {
			fprintf(stderr, "myshell:MAX_LINE over (max%d)\n", MAX_LINE);
			continue;
		}
		
		if(strspn(line," \t\r\n")==strlen(line)) 
			continue;

        if(line && *line!='\n')
		    if(history_count<MAX_HISTORY)
			    history[history_count++]=strdup(line);
		
		exec_line(line);
	}
	
	free(line);
	for(int i=0; i<history_count; i++) 
		free(history[i]);

	return 0;
}
