#include <gtest/gtest.h>

#include "../SdkSrc/net_interface_defines.h"
#include "../SdkSrc/message.h"
#include "../SdkSrc/net_interface.h"
#include "../SdkSrc/net_interface_impl.h"
#include "../SdkSrc/defines.h"
#include <boost/filesystem.hpp>

INetChannelInterface *pINetChannelInterface = CClientNetInterfaceImpl::GetImplInstance();

LoggerPtr g_logger_in;

void ServerPushInfo(const int message_type, const SERVER_PUSH_INFO& serverPushInfo)
{
	LOG4CXX_TRACE(g_logger, "+++++ServerPushInfo messagetype = " << message_type
							<< ", message = " << serverPushInfo.message << "+++++");
}

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
}

void LiveStatus(const int ret)
{
	LOG4CXX_TRACE(g_logger_in, "******LiveStatus ret = " << ret << "******");
}


TEST(UserClientSDK, reentrant_EstablishPersistentChannel)
{
	ASSERT_EQ(pINetChannelInterface->EstablishPersistentChannel(LiveStatus), SUCCESS);
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











