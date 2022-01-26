#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdlib.h>
#include "log.h"



static char log_file_prefix[64];
static char log_folder[512] = ".";
static FILE *fp_log_file;
static int log_level;

/**
* scanset 
* %s와 유사하지만 모든 문자열이 아닌 별도로 기술한 범위에 속하는 것만 입력받는 것
*
*
* @ %[abcde]	: a,b,c,d,e에 해당하는 문자만 입력
* @ %[^abcde]	: a,b,c,d,e를 만나면 검색 중단
* @		*		: *은 할당 금지를 지정하는 서식문자
*
*/


void LOGSetLevel(int log_lvl) {
	log_level = log_lvl;
}

void OpenConf(char *log_name) {
	FILE *fp_conf = fopen("Log.conf", "r");
	char ch[7000];
	char path[100];
	char lvl;
	while(fgets(ch, sizeof(path), fp_conf)) {
		if(strncmp(ch, "[PATH]", 6)  == 0) {			
			fscanf(fp_conf, "%*[^=]=%s", path);		// 두번째 '='와 다음 입력문자가 일치하면 버림
//			printf("PATH = %s \n", path);
		}
		
		if(strncmp(ch, "[LEVEL]", 7)  == 0) {			
			fscanf(fp_conf, "%*[^=]=%c", &lvl);
//			printf("LEVEL = %c \n", lvl);
		}
	}
	fclose(fp_conf);
	
	strncpy(log_file_prefix, log_name, 64);
	strncpy(log_folder, path, 512);

	switch(lvl) {
		case 'A':
			LOGSetLevel(LOG_LEVEL_A_FATAL);
			break;
		case 'B':
			LOGSetLevel(LOG_LEVEL_B_ERROR);
			break;
		case 'C':
			LOGSetLevel(LOG_LEVEL_C_WARNING);
			break;
		case 'D':
			LOGSetLevel(LOG_LEVEL_D_INFO);
			break;
		case 'E':
			LOGSetLevel(LOG_LEVEL_E_DEBUG);
			break;
		case 'F':
			LOGSetLevel(LOG_LEVEL_F_TRACE);
			break;
	}


}


int LOGGetLevel() {
	return log_level;
}

/*
* Loglogging(...) 로그 파일을 생성함.
* Log Type, Time , PID, Source file, Function, Line Num, Error
* 의 format으로 로그를 생성
*/
int LOGlogging(char log_type, const char *src_file, const char *func, int line_num, const char *fmt, ...) {
	va_list ap;
	int sz = 0;
	struct timeval tv;
	struct tm *tm1;
	static int day = -1;
	static pid_t pid = -1;
	char src_info[128];
	char filename[1024];

	gettimeofday(&tv, NULL);
	tm1 = localtime(&tv.tv_sec);
	va_start(ap, fmt);

	pid = getpid();

	snprintf(filename, 1024, "%s/%s-%04d%02d%02d.log", log_folder, log_file_prefix, 1900 + tm1->tm_year, tm1->tm_mon + 1, tm1->tm_mday);
	fp_log_file = fopen(filename, "a");

	sz += fprintf(fp_log_file, "(%c) ", log_type);
	sz += fprintf(fp_log_file, "%04d.%02d.%02d / %02d:%02d:%02d / %05d", 1900 + tm1->tm_year, tm1->tm_mon + 1, tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec, pid);
	snprintf(src_info, 128, "%s:%s(%d)", src_file, func, line_num);
	sz += fprintf(fp_log_file, " : %-17.17s : ", src_info);
	sz += vfprintf(fp_log_file, fmt, ap);
	sz += fprintf(fp_log_file, "\n");
	va_end(ap);
	fclose(fp_log_file);
	return sz;
}


