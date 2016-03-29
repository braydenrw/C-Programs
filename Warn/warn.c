//Brayden Roth-White
//Piping in c
//Feb 24 2016

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>
#include <ctype.h>

#define MAX_SIZE 512

int pid;

/*  Parent handler if SIGINT, ask for user input */
void handler1(int sig) {
    signal(SIGINT, SIG_IGN);
    char buff[MAX_SIZE];
    printf("\nEnter command: ");
    fgets(buff, MAX_SIZE, stdin);

    buff[(int)(strlen(buff) - 1)] = '\0';

    write(3, buff, strlen(buff) + 1);
    if(strcmp(buff, "exit") == 0) {
        int status;
        kill(pid, SIGFPE);
        wait(&status);
        printf("Parent closed\n");
        exit(status);
    }
    kill(pid, SIGFPE);
}


/*  Child Handler potentially takes in SIGFPE, SIGINT and SIGALRM */
void handler2(int sig) {
    int alarmTime;
    char *p_buff;
    if(sig == SIGFPE) {
        char buff[MAX_SIZE];
        read(4, buff, MAX_SIZE);
        p_buff = buff;
        if(strcmp(buff, "exit") == 0) {
            printf("Child closed\n");
            exit(0);
        }
        if(isdigit(buff[0])) {
            alarmTime = buff[0] - '0';
            p_buff++;
        } else {
            alarmTime = 5;
        }
        printf("%s\n", p_buff);
        alarm(alarmTime);
        signal(SIGALRM, handler2);
    } else if(sig == SIGALRM){
        printf("%s\n", p_buff);
        alarm(alarmTime);
    } else if(sig == SIGINT) {
        signal(SIGALRM, SIG_IGN);
    }
}

int main(int argc, char **argv) {
    int fd[2];
    int rdr, wtr;
    pipe(fd);
    rdr = fd[0];
    wtr = fd[1];
    if((pid = fork())) { //parent
        close(rdr);
        dup2(wtr, 3);
        while(1) {
            signal(SIGINT, handler1);
            pause();
        }
    } else { //child
        close(wtr);
        dup2(rdr, 4);
        while(1) {
            signal(SIGINT, handler2);
            signal(SIGFPE, handler2);
            pause();
        }
    }

    return 0;
}