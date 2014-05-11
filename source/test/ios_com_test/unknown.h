#ifndef _UNKNOWN_H_
#define _UNKNOWN_H_

#include <Unknwn.h>

#define DECLARE_IUNKNOWN    \
    virtual unsigned long __stdcall AddRef() override   \
    {   \
        return this->NonDelegatingAddRef();   \
    }   \
    virtual unsigned long __stdcall Release() override  \
    {   \
        return this->NonDelegatingRelease();   \
    }   \
    virtual HRESULT __stdcall QueryInterface(const GUID& iid, void** v) \
        override    \
    {   \
        return this->NonDelegateQueryInterface(iid, v);   \
    }   \

class Unknown
{
public:
    Unknown();
    virtual ~Unknown();

    virtual unsigned long __stdcall NonDelegatingAddRef();
    virtual unsigned long __stdcall NonDelegatingRelease();
    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) = 0;

private:
    unsigned long refCount_;
};

#endif