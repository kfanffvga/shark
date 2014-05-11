#include "source/transfer/ios_transfer/ios_service_names.h"

const char* IOSServiceNames::applicationProxyServiceName_ = 
    "com.apple.mobile.installation_proxy";
const char* IOSServiceNames::afcServiceName_ = "com.apple.afc";
const char* IOSServiceNames::afc2ServiceName_ = "com.apple.afc2";
const char* IOSServiceNames::houseArrestServiceName_ = 
    "com.apple.mobile.house_arrest";
const char* IOSServiceNames::imageMounterServiceName_ = 
    "com.apple.mobile.mobile_image_mounter";

const char* IOSServiceNames::GetApplicationProxyServiceName() 
{
    return applicationProxyServiceName_;
}

const char* IOSServiceNames::GetAFCServiceName() 
{
    return afcServiceName_;
}

const char* IOSServiceNames::GetAFC2ServiceName() 
{
    return afc2ServiceName_;
}

const char* IOSServiceNames::GetHouseArrestServiceName() 
{
    return houseArrestServiceName_;
}

const char* IOSServiceNames::GetImageMounterServiceName() 
{
    return imageMounterServiceName_;
}
