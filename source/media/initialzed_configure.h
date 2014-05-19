#ifndef _INITIALZED_CONFIGURE_
#define _INITIALZED_CONFIGURE_

#include "third_party/chromium/base/memory/ref_counted.h"

#include "source/media/media_interface.h"
#include "source/media/unknown.h"

class InitialzedConfigure : public Unknown
                          , public media::IInitialzedConfigure
{
public:
    InitialzedConfigure();
    virtual ~InitialzedConfigure();

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override;

    virtual HRESULT __stdcall RegisterMediaInfoProviderNotifition(
        media::IMediaInfoProviderNotify* mediaInfoProviderNotifition) override;

    virtual HRESULT __stdcall RegisterMediaFileConvertProgress(
        media::IMediaFileConvertProgressNotify* transferProgressNotifition)
        override;

    scoped_refptr<media::IMediaInfoProviderNotify> 
        GetMediaInfoProviderNotifition();

    scoped_refptr<media::IMediaFileConvertProgressNotify> 
        GetMediaFileConvertProgressNotifition();

private:
    scoped_refptr<media::IMediaInfoProviderNotify> mediaInfoProviderNotifition_;
    scoped_refptr<media::IMediaFileConvertProgressNotify> 
        progressNotifition_;
};

#endif