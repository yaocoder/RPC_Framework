#include "user_interface_impl.h"
//#include <boost/asio.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
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
















































































