#include "net_inter_layer.h"
#include "net_data_layer.h"
#include "net_core_layer.h"
#include "message.h"
#include "../common/defines.h"
#include "../common/config_file.h"
#include "../common/path_utils.h"
#include "../common/utils.h"
#include "../common/init_configure.h"
#define LOG_CON_FILE        "\\config\\log.conf"
#define CONF_FILE        	"\\config\\config.ini"


CNetInterLayer::CNetInterLayer(void)
{
	message_id_ = 0;
	pInitConfig_= new CInitConfig;
	pNetCore_	= new CNetCoreLayer;
}

CNetInterLayer::~CNetInterLayer(void)
{
	utils::SafeDelete(pNetCore_);
	utils::SafeDelete(pInitConfig_);
}

bool CNetInterLayer::Init(CUserInterfaceImpl* pUserInterfaceImpl)
{
	std::string log_conf_fullpath, conf_fullpath;

	/** 初始化配置文件 */
#ifdef NEED_CONF_PATH
	
	std::string current_path = path_utils::GetModuleDirectory(utils::GetCurrentModule());

	log_conf_fullpath = current_path + std::string(LOG_CON_FILE);
	pInitConfig_->InitLog4cxx("user_interface", true, log_conf_fullpath);

	conf_fullpath = current_path + std::string(CONF_FILE);
	if(!pInitConfig_->LoadConfiguration(conf_fullpath))
	{
		LOG4CXX_ERROR(g_logger, "CNetInterLayer::Init:LoadConfiguration error. conf_fullpath = " << conf_fullpath);
		return false;
	}

	LOG4CXX_TRACE(g_logger, "current_path = " << current_path);
#else
	pInitConfig_->InitLog4cxx("user_interface", false, log_conf_fullpath);

	conf_fullpath = std::string(".") + std::string(CONF_FILE);
	if(!pInitConfig_->LoadConfiguration(conf_fullpath))
	{
		LOG4CXX_ERROR(g_logger, "CNetInterLayer::Init:LoadConfiguration error. conf_fullpath = " << conf_fullpath);
		return false;
	}
#endif

	pUserInterfaceImpl_ = pUserInterfaceImpl;

	/** 初始化网络库，传入“接收数据回调函数”以及“业务层当前指针”，用于在网络层接到数据后传给业务层 */
	if(!pNetCore_->InitNetCore(this))
	{
		LOG4CXX_ERROR(g_logger, " CNetInterLayer::Init:InitNetCore failed.");
		return false;
	}

	LOG4CXX_INFO(g_logger, " CNetInterLayer::Init:InitNetCore success.");

	/** 开启libevent处理线程 */
	DWORD threadId;
	HANDLE hThrd =  CreateThread(NULL,  0 , ThreadFunc , this ,  0 ,  &threadId);
	if (INVALID_HANDLE_VALUE == hThrd)
	{
		LOG4CXX_ERROR(g_logger, " CNetInterLayer::Init:Thread:netCore:Run() failed. errorcode = " << GetLastError());
		return false;
	}
	::CloseHandle(hThrd);

	LOG4CXX_INFO(g_logger, " CNetInterLayer::Init:Thread:netCore:Run() success.");

	return true;
}

DWORD WINAPI CNetInterLayer::ThreadFunc(LPVOID param)
{
	CNetInterLayer* pThis = static_cast<CNetInterLayer*>(param);
	pThis->pNetCore_->Run();
	return 0;
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

void CNetInterLayer::ReciveData(const std::string& response, const int connection_type)
{

	/** 恢复转义处理*/
	std::string recover_string = utils::ReplaceString(response, "\\\\r\\\\n", "\\r\\n");


#ifdef PUSH_LOGIC
	/** 判断是否为推送消息 */
	if (PERSIST_CONNECTION == connection_type)
	{
		/** 检测长连接异常关闭*/
		if (recover_string.compare(std::string(STR_PTCP_HAS_CLOSED)) == 0)
		{
			return;
		}

		/** 检测到长连接错误 */
		if (recover_string.compare(std::string(STR_PTCP_HAS_ERROR)) == 0)
		{
			return;
		}
	}
#endif

	/** 解析消息ID */
	int message_id = 0;
	if (!pNetDataOpt_->JsonParseMessageId(recover_string, message_id))
	{
		LOG4CXX_WARN(g_logger, "CNetInterLayer::ReciveData invalid, message = " << recover_string);
		return;
	}

	/** 根据传回的请求消息id找到相应的请求阻塞事件，激活事件读取response */
	HANDLE h_event = FindEventByMessageIdAndSetResponse(message_id, recover_string);
	if (h_event)
	{
		::SetEvent(h_event);
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CNetInterLayer::ReciveData other message or timeout message. message_id = "  
					 << message_id << ", response = " << recover_string << ", connection_type = " << connection_type);
	}

}

HANDLE CNetInterLayer::FindEventByMessageIdAndSetResponse( const int message_id, const std::string& response)
{
	NET_MSG newNetMsg;
	newNetMsg.h_event = NULL;
	newNetMsg.response= response;
	NET_MSG oldNetMsg;
	if(map_message_.findAndSet(message_id, newNetMsg, oldNetMsg))
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


int CNetInterLayer::GetResponseByRequest(const int message_id, const int tcp_connect_flag, const std::string& resquest, std::string& response )
{
	/** 阻塞在服务端回复处，创建一个请求id以便对应匹配的回复数据 */
	int ret = SUCCESS;

	HANDLE hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	ret = GetLastError();
	if (ret == ERROR_ALREADY_EXISTS)
	{
		LOG4CXX_WARN(g_logger, "CNetInterLayer::GetResponseByRequest:CreateEvent handle already exist.");
		return ret;
	}

	if(NULL == hEvent)
	{
		LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest:CreateEvent failed. errorcode = " << ret);
		return ret;
	}

	NET_MSG net_msg;
	net_msg.h_event = hEvent;
	net_msg.response = "";

	map_message_.insert(message_id, net_msg);

	if (SHORT_CONNECTION == tcp_connect_flag)
	{
		ret = pNetCore_->AddShortConnectionResquest(resquest);
		if (SUCCESS != ret)
		{
			LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest:AddShortConnectionResquest failed. message_id = " << message_id);
			ClearMapByMessageId(message_id);
			CloseHandle(hEvent);
			return ret;
		}
	}

	if (PERSIST_CONNECTION == tcp_connect_flag)
	{
		ret = pNetCore_->AddPersistConnectionRequest(resquest);
		if (SUCCESS != ret)
		{
			LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest:AddPersistConnectionRequest failed. message_id = " << message_id);
			ClearMapByMessageId(message_id);
			CloseHandle(hEvent);
			return ret;
		}
	}

	int timeout = utils::G<ConfigFile>().read<int>("request.timeout.ms", 10000);
	DWORD dw = ::WaitForSingleObject(hEvent, timeout);
	switch(dw)
	{
	case WAIT_OBJECT_0:
		/** 回应 */
		response = FindResponseByMessageId(message_id);
		if (response.empty())
		{
			LOG4CXX_WARN(g_logger, "CNetInterLayer::GetResponseByRequest:FindResponseByMessageId empty. message_id = " << message_id);
		}
		break;
	case WAIT_TIMEOUT:
		LOG4CXX_WARN(g_logger, "CNetInterLayer::GetResponseByRequest TIMEOUT."<< ", message_id = " << message_id);
		ret  = TIMEOUT;
		break;
	case WAIT_FAILED:
		LOG4CXX_ERROR(g_logger, "CNetInterLayer::GetResponseByRequest error. errorcode = " << GetLastError() << ", message_id = " << message_id);
		ret  = C_OTHER_ERROR;
		break;
	}

	ClearMapByMessageId(message_id);
	CloseHandle(hEvent);

	return ret;
}

int CNetInterLayer::ClosePersistConnection()
{
	return pNetCore_->ClosePersistConnection();
}

