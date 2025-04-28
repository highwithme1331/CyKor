#include <stdio.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#define MAX_ARGS  128
#define MAX_PIPES 32

int builtin(char **argv) {
	if(strcmp(argv[0], "cd")==0) {
		const char *dest=argv[1]?argv[1]:getenv("HOME");
		
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
	
	if(strcmp(argv[0], "exit")==0) 
		exit(0);
	
	return -1;
}

int run_pipeline(char *cmdline, int bg) {
	char *seg_save;
	char *segment=strtok_r(cmdline, "|", &seg_save);
	int in_fd=STDIN_FILENO;
	pid_t pids[MAX_PIPES];
	int n_proc=0;
	int last_status=0;

	while(segment) {
		char *next=strtok_r(NULL, "|", &seg_save);
		int fds[2]; 
		int out_fd=STDOUT_FILENO;

		if(next) {
			if(pipe(fds)==-1) { 
				perror("pipe"); 
				return 1; 
			}
				
			out_fd=fds[1];
		}

		char *argv[MAX_ARGS]; 
		int argc=0;
		char *tok=strtok(segment," \t\n");
		
		while(tok && argc<MAX_ARGS-1) {
			argv[argc++]=tok;
			tok=strtok(NULL," \t\n");
		}
		
		argv[argc]=NULL;
		
		if(argc==0) { 
			segment=next; 
			continue; 
		}

		if(builtin(argv)==0) {
			if(next||in_fd!=STDIN_FILENO)
				fprintf(stderr,"myshell: built-in commands cannot be piped\n");
			
			last_status=0;
		}
		
		else {
			pid_t pid=fork();
			
			if(pid==0) {
				if(in_fd!=STDIN_FILENO) 
					dup2(in_fd,STDIN_FILENO);
				
				if(out_fd!=STDOUT_FILENO) 
					dup2(out_fd,STDOUT_FILENO);
				
				if(next) 
					close(fds[0]);
				
				execvp(argv[0],argv);
				perror("execvp"); _exit(127);
			}
			
			else if(pid>0) 
				pids[n_proc++]=pid;
			
			else 
				perror("fork");
		}

		if(in_fd!=STDIN_FILENO)
			close(in_fd);
		
		if(out_fd!=STDOUT_FILENO) 
			close(out_fd);
		
		in_fd=next?fds[0]:STDIN_FILENO;
		
		segment=next;
	}
	
	if(!bg) {
        for(int i=0; i<n_proc; i++) {
            int st; 
			waitpid(pids[i], &st, 0);
            
			if(i==n_proc-1) 
				last_status=WIFEXITED(st)?WEXITSTATUS(st):1;
        }
    }
	
	else 
        printf("[bg pid %d]\n", pids[n_proc-1]);

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
		getcwd(cwd,sizeof(cwd));
		printf("[%s]$ ",basename(cwd));
		fflush(stdout);

		if(getline(&line,&len,stdin)==-1) 
			break;
		
		if(strspn(line," \t\r\n")==strlen(line)) 
			continue;

		exec_line(line);
	}
	
	free(line);
	
	return 0;
}
