#include "user_interface_impl.h"
#include "message.h"
#include "net_data_layer.h"
#include "net_inter_layer.h"
#include "../common/asio_timer.h"
#include "../common/config_file.h"
#include "../common/defines.h"
#include "../common/mini_dump.h"
#include "../common/utils.h"


CUserInterfaceImpl::CUserInterfaceImpl()
{
	implTimer_ 	 			= NULL;
	pPushMessageOpt_		= NULL;
	liveStatusCb_			= NULL;
	pInterLayer_			= new CNetInterLayer;
	pNetDataOpt_			= new CNetDataLayer;
	b_callback_register_	= false;
}

CUserInterfaceImpl::~CUserInterfaceImpl()
{
	utils::SafeDelete(pInterLayer_);
	utils::SafeDelete(pNetDataOpt_);
}

bool CUserInterfaceImpl::InitSDK()
{

	MiniDumper::Instance().Install();
	if (!pInterLayer_->Init(this))
	{

		return false;
	}

	return true;
}

int CUserInterfaceImpl::EstablishPersistentChannel()
{
	int ret = GetLiveStatus();
	if(SUCCESS != ret)
		return ret;

	/** 开启心跳 */
	ret = HeartBeatDetect();
	if (SUCCESS != ret)
	{
		return OTHER_ERROR;
	}

	return ret;
}

int CUserInterfaceImpl::HeartBeatDetect()
{
	long time_interval = 30;
	implTimer_ = new CImplTimer;
	implTimer_->HeartBeatImpl(OnTimeGetLiveStatus, (void*)this, time_interval);

	return SUCCESS;
}

void CUserInterfaceImpl::OnTimeGetLiveStatus( void* param )
{
	CUserInterfaceImpl *pThis = static_cast<CUserInterfaceImpl*>(param);
	int ret = pThis->GetLiveStatus();
	pThis->pPushMessageOpt_->LiveStatusCB(ret);
}


int CUserInterfaceImpl::GetLiveStatus()
{
	std::string request,response;

	int message_id = pInterLayer_->GetMessageId();
	request = pNetDataOpt_->JsonJoinGetLiveStatus(message_id);


	LOG4CXX_TRACE(g_logger, "--------CUserInterfaceImpl::GetLiveStatus-------- request = " << request);

	int ret = pInterLayer_->GetResponseByRequest(message_id, PERSIST_CONNECTION, request, response);
	if (ret != SUCCESS)
	{
		return ret;
	}

	LOG4CXX_TRACE(g_logger, "********CUserInterfaceImpl::GetLiveStatus******** response = " << response);

	if (pNetDataOpt_->JsonParseResult(response, ret))
	{
		return ret;
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CUserInterfaceImpl::GetFriendsStatus:GetLiveStatus invalid : " << response);
		ret = JSON_INVALID;
		return ret;
	}
}

void CUserInterfaceImpl::StopHeartBeat()
{
	implTimer_->StopHeartBeatImpl();
	utils::SafeDelete(implTimer_);
	pInterLayer_->ClosePersistConnection();
}

void CUserInterfaceImpl::RegisterPushFunc( IPushMessageOpt* pPushMessageOpt )
{
	b_callback_register_ = true;
	pPushMessageOpt_ = pPushMessageOpt;
}


void CUserInterfaceImpl::PushMessageOpt( const std::string& push_message )
{
	LOG4CXX_TRACE(g_logger, "CUserInterfaceImpl::PushMessageOpt : " << push_message);

	if(!b_callback_register_)
		return;

	assert(pPushMessageOpt_ != NULL);

	PUSH_INFO pushInfo;
	pushInfo.message = push_message;

	if (push_message.compare(std::string(STR_PTCP_HAS_ERROR)) == 0)
	{
		pPushMessageOpt_->LocalPushMessageOpt(PTCP_ERROR, pushInfo);
	}

	if (push_message.compare(std::string(STR_PTCP_HAS_CLOSED)) == 0)
	{
		pPushMessageOpt_->LocalPushMessageOpt(PTCP_CLOSED, pushInfo);
	}

	//TODO:推送消息回调,包括异步请求的回复消息
}

int CUserInterfaceImpl::GetResponseByRequestPersistentConnection( const std::string& request, std::string& response )
{
	int message_id = pInterLayer_->GetMessageId();

	/* 为消息加上异步过程标识message_id **/
	std::string new_request, new_response;
	new_request = pNetDataOpt_->JsonJoinGetResponseByRequest(message_id, request);

	LOG4CXX_TRACE(g_logger, "--------CUserInterfaceImpl::GetResponseByRequestPersistentConnection-------- new_request = " << new_request);

	int ret = pInterLayer_->GetResponseByRequest(message_id, PERSIST_CONNECTION, new_request, new_response);
	if (ret != SUCCESS)
	{
		return ret;
	}

	LOG4CXX_TRACE(g_logger, "********CUserInterfaceImpl::GetResponseByRequestPersistentConnection******** new_response = " << new_response);

	if (pNetDataOpt_->JsonParseGetResponseByRequest(new_response, ret, response))
	{
		return ret;
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CUserInterfaceImpl::GetResponseByRequest:JsonParseGetResponseByRequest invalid : " << new_response);
		ret = JSON_INVALID;
		return ret;
	}

	return ret;
}

int CUserInterfaceImpl::GetResponseByRequestShortConnection( const std::string& request, std::string& response )
{
	int message_id = pInterLayer_->GetMessageId();

	/* 为消息加上异步过程标识message_id **/
	std::string new_request, new_response;
	new_request = pNetDataOpt_->JsonJoinGetResponseByRequest(message_id, request);

	LOG4CXX_TRACE(g_logger, "--------CUserInterfaceImpl::GetResponseByRequestShortConnection-------- new_request = " << new_request);

	int ret = pInterLayer_->GetResponseByRequest(message_id, SHORT_CONNECTION, new_request, new_response);
	if (ret != SUCCESS)
	{
		return ret;
	}

	LOG4CXX_TRACE(g_logger, "********CUserInterfaceImpl::GetResponseByRequestShortConnection******** new_response = " << new_response);

	if (pNetDataOpt_->JsonParseGetResponseByRequest(new_response, ret, response))
	{
		return ret;
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CUserInterfaceImpl::GetResponseByRequestShortConnection:JsonParseGetResponseByRequest invalid : " << new_response);
		ret = JSON_INVALID;
		return ret;
	}

	return ret;
}

int CUserInterfaceImpl::SendAsynRequest( const int asyn_request_id, const std::string& request )
{
	std::string new_request = pNetDataOpt_->JsonJoinSendAsynRequest(asyn_request_id, request);

	return pInterLayer_->SendAysnRequestByPersistConnection(new_request);
}

















































































