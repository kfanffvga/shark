#ifndef _INITIALZED_CONFIGURE_
#define _INITIALZED_CONFIGURE_

#include "third_party/chromium/base/memory/ref_counted.h"

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/unknown.h"

class InitialzedConfigure : public Unknown
                          , public ios_transfer::IInitialzedConfigure
{
public:
    InitialzedConfigure();
    virtual ~InitialzedConfigure();

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override;

    virtual HRESULT __stdcall RegisterDeviceNotifition(
        ios_transfer::IDeviceNotifition* deviceNotifition) override;

    virtual HRESULT __stdcall RegisterDeviceTransferProgress(
        ios_transfer::ITransferProgressNotifition* transferProgressNotifition) 
        override;

    scoped_refptr<ios_transfer::IDeviceNotifition> GetDeviceNotifition();
    scoped_refptr<ios_transfer::ITransferProgressNotifition> 
        GetTransferProgressNotifition();

private:
    scoped_refptr<ios_transfer::IDeviceNotifition> deviceNotifition_;
    scoped_refptr<ios_transfer::ITransferProgressNotifition> 
        progressNotifition_;
};

#endif