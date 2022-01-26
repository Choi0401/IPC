#include "header.h"

int main() {
	/* Unix Domain Socket(UDP) */
	char data[MAX_DATA + 1] = {'\0', };
	int msg_size = 0, uds_sockfd = 0;
	
	OpenConf("D-log");

	struct sockaddr_un uds_servaddr;
	int uds_addrlen = sizeof(uds_servaddr);

	if(access( SOCK_LOCALFILE, F_OK ) == 0)
		unlink(SOCK_LOCALFILE);

	/* Unix Socket */
	LOG_CALL(uds_sockfd = socket(AF_UNIX, SOCK_DGRAM, IPPROTO_IP));
	if(uds_sockfd  < 0) {
		LOG_ERROR("ERROR : Unix socket error");
		err_sys("ERROR : Unix socket error");
	}

	memset(&uds_servaddr, 0, sizeof(uds_servaddr));
	uds_servaddr.sun_family = AF_UNIX;
	strcpy(uds_servaddr.sun_path, SOCK_LOCALFILE);

	/* Unix bind */
	int ret_udsbind = 0;
	LOG_CALL(ret_udsbind = bind(uds_sockfd, (struct sockaddr*)&uds_servaddr, sizeof(uds_servaddr)));
	if(ret_udsbind < 0) {
		LOG_ERROR("ERROR : Unix bind error");
		err_sys("ERROR : Unix bind error");
	}

	/* UDP Socket */
	int udp_sockfd = 0;
	LOG_CALL(udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0));
	if(udp_sockfd < 0) {
		LOG_ERROR("ERROR : UDP socket error");
		err_sys("ERROR : UDP socket error");
	}

	/* UDP bind */
	struct sockaddr_in udp_servaddr;
	memset(&udp_servaddr, 0, sizeof(udp_servaddr));
	udp_servaddr.sin_family = AF_INET;
	udp_servaddr.sin_port = htons(PORT_UDP);
	udp_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret_recv;

	while(1) {
		if((ret_recv = recvfrom(uds_sockfd, data, sizeof(data), 0, (struct sockaddr*)&uds_servaddr, (socklen_t*)&uds_addrlen) ) < 0)
			err_sys("ERROR : Unix recvfrom error");
		if(ret_recv > 0) {
			msg_size += ret_recv;
			printf("Received Message from C : %s \n", data);
			printf("===================================\n");
			printf("Message size : %d Bytes\n", ret_recv);
			printf("Total Messag size : %d Bytes \n", msg_size);			
			printf("===================================\n");

			/* UDP sendto */
			if(sendto(udp_sockfd, data, ret_recv, 0, (struct sockaddr*)&udp_servaddr, sizeof(udp_servaddr)) < 0) {
				LOG_ERROR("UDP sendto error");
				err_sys("UDP sendto error");
			}
			memset(data, 0x00, sizeof(data));

		}

	}	// while

	return 0;

}
