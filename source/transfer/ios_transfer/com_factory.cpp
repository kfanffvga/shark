#include "source/transfer/ios_transfer/com_factory.h"

#include "source/transfer/ios_transfer/class_provider.h"

ComFactory::ComFactory()
    : refCount_(0)
{

}

ComFactory::~ComFactory()
{

}

HRESULT ComFactory::QueryInterface(const GUID& iid, void** v)
{
    if (iid == IID_IUnknown)
        *v = reinterpret_cast<void**>(dynamic_cast<IUnknown*>(this));
    else if (iid == IID_IClassFactory)
        *v = reinterpret_cast<void**>(dynamic_cast<IClassFactory*>(this));
    else
    {
        *v = nullptr;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown*>(*v)->AddRef();
    return S_OK;
}

unsigned long ComFactory::AddRef()
{
    return ++refCount_;
}

unsigned long ComFactory::Release()
{
    --refCount_;
    if (0 == refCount_)
    {
        delete this;
        return 0;
    }

    return refCount_;
}

HRESULT ComFactory::LockServer(int lock)
{
    return S_OK;
}

HRESULT ComFactory::CreateInstance(IUnknown* unknownOuter, const GUID& iid, 
                                   void** v)
{
    return ClassProvider::GetInstance()->QueryInterface(iid, v);
}
