#ifndef _IOS_INTERFACE_H_
#define _IOS_INTERFACE_H_

#include <string>
#include <utility>
#include <vector>

#include <Unknwn.h>

namespace ios_transfer
{
typedef int DeviceID;

enum ApplicationPropertyType
{
    INT64_TYPE = 0,
    FLOAT_TYPE,
    BOOLEAN_TYPE,
    STRING_TYPE,
    STRING_ARRAY_TYPE
};

enum ITunesCommonFolderType
{
    MOVIE_FOLDER = 0,
    MUSIC_FOLDER,
    TELEVISION_FOLDER,
    RING_FOLDER,
    PHOTO_FOLDER,
    UNKNOWN_FOLDER
};

typedef std::vector<std::pair<std::wstring, ApplicationPropertyType>> 
    ApplicationNamesDescribe;

// 此处是自定义错误码
static const HRESULT E_DEVICE_NOT_FOUND = 0x52013001L;
static const HRESULT E_FILE_NOT_FOUND = 0x52013002L;
static const HRESULT E_APPLICATION_NOT_FOUND = 0x52013003L;
static const HRESULT E_TRANSFER_ID_NOT_FOUND = 0x52013004L;
static const HRESULT E_APPLICATION_DOCUMENT_FOLDER_NOT_FOUND = 0x52013005L;

// 此处是接口的guid定义，接口的guid名称一律用IID接口名称或者CLSID接口名称来命名

// {7B5ED7CA-D01B-4A06-8C5D-F61B1F3B3A20}
static const GUID CLSID_IOSTransfer = 
    {0x7b5ed7ca, 0xd01b, 0x4a06, 0x8c, 0x5d, 0xf6, 0x1b, 0x1f, 0x3b, 0x3a, 
     0x20};

// {E4B91FB7-62A7-4BCA-8B1B-5B8410879205}
static const GUID IID_Transfer = 
    {0xe4b91fb7, 0x62a7, 0x4bca, 0x8b, 0x1b, 0x5b, 0x84, 0x10, 0x87, 0x92, 
     0x05};

// {36689151-CB31-45DF-AEAF-B18D67482FEC}
static const GUID IID_DeviceDescribe = 
    {0x36689151, 0xcb31, 0x45df, 0xae, 0xaf, 0xb1, 0x8d, 0x67, 0x48, 0x2f, 
     0xec};

// {F24A765D-E149-4AED-A229-DAACC958043B}
static const GUID IID_InitialzedConfigure = 
    {0xf24a765d, 0xe149, 0x4aed, 0xa2, 0x29, 0xda, 0xac, 0xc9, 0x58, 0x04, 
     0x3b};

// {4882DDA3-6CF0-48E8-A520-1BA4CAFCA899}
static const GUID IID_IDeviceNotifition = 
    {0x4882dda3, 0x6cf0, 0x48e8, 0xa5, 0x20, 0x1b, 0xa4, 0xca, 0xfc, 0xa8, 
     0x99};

// {9A705877-27BC-4646-854C-01F6E1778761}
static const GUID IID_ITransferProgressNotifition = 
    {0x9a705877, 0x27bc, 0x4646, 0x85, 0x4c, 0x01, 0xf6, 0xe1, 0x77, 0x87, 
     0x61};


// 此处是接口的具体定义，其中某些接口要外部程序实现,
// 内部提供的接口，除了单纯为了得到数据的接口之外，全部以异步实现
// 所有的字符返回都要由调用者自行释放

// 字符串列表的枚举器 
class IStringListEnumerator : public IUnknown
{
public:
    virtual HRESULT __stdcall BeginEnum() = 0;
    virtual HRESULT __stdcall Next(wchar_t** value) = 0;
    virtual HRESULT __stdcall GetCount(int* count) = 0;
};

// 应用工具属性名称枚举器
class IApplicationKeyNamesEnumerator : public IUnknown
{
public:
    virtual HRESULT __stdcall BeginEnum() = 0;
    virtual HRESULT __stdcall Next(wchar_t** value, 
                                   ApplicationPropertyType* propertyType) = 0;
    virtual HRESULT __stdcall GetCount(int* count) = 0;
};

// ITunes里拥有的文件夹类型枚举器
class IITunesCommonFolderTypeEnumerator : public IUnknown
{
public:
    virtual HRESULT __stdcall BeginEnum() = 0;
    virtual HRESULT __stdcall Next(ITunesCommonFolderType* folderType) = 0;
    virtual HRESULT __stdcall GetCount(int* count) = 0;
};

// 该接口为外部实现接口，目的是让应用程序收到设备侦听的结果，从而进行操作
class IDeviceNotifition : public IUnknown
{
public:
    virtual HRESULT __stdcall NotifyDeviceArrive(DeviceID id) = 0;
    virtual HRESULT __stdcall NotifyDeviceRemove(DeviceID id) = 0;
    virtual HRESULT __stdcall NotifyDevicePropertyChange(DeviceID id) = 0;
};

// 该接口为外部实现接口，目的是让应用程序收到传送文件的进度
class ITransferProgressNotifition : public IUnknown
{
public:
    enum ErrorCode
    {
        BUILD_CONNECT_ERROR = 0,
        DIRECTORY_NOT_FOUND, 
        READ_LOCAL_FILE_ERROR,
        WRITE_REMOTE_FILE_ERROR,
        UNKNOWN_ERROR
    };
    
    virtual HRESULT __stdcall NotifyTransferBeginning(int transferTaskID) = 0;
    virtual HRESULT __stdcall NotifyTransferProgress(int transferTaskID, 
                                                     int precent) = 0;
    virtual HRESULT __stdcall NotifyTransferFinished(int transferTaskID, 
                                                     bool success) = 0;
    virtual HRESULT __stdcall NotifyTransferError(int transferTaskID, 
                                                  enum ErrorCode code) = 0;
};

// 该接口定义了文件从pc到ios的传输
class ITransfer : public IUnknown
{
public:
    virtual HRESULT __stdcall TransferFileToITunes(
        DeviceID id, const std::wstring& filePath,
        ITunesCommonFolderType folderType, int* transferTaskID) = 0;
    virtual HRESULT __stdcall TransferFileAsIOSRing(
        DeviceID id, const std::wstring& filePath, int* transferTaskID) = 0;
    virtual HRESULT __stdcall TransferFileToApplication(
        DeviceID id, const std::wstring& filePath, 
        const std::wstring& applicationID, int* transferTaskID) = 0;
    virtual HRESULT __stdcall CancelTransferTask(int transferTaskID) = 0;
};

// 该接口定义安装与ios系统的应用的信息获取
class IApplicationInfo : public IUnknown
{
public:
    virtual HRESULT __stdcall GetApplicationPropertyNameCollection(
        ApplicationNamesDescribe* names) = 0;
    virtual HRESULT __stdcall GetApplicationIntValueByName(
        const std::wstring& key, __int64* value) = 0;
    virtual HRESULT __stdcall GetApplicationFloatValueByName(
        const std::wstring& key, float* value) = 0;
    virtual HRESULT __stdcall GetApplicationStringValueByName(
        const std::wstring& key, std::wstring* value) = 0;
    virtual HRESULT __stdcall GetApplicationStringArrayValueByName(
        const std::wstring& key, std::vector<int>* values) = 0;
    virtual HRESULT __stdcall GetApplicationIcon(std::shared_ptr<char> buffer, 
                                                 int size, int* realSize) = 0;
};

// 该接口定义ios系统的信息获取
class IDeviceDescribe : public IUnknown
{
public:
    virtual HRESULT __stdcall GetDeviceFreeSpace(DeviceID id, 
                                                 __int64* size) = 0;
    virtual HRESULT __stdcall GetTotalSpace(DeviceID id, __int64* size) = 0;
    virtual HRESULT __stdcall GetSerialNumber(DeviceID id, 
                                              std::wstring* serialNumber) = 0;
    virtual HRESULT __stdcall GetUniqueDefineID(
        DeviceID id, std::wstring* uniqueDefineID) = 0;
    virtual HRESULT __stdcall GetApplicationCount(DeviceID id, int* count) = 0;
    virtual HRESULT __stdcall GetApplicationIDs(
        DeviceID id, std::vector<std::wstring>* applicationIDs) = 0;
    virtual HRESULT __stdcall GetApplicationInfo(
        DeviceID id, const std::wstring& applicationID,
        IApplicationInfo** applicationInfo) = 0;
    virtual HRESULT __stdcall GetITunesCommonFolderType(
        DeviceID id, std::vector<ITunesCommonFolderType>* folderTypes) = 0;

};

// 该接口定义了让该模块正常工作之前的初始化参数配置
class IInitialzedConfigure : public IUnknown
{
public:
    virtual HRESULT __stdcall RegisterDeviceNotifition(
        IDeviceNotifition* deviceNotifition) = 0;
    virtual HRESULT __stdcall RegisterDeviceTransferProgress(
        ITransferProgressNotifition* transferProgressNotifition) = 0;
};

}

#endif