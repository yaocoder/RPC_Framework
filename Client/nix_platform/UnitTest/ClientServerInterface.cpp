//============================================================================
// Name        : UserClient.cpp
// Author      : yaocoder
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <assert.h>
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	testing::GTEST_FLAG(filter) = "UserClientSDK.reentrant*";

	RUN_ALL_TESTS();

	printf("按任意键结束\n");
	getchar();
	return 0;
}
