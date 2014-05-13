#include "source/transfer/ios_transfer/util.h"

#include <cassert>
#include <Windows.h>

#include "third_party/chromium/base/basictypes.h"

using std::wstring;

void WStringToWChar(const wstring& s, wchar_t** c)
{
    assert(!(*c));

    const uint32 bufferSize = (s.size() + 1) * sizeof(wchar_t);
    *c = reinterpret_cast<wchar_t*>(CoTaskMemAlloc(bufferSize));
    memset(*c, 0, bufferSize);
    memcpy(*c, s.c_str(), s.size() * sizeof(wchar_t));
}
