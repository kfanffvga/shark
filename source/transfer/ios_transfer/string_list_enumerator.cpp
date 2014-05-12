#include "source/transfer/ios_transfer/string_list_enumerator.h"

using std::wstring;

StringListEnumerator::StringListEnumerator()
    : valueEnumerator_()
{

}

StringListEnumerator::~StringListEnumerator()
{

}

HRESULT StringListEnumerator::NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v)
{
    return E_NOTIMPL;
}

HRESULT StringListEnumerator::BeginEnum()
{
    return valueEnumerator_.BeginEnum();
}

HRESULT StringListEnumerator::GetCount(int* count)
{
    return valueEnumerator_.GetCount(count);
}

HRESULT StringListEnumerator::Next(wchar_t** value)
{
    if (*value)
        return E_POINTER;

    wstring element;
    HRESULT hr = valueEnumerator_.GetNextElement(&element);
    const uint32 bufferSize = (element.size() + 1) * sizeof(wchar_t);
    *value = reinterpret_cast<wchar_t*>(CoTaskMemAlloc(bufferSize));
    memset(*value, 0, bufferSize);
    memcpy(*value, element.c_str(), element.size() * sizeof(wchar_t));
    return hr;
}

void StringListEnumerator::AddElement(const wstring& element)
{
    valueEnumerator_.AddElement(element);
}
