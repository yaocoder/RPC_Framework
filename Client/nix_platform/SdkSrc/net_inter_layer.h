#ifndef function_layer_h__
#define function_layer_h__

#include "net_interface_impl.h"
#include "threadSafe_list.h"
#include <semaphore.h>

class CNetCoreLayer;
class CNetDataLayer;

class CNetInterLayer
{
public:
	CNetInterLayer(void);
	~CNetInterLayer(void);

	bool Init(CClientNetInterfaceImpl* pUserInterfaceImpl, const std::string& ip, const int port);

	/**
	 * @brief 处理从网络层接收的数据
	 * @param 业务消息回复
	 * @param 连接类型
	 * @return 
	*/
	void ReciveData(const std::string& response, const int connection_type);

	int	 GetMessageId();

	int	 GetResponseByRequest(const int message_id, const int tcp_connect_flag, const std::string& request, std::string& response);

	int  SendAysnRequestByPersistConnection(const std::string& request);

	int  ClosePersistConnection();

private:

	static void* ThreadFunc(void* param);
	sem_t*		FindEventByMessageIdAndSetResponse(const int message_id, const std::string& response);
	std::string FindResponseByMessageId(const int message_id);
	void		ClearMapByMessageId(const int message_id);
	void		maketimeout(struct timespec *tsp, long milliseconds);


	static void* ThreadPushFunc(void* param);
	void CallServerPushMessageOpt();
	bool b_push_thread_run_;
	boost::mutex mutex_push_;
	boost::condition_variable_any cond_push_;
	CThreadSafeList<std::string> list_push_message_;

private:

	boost::mutex mutex_;

	CNetDataLayer*	pNetDataOpt_;
	CNetCoreLayer*	pNetCore_;

	typedef struct net_msg_
	{
		sem_t* h_event;
		std::string response;
	}NET_MSG;
	CThreadSafeMap<int, NET_MSG> map_message_;
	int  message_id_;
	int request_reponse_timeout_;

	CClientNetInterfaceImpl* pUserInterfaceImpl_;
};
#endif // function_layer_h__
