#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#include <map>

#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/memory/singleton.h"

struct DeviceInfo;

class DeviceManager
{
public:
    static DeviceManager* GetInstance();

    ~DeviceManager();
    void Init();
    
    // 注意： 该函数不允许外部类访问
    void ReceiveDeviceInOrOutInfo(bool isIn, const std::string& udid, 
                                  int connectType);

    std::shared_ptr<DeviceInfo> GetDeviceInfoByDeviceID(int deviceID);

private:
    friend struct DefaultSingletonTraits<DeviceManager>; 

    DeviceManager();

    void RemoveDevice(const std::string& udid);
    void AddDevice(const std::string& udid);

    bool GetDeviceInfo(const std::string& udid, DeviceInfo* info);
    
    std::map<int, std::shared_ptr<DeviceInfo>> deviceInfos_;
    bool initialzed_;
    base::Lock deviceInfoLock_;
};

#endif