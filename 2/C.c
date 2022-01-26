#include "header.h"

int cnt_fd_socket = 0;
int fd_biggest;
int fd_socket[MAX_FD_SOCKET];

int main() {
	/* TCP Socket */
	int tcp_listenfd = 0, msg_size = 0;
	char data[MAX_DATA + 1] = {'\0', };

	OpenConf("C-log");

	LOG_CALL(tcp_listenfd = socket(PF_INET, SOCK_STREAM, 0));
	if(tcp_listenfd < 0) {
		LOG_ERROR("ERROR : TCP socket error");
		err_sys("ERROR : TCP socket error");
	}

	/* setsockopt */
	int optval = 1;
	LOG_CALL(setsockopt(tcp_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)));

	/* TCP bind */
	struct sockaddr_in tcp_servaddr;
	socklen_t tcp_servaddrlen = sizeof(tcp_servaddr);
	memset(&tcp_servaddr, 0, sizeof(tcp_servaddr));
	tcp_servaddr.sin_family = AF_INET;
	tcp_servaddr.sin_port = htons(PORT_TCP);
	tcp_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret_tcpbind;
	LOG_CALL(ret_tcpbind = bind(tcp_listenfd, (struct sockaddr*)&tcp_servaddr, sizeof(tcp_servaddr)));
	if (tcp_listenfd < 0) {
		LOG_ERROR("ERROR : TCP bind error");
		err_sys("ERROR : TCP bind error");
	}

	/* TCP listen */
	int ret_listen;
	LOG_CALL(ret_listen=listen(tcp_listenfd, SOMAXCONN));
	if(ret_listen < 0) {
		LOG_ERROR("ERROR : TCP listen error");
		err_sys("ERROR : TCP listen error");
	}

	/* ADD Socket in Array */
	add_socket(tcp_listenfd);

	/* NON-BLOCK for tcp_listendfd */
	int tcp_nblistenfd = fcntl(tcp_listenfd, F_GETFL, 0);
	fcntl(tcp_listenfd, F_SETFL, tcp_nblistenfd | O_NONBLOCK);

	/* Unix Domain Socket(UDP) */
	int uds_sockfd;
	struct sockaddr_un uds_servaddr;

	if(access(SOCK_LOCALFILE, F_OK) == 0 )
		unlink(SOCK_LOCALFILE);


	LOG_CALL(uds_sockfd = socket(AF_UNIX, SOCK_DGRAM, IPPROTO_IP));
	if(uds_sockfd < 0) {
		LOG_ERROR("ERROR : Unix socket error");
		err_sys("ERROR : Unix socket error");
	}

	memset(&uds_servaddr, 0, sizeof(uds_servaddr));
	uds_servaddr.sun_family = AF_UNIX;
	strcpy(uds_servaddr.sun_path, SOCK_LOCALFILE);

	/* Unix bind */
	int ret_udsbind = 0;
	LOG_CALL(ret_udsbind = bind(uds_sockfd, (struct sockaddr*) &uds_servaddr, sizeof(uds_servaddr)));
	if(ret_udsbind < 0) {
		LOG_ERROR("ERROR : Unix bind error");
		err_sys("ERROR : Unix bind error");
	}

	struct sockaddr_in tcp_clntaddr;
	socklen_t tcp_clntaddrlen = sizeof(tcp_clntaddr);
	fd_set fds_send, fds_read;
	int ret_sel= 0, ret_read = 0, connfd = 0;

	while(1) {
		/* Accept */
		fd_biggest = mk_fds(&fds_read, fd_socket);
		if((ret_sel = select(fd_biggest + 1, &fds_read, NULL, NULL, NULL)) == -1){
			LOG_ERROR("ERROR : select error");
			err_sys("ERROR : select error");
		}
		if(FD_ISSET(tcp_listenfd, &fds_read)) {
			connfd = accept(tcp_listenfd, (struct sockaddr*)&tcp_clntaddr, &tcp_clntaddrlen);
			if(connfd == -1) {
				err_sys("ERROR : TCP accept error");
				continue;

			}
			if(add_socket(connfd) == -1) 
				err_sys("ERROR : Failed to add socket \n");
			printf("Accept : Add socket(%d) \n", connfd);
			continue;
		}
		/* read */
		for(int i = 1; i < cnt_fd_socket; i++) {
			if(FD_ISSET(fd_socket[i], &fds_read)) {
				if((ret_read = read(fd_socket[i], data, DATA_SIZE) ) < 0) {
					LOG_ERROR("ERROR : TCP read error");
					err_sys("ERROR : TCP read error");
				}
				else {
					if(ret_read == 0) {
						LOG_WARNING("WARNING : Session Closed");
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
						if(sendto(uds_sockfd, data, ret_read, 0, (struct sockaddr*)&uds_servaddr, sizeof(uds_servaddr)) < 0) {
							LOG_ERROR("ERROR : Unix sendto error");
							err_sys("ERROR : Unix sendto error");
						}
						memset(data, 0x00, sizeof(data));
					}


				}

			}	// if

		}	//for
	}	// while

	
	return 0;

}

int add_socket(int fd){
	if( cnt_fd_socket < MAX_FD_SOCKET ) {
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
		if( fd_socket[i] == fd ) {
			if( i != (cnt_fd_socket - 1) )
				fd_socket[i] = fd_socket[cnt_fd_socket - 1];
			fd_socket[cnt_fd_socket - 1] = -1;
			flag = 1;
			break;
		}
	}
	if( flag == 0 )
		return -1;
	--cnt_fd_socket;

	return i;
}

int mk_fds(fd_set *fds, int *a_fd_socket) {
	int i, fd_max;
	FD_ZERO(fds);
	for( i = 0, fd_max = -1; i < cnt_fd_socket; i++ ) {
		fd_max = MAX(fd_max, a_fd_socket[i]);
		FD_SET(a_fd_socket[i], fds);
	}
	return fd_max;
}
