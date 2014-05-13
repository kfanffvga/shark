#ifndef _ITUNES_FOLDER_TYPE_ENUMERATOR_H_
#define _ITUNES_FOLDER_TYPE_ENUMERATOR_H_

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/unknown.h"
#include "source/transfer/ios_transfer/value_enumerator.h"

class ITunesFolderTypeEnumerator 
    : public Unknown
    , public ios_transfer::IITunesCommonFolderTypeEnumerator
{
public:
    ITunesFolderTypeEnumerator();
    virtual ~ITunesFolderTypeEnumerator();

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override;
    virtual HRESULT __stdcall BeginEnum() override;
    virtual HRESULT __stdcall Next(
        ios_transfer::ITunesCommonFolderType* propertyType) override;

    virtual HRESULT __stdcall GetCount(int* count) override;

    void AddElement(const ios_transfer::ITunesCommonFolderType& propertyType);

private:
    ValueEnumerator<ios_transfer::ITunesCommonFolderType> valueEnumerator_;

};

#endif