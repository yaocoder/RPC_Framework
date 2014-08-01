
#include "gtest/gtest.h"
#include "../../src/user_net_sdk/user_interface_defines.h"
#include "../../src/user_net_sdk/message.h"
#include "../../src/user_net_sdk/user_interface.h"
#include "../../src/common/EncodingConverter.h"
#include "../../src/common/defines.h"

IUserInterface *userClientSDK = IUserInterface::GetInstance();

TEST(UserClientSDK, 1reentrant_InitSDK)
{

	ASSERT_TRUE(userClientSDK->InitSDK());
}










