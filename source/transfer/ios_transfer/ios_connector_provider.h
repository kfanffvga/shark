#ifndef _IOS_CONNECTOR_PROVIDER_H_
#define _IOS_CONNECTOR_PROVIDER_H_

#include <memory>
#include <string>

#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/memory/singleton.h"

namespace base
{
class Lock;
}

struct lockdownd_service_descriptor;
struct lockdownd_client_private;
struct house_arrest_client_private;
struct mobile_image_mounter_client_private;
struct idevice_private;
struct afc_client_private;

typedef std::unique_ptr<idevice_private, int16 (*)(idevice_private*)> 
    DeviceConnector;

typedef std::unique_ptr<lockdownd_service_descriptor, 
                        int16 (*)(lockdownd_service_descriptor*)> 
    LockdowndServiceDescriptor;

typedef std::unique_ptr<lockdownd_client_private, 
                   int16 (*)(lockdownd_client_private*)> LockdowndClient;

typedef std::unique_ptr<house_arrest_client_private, 
                   int16 (*)(house_arrest_client_private*)> HouseArrestClient;

typedef std::unique_ptr<mobile_image_mounter_client_private, 
                   int16 (*)(mobile_image_mounter_client_private*)>
    ImageMounterClient;

class IOSConnectorProvider
{
public:
    static IOSConnectorProvider* GetInstance();

    ~IOSConnectorProvider();

    DeviceConnector GetDeviceConnector(const std::string& udid);

    LockdowndClient GetLockdowndCliect(idevice_private* device, 
                                       const std::string& label);
    
    LockdowndServiceDescriptor GetlockdowndServiceDescriptor(
        lockdownd_client_private* client, const std::string& serviceName);

    HouseArrestClient GetHouseArrestClient(
        idevice_private* device, lockdownd_service_descriptor* desc, 
        const std::string& applicationID);

    afc_client_private* GetAFCClientByHouseArrsetClient(
        house_arrest_client_private* houseArrset);

private: 
    friend struct DefaultSingletonTraits<IOSConnectorProvider>;

    IOSConnectorProvider();

    std::unique_ptr<base::Lock> lock_;
};

#endif