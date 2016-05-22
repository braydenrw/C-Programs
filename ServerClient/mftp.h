#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>


#ifndef MFTP_H
#define MFTP_H

int get(int datafd, int sc_fd, char *sourcePath);

#endif //MFTP_H
