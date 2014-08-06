#ifndef user_interface_defines_h__
#define user_interface_defines_h__

#ifdef WIN32
#include <winsock2.h>  
#include <windows.h>
#endif

#include <string>

typedef struct PushInfo
{
	std::string message;
}PUSH_INFO;


#define STR_PTCP_HAS_CLOSED "TCP_CLOSED"
#define STR_PTCP_HAS_ERROR	"TCP_ERROR"

typedef void (*LiveStatusCB)(const int ret);


enum PushMessage
{
	PTCP_ERROR		= 3103,
	PTCP_CLOSED		= 3104,
};

enum IReturnCode
{
	SUCCESS = 0,

	PTCP_HAS_CLOSED		= 7, 						

	REQ_RES_TIMEOUT		= -5,
	CONN_OTHER_ERROR	= -6,
	CANT_CONNECT_SERVER = -7,
	JSON_INVALID		= -8,
	REQ_RES_OTHER_ERROR = -9,
	JSON_PARSE_ERROR	= -10,

	OTHER_ERROR			= -1000,
};


#endif // user_interface_defines_h__
