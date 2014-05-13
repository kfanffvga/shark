#ifndef __APPLICATION_KEY_NAMES_ENUMERATOR_H_
#define __APPLICATION_KEY_NAMES_ENUMERATOR_H_

#include <utility>

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/unknown.h"
#include "source/transfer/ios_transfer/value_enumerator.h"

class ApplicationKeyNamesEnumerator 
    : public Unknown
    , public ios_transfer::IApplicationKeyNamesEnumerator
{
public:
    typedef std::pair<std::wstring, ios_transfer::ApplicationPropertyType> 
        ApplicationNameDescribe;

    ApplicationKeyNamesEnumerator();
    virtual ~ApplicationKeyNamesEnumerator();

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override;
    virtual HRESULT __stdcall BeginEnum() override;
    virtual HRESULT __stdcall Next(
        wchar_t** key, 
        ios_transfer::ApplicationPropertyType* propertyType) override;

    virtual HRESULT __stdcall GetCount(int* count) override;

    void AddElement(const ApplicationNameDescribe& desc);

private: 
    ValueEnumerator<ApplicationNameDescribe> valueEnumerator_;
};

#endif