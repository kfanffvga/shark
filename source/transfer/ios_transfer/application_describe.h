#ifndef _APPLICATION_DESCRIBE_H_
#define _APPLICATION_DESCRIBE_H_

#include <memory>

#include "third_party/chromium/base/basictypes.h"

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/unknown.h"

struct DeviceInfo;

class ApplicationDescribe : public Unknown
                          , public ios_transfer::IApplicationInfo
{
public:
    ApplicationDescribe(std::weak_ptr<DeviceInfo> deviceInfo,
                        const std::string& applicationInfo);
    virtual ~ApplicationDescribe();

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override;

protected:
    virtual HRESULT __stdcall GetApplicationPropertyNameCollection(
        ios_transfer::ApplicationNamesDescribe* names) override;

    virtual HRESULT __stdcall GetApplicationIntValueByName(
        const std::wstring& key, __int64* value) override;

    virtual HRESULT __stdcall GetApplicationFloatValueByName(
        const std::wstring& key, float* value) override;

    virtual HRESULT __stdcall GetApplicationStringValueByName(
        const std::wstring& key, std::wstring* value) override;

    virtual HRESULT __stdcall GetApplicationStringArrayValueByName(
        const std::wstring& key, std::vector<int>* values) override;

    virtual HRESULT __stdcall GetApplicationIcon(
        std::shared_ptr<char> buffer, int size, int* realSize) override;

private:
    void FillkeyAndTypeInDesc(ios_transfer::ApplicationNamesDescribe* names,
                              void* node, const std::wstring& currentKey);

    std::weak_ptr<DeviceInfo> deviceInfo_;
    std::string applicationInfo_;
};

#endif