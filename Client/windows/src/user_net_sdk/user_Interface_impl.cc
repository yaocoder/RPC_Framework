#include "user_interface_impl.h"
#include "message.h"
#include "net_data_layer.h"
#include "net_inter_layer.h"
#include "../common/config_file.h"
#include "../common/defines.h"
#include "../common/mini_dump.h"
#include "../common/utils.h"


CUserInterfaceImpl::CUserInterfaceImpl()
{

}

CUserInterfaceImpl::~CUserInterfaceImpl()
{
	
}

bool CUserInterfaceImpl::InitSDK()
{
	pInterLayer_	= new CNetInterLayer;
	pNetDataOpt_	= new CNetDataLayer;

	MiniDumper::Instance().Install();
	if (!pInterLayer_->Init(this))
	{

		return false;
	}

	return true;
}

void CUserInterfaceImpl::UninitSDK()
{
	utils::SafeDelete(pInterLayer_);
	utils::SafeDelete(pNetDataOpt_);
}

int CUserInterfaceImpl::EstablishPersistentChannel()
{
	return SUCCESS;
}

void CUserInterfaceImpl::RegisterPushFunc( IPushMessageOpt* pPushMessageOpt )
{

}

void CUserInterfaceImpl::PushMessageOpt( const std::string& push_message )
{

}

int CUserInterfaceImpl::GetResponseByRequestPersistentConnection( const std::string& request, std::string& response )
{
	return SUCCESS;
}

int CUserInterfaceImpl::GetResponseByRequestShortConnection( const std::string& request, std::string& response )
{
	return SUCCESS;
}

int CUserInterfaceImpl::SendAsynRequest( const int asyn_request_id, const std::string& request )
{
	return SUCCESS;
}
















































































