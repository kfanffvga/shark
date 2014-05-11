#include "source/transfer/ios_transfer/class_provider.h"

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/initialzed_configure.h"
#include "source/transfer/ios_transfer/device_describe.h"
#include "source/transfer/ios_transfer/device_manager.h"

using ios_transfer::IDeviceNotifition;
using ios_transfer::ITransferProgressNotifition;

ClassProvider::ClassProvider()
    : initialzedConfigure_()
{

}

ClassProvider::~ClassProvider()
{

}

HRESULT ClassProvider::QueryInterface(const GUID& iid, void** v)
{
    DeviceManager::GetInstance()->Init();
    if (ios_transfer::IID_Transfer == iid)
    {
        *v = nullptr;
    }
    else if (ios_transfer::IID_DeviceDescribe == iid)
    {
        if (!deviceDescribe_)
            deviceDescribe_ = new DeviceDescribe();

        return deviceDescribe_->QueryInterface(iid, v);
    }
    else if (ios_transfer::IID_InitialzedConfigure == iid)
    {
        if (!initialzedConfigure_)
            initialzedConfigure_ = new InitialzedConfigure();

        return initialzedConfigure_->QueryInterface(iid, v);
    }
    else 
    {
        return E_NOTIMPL;
    }

    return S_OK;
}

ClassProvider* ClassProvider::GetInstance()
{
    return Singleton<ClassProvider>::get();
}

scoped_refptr<IDeviceNotifition> ClassProvider::GetDeviceNotifition()
{
    return initialzedConfigure_->GetDeviceNotifition();
}

scoped_refptr<ITransferProgressNotifition> 
    ClassProvider::GetTransferProgressNotifition()
{
    return initialzedConfigure_->GetTransferProgressNotifition();
}
