#include "header.h"

typedef struct msg_buf {
	long type;							// Message type, must be > 0
	char data[MAX_DATA + 1];				// Data
} message;

int main() {
	int mq_id = 0, input_num = 0;		// Message Queue ID
	message msg_buf;		
	msg_buf.type = 1;
	
	OpenConf("A-log");

	/* MQ ID 생성(msgget) */
	LOG_CALL(mq_id = msgget((key_t) MQ_KEY, IPC_CREAT | 0666));
	if(mq_id == -1) {
		LOG_ERROR("ERROR : msgget error");
		err_sys("ERROR : msgget error");
	}

	printf("Queue ID is %d \n", mq_id);

	while(1) {
		/* 1.사용자 입력 / 2.파일 입력 / 3.종료 */
		printf("1. Keyboard / 2. Textfile / 3. Exit : ");
		scanf("%d", &input_num);

		/* 3. 종료 */
		if(input_num == 3) {
			break;
		}

		/* 1. 사용자 입력 */ 
		else if(input_num == 1) {
			while(1) {
				printf("Input Message : ");
				scanf(" %[^\n]s", msg_buf.data);

				if(!strncmp(msg_buf.data, "/quit", 5))
					break;
				
				/* 데이터 전송(msgsnd) */
				if(msgsnd(mq_id, &msg_buf, strlen(msg_buf.data), 0 ) == -1) {
					err_sys("ERROR : msgsnd error");
				}
				memset(msg_buf.data, 0x00, sizeof(msg_buf.data));

			}	// while
		} // else if

		/* 2. 파일 입력 */
		else if(input_num == 2) {
			char file_name[20];
			printf("Input File : ");
			scanf("%s", file_name);

			FILE *fp = fopen(file_name, "rb");		// File read
			int file_size;
			fseek(fp, 0, SEEK_END);					// fp -> end position
			file_size = ftell(fp);
			printf("Message Size = %d Bytes \n", file_size);
			fseek(fp, 0, SEEK_SET);					// fp -> first position

			int ret_fread;
			while(!feof(fp)) {
				/* 파일 입력(fread) */
				if((ret_fread = fread(msg_buf.data, sizeof(char), DATA_SIZE, fp)) == 0)
					break;
			
				/* 메시지 전송(msgsnd) */
				if(msgsnd(mq_id, &msg_buf, ret_fread, 0) == -1) {
					err_sys("ERROR : msgsnd error");
				}
				memset(msg_buf.data, 0x00, sizeof(msg_buf.data));
			}
			fclose(fp);
		}	// else if
	}	// while
}


