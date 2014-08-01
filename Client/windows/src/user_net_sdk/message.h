#ifndef MESSAGE_H
#define MESSAGE_H

#define TOKEN_LENGTH			5
#define TOKEN_STR				"@3%^1"

/* JK(JSON KEY) */
#define PROTO_VERSION			"1.0" 
#define JK_MESSAGE_ID			"mid"
#define JK_MESSAGE_TYPE 		"mt"
#define JK_PROTO_VERSION		"pv"
#define JK_LOGIC_PROCESS_TYPE	"lpt"
#define JK_RESULT    			"rt"
#define JK_CLINET_SFD			"cfd"
#define JK_CLINET_SFD_ID		"cfdid"

#define JK_SESSION_ID			"sid"

#define JK_USERNAME				"username"
#define JK_PASSWORD				"password"
#define JK_NEW_PASSWORD			"newpass"
#define JK_USER_OTHER_INFO      "uoi"
#define JK_SECURITY_MAIL		"sm"
#define JK_CLIENT_PLATFORM		"cp"

#define JK_USER_ONLINE_STATUS   "uls"

#define JK_IM_SERVER_NO         "isn"
#define JK_RELAY_MESSAGE        "rm"
#define JK_RELAY_MESSAGE_GUID   "rmg"
#define JK_RELAY_MESSAGE_TIMESTAMP "rmt"

/*从IM服务器发送到转发服务器的消息类型，是请求中转的消息 还是  中转消息的发送结果（用于确认是否删除消息队列中的已存储消息）*/
#define JK_P2RELAY_MESSAGE_TYPE  "p2rmt"

#define JK_ONLINE_SERVER_NO		"osn"
#define JK_ONLINE_SERVER_FD		"osf"
#define JK_ONLINE_SERVER_FD_ID	"osfi"

#define JK_CREATE_TIME			"ct"


#define JK_PUSH_MESSAGE_TYPE	"pmt"

#define JK_CUSTOM_STRING		"cs"

#define JK_ASYN_REQUEST_ID		"ari"
#define JK_ASYN_REQUEST			"ar"

enum P2RelayMessageType
{
	NOTIFY_OFFLINE = 4000,
	RELAY_NOTIFY_OFFLINE = 4001,

	HEATBEAT_DETECT = 4002,
	HEATBEAT_DETECT_RESPONSE = 4003,
};

enum TcpConnectFlag
{
	SHORT_CONNECTION,
	PERSIST_CONNECTION,
};

enum LogicProcessType
{
	ACCOUNT_BUSINESS_PROCESS = 0,

	IM_SERVER_DIRECT		= 6,
	IM_SERVER_RELAY			= 7,
	IM_SERVER_RELAY_REQUEST = 8,

	ALARM_SERVER_RELAY = 10,
};

#endif

