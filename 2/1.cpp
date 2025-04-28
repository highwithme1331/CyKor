#include <stdio.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>

int main() {
    char cwd[PATH_MAX];

    while(1) {
        getcwd(cwd, sizeof(cwd));
        printf("[%s]$ ", basename(cwd));
        fflush(stdout);

        char line[1024];
        if(!fgets(line, sizeof(line), stdin)) 
            break;
    }

    return 0;
}
