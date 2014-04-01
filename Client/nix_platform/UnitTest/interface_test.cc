#include <gtest/gtest.h>

#include "../SdkSrc/net_interface_defines.h"
#include "../SdkSrc/message.h"
#include "../SdkSrc/net_interface.h"
#include "../SdkSrc/net_interface_impl.h"
#include "../SdkSrc/defines.h"
#include <boost/filesystem.hpp>

LoggerPtr g_logger_in;

class MyPushMessageOpt : public IPushMessageOpt
{
public:

	void LocalPushMessageOpt(const int message_type, const PUSH_INFO& pushInfo)
	{
		LOG4CXX_TRACE(g_logger_in, "+++++LocalPushMessageOpt messagetype = " << message_type
								<< ", message = " << pushInfo.message << "+++++");
	}

	void LiveStatusCB(const int ret)
	{
		LOG4CXX_TRACE(g_logger_in, "******LiveStatus ret = " << ret << "******");
	}

	void AsynServerResponseOpt(const int asyn_id, const PUSH_INFO& serverPushInfo)
	{

	}

	void ServerPushMessageOpt(const int message_type, const PUSH_INFO& pushInfo)
	{

	}
};

INetChannelInterface *pINetChannelInterface = CClientNetInterfaceImpl::GetImplInstance();



void InitLogger(const std::string& file_path, const std::string& project_name)
{
	PropertyConfigurator::configure(file_path);
	g_logger_in = Logger::getLogger(project_name);
}

bool GetCurrentPath(std::string& current_path);

TEST(UserClientSDK, reentrant_InitSDK)
{
	std::string ip = "192.168.14.234";
	int port = 6111;
	std::string file_path, project_name;
	GetCurrentPath(file_path);
	file_path = file_path + "/conf/log.conf";
	project_name = "NetChannelInterface";
	InitLogger(file_path, project_name);

	ASSERT_TRUE(pINetChannelInterface->Init(ip, port, g_logger_in));

	IPushMessageOpt *pPushMessageOpt = new MyPushMessageOpt;
	pINetChannelInterface->RegisterPushFunc(pPushMessageOpt);
}




TEST(UserClientSDK, reentrant_EstablishPersistentChannel)
{
	ASSERT_EQ(pINetChannelInterface->EstablishPersistentChannel(), SUCCESS);
}














bool GetCurrentPath(std::string& current_path)
{
	try
	{
		boost::filesystem::path path = boost::filesystem::current_path();
		current_path = path.string();
		return true;
	}
	catch (boost::filesystem::filesystem_error & e)
	{
		//cout << "current_path : " << current_path << ", error description :" << e.what() << endl;
		return false;
	}
}











