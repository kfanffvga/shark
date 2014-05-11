#ifndef _COM_FACTORY_
#define _COM_FACTORY_

#include <Unknwn.h>

class ComFactory : public IClassFactory
{
public:
    ComFactory();
    virtual ~ComFactory();

    virtual HRESULT __stdcall QueryInterface(const GUID& iid, 
                                             void** v) override;
    virtual unsigned long __stdcall AddRef() override;
    virtual unsigned long __stdcall Release() override;

protected:
    // IClassFactory
    virtual HRESULT __stdcall LockServer(int lock) override;
    virtual HRESULT __stdcall CreateInstance(IUnknown* unknownOuter, 
                                             const GUID& iid, 
                                             void** v) override;

private:
    unsigned long refCount_;
};

#endif