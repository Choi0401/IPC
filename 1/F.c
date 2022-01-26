#include "header.h"

int main() {
	/* Shared Memory */
	int msg_size = 0, shmid = 0;
	char data[MAX_DATA + 1] = {'\0',};
	FILE *fp;
	if((shmid = shmget((key_t)SHM_KEY, sizeof(data), 0666 | IPC_CREAT) ) == -1)
		err_sys("ERROR : shmget error");

	void *shmaddr = NULL;
	if((shmaddr = shmat(shmid, NULL, 0)) == (void*) -1)
		err_sys("ERROR : shmat error");

	memset((char*)shmaddr, 0x00, sizeof(shmaddr));

	sem_t *shm_sema;
	if((shm_sema = sem_open("/shm_sema", 0, 0666, 0)) == SEM_FAILED)
		err_sys("ERROR : sem_open error");

	if(access("Message.txt", F_OK) == 0)
		unlink("Message.txt");

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

				/* File Open */
				fp = fopen("Message.txt", "a");
				fputs(data, fp);
				fclose(fp);

				memset(data, 0x00, sizeof(data));

			}
			else usleep(50);

		}
	}

	return 0;
}
