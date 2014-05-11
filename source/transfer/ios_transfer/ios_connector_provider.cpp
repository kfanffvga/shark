#include "source/transfer/ios_transfer/ios_connector_provider.h"

#include <algorithm>

#include "third_party/chromium/base/synchronization/lock.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/libimobiledevice.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/lockdown.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/house_arrest.h"

using base::Lock;
using base::AutoLock;
using std::string;
using std::move;

LockdowndClient IOSConnectorProvider::GetLockdowndCliect(
    idevice_private* device, const string& label)
{
    AutoLock l(*lock_);
    if (!device)
        return move(LockdowndClient(nullptr, lockdownd_client_free));

    lockdownd_client_t c = nullptr;
    auto lockDownResult = 
        lockdownd_client_new_with_handshake(device, &c, label.c_str());

    if (lockDownResult != LOCKDOWN_E_SUCCESS)
        return move(LockdowndClient(nullptr, lockdownd_client_free));

    return move(LockdowndClient(c, lockdownd_client_free));
}

LockdowndServiceDescriptor IOSConnectorProvider::GetlockdowndServiceDescriptor(
        lockdownd_client_private* client, const string& serviceName)
{
    AutoLock l(*lock_);
    if (!client)
        return move(
            LockdowndServiceDescriptor(nullptr, 
                                       lockdownd_service_descriptor_free));

    lockdownd_service_descriptor_t desc = nullptr;
    auto error = lockdownd_start_service(client, serviceName.c_str(), &desc);
    if (error != LOCKDOWN_E_SUCCESS)
        return move(
            LockdowndServiceDescriptor(nullptr, 
                                       lockdownd_service_descriptor_free));

    return move(LockdowndServiceDescriptor(desc, 
                                           lockdownd_service_descriptor_free));
}

HouseArrestClient IOSConnectorProvider::GetHouseArrestClient(
    idevice_private* device, lockdownd_service_descriptor* desc, 
    const string& applicationID)
{
    AutoLock l(*lock_);
    HouseArrestClient client(nullptr, house_arrest_client_free);
    if (applicationID.empty())
        return move(client);

    house_arrest_client_t h = nullptr;
    auto error = house_arrest_client_new(device, desc, &h);
    client.reset(h);

    if ((error != HOUSE_ARREST_E_SUCCESS) || (!h))
        return move(HouseArrestClient(nullptr, house_arrest_client_free));

    // 与相应的应用连起来
    error = 
        house_arrest_send_command(h, "VendDocuments", applicationID.c_str());
    if (error != HOUSE_ARREST_E_SUCCESS)
        return move(HouseArrestClient(nullptr, house_arrest_client_free));

    plist_t dict = nullptr;
    error = house_arrest_get_result(h, &dict);
    if ((error != HOUSE_ARREST_E_SUCCESS) || (!dict))
        return move(HouseArrestClient(nullptr, house_arrest_client_free));

    plist_t node = plist_dict_get_item(dict, "Error");
    if (node)
        return move(HouseArrestClient(nullptr, house_arrest_client_free));

    return move(client);
}

afc_client_private* IOSConnectorProvider::GetAFCClientByHouseArrsetClient(
    house_arrest_client_private* houseArrset)
{
    // TODO: 不知道是否afc_client_t的值是否需要释放
    AutoLock l(*lock_);
    afc_client_t afcc = nullptr;
    auto error = 
        afc_client_new_from_house_arrest_client(houseArrset, &afcc);
    if (error != AFC_E_SUCCESS)
        return nullptr;

    return afcc;
}

DeviceConnector IOSConnectorProvider::GetDeviceConnector(const string& udid)
{
    AutoLock l(*lock_);
    DeviceConnector device(nullptr, idevice_free);
    idevice_t d = nullptr;
    if (idevice_new(&d, udid.c_str()) != IDEVICE_E_SUCCESS)
        return move(device);
    
    device.reset(d);
    return move(device);
}

IOSConnectorProvider* IOSConnectorProvider::GetInstance()
{
    return Singleton<IOSConnectorProvider>::get();    
}

IOSConnectorProvider::~IOSConnectorProvider()
{

}

IOSConnectorProvider::IOSConnectorProvider()
    : lock_(new Lock())
{

}
