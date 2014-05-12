#include "source/transfer/ios_transfer/device_describe.h"

#include <algorithm>

#include "third_party/chromium/base/string_piece.h"
#include "third_party/chromium/base/sys_string_conversions.h"
#include "third_party/chromium/base/synchronization/lock.h"

#include "source/transfer/ios_transfer/device_info_provider.h"
#include "source/transfer/ios_transfer/device_manager.h"
#include "source/transfer/ios_transfer/class_provider.h"
#include "source/transfer/ios_transfer/application_describe.h"
#include "source/transfer/ios_transfer/string_list_enumerator.h"

using std::wstring;
using std::vector;
using std::weak_ptr;
using std::for_each;
using std::find_if;
using base::SysUTF8ToWide;
using base::SysWideToUTF8;
using base::Lock;
using base::AutoLock;
using ios_transfer::DeviceID;
using ios_transfer::IApplicationInfo;
using ios_transfer::ITunesCommonFolderType;
using ios_transfer::IStringListEnumerator;

DeviceDescribe::DeviceDescribe()
    : Unknown()
    , lock_(new Lock())
{

}

DeviceDescribe::~DeviceDescribe()
{

}


HRESULT DeviceDescribe::NonDelegateQueryInterface(const GUID& iid, void** v)
{
    if (ios_transfer::IID_DeviceDescribe == iid)
    {
        *v = dynamic_cast<IDeviceDescribe*>(this);
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

HRESULT DeviceDescribe::GetDeviceFreeSpace(DeviceID id, int64* size)
{
    if (!size)
        return E_POINTER;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return E_INVALIDARG;
    {
        AutoLock l(*lock_);
        *size = deviceInfo->storageInfo.FreeSpace;
    }
    return S_OK;
}

HRESULT DeviceDescribe::GetTotalSpace(DeviceID id, int64* size)
{
    if (!size)
        return E_POINTER;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return E_INVALIDARG;

    {
        AutoLock l(*lock_);
        *size = deviceInfo->storageInfo.TotalSpace;
    }
    return S_OK;
}

HRESULT DeviceDescribe::GetSerialNumber(DeviceID id, wstring* serialNumber)
{
    if (!serialNumber)
        return E_POINTER;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return E_INVALIDARG;

    {
        AutoLock l(*lock_);
        *serialNumber = SysUTF8ToWide(deviceInfo->productInfo.SerialNumber);
    }
    return S_OK;
}

HRESULT DeviceDescribe::GetUniqueDefineID(DeviceID id, wstring* uniqueDefineID)
{
    if (!uniqueDefineID)
        return E_POINTER;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return E_INVALIDARG;

    {
        AutoLock l(*lock_);
        *uniqueDefineID = SysUTF8ToWide(deviceInfo->productInfo.UniqueDefineID);
    }
    return S_OK;
}

HRESULT DeviceDescribe::GetApplicationCount(DeviceID id, int* count)
{
    if (!count)
        return E_POINTER;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return E_INVALIDARG;

    {
        AutoLock l(*lock_);
        *count = deviceInfo->applicationInfos.size();
    }
    return S_OK;
}

HRESULT DeviceDescribe::GetApplicationIDs(
    DeviceID id, IStringListEnumerator** applicationIDs)
{
    if (*applicationIDs)
        return E_POINTER;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return E_INVALIDARG;
    
    {
        AutoLock l(*lock_);
        StringListEnumerator* enumerator = new StringListEnumerator();
        auto procedure = [enumerator] (const SingleApplicationInfo& app)
        {
            enumerator->AddElement(SysUTF8ToWide(app.ID));
        };
        for_each(deviceInfo->applicationInfos.begin(), 
                 deviceInfo->applicationInfos.end(), procedure);
        enumerator->AddRef();
        *applicationIDs = enumerator;
    }
    return S_OK;
}

HRESULT DeviceDescribe::GetApplicationInfo(DeviceID id, 
                                           const wstring& applicationID, 
                                           IApplicationInfo** applicationInfo)
{
    if (*applicationInfo)
        return E_POINTER;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return ios_transfer::E_DEVICE_NOT_FOUND;

    auto condition = [applicationID] (const SingleApplicationInfo& info) -> bool
    {
        return info.ID == SysWideToUTF8(applicationID);
    };

    {
        AutoLock l(*lock_);
        auto appInfo = find_if(deviceInfo->applicationInfos.begin(), 
            deviceInfo->applicationInfos.end(), condition);
        if (deviceInfo->applicationInfos.end() == appInfo)
            return ios_transfer::E_APPLICATION_NOT_FOUND;

        *applicationInfo = 
            new ApplicationDescribe(weak_ptr<DeviceInfo>(deviceInfo), 
            appInfo->DetailInfo);
    }

    return S_OK;
}

HRESULT DeviceDescribe::GetITunesCommonFolderType(
    DeviceID id, vector<ITunesCommonFolderType>* folderTypes)
{
    if (!folderTypes)
        return E_POINTER;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return E_INVALIDARG;

    return E_NOTIMPL;
}