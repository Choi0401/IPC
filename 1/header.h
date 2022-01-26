#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <malloc.h>

#define TRUE	1
#define FALSE	0

#define DATA_SIZE	10240
#define MAX_DATA	10240	
#define MQ_KEY		1234
#define PORT_TCP	12345
#define	PORT_UDP	54321
#define SHM_KEY		7777


#define MAX_FD_SOCKET 0xff
#define MAX(a,b) a > b ? a : b

#define SOCK_LOCALFILE "UDS_FILE"


void createSocket(int *sockfd, struct sockaddr_in servaddr, int *nbFlag);
void closeSocket(int *sockfd);
int add_socket(int fd);
int del_socket(int fd);
int mk_fds(fd_set *fds, int *a_fd_socket);


void err_sys(const char *message) {
	perror(message);
	exit(1);
}

