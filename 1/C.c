#include "header.h"

int cnt_fd_socket = 0;
int fd_biggest;
int fd_socket[MAX_FD_SOCKET];

int main() {
	/* TCP Socket */
	char data[MAX_DATA + 1] = {'\0', };
	int tcp_listenfd = 0, msg_size = 0;

	if((tcp_listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
		err_sys("ERROR : TCP socket error");

	/* setsockopt */
	int optval = 1;
	setsockopt(tcp_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	/* TCP bind */
	struct sockaddr_in tcp_servaddr;
	socklen_t tcp_servaddrlen = sizeof(tcp_servaddr);
	memset(&tcp_servaddr, 0, sizeof(tcp_servaddr));
	tcp_servaddr.sin_family = AF_INET;
	tcp_servaddr.sin_port = htons(PORT_TCP);					// htons(...) : 빅엔디안 형식으로 변경
	tcp_servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 			// IP 주소 자동으로 할당

	if (bind(tcp_listenfd, (struct sockaddr*)&tcp_servaddr, sizeof(tcp_servaddr)) < 0)
		err_sys("ERROR : TCP bind error");

	/* TCP listen */
	if(listen(tcp_listenfd, SOMAXCONN) < 0)
		err_sys("ERROR : TCP listen error");

	/* ADD Socket in Array */
	add_socket(tcp_listenfd);

	/* NON-BLOCK for tcp_listendfd */
	int tcp_nblistenfd = fcntl(tcp_listenfd, F_GETFL, 0);			// F_GETFL을 사용하면 기존 flag 값을 읽어옴
	fcntl(tcp_listenfd, F_SETFL, tcp_nblistenfd | O_NONBLOCK);		// O_NONBLOCK  flag를 OR연산으로 더한 뒤 F_SETFL을 이용하면 넌블러킹 모드로 적용

	/* Unix Domain Socket(UDP) */
	int uds_sockfd = 0;
	struct sockaddr_un uds_servaddr;

	if(access( SOCK_LOCALFILE, F_OK ) == 0)
		unlink(SOCK_LOCALFILE);

	if((uds_sockfd = socket(AF_UNIX, SOCK_DGRAM, IPPROTO_IP)) < 0) 
		err_sys("ERROR : Unix socket error");

	memset(&uds_servaddr, 0, sizeof(uds_servaddr));
	uds_servaddr.sun_family = AF_UNIX;
	strcpy(uds_servaddr.sun_path, SOCK_LOCALFILE);

	/* Unix bind */
	if(bind(uds_sockfd, (struct sockaddr*) &uds_servaddr, sizeof(uds_servaddr)) < 0)
		err_sys("ERROR : Unix bind error");

	struct sockaddr_in tcp_clntaddr;
	socklen_t tcp_clntaddrlen = sizeof(tcp_clntaddr);
	fd_set fds_read;
	int ret_sel = 0, ret_read = 0, connfd = 0;

	while(1) {
		/* Accept */
		fd_biggest = mk_fds(&fds_read, fd_socket);
		/* fds_read 구조체의 fd_biggest + 1까지 read 이벤트가 있는지 감시하겠다  */
		if((ret_sel = select(fd_biggest + 1, &fds_read, NULL, NULL, NULL)) == -1) 
			err_sys("ERROR : select error");
		/* tcp_listenfd 이벤트가 발생하였는지( connect가 들어왔는지 ) */
		if(FD_ISSET(tcp_listenfd, &fds_read)) {
			connfd = accept(tcp_listenfd, (struct sockaddr*)&tcp_clntaddr, &tcp_clntaddrlen);
			if(connfd == -1) {
				err_sys("ERROR : TCP accept error");
				continue;
			}
			/* Accept success */
			if(add_socket(connfd) == -1) 
				err_sys("ERROR : Failed to add socket \n");
			printf("Accept : Add socket(%d) \n", connfd);
			continue;
		}
		/* read */
		for(int i = 1; i < cnt_fd_socket; i++) {
			if(FD_ISSET(fd_socket[i], &fds_read)) {
				if((ret_read = read(fd_socket[i], data, DATA_SIZE )) < 0)
					err_sys("ERROR : TCP read error");
				else {
					printf("ret_read = %d \n",ret_read);
					if(ret_read == 0) {
						printf("connfd(%d) : Session Closed \n", fd_socket[i]);
						del_socket(fd_socket[i]);
					}
					else {
						msg_size += ret_read;
						printf("Received Message from B : %s \n", data);
						printf("====================================\n");
						printf("Message size : %d Bytes \n", ret_read);
						printf("Total Message size : %d Bytes \n", msg_size);
						printf("====================================\n");
						
						/* sendto */
						if(sendto(uds_sockfd, data, ret_read, 0, (struct sockaddr*)&uds_servaddr, sizeof(uds_servaddr)) < 0) 
							err_sys("ERROR : Unix sendto error");
						memset(data, 0x00, sizeof(data));
					}


				}

			}	// if

		}	//for
	}	// while

	
	return 0;

}

int add_socket(int fd){
	if(cnt_fd_socket < MAX_FD_SOCKET) {
		fd_socket[cnt_fd_socket] = fd;
		return ++cnt_fd_socket;
	}
	else
		return -1;
}

int del_socket(int fd) {
	int i, flag;
	flag = 0;
	close(fd);

	for(int i = 0; i < cnt_fd_socket; i++) {
		if(fd_socket[i] == fd) {
			if(i != (cnt_fd_socket - 1))
				fd_socket[i] = fd_socket[cnt_fd_socket - 1];
			fd_socket[cnt_fd_socket - 1] = -1;
			flag = 1;
			break;
		}
	}
	if(flag == 0)
		return -1;
	--cnt_fd_socket;

	return i;
}

int mk_fds(fd_set *fds, int *a_fd_socket) {
	int i, fd_max;
	FD_ZERO(fds);
	for(i = 0, fd_max = -1; i < cnt_fd_socket; i++) {
		fd_max = MAX(fd_max, a_fd_socket[i]);
		FD_SET(a_fd_socket[i], fds);			// fds_read에 감시할 파일 디스크립터 세팅
	}
	return fd_max;
}

/**
* int accept(int sock, struct sockaddr *addr, socklen_t *addrlen);
* 소켓의 연결을 수락하는 함수
*
* @param sock		: 연결을 기다리는 소켓 디스크립터
* @param addr		: 받아들인 Client 주소 정보가 저장될 구조체의 주소 값
* @param addrlen	: sockaddr 구조체의 길이가 저장된 변수의 주소값
*
* @return 성공		: 소켓 파일 지정번호(n > 0)
* @return 실패		: -1
*/

/**
* int select(int n, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
* 어느 소켓의 fd에 read, write, exception이 발생했는지 확인하는 함수 
* fd_set을 전달하여 호출하면변화가 발생한 소켓의 디스크립터만 1로 설정
*
* @param n			: 검사 대상이 되는 파일 디스크립터의 수
* @param readfds	: 읽기가 가능한지 감시
* @param writefds	: 쓰기가 가능한지 감시
* @param exceptfds	: 예외가 발생했거나 대역을 넘어서는 소켓이 존재하는지 감시
* @param timeout	: select 함수가 기다리는 시간 

* @return 성공		: 조건을 만족하는 소켓의 개수
* @return 실패		: -1

*/
