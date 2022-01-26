#include "header.h"

typedef struct msg_buf {
	long type;				// Message type, must be > 0
	char data[MAX_DATA + 1];	// Data
} message;

int main() {
	int mq_id = 0, input_num = 0;				// Message Queue ID
	message msg_buf;		
	msg_buf.type = 1;
	

	/* MQ ID 생성(msgget) */
	if((mq_id = msgget((key_t) MQ_KEY, IPC_CREAT | 0666)) == -1) {
		err_sys("ERROR : msgget error");
	}
	
	printf("Queue ID is %d \n", mq_id);

	while(1) {
		/* 1.사용자 입력 / 2.파일 입력 / 3.종료 */
		printf("1. Keyboard / 2. Textfile / 3. Exit : ");
		scanf("%d", &input_num);
		
		/* 3. 종료 */
		if( input_num == 3 ) {
			break;
		}

		/* 1. 사용자 입력 */ 
		else if( input_num == 1 ) {
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
		else if(input_num == 2)  {
			char file_name[20];
			printf("Input File : ");
			scanf("%s", file_name);

			FILE *fp = fopen(file_name, "rb");		// File read
			int file_size;
			fseek(fp, 0, SEEK_END);					// fp -> end position
			file_size = ftell(fp);					// return fp position value
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


/**
* int msgget(key_t key, int msgflg);
* System V의 메시지 큐 ID를 얻어오는 함수
*
* @param key	: 메시지 큐를 얻어 올 때 사용하는 고유 key 
* @param msgflg	: flag에는 IPC_CREAT과 IPC_EXCL이 있다.
*				  IPC_CREATE : 메시지 큐가 없으면 새로 생성
*				  IPC_EXCL	 : 메시지 큐가 존재하면 msgget은 에러를 리턴
*
* @return 성공	: 메시지 큐의 ID 
* @return 실패	: -1
*/

/**
* int msgsnd(int msqid, const void *msgp, size_T msgsz, int msgflg);
* 메시지를 보내는 함수
*
* @param msqid	: 메시지 큐의 ID
* @param msgp	: msgp는 void*이지만 구조체 형식으로 정의
* @param msgsz	: 메시지 큐에 전송할 데이터의 사이즈(type을 제외한 크기)
* @param msgflg	: 큐의 공간이 없을 때 msgsnd의 동작은 Blocking인데 이에 대한 옵션
*				  IPC_NOWAIT : Blocking되지 않고 실패					
*
* @return 성공	: 0
* @return 실패	: -1 
*/
