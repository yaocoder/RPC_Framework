#ifndef user_interface_impl_h__
#define user_interface_impl_h__


#include "user_interface.h"
#include "../common/TemplateTimer.h"

class CNetDataLayer;
class CNetInterLayer;
class  CUserInterfaceImpl:public IUserInterface
{
public:

	CUserInterfaceImpl();

	~CUserInterfaceImpl();

	bool InitSDK();

	void UninitSDK();

	int EstablishPersistentChannel();

	void RegisterPushFunc(IPushMessageOpt* pPushMessageOpt);
	void PushMessageOpt(const std::string& push_message);

	int GetResponseByRequestPersistentConnection(const std::string& request, std::string& response);

	int GetResponseByRequestShortConnection(const std::string& request, std::string& response);

	int SendAsynRequest(const int asyn_request_id, const std::string& request);

private:

	
	CNetDataLayer*	pNetDataOpt_;
	CNetInterLayer* pInterLayer_;


};


#endif
