#ifndef __LOG_H__
#define __LOG_H__

#define LOG_LEVEL_A_FATAL	0
#define LOG_LEVEL_B_ERROR	10
#define LOG_LEVEL_C_WARNING	20
#define LOG_LEVEL_D_INFO	30
#define LOG_LEVEL_E_DEBUG	40
#define LOG_LEVEL_F_TRACE	50


/*	각각의 Level에서 Log를 생성가능한 상태인지를 체크하는 Macro */
#define LOG_IS_FATAL	(LOGGetLevel() >= LOG_LEVEL_A_FATAL)
#define LOG_IS_ERROR	(LOGGetLevel() >= LOG_LEVEL_B_ERROR)
#define LOG_IS_WARNING	(LOGGetLevel() >= LOG_LEVEL_C_WARNING)
#define LOG_IS_INFO		(LOGGetLevel() >= LOG_LEVEL_D_INFO)
#define LOG_IS_DEBUG	(LOGGetLevel() >= LOG_LEVEL_E_DEBUG)
#define LOG_IS_TRACE	(LOGGetLevel() >= LOG_LEVEL_F_TRACE)

void OpenConf(char *ch);
int LOGlogging(char log_type, const char *src_file, const char *func, int line_num, const char *fmt, ...);
int LOGGetLevel();
void LOGSetLevel(int log_lvl);


#define LOG_Message(...)\
		LOGlogging('M', __FILE__, __func__, __LINE__, __VA_ARGS__);

/* 함수의 실행 시작과 종료를 Log로 생성하는 Macro 함수 */
#define LOG_CALL(func)\
		LOG_TRACE("%s is Starting...", #func);\
		func;\
		LOG_TRACE("%s is End", #func)

/* Trace Log를 생성하는 Macro 함수 */  
#define LOG_TRACE(...) \
	do{ \
		if(LOG_IS_TRACE) { \
			LOGlogging('F', __FILE__, __func__, __LINE__, __VA_ARGS__);\
		} \
	}while(0)

/* Debug Log를 생성하는 Macro 함수 */
#define LOG_DEBUG(...) \
	do{ \
		if(LOG_IS_DEBUG){ \
			LOGlogging('E', __FILE__, __func__, __LINE__, __VA_ARGS__);\
		} \
	}while(0)

/* 중요 정보 Log를 생성하는 Macro 함수 */
#define LOG_INFO(...) \
	do{ \
		if(LOG_IS_INFO) { \
			LOGlogging('D', __FILE__, __func__, __LINE__, __VA_ARGS__);\
		} \
	}while(0)

/* Warning Log를 생성하는 Macro 함수 */
#define LOG_WARNING(...)\
	do{ \
		if(LOG_IS_WARNING) { \
			LOGlogging('C', __FILE__, __func__, __LINE__, __VA_ARGS__);\
		} \
	}while(0)

/* Error Log를 생성하는 Macro 함수 */
#define LOG_ERROR(...) \
	do{ \
		if(LOG_IS_ERROR) { \
			LOGlogging('B', __FILE__, __func__, __LINE__, __VA_ARGS__);\
		}\
	}while(0)

/* Fatal Log를 생성하는 Macro 함수 */
#define LOG_FATAL(...) \
	do{ \
		if(LOG_IS_FATAL) { \
			LOGlogging('A', __FILE__, __func__, __LINE__, __VA_ARGS__);\
		}\
	}while(0)

#endif	//__LOG_H__
