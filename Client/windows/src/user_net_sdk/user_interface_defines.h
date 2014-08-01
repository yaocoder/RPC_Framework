#ifndef user_interface_defines_h__
#define user_interface_defines_h__

#ifdef WIN32
#include <winsock2.h>  
#include <windows.h>
#endif

#include <string>
#include <vector>
#include <map>
#include <set>

typedef struct updateFileInfo_
{
	std::string	update_version;
	std::string file_url;
	std::string file_description;
	std::string file_checksum;
}UPDATE_FILE_INFO;


/**
 * @brief   好友信息
 */
typedef struct friendInfo_
{
	std::string username;
	std::string nickname;
	std::string signature;
	int			team_id;
}FRIEND_INFO;

/**
 * @brief   用户好友信息
 */
typedef struct friendsList_
{
	std::string friends_team_describe;
	std::vector<FRIEND_INFO> vec_friend_info;
}FRIENDS_LIST;

/**
 * @brief   设备信息
 */
typedef struct deviceInfo_
{
	std::string device_guid;
	std::string device_name;
	std::string user_name;
	int device_type;
	int device_team_id;
	int device_permission;
	std::string longitude;
	std::string latitude;	
}DEVICE_INFO;

/**
 * @brief   用户设备信息
 */
typedef struct devicesList_
{
	std::string devices_team_describe;
	std::vector<DEVICE_INFO> vec_device_info;
}DEVICES_LIST;

/**
 * @brief   用户基本信息
 */
typedef struct userInfo_
{
	std::string username;
	std::string nickname;
	int	gender;
	std::string birth_date;
	std::string name;
	std::string chinese_birth;
	std::string western_birth;
	std::string city;
	std::string address;
	std::string job;
	std::string phone;
	std::string signature;
	std::string security_mail;
	std::string	head_url;
	int			user_level;
	std::string update_user_level_date;
    //...
}USER_INFO;

typedef struct userNoticeEvent_
{
	std::string username;
	long event_id;
	int event_year;
	int event_month;
	int event_day;
	std::string event_time;
	std::string event_text;
	//...
}USER_NOTICE_EVENT;

enum gender
{
	UNKNOWN,
	MALE,
	FEMALE,
};


/**
 * @brief   设备通道信息
 */
typedef struct deviceChannelInfo_
{
	int channel_no;
	int channel_permission;
	std::string channel_name;
	std::string longitude;
	std::string latitude;
}DEVICE_CHANNEL_INFO;


/**
 * @brief   群组信息
 */
typedef struct groupInfo_
{
	std::string group_id;
	int group_num;
	std::string group_creater;
	std::string group_name;
	std::string group_type;
	std::string group_describe;
	std::string group_team_info;
	std::string create_time;
	std::string callboard;
	int member_count;
}GROUP_INFO;

/**
 * @brief   群组成员信息
 */
typedef struct groupMemberInfo_
{
	std::string group_member_name;
	std::string group_member_nickname;
	int			group_team_id;
}GROUP_MEMBER_INFO;


/**
 * @brief   离线消息
 */
typedef struct offlineMessage_
{
	int  message_type;
	std::string from_name;
	std::string nickname;
	std::string message;
	std::string send_timestamp;
	std::string group_id;

	//被踢出群的提示
	int push_message_type;
	std::string group_name;
}OFFLINE_MESSAGE;

typedef struct searchUserConditions_
{
	std::string nickname;
	int gender;
	int age;
	std::string address;
}SEARCH_USER_CONDITIONS;

typedef struct ServerPushInfo
{
	std::string username;			//加强验证，防止串话
	std::string peer_name;
	std::string peer_nickname; 
	std::string message;
	std::string group_id;
	std::string group_name;
	std::string send_timestamp;
	int	device_type;
	std::string device_name;
}SERVER_PUSH_INFO;

const int DISABLE_FLAG = -1;

typedef void (*ServerPushCallBack_Info)(const int message_type, const SERVER_PUSH_INFO& serverPushInfo);

/**
* @brief 获取所有好友的在线状态的回调函数
* @param [out]	获取是否成功
* @param [out]	好友在线状态
* @return 
*/
typedef void  (*FriendsOrDevicesStatusCallBack)(const int ret, 
												const std::map<std::string, int>& map_friendsOrDevices_status);

/**
* @brief 获取群组成员的在线状态的回调函数
* @param [out]	获取是否成功
* @param [out]	群组id
* @param [out]	好友在线状态
* @return 
*/
typedef void  (*GroupMembersStatusCallBack)(const int ret, 
											const std::string& group_id, 
											const std::map<std::string, int>& map_members_status);


enum IReturnCode
{
	SUCCESS = 0,

	FAILED  = -1,
	/** DeviceRegisterResponse */
	DEVICE_HAS_EXIST = 1,
	DEVICE_NOT_EXIST = 2,
	DEVICE_NOT_BIND = 3,


	/** LoginResponse */
	USER_HAS_EXIST = 1,
	USER_NOT_EXIST = 2,
	PASSWORD_ERROR = 3,

	REDIS_OPT_ERROR = -70,
	OTHER_ERROR = -100,

	/** SQL */
	MY_SQL_ERROR = -50,
	SQL_NOT_FIND = 51,
	SQL_FIND = 52,

	JSON_PARSE_ERROR = -63,

	/** Session Response */
	SESSION_NOT_EXSIT = 4,

	/** AddFriendResponse */
	WAIT_FRIEND_VERIFY = 5,
	FRIEND_REFUSED = 6,

	/** DeviceRegisterResponse */
	DEVICE_HAS_REGISTER = 7,

	TIMEOUT = -101,
	C_OTHER_ERROR = -102,
	CANT_CONNECT_SERVER = -111,
	STRING_NULL = -103,
	JSON_INVALID = -104,
	PTCP_HAS_CLOSED = -1000, 

	DONT_NEED_UPDATE = -500,

	GENERATE_PASS_ERROR = -501,

	ERROR_DEVICE_TYPE = -502,

	SECURITY_MAIL_ERROR = 8,
	SEND_MAIL_FAILED = 9,
	SECURITY_MAIL_NOT_EXIST = 10,
};

enum DevicePermission
{
	PRIVATE,	//私有，自己可见
	SHARE,		//共享，对好友可见
	PUBLIC,		//公开，对群组可见
	OPEN,		//开放，对所有人可见
};


typedef enum online_status
{
	OFFLINE,
	ONLINE,
	LEAVING,
	HIDING,

	ONLINE_CONFLICT,
}
ONLINE_STATUS;

enum DeviceChannelAttribute
{
	HIDE,
	SHOW,
};


enum DeviceType
{
	DVR = 1,
	IPC,
	NVR,
	DVC,
	MPD,
};


#define STR_PTCP_HAS_CLOSED "TCP_CLOSED"
#define STR_PTCP_HAS_ERROR	"TCP_ERROR"


enum PushMessage
{
	REQUEST_ADD_FRIEND_ = 1231,

	REPLY_ADD_FRIEND_ = 1233,

	REQUEST_JOIN_GROUP_ = 1235,

	REPLY_JOIN_GROUP_ = 1237,

	PUSH_USER_CHAT_MESSAGE_ = 1239,

	PUSH_GROUP_CHAT_MESSAGE_ = 1241,

	PUSH_USER_DEVICEBIND_MESSAGE_ = 1245,

	PUSH_USER_DEVICEUPDATE_MESSAGE_ = 1247,

	PUSH_USER_REMOVEBIND_MESSAGE_ = 1249,

	PUSH_MESSAGETYPE_TO_GROUPMEMBER_ = 1265,

	NOTIFY_OFFLINE_ = 3102,

	PTCP_ERROR = 3103,

	PTCP_CLOSED = 3104,
};

#endif // user_interface_defines_h__
