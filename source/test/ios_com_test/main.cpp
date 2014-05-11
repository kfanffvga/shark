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

    while (true)
    {
        if (!deviceNotifyReceiver->GetDeviceIDs().empty())
        {
            DeviceID id = deviceNotifyReceiver->GetDeviceIDs()[0];
            IDeviceDescribe* deviceDesc;
            v->QueryInterface(ios_transfer::IID_DeviceDescribe, 
                              reinterpret_cast<void**>(&deviceDesc));

            vector<wstring> appIDs;
            deviceDesc->GetApplicationIDs(id, &appIDs);
            int count = 0;
            deviceDesc->GetApplicationCount(id, &count);
            deviceDesc->Release();

        }
        Sleep(100);
    }

    v->Release();
    CoUninitialize();
    return 0;
}