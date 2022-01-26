#include "header.h"

/* Linked List Queue */
typedef struct Node {
	char *data;
	struct Node *next;
} Node;

typedef struct Queue {
	Node *front;
	Node *rear;
} Queue;

typedef struct Thread {
	int sockfd;
	Queue q;
} Thread;

void init_queue(Queue *q);
void Enqueue(Queue *q, char *_data);
int isEmpty(Queue *q);
void Dequeue(Queue *q, char **_data);
void print_queue(Queue *q);

void *thread_func(void *);

pthread_mutex_t en_mutex;
pthread_mutex_t de_mutex;

int main() {
	/* UDP Socket */
	int udp_sockfd = 0;

	OpenConf("E-log");

	LOG_CALL(udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0));
	if(udp_sockfd  < 0) {
		LOG_ERROR("ERROR : UDP socket error");
		err_sys("ERROR : UDP socket error");
	}

	/* UDP bind */
	struct sockaddr_in udp_servaddr;
	int servlen = sizeof(udp_servaddr);
	memset(&udp_servaddr, 0, servlen);
	udp_servaddr.sin_family = AF_INET;
	udp_servaddr.sin_port = htons(PORT_UDP);
	udp_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret_udpbind;
	LOG_CALL(ret_udpbind = bind(udp_sockfd, (struct sockaddr*)&udp_servaddr, sizeof(udp_servaddr)));
	if(ret_udpbind< 0) {
		LOG_ERROR("ERROR : UDP bind error");
		err_sys("ERROR : UDP bind error");
	}

	/* Shared Memory */
	int shmid = 0;
	LOG_CALL(shmid = shmget((key_t)SHM_KEY, MAX_DATA, IPC_CREAT | 0666));
	if(shmid == -1) {
		LOG_ERROR("ERROR : shmget error");
		err_sys("ERROR : shmget error");
	}

	printf("Shared Memory ID is %d \n", shmid);

	/* shmat */
	void *shmaddr = NULL;
	LOG_CALL(shmaddr = shmat(shmid, NULL, 0));
	if(shmaddr == (void*) -1) {
		LOG_ERROR("ERROR : shmat error");
		err_sys("ERROR : shmat error");
	}

	/* Semaphore */
	sem_t *shm_sema;
	sem_unlink("/shm_sema");

	LOG_CALL(shm_sema = sem_open("/shm_sema", O_CREAT, 0666, 1));
	if( shm_sema == NULL ) {
		LOG_ERROR("ERROR : sem_open error");
		err_sys("ERROR : sem_open error");
	}

	Thread thread;
	thread.sockfd = udp_sockfd;
	init_queue(&thread.q);
	pthread_t udp_thread;
	pthread_create(&udp_thread, NULL, thread_func, (void*)&thread);

	char *deq_data;
	while(1) {
		if(!isEmpty(&thread.q) && strlen((char*)shmaddr) == 0) {
			sem_wait(shm_sema);
			Dequeue(&thread.q, &deq_data);
			size_t ch_size = malloc_usable_size(deq_data)/sizeof(char);
			memcpy((char*)shmaddr, deq_data, ch_size);
			free(deq_data);
			sem_post(shm_sema);
		}
		else usleep(50);
	}
	return 0;
}


void init_queue(Queue *q) {
	q->front = NULL;
	q->rear = NULL;
}

void Enqueue(Queue *q, char *_data){ 
	pthread_mutex_lock(&en_mutex);
	Node *newNode = (Node*)malloc(sizeof(Node));
	newNode->next = NULL;
	newNode->data = (char*)malloc(strlen(_data) + 1);
	memset(newNode->data, 0x00, strlen(_data) + 1);
	memcpy(newNode->data, _data, strlen(_data) + 1);

//	printf("_data(%ld Bytes) : %s \n", strlen(_data), _data);
//	printf("newNode->data(%ld Bytes) : %s \n", strlen(newNode->data), newNode->data);

	if(isEmpty(q)) {
		q->front = newNode;
		q->rear = newNode;
	}
	else {
		q->rear->next = newNode;
		q->rear = newNode;
	}
	pthread_mutex_unlock(&en_mutex);
}


int isEmpty(Queue *q) {
	if(q->front == NULL)
		return TRUE;
	else
		return FALSE;

}

void Dequeue(Queue *q, char **_data) {
	pthread_mutex_lock(&de_mutex);
	Node *delNode = (Node*)malloc(sizeof(Node));
	if(isEmpty(q)) {
		printf("Queue is empty \n");
	}
	else {
		delNode = q->front;
		int len = strlen(delNode->data);
		*_data = (char*)malloc(len + 1);
		memset(*_data, 0x00, len + 1);
		memcpy(*_data, delNode->data, len + 1);			
//		printf("*_data(%ld Bytes) : %s \n", strlen(*_data), *_data);
//		printf("delNode->data(%ld Bytes) : %s \n",  strlen(delNode->data, delNode->data));
		q->front = q->front->next;
	}
	free(delNode->data);
	free(delNode);
	pthread_mutex_unlock(&de_mutex);
}
void print_queue(Queue *q){
	Node *tmp_node = (Node*)malloc(sizeof(Node));
	tmp_node = q->front;
	while( tmp_node != NULL ) {
		printf("%s \n", tmp_node->data);
		tmp_node = tmp_node->next;
	}
}

void *thread_func(void *arg){
	Thread *_th = (Thread*)arg;
	int msg_size = 0;
	char _data[MAX_DATA + 1] = {'\0', };
	while(1) {		
		int ret_recv;
		if((ret_recv = recvfrom(_th->sockfd, _data, sizeof(_data), 0, NULL, NULL)) < 0) {

			LOG_ERROR("UDP recvfrom error");
			err_sys("UDP recvfrom error");
		}
		if(ret_recv > 0) {
//			printf("EnQueue is running.. \n");
			Enqueue(&_th->q, _data);
//			print_queue(&_th->q);
			msg_size += ret_recv;
			printf("Received Message from D : %s \n", _data);
			printf("===================================\n");
			printf("Message Size  : %d Bytes\n", ret_recv);
			printf("Total Message : %d Bytes\n", msg_size);
			printf("===================================\n");
			memset(_data, 0x00, sizeof(_data));
		}
	}
	return NULL;
}
