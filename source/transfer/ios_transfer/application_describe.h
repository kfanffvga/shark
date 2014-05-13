#ifndef _APPLICATION_DESCRIBE_H_
#define _APPLICATION_DESCRIBE_H_

#include <memory>

#include "third_party/chromium/base/basictypes.h"

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/unknown.h"

struct DeviceInfo;
class ApplicationKeyNamesEnumerator;

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
        ios_transfer::IApplicationKeyNamesEnumerator** names) override;

    virtual HRESULT __stdcall GetApplicationIntValueByName(
        const wchar_t* key, __int64* value) override;

    virtual HRESULT __stdcall GetApplicationFloatValueByName(
        const wchar_t* key, float* value) override;

    virtual HRESULT __stdcall GetApplicationStringValueByName(
        const wchar_t* key, wchar_t** value) override;

    virtual HRESULT __stdcall GetApplicationStringArrayValueByName(
        const wchar_t* key, 
        ios_transfer::IStringListEnumerator** values) override;

    virtual HRESULT __stdcall GetApplicationIcon(char** buffer, 
                                                 int* size) override;

private:
    void FillkeyAndTypeInDesc(ApplicationKeyNamesEnumerator* names,
                              void* node, const std::wstring& currentKey);

    std::weak_ptr<DeviceInfo> deviceInfo_;
    std::string applicationInfo_;
};

#endif