#ifndef _VALUE_ENUMERATOR_H_
#define _VALUE_ENUMERATOR_H_

#include <list>
#include <WinError.h>

#include "third_party/chromium/base/synchronization/lock.h"

#include "source/transfer/ios_transfer/ios_interface.h"

template<typename T> 
class ValueEnumerator
{
public:
    ValueEnumerator() 
        : values_()
        , iterator_(values_.begin())
        , isValuesChange_(false)
        , lock_()
    {

    }

    ~ValueEnumerator()
    {

    }

    void AddElement(const T& element)
    {
        base::AutoLock l(lock_);
        if (iterator_ != values_.begin())
            isValuesChange_ = true;

        values_.push_back(element);
    }

    HRESULT BeginEnum()
    {
        base::AutoLock l(lock_);
        if (values_.empty())
            return S_FALSE;

        iterator_ = values_.begin();
        isValuesChange_ = false;
        return S_OK;
    }

    HRESULT GetCount(int* count)
    {
        base::AutoLock l(lock_);
        *count = values_.size();
        return S_OK;
    }

    HRESULT GetNextElement(T* element)
    {
        base::AutoLock l(lock_);
        if (isValuesChange_)
            return ios_transfer::E_ENUMERATOR_POINTER_IS_CONTAMINATED;

        if (values_.end() == iterator_)
            return OLE_E_ENUM_NOMORE;

        *element = *iterator_;
        ++iterator_;
        return S_OK;
    }

private:
    std::list<T> values_;
    typename std::list<T>::iterator iterator_;
    bool isValuesChange_;
    base::Lock lock_;
};
#endif