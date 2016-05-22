#define MY_PORT_NUMBER 49999
#define MAX_COMMAND_LEN 256
#define MAX_FILE_NAME_SIZE 256
#define MAX_BYTE_SIZE 512
#define MAX_ERROR_LEN 512

#include "mftp.h"

int init_listen(){
  	int listenfd;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
        fprintf(stderr, "Error with socket");
        exit(1);
	}
	return listenfd;
}

struct sockaddr_in init_sockAddr(int portNum){
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(portNum);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	return servAddr;
}

char *getPort(int datafd, struct sockaddr_in servAddr) {
	if(bind(datafd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("bind");
		exit(1);
	}

	char* portno = malloc(16);
	struct sockaddr_in getPort;
	unsigned int len = sizeof(getPort);
	if(getsockname(datafd, (struct sockaddr *) &getPort, &len) == -1) {
		perror("getsockname");
		return "E";
 	}
	int port = ntohs(getPort.sin_port);
	sprintf(portno, "%d", port);
	return portno;
}

int dataConnect(int listenfd) {
	struct sockaddr_in clientAddr;
	unsigned int length = sizeof(struct sockaddr_in);
	int connectfd;
	if((connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length)) < 0) {
		perror("accept");
		exit(1);
	}
	return connectfd; 
}

int rcd(char *path) {
	if(chdir(path) < 0) {
		printf("RCD to '%s' failed with error %s\n",path, strerror(errno));
		return errno;
	} else {
		printf("Child %d: Current directory changed to %s\n",getpid(),path);
		return 0;
	}
}

void rls(int datafd) {
	int status;
	if(!(status = fork())){
		close(1); dup(datafd);
		execlp("ls", "ls", "-l", NULL);
		perror("execlp");
		exit(1);
	} else {
		wait(&status);
	}
}

int get(int datafd, int sc_fd, char *path) {
	int fd, bytes, connectfd = sc_fd;
	char byteBuff[MAX_BYTE_SIZE], errMsg[MAX_ERROR_LEN], check[MAX_COMMAND_LEN];
	if((fd = open(path, O_RDONLY, 0600)) < 0) {
		printf("Child %d: get on '%s' failed with error %s\n", getpid(), path, strerror(errno));
		strcat(strcat(strcpy(errMsg, "E"), strerror(errno)), "\n");
		write(connectfd, errMsg, MAX_ERROR_LEN);
		return 1;
	}
	write(connectfd, "A\n", 2);
	read(connectfd, &check, 2);
	if(check[0] == 'E') return 1;

	while((bytes = read(fd, byteBuff, MAX_BYTE_SIZE)) > 0)
		write(datafd, byteBuff, bytes);

	close(fd);
	return 0;
}

void show(int datafd, char *source) {
	int status;
	if(!(status = fork())){
		close(1); dup(datafd);
		execlp("cat", "cat", source, NULL);
		perror("execlp");
		exit(1);
	} else {
		wait(&status);
	}
}

int put_f(int datafd, int connectfd, char *sourcePath) {
	int fd, bytes;
	char *targetPath, *source;
	char byteBuff[MAX_BYTE_SIZE], fileBuff[MAX_FILE_NAME_SIZE], errMsg[MAX_ERROR_LEN];
	if((targetPath = getcwd(fileBuff, MAX_FILE_NAME_SIZE)) == NULL) {
		printf("Child %d: put to '%s' failed with error %s\n", getpid(), sourcePath, strerror(errno));
		strcat(strcat(strcpy(errMsg, "E"), strerror(errno)), "\n");
		write(connectfd, errMsg, MAX_ERROR_LEN);
		return 1;
	}

	while(sourcePath != NULL)
		source = strsep(&sourcePath, "/");

	strcat(targetPath, "/");
	strcat(targetPath, source);
	if((fd = open(targetPath, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0) {
		printf("Child %d: put to '%s' failed with error %s\n", getpid(), sourcePath, strerror(errno));
		strcat(strcat(strcpy(errMsg, "E"), strerror(errno)), "\n");
		write(connectfd, errMsg, MAX_ERROR_LEN);
		return 1;
	}
	write(connectfd, "A\n", 2);
	while((bytes = read(datafd, byteBuff, MAX_BYTE_SIZE)) > 0)
		write(fd, byteBuff, bytes);

	close(fd);
	return 0;
}

int server_run(int listenfd, struct sockaddr_in servAddr) {
	int connectfd;
	listen(listenfd, 4);
	socklen_t length = sizeof(struct sockaddr_in);
	struct sockaddr_in clientAddr;
	struct hostent *hostEntry;
	struct sockaddr_in dataAddr;
	char buff[MAX_COMMAND_LEN];
	char retBuff[MAX_COMMAND_LEN];
	char errMsg[MAX_ERROR_LEN];
	int err;
	int datafd;
	int listen_datafd;

	while(1) {
		waitpid(-1, NULL, WNOHANG);
		if((connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length)) < 0) {
			perror("Error with accept");
			exit(1);
		}
		if(!fork()) {				
			hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET);
			printf("Child %d: Connection accepted from host %s \n", getpid(), hostEntry->h_name);
			while(1) {
				if(!read(connectfd, &buff, MAX_COMMAND_LEN)){
					printf("Child %d: Error: Connection dropped.\n", getpid());
					return 1;
				}
				if(buff[0] == 'Q') {
					printf("Child %d: Client from %s disconnected.\n", getpid(), hostEntry->h_name);
					write(connectfd, "A\n", 2);
					return 0;
				} else if(buff[0] == 'C') {
					if(!(err = rcd(buff + 1))) {
						write(connectfd, "A\n", 2);
					} else {
						strcpy(errMsg, "E");
						strcat(errMsg, strerror(err));
						strcat(errMsg, "\n");
						write(connectfd, errMsg, MAX_ERROR_LEN);
					}
				} else if(buff[0] == 'D') {
					listen_datafd = init_listen();
					dataAddr = init_sockAddr(0);
					char *dataport = getPort(listen_datafd, dataAddr);
					if(dataport[0] != 'E') {
						strcpy(retBuff, "A");
						strcat(retBuff, dataport);
						strcat(retBuff, "\n");
					} else {
						strcpy(retBuff, dataport);
						strcat(retBuff, "\n");
					}
					write(connectfd, retBuff, (int)strlen(retBuff));
					listen(listen_datafd, 1);
					datafd = dataConnect(listen_datafd);	
				} else if(buff[0] == 'L') {
					if(!listen_datafd) {
						printf("Child %d: Error: Connection dropped.\n", getpid());
						return 1;
					} else {
						rls(datafd);
						write(connectfd, "A\n", 2);
						close(datafd);
						close(listen_datafd);
					}
				} else if(buff[0] == 'G') {
					if(!listen_datafd) {
						printf("Child %d: Error: Connection dropped.\n", getpid());
					} else {
						get(datafd, connectfd, buff+1);
						close(datafd);
						close(listen_datafd);
					}
				} else if(buff[0] == 'S') {
					if(!listen_datafd) {
						printf("Child %d: Error: Connection dropped.\n", getpid());
						return 1;
					} else {
						show(datafd, buff + 1);
						write(connectfd, "A\n", 2);
						close(datafd);
						close(listen_datafd);
					}
				} else if(buff[0] == 'P') {
					if(!listen_datafd) {
						printf("Child %d: Client from %s disconnected.\n", getpid(), hostEntry->h_name);
					} else {
						put_f(datafd, connectfd, buff+1);
						close(datafd);
						close(listen_datafd);
					}
				}
				memset(&buff[0], 0, sizeof(buff));
			} 
		} else {
			close(connectfd);
		}
	}
}

int main(int argc, char **argv){
	int listenfd = init_listen();
	struct sockaddr_in servAddr = init_sockAddr(MY_PORT_NUMBER);

	if(bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("Error with bind");
		exit(1);
	}

	return server_run(listenfd, servAddr);
}

