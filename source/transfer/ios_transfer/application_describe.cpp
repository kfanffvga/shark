#include "source/transfer/ios_transfer/application_describe.h"

#include <functional>
#include <utility>

#include "third_party/chromium/base/string_piece.h"
#include "third_party/chromium/base/sys_string_conversions.h"
#include "third_party/lib_plist/include/plist/plist.h"

using std::weak_ptr;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::function;
using std::vector;
using std::wstring;
using std::pair;
using std::make_pair;
using base::SysUTF8ToWide;
using ios_transfer::ApplicationNamesDescribe;
using ios_transfer::ApplicationPropertyType;

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
    ApplicationNamesDescribe* names)
{
    if (!deviceInfo_.lock())
        return ios_transfer::E_DEVICE_NOT_FOUND;

    auto plistDestroyer = [] (plist_t* p) { plist_free(*p); };
    unique_ptr<plist_t, function<void (plist_t*)>> pl(nullptr, plistDestroyer);
    plist_from_xml(applicationInfo_.c_str(), applicationInfo_.size(), pl.get());
    if (!pl)
        return E_FAIL;

    FillkeyAndTypeInDesc(names, *pl, L"");
    return S_OK;
}

HRESULT ApplicationDescribe::GetApplicationIntValueByName(const wstring& key, 
                                                          __int64* value)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationFloatValueByName(const wstring& key, 
                                                            float* value)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationStringValueByName(const wstring& key, 
                                                             wstring* value)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationStringArrayValueByName(
    const wstring& key, vector<int>* values)
{
    return E_NOTIMPL;
}

HRESULT ApplicationDescribe::GetApplicationIcon(shared_ptr<char> buffer, 
                                                int size, int* realSize)
{
    return E_NOTIMPL;
}

void ApplicationDescribe::FillkeyAndTypeInDesc(
    ApplicationNamesDescribe* names, plist_t node, const wstring& currentKey)
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
            names->push_back(make_pair(key, ios_transfer::BOOLEAN_TYPE));
            break;

        case PLIST_UINT:
            names->push_back(make_pair(key, ios_transfer::INT64_TYPE));
            break;

        case PLIST_REAL:
            names->push_back(make_pair(key, ios_transfer::FLOAT_TYPE));
            break;

        case PLIST_STRING: 
        case PLIST_DATE: 
        case PLIST_UID: 
        case PLIST_KEY:
            names->push_back(make_pair(key, ios_transfer::STRING_TYPE));
            break;

        case PLIST_ARRAY:
            names->push_back(make_pair(key, ios_transfer::STRING_ARRAY_TYPE));
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
