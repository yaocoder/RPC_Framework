#ifndef user_interface_h__
#define user_interface_h__

#include "user_interface_defines.h"

#ifndef UNIT_TEST

#ifdef _DLL_MODE
#define DLL_MODE_API __declspec(dllexport)
#else
#define DLL_MODE_API __declspec(dllimport)
#endif

#endif

#ifndef UNIT_TEST
class  DLL_MODE_API IUserInterface
#else
class  IUserInterface
#endif
{
public:
	virtual ~IUserInterface() {}

	static IUserInterface* GetInstance();

	static void DestroyImplInstance(const IUserInterface* pImpl);

	/** ================= ³õÊ¼»¯²Ù×÷ =============**/

	virtual bool InitSDK() = 0;

	virtual void UninitSDK() = 0;


private:

	static IUserInterface* instance_;

};

#endif // user_interface_h__