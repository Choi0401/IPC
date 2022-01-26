#include "header.h"

typedef struct msg_buf {
	long type;
	char data[MAX_DATA + 1];
} message;

int main() {
	/* Message Queue */
	int mq_id= 0, msg_size = 0;
	message msg_buf;
	msg_buf.type = 1;

	OpenConf("B-log");

	memset(msg_buf.data, '\0', sizeof(msg_buf.data));
	LOG_CALL(mq_id = msgget((key_t) MQ_KEY, IPC_CREAT | 0666));
	if(mq_id == -1) {
		LOG_ERROR("ERROR : msgget error");
		err_sys("ERROR : msgget error");
	}

	printf("Queue ID is %d \n", mq_id);

	/* TCP Socket */
	int tcp_sockfd = 0, tcp_sockopt = 0;
	LOG_CALL(tcp_sockfd = socket(PF_INET, SOCK_STREAM, 0) );
	if (tcp_sockfd < 0) {
		LOG_ERROR("ERROR : TCP socket error");
		err_sys("ERROR : TCP socket error");
	}

	struct sockaddr_in tcp_servaddr;
	memset(&tcp_servaddr, 0, sizeof(tcp_servaddr));			// 0
	tcp_servaddr.sin_family = AF_INET;						// IPv4
	tcp_servaddr.sin_port = htons(PORT_TCP);				// Port
	tcp_servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");	

	/* TCP Socket NON-BLOCK */
	int tcp_nbflag = fcntl(tcp_sockfd, F_GETFL, 0);
	fcntl(tcp_sockfd, F_SETFL, tcp_nbflag | O_NONBLOCK);

	int ret_msgrcv = 0, ret_connect = -1;
	fd_set fdset_w;
	while(1) {
		/* 데이터 수신(msgrcv) */
		if((ret_msgrcv = msgrcv(mq_id, &msg_buf, sizeof(msg_buf.data), 0, 0)) == -1)  {
			LOG_ERROR("ERROR : msgrcv error");
			err_sys("ERROR : msgrcv error");
		}
		else {
			if(ret_msgrcv != 0) {
				msg_size += ret_msgrcv;
				printf("Recevied Message from A : %s \n", msg_buf.data);
				printf("==================================\n");
				printf("Message size : %d Bytes \n", ret_msgrcv);
				printf("Total Message size : %d Bytes \n", msg_size);
				printf("==================================\n");

				/* Connect */
				if(ret_connect != 0) {
					ret_connect = connect(tcp_sockfd, (struct sockaddr*)&tcp_servaddr, sizeof(tcp_servaddr)); 
				}
				socklen_t tcp_optlen = sizeof(tcp_sockopt);
				if(getsockopt(tcp_sockfd, SOL_SOCKET, SO_ERROR, &tcp_sockopt, &tcp_optlen) == -1){
					LOG_ERROR("ERROR : getsockopt error");
					err_sys("ERROR : getsockopt error");
				}

				/* Error handling */
				if(tcp_sockopt) {
					printf("TCP ERROR : %s(%d)  \n", strerror(tcp_sockopt), tcp_sockopt);
					/* 1. ERROR : Broken Pipe(32) */
					if(tcp_sockopt == 32) {
						LOG_WARNING("WARNING : Broken Pipe(32)");
						ret_connect = -1;
						closeSocket(&tcp_sockfd);
						createSocket(&tcp_sockfd, tcp_servaddr, &tcp_nbflag);
					}
					/* 2. Connection Refused(111) */
					else if(tcp_sockopt == 111) {
						LOG_WARNING("WARNING : Connection Refused(111)");
						ret_connect = -1;
						closeSocket(&tcp_sockfd);
						createSocket(&tcp_sockfd, tcp_servaddr, &tcp_nbflag);
					}

				}	// (tcp_sockopt)
				/* Connection success */
				else if(tcp_sockopt == 0) {
					FD_ZERO(&fdset_w);
					FD_SET(tcp_sockfd, &fdset_w);
					select(tcp_sockfd + 1, NULL, &fdset_w, NULL, NULL);
					if(FD_ISSET(tcp_sockfd, &fdset_w)) {
						int ret_write;
						if((ret_write = write(tcp_sockfd, msg_buf.data, ret_msgrcv)) < 0) {
							LOG_ERROR("ERROR : write error");
							err_sys("ERROR : write error");
						}
						memset(msg_buf.data, 0x00, sizeof(msg_buf.data));
					}
				}
			}	// if(strlen
		}	// else
	}	// while

	return 0;
}

void closeSocket(int *sockfd) {
	close(*sockfd);
}

void createSocket(int *sockfd, struct sockaddr_in servaddr, int *nbFlag) {
	/* New Socket Create */
	if((*sockfd = socket(PF_INET, SOCK_STREAM, 0) ) < 0)
		err_sys("ERROR : socket error");
	printf("New Socket : %d \n", *sockfd);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;				// IPv4
	servaddr.sin_port = htons(PORT_TCP);		// Port
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");	
	/* NON_BLOCK */
	*nbFlag = fcntl(*sockfd, F_GETFL, 0);
	fcntl(*sockfd, F_SETFL, *nbFlag | O_NONBLOCK);
}