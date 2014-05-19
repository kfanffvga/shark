#include "source/media/class_provider.h"

#include "source/media/media_interface.h"

ClassProvider::ClassProvider()
{

}

ClassProvider::~ClassProvider()
{

}

HRESULT ClassProvider::QueryInterface(const GUID& iid, void** v)
{
    if (media::IID_IMediaFileConvertor == iid)
    {
        
    }
    else if (media::IID_IAsyncMediaInfoProvider== iid)
    {
        
    }
    else if (media::IID_IInitialzedConfigure == iid)
    {
        
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
