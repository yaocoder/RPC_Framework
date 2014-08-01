#include "user_interface_impl.h"

IUserInterface* IUserInterface::instance_ = NULL;

IUserInterface* IUserInterface::GetInstance()
{

	instance_ = new CUserInterfaceImpl;
	return instance_;	
}

void IUserInterface::DestroyImplInstance(const IUserInterface* pImpl)
{
	if(pImpl != NULL)
	{
		delete pImpl;
		pImpl = NULL;
	}
}


