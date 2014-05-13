#include "source/transfer/ios_transfer/application_key_names_enumerator.h"

#include "source/transfer/ios_transfer/util.h"

ApplicationKeyNamesEnumerator::ApplicationKeyNamesEnumerator()
    : valueEnumerator_()
{

}

ApplicationKeyNamesEnumerator::~ApplicationKeyNamesEnumerator()
{

}

HRESULT ApplicationKeyNamesEnumerator::NonDelegateQueryInterface(
    const GUID& iid, void** v)
{
    return E_NOTIMPL;
}

HRESULT ApplicationKeyNamesEnumerator::BeginEnum()
{
    return valueEnumerator_.BeginEnum();
}

HRESULT __stdcall ApplicationKeyNamesEnumerator::Next(
    wchar_t** key, ios_transfer::ApplicationPropertyType* propertyType)
{
    if (*key || propertyType)
        return E_POINTER;

    ApplicationKeyNamesEnumerator::ApplicationNameDescribe element;
    HRESULT hr = valueEnumerator_.GetNextElement(&element);
    WStringToWChar(element.first, key);
    *propertyType = element.second;
    return hr;
}

HRESULT ApplicationKeyNamesEnumerator::GetCount(int* count)
{
    return valueEnumerator_.GetCount(count);
}

void ApplicationKeyNamesEnumerator::AddElement(
    const ApplicationKeyNamesEnumerator::ApplicationNameDescribe& desc)
{
    valueEnumerator_.AddElement(desc);
}
