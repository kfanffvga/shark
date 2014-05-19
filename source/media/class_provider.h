#ifndef _CLASS_PROVIDER_
#define _CLASS_PROVIDER_

#include "third_party/chromium/base/memory/singleton.h"
#include "third_party/chromium/base/memory/ref_counted.h"

namespace media
{
class IMediaInfoProviderNotify;
class IMediaFileConvertProgressNotify;
}

class ClassProvider
{
public:
    static ClassProvider* GetInstance();

    ~ClassProvider();

    HRESULT QueryInterface(const GUID& iid, void** v);

    scoped_refptr<media::IMediaInfoProviderNotify> GetMediaInfoProviderNotify();
    scoped_refptr<media::IMediaFileConvertProgressNotify> 
        GetMediaFileConvertProgressNotify();

private:
    friend struct DefaultSingletonTraits<ClassProvider>;

    ClassProvider();

    DISALLOW_COPY_AND_ASSIGN(ClassProvider);
};

#endif