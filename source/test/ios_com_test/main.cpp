#include <vector>
#include <algorithm>

#include <Windows.h>
#include <vfwmsgs.h>

#include "third_party/chromium/base/win/scoped_comptr.h"

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/test/ios_com_test/unknown.h"

using std::wstring;
using std::vector;
using std::find_if;
using base::win::ScopedComPtr;
using ios_transfer::IDeviceNotifition;
using ios_transfer::IDeviceDescribe;
using ios_transfer::IInitialzedConfigure;
using ios_transfer::DeviceID;
using ios_transfer::IStringListEnumerator;
using ios_transfer::IApplicationKeyNamesEnumerator;

class DeviceNotifyReceiver : public Unknown
                           , public IDeviceNotifition
{
public:
    DeviceNotifyReceiver() 
        : Unknown()
        , deviceIDs_()
    {

    }
    
    virtual ~DeviceNotifyReceiver()
    {

    }

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override
    {
        if (ios_transfer::IID_IDeviceNotifition == iid)
        {
            *v = dynamic_cast<IDeviceNotifition*>(this);
            AddRef();
            return S_OK;
        }
        else if (IID_IUnknown == iid)
        {
            *v = dynamic_cast<IUnknown*>(this);
            AddRef();
            return S_OK;
        }
        return VFW_E_NO_INTERFACE; 
    }

    virtual HRESULT __stdcall NotifyDeviceArrive(DeviceID id) override
    {
        deviceIDs_.push_back(id);
        return S_OK;
    }

    virtual HRESULT __stdcall NotifyDeviceRemove(DeviceID id) override
    {
        auto condition = [&id] (const DeviceID& did) -> bool
        {
            return did == id;
        };
        auto r = find_if(deviceIDs_.begin(), deviceIDs_.end(), condition);
        if (r != deviceIDs_.end())
        {
            deviceIDs_.erase(r);
            return S_OK;
        }
        return E_FAIL;
    }
    virtual HRESULT __stdcall NotifyDevicePropertyChange(DeviceID id) override
    {
        return E_NOTIMPL;
    };

    vector<DeviceID> GetDeviceIDs()
    {
        return deviceIDs_;
    }
    
private: 
    vector<DeviceID> deviceIDs_;
};

int main(int argc, wchar_t* argv[])
{
    CoInitialize(nullptr);
    IInitialzedConfigure* v = nullptr;
    HRESULT hr = CoCreateInstance(ios_transfer::CLSID_IOSTransfer, nullptr, 
                                  CLSCTX_INPROC_SERVER, 
                                  ios_transfer::IID_InitialzedConfigure, 
                                  reinterpret_cast<void**>(&v));
    if (FAILED(hr))
        return 0;

    scoped_refptr<DeviceNotifyReceiver> deviceNotifyReceiver(
        new DeviceNotifyReceiver());
    v->RegisterDeviceNotifition(deviceNotifyReceiver.get());

    DeviceID alreadRunID = -1;
    while (true)
    {
        if (!deviceNotifyReceiver->GetDeviceIDs().empty())
        {
            DeviceID id = deviceNotifyReceiver->GetDeviceIDs()[0];
            if (id == alreadRunID)
                continue;
            
            IDeviceDescribe* deviceDesc;
            v->QueryInterface(ios_transfer::IID_DeviceDescribe, 
                              reinterpret_cast<void**>(&deviceDesc));

            IStringListEnumerator* enumerator = nullptr;
            deviceDesc->GetApplicationIDs(id, &enumerator);
            wchar_t* c = nullptr;
            enumerator->BeginEnum();
            while (enumerator->Next(&c) == S_OK)
            {
                ios_transfer::IApplicationInfo* applicationInfo = nullptr;
                deviceDesc->GetApplicationInfo(id, c, &applicationInfo);

                IApplicationKeyNamesEnumerator* applicationKeyNameEnum = nullptr;
                applicationInfo->GetApplicationPropertyNameCollection(
                    &applicationKeyNameEnum);
                applicationKeyNameEnum->BeginEnum();
                wchar_t* keyName = nullptr;
                ios_transfer::ApplicationPropertyType valueType = 
                    ios_transfer::ApplicationPropertyType::UNKNOWN_TYPE;
                while (applicationKeyNameEnum->Next(&keyName, &valueType) == S_OK)
                {
                    if (valueType == ios_transfer::ApplicationPropertyType::INT64_TYPE)
                    {
                        int64 aa = 0;
                        applicationInfo->GetApplicationIntValueByName(keyName, &aa);
                    }
                    else if (valueType == ios_transfer::ApplicationPropertyType::STRING_ARRAY_TYPE)
                    {
                        IStringListEnumerator* stringEnum = nullptr;
                        applicationInfo->GetApplicationStringArrayValueByName(keyName, &stringEnum);
                    }
                    CoTaskMemFree(keyName);
                    keyName = nullptr;
                }

                applicationInfo->Release();
                CoTaskMemFree(c);
                c = nullptr;
            }

            enumerator->Release();
            int count = 0;
            deviceDesc->GetApplicationCount(id, &count);
            deviceDesc->Release();

            ios_transfer::ITransfer* transfer = nullptr;
            hr = v->QueryInterface(ios_transfer::IID_ITransfer, 
                                   reinterpret_cast<void**>(&transfer));

            int transferID = 0;
            transfer->TransferFileToApplication(
                id, L"D:/study/T-REC-H.265-201304-I!!PDF-E.pdf", 
                L"com.adobe.Adobe-Reader", &transferID);

            alreadRunID = id;
        }
        Sleep(100);
    }

    v->Release();
    CoUninitialize();
    return 0;
}