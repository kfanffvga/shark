#ifndef _CLASS_PROVIDER_
#define _CLASS_PROVIDER_

#include "third_party/chromium/base/memory/singleton.h"
#include "third_party/chromium/base/memory/ref_counted.h"

class InitialzedConfigure;
class DeviceDescribe;

namespace ios_transfer
{
class IDeviceNotifition;
class ITransferProgressNotifition;
}

class ClassProvider
{
public:
    static ClassProvider* GetInstance();

    ~ClassProvider();
    HRESULT QueryInterface(const GUID& iid, void** v);

    scoped_refptr<ios_transfer::IDeviceNotifition> GetDeviceNotifition();
    scoped_refptr<ios_transfer::ITransferProgressNotifition> 
        GetTransferProgressNotifition();

private:
    friend struct DefaultSingletonTraits<ClassProvider>;

    ClassProvider();

    scoped_refptr<InitialzedConfigure> initialzedConfigure_;
    scoped_refptr<DeviceDescribe> deviceDescribe_;

    DISALLOW_COPY_AND_ASSIGN(ClassProvider);
};

#endif