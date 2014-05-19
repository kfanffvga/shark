#include "source/transfer/ios_transfer/application_describe.h"

#include <functional>
#include <utility>

#include <boost/algorithm/string/split.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "third_party/chromium/base/string_piece.h"
#include "third_party/chromium/base/sys_string_conversions.h"
#include "third_party/chromium/base/string_number_conversions.h"
#include "third_party/chromium/base/memory/ref_counted.h"

#include "source/transfer/ios_transfer/application_key_names_enumerator.h"
#include "source/transfer/ios_transfer/string_list_enumerator.h"
#include "source/transfer/ios_transfer/util.h"

using std::weak_ptr;
using std::string;
using std::unique_ptr;
using std::wstring;
using std::pair;
using std::make_pair;
using std::vector;
using boost::algorithm::split;
using boost::gregorian::date_duration;
using boost::gregorian::date;
using boost::gregorian::date_from_iso_string;
using base::SysUTF8ToWide;
using base::SysWideToUTF8;
using base::Uint64ToString;
using base::DoubleToString;
using base::StringToDouble;
using base::StringToInt64;
using ios_transfer::ApplicationPropertyType;
using ios_transfer::IApplicationKeyNamesEnumerator;
using ios_transfer::IStringListEnumerator;

ApplicationDescribe::ApplicationDescribe(weak_ptr<DeviceInfo> deviceInfo,
                                         const string& applicationInfo)
    : Unknown()
    , deviceInfo_(deviceInfo)
    , applicationInfo_(applicationInfo)
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

    plist_t pl = nullptr;
    plist_from_xml(applicationInfo_.c_str(), applicationInfo_.size(), &pl);
    if (!pl)
        return E_FAIL;

    unique_ptr<void, void (*)(void*)> root(pl, plist_free);
    FillkeyAndTypeInDesc(enumerator.get(), root.get(), L"");
    *names = dynamic_cast<IApplicationKeyNamesEnumerator*>(enumerator.get());
    (*names)->AddRef();
    return S_OK;
}

HRESULT ApplicationDescribe::GetApplicationIntValueByName(const wchar_t* key, 
                                                          __int64* value)
{
    if (!key || !value)
        return E_POINTER;

    vector<string> values;
    ApplicationPropertyType type = ios_transfer::UNKNOWN_TYPE;
    if (!FindValueByKey(wstring(key), &values, &type))
        return E_INVALIDARG;

    if (type != ios_transfer::INT64_TYPE)
        return E_INVALIDARG;

    if (values.empty())
        return E_FAIL;

    return StringToInt64(values[0], value) ? S_OK : E_FAIL;
}

HRESULT ApplicationDescribe::GetApplicationFloatValueByName(const wchar_t* key, 
                                                            float* value)
{
    if (!key || !value)
        return E_POINTER;

    vector<string> values;
    ApplicationPropertyType type = ios_transfer::UNKNOWN_TYPE;
    if (!FindValueByKey(wstring(key), &values, &type))
        return E_INVALIDARG;

    if (type != ios_transfer::FLOAT_TYPE)
        return E_INVALIDARG;

    if (values.empty())
        return E_FAIL;

    return StringToDouble(values[0], 
                          reinterpret_cast<double*>(value)) ? S_OK : E_FAIL;
}

HRESULT ApplicationDescribe::GetApplicationStringValueByName(const wchar_t* key, 
                                                             wchar_t** value)
{
    if (!key || !value)
        return E_POINTER;

    vector<string> values;
    ApplicationPropertyType type = ios_transfer::UNKNOWN_TYPE;
    if (!FindValueByKey(wstring(key), &values, &type))
        return E_INVALIDARG;

    if (type != ios_transfer::STRING_TYPE)
        return E_INVALIDARG;

    if (values.empty())
        return E_FAIL;

    WStringToWChar(SysUTF8ToWide(values[0]), value);
    return S_OK;
}

HRESULT ApplicationDescribe::GetApplicationStringArrayValueByName(
    const wchar_t* key, IStringListEnumerator** values)
{
    if (!key || *values)
        return E_POINTER;

    vector<string> v;
    ApplicationPropertyType type = ios_transfer::UNKNOWN_TYPE;
    if (!FindValueByKey(wstring(key), &v, &type))
        return E_INVALIDARG;

    if (type != ios_transfer::STRING_ARRAY_TYPE)
        return E_INVALIDARG;

    scoped_refptr<StringListEnumerator> enumerator;
    for (auto i = v.begin(); i != v.end(); ++i)
        enumerator->AddElement(SysUTF8ToWide(*i));

    *values = dynamic_cast<IStringListEnumerator*>(enumerator.get());
    return S_OK;
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
    plist_t val = nullptr;
    do {
        char* k = nullptr;
        plist_dict_next_item(node, it, &k, &val);
        if (!val)
            break;

        if (!k)
            continue;

        wstring key = SysUTF8ToWide(string(k));
        free(k);

        if (!currentKey.empty())
            key = currentKey + L"->" + key;

        auto type = plist_get_node_type(val);
        if (PLIST_DICT == type)
        {
            FillkeyAndTypeInDesc(names, val, key);
        }
        else
        {
            ApplicationPropertyType propertyType = 
                PListTypeToApplicationPropertyType(type);
            if (propertyType != ios_transfer::UNKNOWN_TYPE)
                names->AddElement(make_pair(key, propertyType));
        }
    } while (val);

    free(it);
}

bool ApplicationDescribe::FindValueByKey(wstring key, vector<string>* values, 
                                         ApplicationPropertyType* valueType)
{
    auto strongDeviceInfo = deviceInfo_.lock();
    if (!strongDeviceInfo)
        return false;

    if (!values)
        return false;

    vector<string> keys;
    while (!key.empty())
    {
        wstring item;
        auto pos = key.find(L"->", 0);
        if (pos != wstring::npos)
        {
            item = key.substr(0, pos);
            key = key.substr(pos + 2, key.length() - pos - 2);
        }
        else 
        {
            item = key;
            key = L"";
        }
        keys.push_back(SysWideToUTF8(item));
    }

    if (keys.empty())
        return false;

    plist_t pl = nullptr;
    plist_from_xml(applicationInfo_.c_str(), applicationInfo_.size(), &pl);
    if (!pl)
        return false;

    unique_ptr<void, void (*)(void*)> root(pl, plist_free);
    auto dict = root.get();
    for (auto itemKey = keys.begin(); itemKey != keys.end(); ++itemKey)
    {
        dict = plist_dict_get_item(dict, itemKey->c_str());
        if (!dict)
            return false;

        plist_type plistType = plist_get_node_type(dict);
        if (plistType != PLIST_DICT)
        {
            *valueType = PListTypeToApplicationPropertyType(plistType);
            if (plistType != PLIST_ARRAY)
            {
                string value;
                if (!GetValueByPListNode(dict, &value))
                    return false;

                values->push_back(value);
            }
            else
            {
                return GetArrayValueByPlistNode(dict, values);
            }
        }
    }
    return true;
}

ApplicationPropertyType ApplicationDescribe::PListTypeToApplicationPropertyType(
    plist_type type)
{
    switch (type)
    {
        case PLIST_BOOLEAN:
            return ios_transfer::BOOLEAN_TYPE;

        case PLIST_UINT:
            return ios_transfer::INT64_TYPE;

        case PLIST_REAL:
            return  ios_transfer::FLOAT_TYPE;

        case PLIST_STRING: 
        case PLIST_DATE: 
        case PLIST_UID: 
        case PLIST_KEY:
            return ios_transfer::STRING_TYPE;

        case PLIST_ARRAY:
            return ios_transfer::STRING_ARRAY_TYPE;

        default: 
            return ios_transfer::UNKNOWN_TYPE;
    }
}

bool ApplicationDescribe::GetValueByPListNode(void* node, string* value)
{
    plist_type plistType = plist_get_node_type(node);
    switch(plistType)
    {
        case PLIST_BOOLEAN:
            {
                uint8 val = 0;
                plist_get_bool_val(node, &val);
                *value = (1 == val ? "true" : "false");
            }
            break;

        case PLIST_UINT:
            {
                uint64 val = 0;
                plist_get_uint_val(node, &val);
                *value = Uint64ToString(val);
            }
            break;

        case PLIST_REAL:
            {
                double val = 0;
                plist_get_real_val(node, &val);
                *value = DoubleToString(val);
            }
            break;

        case PLIST_STRING:
            {
                char* val = nullptr;
                plist_get_string_val(node, &val);
                *value = string(val);
                free(val);
            }
            break;

        case PLIST_DATE:
            {
                int32 second = 0;
                int32 microSecond = 0;
                plist_get_date_val(node, &second, &microSecond);
                date d = date_from_iso_string("2001-01-01");
                d = d + date_duration(second / 86400);
                *value = to_iso_extended_string(d);
            }
            break;

        case PLIST_KEY:
            {
                char* val = nullptr;
                plist_get_key_val(node, &val);
                *value = string(val);
                free(val);
            }
            break;

        case PLIST_UID:
            {
                uint64 val = 0;
                plist_get_uid_val(node, &val);
                *value = Uint64ToString(val);
            }
            break;

        default: 
            return false;
    }
    return true;
}

bool ApplicationDescribe::GetArrayValueByPlistNode(
    void* node, vector<string>* values)
{
    if (!node || !values)
        return false;

    auto size = plist_array_get_size(node);
    for (uint32 i = 0; i < size; ++i)
    {
        auto item = plist_array_get_item(node, i);
        string val;
        GetValueByPListNode(item, &val);
        values->push_back(val);
    }
    return true;
}
