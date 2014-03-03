#include "net_interface.h"
#include "net_interface_impl.h"

INetChannelInterface*  INetChannelInterface::instance_ = NULL;

INetChannelInterface* INetChannelInterface::GetImplInstance()
{
	if(instance_ == NULL)
	{
		instance_ = new CClientNetInterfaceImpl;
	}

	return instance_;
}

void INetChannelInterface::DestroyImplInstance(const INetChannelInterface* pImpl)
{
	if(pImpl != NULL)
	{
		delete pImpl;
		pImpl = NULL;
	}
}
