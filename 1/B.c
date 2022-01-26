#include "header.h"

typedef struct msg_buf {
	long type;
	char data[MAX_DATA + 1];
}message;

int main() {
	/* Message Queue */
	int mq_id = 0, msg_size = 0;
	message msg_buf;
	msg_buf.type = 1;

	memset(msg_buf.data, '\0', sizeof(msg_buf.data));
	if ((mq_id = msgget((key_t) MQ_KEY, IPC_CREAT | 0666)) == -1) {
		err_sys("ERROR : msgget error");
	}

	printf("Queue ID is %d \n", mq_id);

	/* TCP Socket */
	int tcp_sockfd = 0, tcp_sockopt = 0;
	if ((tcp_sockfd = socket(PF_INET, SOCK_STREAM, 0) ) < 0)
		err_sys("ERROR : TCP socket error");

	struct sockaddr_in tcp_servaddr;
	memset(&tcp_servaddr, 0, sizeof(tcp_servaddr));			// 0
	tcp_servaddr.sin_family = AF_INET;						// IPv4
	tcp_servaddr.sin_port = htons(PORT_TCP);				// Port
	tcp_servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");	// unsigned long	

	/* TCP Socket NON-BLOCK */
	int tcp_nbflag = fcntl(tcp_sockfd, F_GETFL, 0);
	fcntl(tcp_sockfd, F_SETFL, tcp_nbflag | O_NONBLOCK);

	int ret_msgrcv = 0, ret_connect = -1;
	fd_set fdset_w;
	while(1) {
		/* 데이터 수신(msgrcv) */
		if((ret_msgrcv = msgrcv(mq_id, &msg_buf, sizeof(msg_buf.data), 0, 0)) == -1) {
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
				if(getsockopt(tcp_sockfd, SOL_SOCKET, SO_ERROR, &tcp_sockopt, &tcp_optlen) == -1)
					err_sys("ERROR : getsockopt error");

				/* Error handling */
				if(tcp_sockopt) {
					printf("TCP ERROR : %s(%d)  \n", strerror(tcp_sockopt), tcp_sockopt);
					/* 1. ERROR : Broken Pipe(32) */
					if(tcp_sockopt == 32) {						
						ret_connect = -1;
						closeSocket(&tcp_sockfd);
						createSocket(&tcp_sockfd, tcp_servaddr, &tcp_nbflag);
					}
					/* 2. Connection Refused(111) */
					else if(tcp_sockopt == 111) {
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
					if( FD_ISSET(tcp_sockfd, &fdset_w) ) {
						int ret_write;
						if((ret_write = write(tcp_sockfd, msg_buf.data, ret_msgrcv)) < 0) {
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

/**
* ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
* 메시지 큐 ID의 메시지를 하나 읽고 그 메시지를 큐에서 제거하는 함수 
*
* @param msqid	: 메시지 큐의 ID
* @param msgp	: 읽어들인 메시지를 msgp가 가리키는 주소에 위치시킴. 쉽게 말해 메시지
* @param msgsz	: 메시지를 읽을 크기
* @param msgtyp	: 0, > 0, < 0 으로 동작이 나뉨
*				  0		: 큐의 첫번째 메시지를 읽어온다.
*				  > 0	: 그 값과 동일한 메시지 타입을 갖는 메시지를 읽어온다.
*				  < 0	: msgtyp의 절대값 이하의 가장 작은 메시지를 읽어온다.
*
* @param msgflg	: 4개의 flag가 존재(IPC_NOWAIT, MSG_COPY, MSG_EXCEPT, MSG_NOERROR)
*
* @return 성공	: 받은 Bytes 수
* @return 실패	: -1
*	
*/

/**
* int socket(int domain, int type, int protocol);
* 소켓을 생성하여 반환하는 함수
*
* @param domain	: 인터넷을 통해 통신할 지, 같은 시스템 내의 프로세스끼리 통신할 지의 여부를 설정
*				  @PF_INET, AF_INET : IPv4					
*
* @param type	: 데이터의 전송형태 지정
*				  @SOCK_STREAM	: TCP/IP
*				  @SOCK_DGRAM	: UDP/IP
*
* @return 성공	: 0이상
* @return 실패	: -1
*/

/**
* int bind(int sockfd, struct sockaddr *myaddr, socklen_t addlen);
* 소켓에 주소를 할당해주는 함수
*
* @param sockfd		: 소켓 식별자
* @param myaddr		: 구조체에 주소 정보 할당
* @param addrlen	: myaddr 구조체의 크기
*
* @return 성공		: 0
* @return 실패		: -1
*/

/**
* int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
* 클라이언트 소켓을 생성하고, 서버로 연결을 요청        
*
* @param sockfd	: 소켓 식별자
* @param addr	: 연결 요청을 보낼 서버의 주소 정보를 지닌 구조체
* @param addlen	: serv_addr 포인터가 가르키는 주소 정보 구조체의 크기
*
* @return 성공	: 0 
* @return 실패	: -1
*/

/**
* ssize_t write(int fd, const void *buf, size_t nbytes);
* 파일 쓰기
*
* @param fd		: 파일 디스크립터
* @param buf	: 파일에 쓰기를 할 내용을 담은 버퍼
* @param nbytes	: 쓰기할 바이트 수
*
* @return 성공	: 바이트 수
* @return 실패	: -1
*/
