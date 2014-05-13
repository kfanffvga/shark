#include "source/transfer/ios_transfer/application_describe.h"

#include <functional>
#include <utility>

#include "third_party/chromium/base/string_piece.h"
#include "third_party/chromium/base/sys_string_conversions.h"
#include "third_party/lib_plist/include/plist/plist.h"
#include "third_party/chromium/base/memory/ref_counted.h"

#include "source/transfer/ios_transfer/application_key_names_enumerator.h"

using std::weak_ptr;
using std::string;
using std::unique_ptr;
using std::function;
using std::wstring;
using std::pair;
using std::make_pair;
using base::SysUTF8ToWide;
using ios_transfer::ApplicationPropertyType;
using ios_transfer::IApplicationKeyNamesEnumerator;
using ios_transfer::IStringListEnumerator;

ApplicationDescribe::ApplicationDescribe(weak_ptr<DeviceInfo> deviceInfo,
                                         const string& applicationInfo)
    : Unknown()
    , deviceInfo_(deviceInfo)
    , applicationInfo_()
{

}

ApplicationDescribe::~ApplicationDescribe()
{

}

HRESULT ApplicationDescribe::NonDelegateQueryInterface(const GUID& iid, 
                                                       void** v)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationPropertyNameCollection(
    IApplicationKeyNamesEnumerator** names)
{
    if (*names)
        return E_POINTER;

    if (!deviceInfo_.lock())
        return ios_transfer::E_DEVICE_NOT_FOUND;

    scoped_refptr<ApplicationKeyNamesEnumerator> enumerator = 
        new ApplicationKeyNamesEnumerator();

    auto plistDestroyer = [] (plist_t* p) { plist_free(*p); };
    unique_ptr<plist_t, function<void (plist_t*)>> pl(nullptr, plistDestroyer);
    plist_from_xml(applicationInfo_.c_str(), applicationInfo_.size(), pl.get());
    if (!pl)
        return E_FAIL;

    FillkeyAndTypeInDesc(enumerator.get(), *pl, L"");
    *names = dynamic_cast<IApplicationKeyNamesEnumerator*>(enumerator.get());
    (*names)->AddRef();
    return S_OK;
}

HRESULT ApplicationDescribe::GetApplicationIntValueByName(const wchar_t* key, 
                                                          __int64* value)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationFloatValueByName(const wchar_t* key, 
                                                            float* value)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationStringValueByName(const wchar_t* key, 
                                                             wchar_t** value)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationStringArrayValueByName(
    const wchar_t* key, IStringListEnumerator** values)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationIcon(char** buffer, int* size)
{
    return E_NOTIMPL;
}

void ApplicationDescribe::FillkeyAndTypeInDesc(
    ApplicationKeyNamesEnumerator* names, plist_t node, 
    const wstring& currentKey)
{
    // 由于此处的plist是app的，因此第一个肯定是dict，
    plist_dict_iter it = nullptr;
    plist_dict_new_iter(node, &it);
    auto plistDestroyer = [] (plist_t* p) { plist_free(*p); };
    unique_ptr<plist_t, function<void (plist_t*)>> val(nullptr, 
                                                       plistDestroyer);
    do {
        char* k = nullptr;
        plist_dict_next_item(node, it, &k, val.get());
        wstring key = SysUTF8ToWide(string(k));
        free(k);

        if (!currentKey.empty())
            key = currentKey + L"->" + key;

        auto type = plist_get_node_type(*val);
        switch (type)
        {
        case PLIST_BOOLEAN:
            names->AddElement(make_pair(key, ios_transfer::BOOLEAN_TYPE));
            break;

        case PLIST_UINT:
            names->AddElement(make_pair(key, ios_transfer::INT64_TYPE));
            break;

        case PLIST_REAL:
            names->AddElement(make_pair(key, ios_transfer::FLOAT_TYPE));
            break;

        case PLIST_STRING: 
        case PLIST_DATE: 
        case PLIST_UID: 
        case PLIST_KEY:
            names->AddElement(make_pair(key, ios_transfer::STRING_TYPE));
            break;

        case PLIST_ARRAY:
            names->AddElement(make_pair(key, ios_transfer::STRING_ARRAY_TYPE));
            break;

        case PLIST_DICT:
            FillkeyAndTypeInDesc(names, *val, key);
            break;

        default: 
            break;
        }
    } while (val.get());

    free(it);
}
