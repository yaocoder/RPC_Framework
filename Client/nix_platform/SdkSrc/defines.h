/*
 * defines.h
 *
 *  Created on: 2012-9-12
 *      Author: yaowei
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <list>
#include <event.h>

#ifdef LINUX_PLATFORM

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
using namespace log4cxx;
extern LoggerPtr g_logger;

#define ANDROID_LOG_INFO(a)
#define ANDROID_LOG_WARN(a)
#define ANDROID_LOG_ERROR(a)
#define ANDROID_LOG_FATAL(a)

#define LOGI(...)
#define LOGW(...)
#define LOGE(...)

#define LOG_T(s, ...)
#define LOG_I(s, ...)
#define LOG_W(s, ...)
#define LOG_E(s, ...)

#else

#define LoggerPtr (void*)
#define LOG4CXX_TRACE(a,b)
#define LOG4CXX_DEBUG(a,b)
#define LOG4CXX_INFO(a,b)
#define LOG4CXX_WARN(a,b)
#define LOG4CXX_ERROR(a,b)
#define LOG4CXX_FATAL(a,b)

extern char *m_logurl;
extern FILE *g_f;

#define LOGNAME_FLAG "UserClientSDK"

#define WRITE_LOG(level, s, ...) do { \
if(NULL == g_f) {\
break;}\
std::time_t time_now = std::time(NULL); \
tm* tm_now = localtime(&time_now); \
char time_str[50] = {0}; \
std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_now); \
char log_string[300] = {0}; \
snprintf(log_string, sizeof(log_string), "%s %s %s - %s \n", time_str, level, LOGNAME_FLAG, s);\
flockfile (g_f); \
fprintf(g_f, log_string, __VA_ARGS__); \
fflush(g_f); \
funlockfile (g_f); \
} while (0)

#define LOG_T(s, ...)     WRITE_LOG("TRACE", s, __VA_ARGS__)
#define LOG_I(s, ...)     WRITE_LOG("INFO", s, __VA_ARGS__)
#define LOG_W(s, ...)     WRITE_LOG("WARN", s, __VA_ARGS__)
#define LOG_E(s, ...)     WRITE_LOG("ERROR", s, __VA_ARGS__)

#endif

#ifdef APPLE_PLATFORM
#define LOGI(...)
#define LOGW(...)
#define LOGE(...)
#endif


#ifdef ANDROID_PLATFORM

#include <android/log.h>
#define LOG_TAG "AccountSystem"
#define ANDROID_LOG_INFO(a)  __android_log_write(ANDROID_LOG_INFO, LOG_TAG, a)
#define ANDROID_LOG_WARN(a)  __android_log_write(ANDROID_LOG_WARN, LOG_TAG, a)
#define ANDROID_LOG_ERROR(a)  __android_log_write(ANDROID_LOG_ERROR, LOG_TAG, a)
#define ANDROID_LOG_FATAL(a)  __android_log_write(ANDROID_LOG_FATAL, LOG_TAG, a)

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#endif

#define DATA_BUFFER_SIZE 10240
#define CRLF "\r\n"



#endif /* DEFINES_H_ */
