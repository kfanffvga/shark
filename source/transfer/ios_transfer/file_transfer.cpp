#include "source/transfer/ios_transfer/file_transfer.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <utility>

#include "third_party/chromium/base/threading/thread.h"
#include "third_party/chromium/base/file_path.h"
#include "third_party/chromium/base/file_util.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/sys_string_conversions.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/afc.h"

#include "source/transfer/ios_transfer/class_provider.h"
#include "source/transfer/ios_transfer/device_manager.h"
#include "source/transfer/ios_transfer/device_info_provider.h"
#include "source/transfer/ios_transfer/ios_connector_provider.h"
#include "source/transfer/ios_transfer/ios_service_names.h"

using std::string;
using std::wstring;
using std::ifstream;
using std::pair;
using std::make_pair;
using std::unique_ptr;
using std::function;
using base::Thread;
using base::Lock;
using base::AutoLock;
using base::SysWideToUTF8;
using base::Bind;
using file_util::PathExists;
using file_util::GetFileSize;
using ios_transfer::DeviceID;
using ios_transfer::ITunesCommonFolderType;
using ios_transfer::ITransferProgressNotifition;

namespace 
{
const wstring applicationDocumentDirctory = L"/Documents";
}

FileTransfer::FileTransfer()
    : Unknown()
    , thread_(new Thread("transfer"))
    , taskID_(0)
    , tasks_()
    , lock_(new Lock())
{

}

FileTransfer::~FileTransfer()
{
    if (thread_->IsRunning())
        thread_->Stop();
}

HRESULT FileTransfer::NonDelegateQueryInterface(const GUID& iid, void** v)
{
    if (ios_transfer::IID_DeviceDescribe == iid)
    {
        *v = dynamic_cast<ITransfer*>(this);
        AddRef();
        return S_OK;
    }
    else if (IID_IUnknown == iid)
    {
        *v = dynamic_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    return ClassProvider::GetInstance()->QueryInterface(iid, v);
}

HRESULT FileTransfer::TransferFileToITunes(DeviceID id, const wchar_t* filePath, 
                                           ITunesCommonFolderType folderType, 
                                           int* transferTaskID)
{
    return E_NOTIMPL;
}

HRESULT FileTransfer::TransferFileAsIOSRing(DeviceID id, 
                                            const wchar_t* filePath, 
                                            int* transferTaskID)
{
    return E_NOTIMPL;
}

HRESULT FileTransfer::TransferFileToApplication(DeviceID id, 
                                                const wchar_t* filePath, 
                                                const wchar_t* applicationID, 
                                                int* transferTaskID)
{
    FilePath pathValue(filePath);
    if (!PathExists(pathValue))
        return ios_transfer::E_FILE_NOT_FOUND;

    auto deviceInfo = DeviceManager::GetInstance()->GetDeviceInfoByDeviceID(id);
    if (!deviceInfo)
        return ios_transfer::E_DEVICE_NOT_FOUND;

    wstring appid(applicationID);
    auto condition = [appid] (const SingleApplicationInfo& info) -> bool
    {
        return info.ID == SysWideToUTF8(appid);
    };

    string udid;
    {
        AutoLock l(*lock_);
        udid = deviceInfo->productInfo.UniqueDefineID;

        auto appInfo = find_if(deviceInfo->applicationInfos.begin(), 
                               deviceInfo->applicationInfos.end(), condition);
        if (deviceInfo->applicationInfos.end() == appInfo)
            return ios_transfer::E_APPLICATION_NOT_FOUND;

        ++taskID_;
        *transferTaskID = taskID_;
    }
    if (!thread_->IsRunning())
        thread_->Start();

    thread_->message_loop()->PostTask(
        FROM_HERE, 
        Bind(&FileTransfer::TransferFileToApplicationPrivate, this, 
             udid, SysWideToUTF8(appid), pathValue.value(), *transferTaskID));
    return S_OK;
}

HRESULT FileTransfer::CancelTransferTask(int transferTaskID)
{
    AutoLock l(*lock_);
    auto res = find(tasks_.begin(), tasks_.end(), transferTaskID);
    if (tasks_.end() == res)
        return ios_transfer::E_TRANSFER_ID_NOT_FOUND;

    tasks_.erase(res);
    return S_OK;
}

void FileTransfer::TransferFileToApplicationPrivate(const string& udid, 
                                                    const string& applicationID, 
                                                    const wstring& filePath, 
                                                    int transferTaskID)
{
    auto transferProcessNotify = 
        ClassProvider::GetInstance()->GetTransferProgressNotifition();
    if (transferProcessNotify)
        transferProcessNotify->NotifyTransferBeginning(transferTaskID);

    auto flagDestroyer = 
        [transferTaskID, transferProcessNotify] (const bool* needToNotify)
    {
        if (transferProcessNotify && (*needToNotify))
            transferProcessNotify->NotifyTransferError(
                transferTaskID, 
                ITransferProgressNotifition::BUILD_CONNECT_ERROR);
    };
    unique_ptr<bool, function<void (bool*)>> notifyToConnectErrorFlag(
        new bool(true), flagDestroyer);
    afc_client_t afcc = nullptr;
    auto iosConnectorProvider = IOSConnectorProvider::GetInstance();
    auto device = iosConnectorProvider->GetDeviceConnector(udid);
    if (!device)
        return;

    auto client = iosConnectorProvider->GetLockdowndCliect(
        device.get(), "transfer_to_application");
    if (!client)
        return;

    auto desc = iosConnectorProvider->GetlockdowndServiceDescriptor(
        client.get(), IOSServiceNames::GetHouseArrestServiceName());
    if (!desc)
        return;

    auto houseArrest = iosConnectorProvider->GetHouseArrestClient(
        device.get(), desc.get(), applicationID);
    if (!houseArrest)
        return;

    afcc = iosConnectorProvider->GetAFCClientByHouseArrsetClient(
        houseArrest.get());

    if (afcc)
        *notifyToConnectErrorFlag = false;
    
    FilePath srcPath(filePath);
    FilePath dstPath(applicationDocumentDirctory);
    dstPath = dstPath.Append(srcPath.BaseName());
    auto success = TransferFileToIOSDevice(afcc, srcPath.value(), 
                                           dstPath.value(), transferTaskID);
    if (transferProcessNotify)
        transferProcessNotify->NotifyTransferFinished(transferTaskID, success);    
}

bool FileTransfer::TransferFileToIOSDevice(afc_client_private* afcc,
                                           const wstring& srcPath, 
                                           const wstring& dstPath, 
                                           int transferTaskID)
{
    if (!afcc)
        return false;

    uint64 fileHandle = 0;
    ITransferProgressNotifition::ErrorCode error = 
        ITransferProgressNotifition::UNKNOWN_ERROR;
    bool success = false;
    do {
         auto afcError = afc_file_open(afcc, 
                                       SysWideToUTF8(dstPath.c_str()).c_str(), 
                                       AFC_FOPEN_APPEND, &fileHandle);
        if (afcError != AFC_E_SUCCESS)
        {
            error = ITransferProgressNotifition::DIRECTORY_NOT_FOUND;
            break;
        }

        int64 fileSize = 0;
        if (!GetFileSize(FilePath(srcPath), &fileSize))
            break;

        ifstream stream(srcPath.c_str(), std::ios_base::binary);
        const int bufferSize = 1024* 1024;
        // 存放数据大小，已消耗多少
        pair<unique_ptr<char>, pair<int, int>> buffer(
            unique_ptr<char>(new char[bufferSize]), make_pair(0, 0));
        int64 sendedTotalSize = 0;
        do {
            // 从文件中读取数据到缓存
            if (buffer.second.first == buffer.second.second)
            {
                int restSize = static_cast<int>(
                    fileSize - static_cast<int64>(stream.tellg()));
                int readSize = std::min(bufferSize, restSize);
                stream.read(buffer.first.get(), readSize);
                buffer.second.first = readSize;
                buffer.second.second = 0;
            }

            if (0 == buffer.second.first)
            {
                error = ITransferProgressNotifition::READ_LOCAL_FILE_ERROR;
                break;
            }
            // 从缓存中把数据写入ios
            char* p = buffer.first.get() + buffer.second.second;
            int writeSize = 
                std::min(buffer.second.first - buffer.second.second, 65536);
            int realWriteSize = 0;
            afcError = 
                afc_file_write(afcc, fileHandle, p, writeSize, 
                               reinterpret_cast<uint32*>(&realWriteSize));
            if (afcError != AFC_E_SUCCESS)
            {
                error = ITransferProgressNotifition::WRITE_REMOTE_FILE_ERROR;
                break;
            }
            buffer.second.second += realWriteSize;
            sendedTotalSize += realWriteSize;
            auto transferProcessNotify = 
                ClassProvider::GetInstance()->GetTransferProgressNotifition();
            if (transferProcessNotify)
                transferProcessNotify->NotifyTransferProgress(
                    transferTaskID, 
                    static_cast<int>(sendedTotalSize / fileSize * 100));

        } while (sendedTotalSize < fileSize);
        success = true;
    } while (false);
    if (!success)
    {
        auto transferProcessNotify = 
            ClassProvider::GetInstance()->GetTransferProgressNotifition();
        if (transferProcessNotify)
            transferProcessNotify->NotifyTransferError(transferTaskID, error);
    }

    return success;
}
