#ifndef user_interface_defines_h__
#define user_interface_defines_h__

#ifdef WIN32
#include <winsock2.h>  
#include <windows.h>
#endif

#include <string>

#ifdef USE_LOG4CXX

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
using namespace log4cxx;
extern LoggerPtr g_logger;

#else

#define LOG4CXX_TRACE(a,b)
#define LOG4CXX_DEBUG(a,b)
#define LOG4CXX_INFO(a,b)
#define LOG4CXX_WARN(a,b)
#define LOG4CXX_ERROR(a,b)
#define LOG4CXX_FATAL(a,b)

#endif

typedef struct ServerPushInfo
{
	std::string username;			//加强验证，防止串话
	std::string message;
	std::string send_timestamp;
}SERVER_PUSH_INFO;

typedef void (*ServerPushCallBack_Info)(const int message_type, const SERVER_PUSH_INFO& serverPushInfo);

#define STR_PTCP_HAS_CLOSED "TCP_CLOSED"
#define STR_PTCP_HAS_ERROR	"TCP_ERROR"

typedef void (*LiveStatusCB)(const int ret);


enum PushMessage
{
	NOTIFY_OFFLINE_ = 3102,
	PTCP_ERROR		= 3103,
	PTCP_CLOSED		= 3104,
};

enum OnlineStatus
{
	OFFLINE = 0,
	ONLINE,
	LEAVING,
	HIDING,
};

enum IReturnCode
{
	SUCCESS = 0,

	USER_HAS_EXIST		= 2,						//用户已经存在
	USER_NOT_EXIST		= 3,						//用户不存在
	PASSWORD_ERROR		= 4,						//密码错误
	SESSION_NOT_EXSIT	= 5,						//登录session不存在（登录已过期）
	SQL_NOT_FIND		= 6,						
	PTCP_HAS_CLOSED		= 7, 						

	GENERATE_PASS_ERROR = -2,						
	REDIS_OPT_ERROR		= -3,
	MY_SQL_ERROR		= -4,
	REQ_RES_TIMEOUT		= -5,
	CONN_OTHER_ERROR	= -6,
	CANT_CONNECT_SERVER = -7,
	JSON_INVALID		= -8,
	REQ_RES_OTHER_ERROR = -9,
	JSON_PARSE_ERROR	= -10,
	SEND_MAIL_FAILED	= -11,

	OTHER_ERROR			= -1000,
};


#endif // user_interface_defines_h__
