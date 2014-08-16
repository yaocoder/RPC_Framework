#ifndef user_interface_impl_h__
#define user_interface_impl_h__


#include "user_interface.h"


class CNetDataLayer;
class CNetInterLayer;
class CImplTimer;
class  CUserInterfaceImpl:public IUserInterface
{
public:

	CUserInterfaceImpl();

	~CUserInterfaceImpl();

	bool InitSDK();

	int EstablishPersistentChannel();

	void RegisterPushFunc(IPushMessageOpt* pPushMessageOpt);
	void PushMessageOpt(const std::string& push_message);

	int GetResponseByRequestPersistentConnection(const std::string& request, std::string& response);

	int GetResponseByRequestShortConnection(const std::string& request, std::string& response);

	int SendAsynRequest(const int asyn_request_id, const std::string& request);

private:

	/**
	* @brief 心跳检测与服务器的连接状态是否正常（对服务器后台起到防止半开连接的作用）
	* @param [in] 心跳状态回调函数
	* @return
	*/
	int HeartBeatDetect();
	/**
	* @brief 获取与服务器的连接状态是否正常
	* @return
	*/
	int GetLiveStatus();
	void StopHeartBeat();
	static void OnTimeGetLiveStatus(void* param);

private:

	CImplTimer*  implTimer_;
	LiveStatusCB liveStatusCb_;

	CNetDataLayer*	pNetDataOpt_;
	CNetInterLayer* pInterLayer_;

	IPushMessageOpt* pPushMessageOpt_;

	bool b_callback_register_;


};


#endif
