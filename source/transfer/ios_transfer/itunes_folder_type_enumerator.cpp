#include "source/transfer/ios_transfer/itunes_folder_type_enumerator.h"

using ios_transfer::ITunesCommonFolderType;

ITunesFolderTypeEnumerator::ITunesFolderTypeEnumerator()
    : valueEnumerator_()
{

}

ITunesFolderTypeEnumerator::~ITunesFolderTypeEnumerator()
{

}

HRESULT ITunesFolderTypeEnumerator::NonDelegateQueryInterface(
    const GUID& iid, void** v)
{
    return E_NOTIMPL;
}

HRESULT ITunesFolderTypeEnumerator::BeginEnum()
{
    return valueEnumerator_.BeginEnum();
}

HRESULT __stdcall ITunesFolderTypeEnumerator::Next(
    ITunesCommonFolderType* propertyTypee)
{
    if (propertyTypee)
        return E_POINTER;

    return valueEnumerator_.GetNextElement(propertyTypee);
}

HRESULT ITunesFolderTypeEnumerator::GetCount(int* count)
{
    return valueEnumerator_.GetCount(count);
}

void ITunesFolderTypeEnumerator::AddElement(
    const ITunesCommonFolderType& element)
{
    valueEnumerator_.AddElement(element);
}
