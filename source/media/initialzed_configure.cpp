#include "source/media/initialzed_configure.h"

#include "source/media/class_provider.h"

using media::IMediaInfoProviderNotify;
using media::IMediaFileConvertProgressNotify;
using media::IInitialzedConfigure;

InitialzedConfigure::InitialzedConfigure()
    : mediaInfoProviderNotifition_()
    , progressNotifition_()
{

}

InitialzedConfigure::~InitialzedConfigure()
{

}

HRESULT InitialzedConfigure::RegisterMediaInfoProviderNotifition(
    IMediaInfoProviderNotify* mediaInfoProviderNotifition)
{
    if (!mediaInfoProviderNotifition)
        return E_FAIL;

    mediaInfoProviderNotifition_ = mediaInfoProviderNotifition;
    return S_OK;
}

HRESULT InitialzedConfigure::RegisterMediaFileConvertProgress(
    IMediaFileConvertProgressNotify* transferProgressNotifition)
{
    if (!transferProgressNotifition)
        return E_FAIL;

    progressNotifition_ = transferProgressNotifition;
    return S_OK;
}

HRESULT InitialzedConfigure::NonDelegateQueryInterface(const GUID& iid, 
                                                       void** v)
{
    if (media::IID_IInitialzedConfigure == iid)
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

scoped_refptr<IMediaInfoProviderNotify> 
    InitialzedConfigure::GetMediaInfoProviderNotifition()
{
    return mediaInfoProviderNotifition_;
}

scoped_refptr<IMediaFileConvertProgressNotify> 
    InitialzedConfigure::GetMediaFileConvertProgressNotifition()
{
    return progressNotifition_;
}
