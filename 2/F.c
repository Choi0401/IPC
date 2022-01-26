#include "header.h"

int main() {
	OpenConf("F-log");
	int msg_size = 0;
	char data[MAX_DATA + 1] = {'\0',};
	FILE *fp;

	/* Shared Memory */
	int shmid;
	
	LOG_CALL(shmid = shmget((key_t)SHM_KEY, sizeof(data), 0666 | IPC_CREAT));
	if( shmid == -1 ) {
		LOG_ERROR("ERROR : shmget error");
		err_sys("ERROR : shmget error");
	}

	void *shmaddr = NULL;

	LOG_CALL(shmaddr = shmat(shmid, NULL, 0));
	if( shmaddr == (void*) -1 ) {
		LOG_ERROR("ERROR : shmat error");
		err_sys("ERROR : shmat error");
	}

	memset((char*)shmaddr, 0x00, sizeof(shmaddr));

	sem_t *shm_sema;

	LOG_CALL(shm_sema = sem_open("/shm_sema", 0, 0666, 0) );
	if( shm_sema == SEM_FAILED ) {
		LOG_ERROR("ERROR : sem_open error");
		err_sys("ERROR : sem_open error");
	}

	while(1) {
		if(shmid != -1){
			if(strlen( (char *)shmaddr ) != 0) {
				sem_wait(shm_sema);
				memcpy(data, (char *)shmaddr, sizeof(data));
				memset((char *)shmaddr, 0x00, sizeof(data));
				sem_post(shm_sema);
				printf("Recevied Message from E : %s \n", data);
				msg_size += strlen(data);
				printf("===================================\n");
				printf("Message size : %ld Bytes \n", strlen(data));
				printf("Total message size : %d Bytes \n", msg_size);
				printf("===================================\n");

				LOG_Message("%s\n", data);

				memset(data, 0x00, sizeof(data));

			}
			else usleep(50);

		}
	}

	return 0;
}
