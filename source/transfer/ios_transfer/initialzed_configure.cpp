#include "source/transfer/ios_transfer/initialzed_configure.h"

#include "source/transfer/ios_transfer/class_provider.h"

using ios_transfer::IDeviceNotifition;
using ios_transfer::ITransferProgressNotifition;

InitialzedConfigure::InitialzedConfigure()
    : deviceNotifition_()
    , progressNotifition_()
{

}

InitialzedConfigure::~InitialzedConfigure()
{

}

HRESULT InitialzedConfigure::RegisterDeviceNotifition(
    IDeviceNotifition* deviceNotifition)
{
    if (!deviceNotifition)
        return E_FAIL;

    deviceNotifition_ = deviceNotifition;
    return S_OK;
}

HRESULT InitialzedConfigure::RegisterDeviceTransferProgress(
    ITransferProgressNotifition* transferProgressNotifition)
{
    if (!transferProgressNotifition)
        return E_FAIL;

    progressNotifition_ = transferProgressNotifition;
    return S_OK;
}

HRESULT InitialzedConfigure::NonDelegateQueryInterface(const GUID& iid, 
                                                       void** v)
{
    if (ios_transfer::IID_InitialzedConfigure == iid)
    {
        *v = dynamic_cast<IInitialzedConfigure*>(this);
        AddRef();
        return S_OK;
    }
    else if (IID_IUnknown == iid)
    {
        *v = dynamic_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    return ClassProvider::GetInstance()->QueryInterface(iid, v);
}

scoped_refptr<IDeviceNotifition> InitialzedConfigure::GetDeviceNotifition()
{
    return deviceNotifition_;
}

scoped_refptr<ITransferProgressNotifition> 
    InitialzedConfigure::GetTransferProgressNotifition()
{
    return progressNotifition_;
}
