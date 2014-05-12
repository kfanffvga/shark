#ifndef _DEVICE_DESCRIBE_H_
#define _DEVICE_DESCRIBE_H_

#include <string>

#include "third_party/chromium/base/basictypes.h"

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/unknown.h"

namespace base
{
class Lock;
}

class DeviceDescribe : public Unknown
                     , public ios_transfer::IDeviceDescribe
{
public:
    DeviceDescribe();
    virtual ~DeviceDescribe();

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override;

protected:
    virtual HRESULT __stdcall GetDeviceFreeSpace(ios_transfer::DeviceID id, 
                                                 int64* size) override;
    virtual HRESULT __stdcall GetTotalSpace(ios_transfer::DeviceID id, 
                                            int64* size) override;
    virtual HRESULT __stdcall GetSerialNumber(
        ios_transfer::DeviceID id, std::wstring* serialNumber) override;
    virtual HRESULT __stdcall GetUniqueDefineID(
        ios_transfer::DeviceID id, std::wstring* uniqueDefineID) override;
    virtual HRESULT __stdcall GetApplicationCount(ios_transfer::DeviceID id, 
                                                  int* count) override;
    virtual HRESULT __stdcall GetApplicationIDs(
        ios_transfer::DeviceID id, 
        ios_transfer::IStringListEnumerator** applicationIDs) override;
    virtual HRESULT __stdcall GetApplicationInfo(
        ios_transfer::DeviceID id, const std::wstring& applicationID,
        ios_transfer::IApplicationInfo** applicationInfo) override;
    virtual HRESULT __stdcall GetITunesCommonFolderType(
        ios_transfer::DeviceID id, 
        std::vector<ios_transfer::ITunesCommonFolderType>* folderTypes) 
        override;

private: 
    std::unique_ptr<base::Lock> lock_;
};

#endif