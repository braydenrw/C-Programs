// Brayden Roth-White
// CS 360
// May 6 2016
#include "mftp.h"

#define MY_PORT_NUMBER 49999
#define MAX_COMMAND_LEN 256
#define MAX_RETURN_LEN 512
#define MAX_FILE_NAME_SIZE 256
#define MAX_BYTE_SIZE 512

int init_connect(char *arg, int portNum) {
	int socketfd;
	struct sockaddr_in servAddr;
	struct hostent* hostEntry;
	struct in_addr **pptr;

	if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error with socket");
		exit(1);
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(portNum);

	if((hostEntry = gethostbyname(arg)) == NULL){
		fprintf(stderr, "Error with gethostbyname()");
		exit(1);
	}

	pptr = (struct in_addr **) hostEntry->h_addr_list;
	memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));

	if((connect(socketfd, (struct sockaddr*) &servAddr, sizeof(servAddr))) < 0){
		perror("Error with connect()");
		exit(1);
	}
	return socketfd;
}

void cd(char *arg) {
	if(chdir(arg) < 0) {
		fprintf(stderr, "Not a valid path\n");
	}
}

char *rcd(char *path) {
	char *serverArg = malloc(MAX_COMMAND_LEN);

	strcpy(serverArg, "C");
	strcat(serverArg, path);
	strcat(serverArg, "\n");
	return serverArg;
}

void ls() {
	system("ls -l | more -20");
}

int get(int datafd, int sc_fd, char *sourcePath) {
	int fd, bytes, socketfd = sc_fd;
	char *targetPath, *source;
	char byteBuff[MAX_BYTE_SIZE];
	char fileBuff[MAX_FILE_NAME_SIZE];
	if((targetPath = getcwd(fileBuff, MAX_FILE_NAME_SIZE)) == NULL) {
		perror("Error with getcwd");
		write(socketfd, "E\n", 2);
		return 1;
	}

	while(sourcePath != NULL)
		source = strsep(&sourcePath, "/");

	strcat(targetPath, "/");
	strcat(targetPath, source);
	if((fd = open(targetPath, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0) {
		perror("Error with open");
		write(socketfd, "E\n", 2);
		return 1;
	}
	write(socketfd, "A\n", 2);
	while((bytes = read(datafd, byteBuff, MAX_BYTE_SIZE)) > 0)
		write(fd, byteBuff, bytes);

	close(fd);
	return 0;
}

void show(int datafd) {
	int status;
	if(!(status = fork())){
		close(0); dup(datafd);
		execlp("more", "more", "-20", NULL);
	} else {
		wait(&status);
	}
}

int put_f(int datafd, char *path) {
	int fd, bytes;
	char byteBuff[MAX_BYTE_SIZE];
	if((fd = open(path, O_RDONLY, 0600)) < 0) {
		perror("Error with open");
		return 1;
	}
	while((bytes = read(fd, byteBuff, MAX_BYTE_SIZE)) > 0)
		write(datafd, byteBuff, bytes);

	close(fd);
	return 0;
}

int clinet_run(char *host) {
	char fullArg[MAX_COMMAND_LEN];
	char *stringp;
	const char *delim = " \n";
	char *arg;
	int socketfd = init_connect(host, MY_PORT_NUMBER);
	while(1) {
		printf("MFTP> "); 
		fgets(fullArg, MAX_COMMAND_LEN, stdin);
		stringp = fullArg;
		arg = strsep(&stringp, delim);
		if(arg[0] == '\n') continue;

		if(!strcmp(arg, "exit")) {
			char retBuff[MAX_RETURN_LEN];
			write(socketfd, "Q\n", 2);
			read(socketfd, &retBuff, MAX_RETURN_LEN);
			if(retBuff[0] == 'A') {
				printf("Disconnected from server normally.\n");
				return 0;
			}
		} else if(!strcmp(arg, "cd")) {
			arg = strsep(&stringp, delim);
			cd(arg);
		} else if(!strcmp(arg, "rcd")) {
			char retBuff[MAX_RETURN_LEN];
			arg = strsep(&stringp, delim);
			char *serverArg = rcd(arg);
			write(socketfd, serverArg, strlen(serverArg)-1);
			read(socketfd, &retBuff, MAX_RETURN_LEN);
			if(retBuff[0] == 'E'){
				printf("Error: %s", retBuff + 1);
			}
		} else if(!strcmp(arg, "ls")) {
			ls();
		} else if(!strcmp(arg, "rls")) {
			char retBuff[MAX_RETURN_LEN];
			write(socketfd, "D\n", 2);
			read(socketfd, &retBuff, MAX_RETURN_LEN);
			if(retBuff[0] == 'E') {
				printf("Error: %s",retBuff + 1);
			} else {
				char retBuff2[MAX_RETURN_LEN];
				int datafd = init_connect(host, atoi(retBuff+1));
				write(socketfd, "L\n", 2);
				read(socketfd, &retBuff2, MAX_RETURN_LEN);
				if(retBuff2[0] == 'A') {
					show(datafd);
				} else {
					fprintf(stderr, "Error\n");
				}
			}
		} else if(!strcmp(arg, "get")) {
			char retBuff[MAX_RETURN_LEN];
			write(socketfd, "D\n", 2);
			read(socketfd, &retBuff, MAX_RETURN_LEN);
			if(retBuff[0] == 'E') {
				printf("Error4: %s",retBuff + 1);
			} else {
				char retBuff2[MAX_RETURN_LEN];
				char serverArg[MAX_COMMAND_LEN];
				int datafd = init_connect(host, atoi(retBuff+1));
				arg = strsep(&stringp, delim);
				strcpy(serverArg, "G");
				strcat(serverArg, arg);
				write(socketfd, serverArg, strlen(serverArg));
				read(socketfd, &retBuff2, MAX_RETURN_LEN);
				if(retBuff2[0] == 'A') {
					get(datafd, socketfd, arg);
				} else {
					printf("Error: %s",retBuff2 + 1);
				}
				close(datafd);
			}
		} else if(!strcmp(arg, "show")) {
			char retBuff[MAX_RETURN_LEN];
			write(socketfd, "D\n", 2);
			read(socketfd, &retBuff, MAX_RETURN_LEN);
			if(retBuff[0] == 'E') {
				printf("Error: %s", retBuff + 1);
			} else {
				char retBuff2[MAX_RETURN_LEN];
				char serverArg[MAX_COMMAND_LEN];
				int datafd = init_connect(host, atoi(retBuff+1));
				arg = strsep(&stringp, delim);
				strcpy(serverArg, "S");
				strcat(serverArg, arg);
				write(socketfd, serverArg, strlen(serverArg));				
				read(socketfd, &retBuff2, MAX_RETURN_LEN);		
				if(retBuff2[0] == 'A'){
					show(datafd);
				} else {
					fprintf(stderr, "Error\n");
				}
				close(datafd);
			}
		} else if(!strcmp(arg, "put")) {
			char retBuff[MAX_RETURN_LEN];
			write(socketfd, "D\n", 2);
			read(socketfd, &retBuff, MAX_RETURN_LEN);
			if(retBuff[0] == 'E') {
				printf("Error: %s", retBuff + 1);
			} else {
				char retBuff2[MAX_RETURN_LEN];
				char serverArg[MAX_COMMAND_LEN];
				int datafd = init_connect(host, atoi(retBuff+1));
				arg = strsep(&stringp, delim);
				strcpy(serverArg, "P");
				strcat(serverArg, arg);
				if(access(arg, F_OK) == -1) {
					printf("Error: %s\n",strerror(errno));
					continue;
				}
				write(socketfd, serverArg, strlen(serverArg));				
				read(socketfd, &retBuff2, MAX_RETURN_LEN);
				if(retBuff2[0] == 'A') {
					put_f(datafd, arg);
				} else {
					printf("Error: %s",retBuff2 + 1);
				}
				close(datafd);
			}
		} else {
			printf("%s command not recognized.\n", arg);
		}
		memset(&fullArg[0], 0, sizeof(fullArg));
	}
}

int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "Command line argument error: please enter a single hostname/IP\n");
		exit(1);
	}
	return clinet_run(argv[1]);
}

