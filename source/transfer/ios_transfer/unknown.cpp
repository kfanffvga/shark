#include "source/transfer/ios_transfer/unknown.h"

Unknown::Unknown()
    : refCount_(0)
{

}

Unknown::~Unknown()
{

}

unsigned long Unknown::NonDelegatingAddRef()
{
    return ++refCount_;
}

unsigned long Unknown::NonDelegatingRelease()
{
    --refCount_;
    if (refCount_ <= 0)
    {
        delete this;
        return 0;
    }
    return refCount_;
}