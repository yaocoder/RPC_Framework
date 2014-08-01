
#include "gtest/gtest.h"


/* UserClientSDK.*-UserClientSDK.GetUserDevices */
int main(int argc, char* argv[])
{
	
	::testing::InitGoogleTest(&argc, argv);
	//testing::GTEST_FLAG(repeat) = 5;
	testing::GTEST_FLAG(filter) = "UserClientSDK.1reentrant*";

	RUN_ALL_TESTS();

	system("pause");
}
