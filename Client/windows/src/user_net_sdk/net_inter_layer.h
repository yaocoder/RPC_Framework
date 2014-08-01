#ifndef function_layer_h__
#define function_layer_h__

#include "user_interface_impl.h"
#include "../common/threadSafe_list.h"

class CInitConfig;
class CNetCoreLayer;
class CNetDataLayer;

class CNetInterLayer
{
public:
	CNetInterLayer(void);
	~CNetInterLayer(void);

	bool Init(CUserInterfaceImpl* pUserInterfaceImpl);

	/**
	 * @brief 处理从网络层接收的数据
	 * @param 业务消息回复
	 * @param 连接类型
	 * @return 
	*/
	void ReciveData(const std::string& response, const int connection_type);

	int	 GetMessageId();

	int	 GetResponseByRequest(const int message_id, const int tcp_connect_flag, const std::string& resquest, std::string& response);

	int  ClosePersistConnection();

private:

	boost::mutex mutex_;


	static DWORD WINAPI ThreadFunc(LPVOID param);


	HANDLE		FindEventByMessageIdAndSetResponse(const int message_id, const std::string& response);

	std::string FindResponseByMessageId(const int message_id);

	void		ClearMapByMessageId(const int message_id);

	CNetDataLayer*	pNetDataOpt_;
	CNetCoreLayer	*pNetCore_;
	CInitConfig		*pInitConfig_;

	typedef struct net_msg_
	{
		HANDLE h_event;
		std::string response;
	}NET_MSG;
	CThreadSafeMap<int, NET_MSG> map_message_;

	int  message_id_;

	CUserInterfaceImpl* pUserInterfaceImpl_;
};
#endif // function_layer_h__
