//Brayden Roth-White
//Piping in c
//Feb 24 2016

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>

typedef struct Piped_Commands {
    char *command;
    char **commandArgs;
} Commands;

Commands* Pipe_init(int size, int *nArgs, char **argv) {
    int i = 1;
    Commands *comm = (Commands*)malloc(sizeof(Commands) * (size + 1));
    for(int j = 0; j < size; j++) {
        comm[j].commandArgs = (char**)malloc(sizeof(char*) * nArgs[j]);
        comm[j].command = strdup(argv[i]);
        for(int k = 0; k < nArgs[j] - 1; k++) {
            comm[j].commandArgs[k] = strdup(argv[i]);
            i++;
        }
        comm[j].commandArgs[nArgs[j] - 1] = NULL;
        i++;
    }
    return comm;
}

int numPipes(int argc, char **argv) {
    int connectors = 0;
    for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], ":") == 0) {
            connectors++;
        }
    }
    return connectors + 1; //There will be 1 more argument than ":"
}

void saveArgs(int *nArgs, int size, char **argv) {
    int j = 1;
    for(int i = 0; i < size; i++) {
        nArgs[i] = 1;
        while(argv[j] != NULL && strcmp(argv[j],":") != 0) {
            nArgs[i] += 1;
            j++;
        }
        j++;
    }
}

/*  Sets up the correct child and parent pieces of the pipe 
    and runs them after forking */
void connect(int n, Commands *comm) {
    int fd[2];
    int rdr, wtr;
    pipe(fd);
    rdr = fd[0];
    wtr = fd[1];
    if(fork()){
        close(wtr);
        dup2(rdr, 0);
        execvp(comm[n].command, comm[n].commandArgs);
        fprintf(stderr,"Error with exec: %s\n", strerror(errno));
    } else {
        close(rdr);
        dup2(wtr, 1);
        if(n > 0) connect(n-1, comm);
    }
}


int main(int argc, char **argv) {
    if(argc < 4) {
        printf("%d\n", argc);
        fprintf(stderr,"Too few arguments\n");
        exit(1);
    }
    if(strcmp(argv[1], ":") == 0) {
        fprintf(stderr,"Pipe cannot be first argument\n");
        exit(2);
    }
    if(strcmp(argv[argc-1], ":") == 0) {
        fprintf(stderr,"Pipe cannot be the last argument\n");
        exit(3);
    }

    int nPipes = numPipes(argc, argv);
    int nArgs[nPipes];
    saveArgs(nArgs, nPipes, argv);
    Commands *comm = Pipe_init(nPipes, nArgs, argv);
    connect(nPipes - 1, comm);

    return 0;
}
