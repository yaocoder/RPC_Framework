#include "net_inter_layer.h"
#include "defines.h"
#include "message.h"
#include "net_data_layer.h"
#include "net_core_layer.h"
#include "utils.h"
#ifdef APPLE_PLATFORM
#include "ios_sem.h"
#endif

CNetInterLayer::CNetInterLayer(void)
{
	message_id_ = 0;
	request_reponse_timeout_ = 0;
	pUserInterfaceImpl_ = NULL;
	pNetCore_ = new CNetCoreLayer;
	pNetDataOpt_ = new CNetDataLayer;
	b_push_thread_run_ = true;
}

CNetInterLayer::~CNetInterLayer(void)
{
	b_push_thread_run_ = false;
	cond_push_.notify_one();
	utils::SafeDelete(pNetCore_);
	utils::SafeDelete(pNetDataOpt_);
}

bool CNetInterLayer::Init(CClientNetInterfaceImpl* pUserInterfaceImpl, const std::string& ip, const int port)
{
	pUserInterfaceImpl_ = pUserInterfaceImpl;

	/** 初始化网络库，传入“接收数据回调函数”以及“业务层当前指针”，用于在网络层接到数据后传给业务层 */
	if(!pNetCore_->InitNetCore(this, ip, port))
	{
		LOG4CXX_ERROR(g_logger, " CNetInterLayer::Init:InitNetCore failed.");
		return false;
	}
	LOG4CXX_INFO(g_logger, " CNetInterLayer::Init:InitNetCore success.");
	LOGI("CNetInterLayer::Init:InitNetCore success.");
	LOG_I("CNetInterLayer::Init:InitNetCore success. %s", "");

	request_reponse_timeout_ = 10000;
	LOG4CXX_TRACE(g_logger, " CNetInterLayer::Init:request_reponse_timeout.s = " << request_reponse_timeout_/1000);

	int ret = pipe(notify_initsdkDone_fd_);
	if (0 != ret) {
		LOG4CXX_ERROR(g_logger, "CNetInterLayer::Init:pipe errorCode = " << ret);
		LOGE("CNetInterLayer::Init:pipe errorCode = %d .", ret);
		LOG_E("CNetInterLayer::Init:pipe errorCode = %d .", ret);
		return false;
	}

	/** 开启libevent处理线程 */
	pthread_t threadId;
	int hThrd = pthread_create(&threadId, NULL, ThreadFunc, this);
	if (0 != hThrd) {
		LOG4CXX_ERROR(g_logger, " CNetInterLayer::Init:ThreadFunc failed.");
		return false;
	}

	/** 开启推送消息回调函数处理线程 */
	hThrd = pthread_create(&threadId, NULL, ThreadPushFunc, this);
	if (0 != hThrd)
	{
		LOG4CXX_ERROR(g_logger, " CNetInterLayer::Init:ThreadPushFunc failed.");
		return false;
	}

	/* 等待线程也初始化完成 */
	fd_set fdsr;
	FD_ZERO(&fdsr);
	FD_SET(notify_initsdkDone_fd_[0], &fdsr);

	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	ret = select(notify_initsdkDone_fd_[0] + 1, &fdsr, NULL, NULL, &tv);
	if (ret < 0)
	{
		LOG4CXX_ERROR(g_logger, " CNetInterLayer::Init error. errorCode = " << errno);
		LOGE("CNetInterLayer::Init error. errorCode = %d.", errno);
		LOG_E("CNetInterLayer::Init error. errorCode = %d.", errno);
		return false;
	}
	else if (ret == 0)
	{
		LOG4CXX_WARN(g_logger, " CNetInterLayer::Init timeout.");
		LOGW("CNetInterLayer::Init timeout.");
		LOG_W("CNetInterLayer::Init timeout. %s ", "");
		return false;
	}
	else
	{
		LOG4CXX_INFO(g_logger, " CNetInterLayer::Init:Thread:netCore:Run() success.");
		LOGI("CNetInterLayer::Init:Thread:netCore:Run() success.");
		LOG_I("CNetInterLayer::Init:Thread:netCore:Run() success. %s", "");
		return true;
	}

	return true;
}

void* CNetInterLayer::ThreadFunc(void* param)
{
	CNetInterLayer* pThis = static_cast<CNetInterLayer*>(param);
	pThis->pNetCore_->Run();
	return NULL;
}

int CNetInterLayer::GetMessageId()
{
	boost::mutex::scoped_lock oLock(mutex_);
	if (message_id_ >= 65535)
	{
		message_id_ = 0;
	}

	return message_id_ = message_id_ + 1;
}

void* CNetInterLayer::ThreadPushFunc(void* param)
{
	CNetInterLayer* pThis = static_cast<CNetInterLayer*>(param);
	pThis->CallServerPushMessageOpt();
	return NULL;
}

void  CNetInterLayer::CallServerPushMessageOpt()
{
	boost::mutex::scoped_lock oLock(mutex_push_);
	std::string push_message;
	while(b_push_thread_run_)
	{
		cond_push_.wait(mutex_push_);
		while(!list_push_message_.empty())
		{
			list_push_message_.pop_front(push_message);
			pUserInterfaceImpl_->PushMessageOpt(push_message);
		}
	}

	return;
}

void CNetInterLayer::ReciveData(const std::string& response, const int connection_type)
{

	/* 恢复转义处理*/
	std::string recover_string = utils::ReplaceString(response, "\\\\r\\\\n", "\\r\\n");

	int message_id = 0;
	/** 判断是否为推送消息,没有异步过程标识(message_id)的为推送消息 */
	if (PERSIST_CONNECTION == connection_type)
	{
		if (!pNetDataOpt_->JsonParseMessageId(recover_string, message_id))
		{
			list_push_message_.push_back(recover_string);
			cond_push_.notify_one();
			return;
		}
	}

	/* 解析消息ID */
	if (!pNetDataOpt_->JsonParseMessageId(recover_string, message_id))
	{
		LOG4CXX_WARN(g_logger, "CNetInterLayer::ReciveData invalid, message = " << recover_string);
		return;
	}

	/* 根据传回的请求消息id找到相应的请求阻塞事件，激活事件读取response */
	sem_t* h_event = FindEventByMessageIdAndSetResponse(message_id, recover_string);
	if (h_event)
	{
		sem_post(h_event);
		//LOG4CXX_INFO(g_logger, "CNetInterLayer::ReciveData: after call sem_post = " << strerror(errno));
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CNetInterLayer::ReciveData other message or timeout message. message_id = "  
					 << message_id << ", response = " << recover_string << ", connection_type = " << connection_type);
	}

}

sem_t* CNetInterLayer::FindEventByMessageIdAndSetResponse( const int message_id, const std::string& response)
{
	NET_MSG newNetMsg;
	newNetMsg.h_event = NULL;
	newNetMsg.response = response;
	NET_MSG oldNetMsg;
	if (map_message_.findAndSet(message_id, newNetMsg, oldNetMsg))
		return oldNetMsg.h_event;
	else
		return NULL;
}

std::string CNetInterLayer::FindResponseByMessageId(const int message_id)
{
	NET_MSG netMsg;
	map_message_.find(message_id, netMsg);
	return netMsg.response;
}


void CNetInterLayer::ClearMapByMessageId( const int message_id )
{
		map_message_.erase(message_id);
}

void CNetInterLayer::maketimeout(struct timespec *tsp, long milliseconds)
{
	struct timeval now;

	/* get current times */
	gettimeofday(&now, NULL);
	tsp->tv_sec = now.tv_sec;
	tsp->tv_nsec = now.tv_usec * 1000;
	tsp->tv_sec += (milliseconds/1000);
	tsp->tv_nsec += (milliseconds%1000) * 1000000;
}
int CNetInterLayer::GetResponseByRequest(const int message_id, const int tcp_connect_flag, const std::string& request, std::string& response )
{
	/** 阻塞在服务端回复处，创建一个请求id以便对应匹配的回复数据 */
	int ret = SUCCESS;
	NET_MSG net_msg;
	net_msg.response = "";

#ifdef APPLE_PLATFORM
	sem_t *pCond;
	std::string sem_name = utils::t2string(message_id);
	pCond = CreateSemaphore(sem_name.c_str(), 0);
	if(SEM_FAILED == pCond)
	{
		LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest:CreateSemaphore failed. error = "
								<< strerror(errno));
		LOG_E("CNetInterLayer::GetResponseByRequest:CreateSemaphore failed. error = %s .", strerror(errno));
		return OTHER_ERROR;
	}
	net_msg.h_event = pCond;
#else
	sem_t cond;
	ret = sem_init(&cond, 0, 0);
	if(0 != ret)
	{
		LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest:CreateEvent failed. errorcode = " << ret);
		return ret;
	}
	net_msg.h_event = &cond;
#endif

	map_message_.insert(message_id, net_msg);

	if (SHORT_CONNECTION == tcp_connect_flag)
	{
		ret = pNetCore_->AddShortConnectionResquest(request);
		if (SUCCESS != ret)
		{
			LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest:AddShortConnectionResquest failed. message_id = " << message_id);
			ClearMapByMessageId(message_id);
			return ret;
		}
	}

	if (PERSIST_CONNECTION == tcp_connect_flag)
	{
		ret = pNetCore_->AddPersistConnectionRequest(request);
		if (SUCCESS != ret)
		{
			LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest:AddPersistConnectionRequest failed. message_id = " << message_id);
			ClearMapByMessageId(message_id);
			return ret;
		}
	}

	int result = 0;
#ifdef APPLE_PLATFORM
	result = SemWaitIos(pCond, request_reponse_timeout_);
#else
	struct timespec timestruct = {0, 0};
	maketimeout(&timestruct, request_reponse_timeout_);
	result = sem_timedwait(&cond, &timestruct);
#endif
	if(0 == result)
	{
		/** 回应 */
		response = FindResponseByMessageId(message_id);
		if (response.empty())
		{
			LOG4CXX_WARN(g_logger,"CNetInterLayer::GetResponseByRequest:FindResponseByMessageId empty. message_id = " << message_id);
		}
	}
	else if(-1 == result)
	{
		ret = errno;
		if(ret == ETIMEDOUT)
		{
			LOG4CXX_WARN(g_logger, "CNetInterLayer::GetResponseByRequest TIMEOUT. message_id = " << message_id);
			ret  = REQ_RES_TIMEOUT;
		}
		else
		{
			LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest error. erorrcode = " << ret <<
										"error = " << strerror(ret) << ", message_id = " << message_id);
		}
	}

#ifdef APPLE_PLATFORM
	DestroySemaphore(pCond);
	ClearSemaphore(sem_name.c_str());
#else
	sem_destroy(&cond);
#endif
	ClearMapByMessageId(message_id);
	return ret;
}

int CNetInterLayer::SendAysnRequestByPersistConnection(const std::string& request)
{
	return pNetCore_->AddPersistConnectionRequest(request);
}

int CNetInterLayer::ClosePersistConnection()
{
	return pNetCore_->ClosePersistConnection();
}

