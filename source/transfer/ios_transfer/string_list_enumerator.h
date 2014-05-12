#ifndef _STRING_LIST_ENUMERATOR_H_
#define _STRING_LIST_ENUMERATOR_H_

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/unknown.h"
#include "source/transfer/ios_transfer/value_enumerator.h"

class StringListEnumerator : public Unknown
                           , public ios_transfer::IStringListEnumerator
{
public:
    StringListEnumerator();
    virtual ~StringListEnumerator();

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override;

    virtual HRESULT __stdcall BeginEnum() override;
    virtual HRESULT __stdcall Next(wchar_t** value) override;
    virtual HRESULT __stdcall GetCount(int* count) override;

    void AddElement(const std::wstring& element);

private: 
    ValueEnumerator<std::wstring> valueEnumerator_;
};

#endif