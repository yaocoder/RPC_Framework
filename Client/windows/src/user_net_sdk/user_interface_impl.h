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

	/** ================= Í¨ÓÃ²Ù×÷ =============**/

	bool InitSDK();

	void UninitSDK();


private:

	
	CNetDataLayer*	pNetDataOpt_;
	CNetInterLayer* pInterLayer_;


	ServerPushCallBack_Info sp_cb_info_;
};


#endif
