#ifndef _IOS_SERVICE_NAMES_H_
#define _IOS_SERVICE_NAMES_H_

class IOSServiceNames
{
public: 
    static const char* GetApplicationProxyServiceName();
    static const char* GetAFCServiceName();
    static const char* GetAFC2ServiceName();
    static const char* GetHouseArrestServiceName();
    static const char* GetImageMounterServiceName();

private:
    static const char* applicationProxyServiceName_;
    static const char* afcServiceName_;
    static const char* afc2ServiceName_;
    static const char* houseArrestServiceName_;
    static const char* imageMounterServiceName_;
};

#endif