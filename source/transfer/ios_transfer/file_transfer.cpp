#include "source/transfer/ios_transfer/file_transfer.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <utility>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>

#include "third_party/chromium/base/threading/thread.h"
#include "third_party/chromium/base/file_path.h"
#include "third_party/chromium/base/file_util.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/synchronization/cancellation_flag.h"
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
using std::shared_ptr;
using std::unique_ptr;
using std::function;
using std::find_if;
using boost::replace_all;
using boost::split;
using boost::wformat;
using base::Thread;
using base::Lock;
using base::CancellationFlag;
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
    if (ios_transfer::IID_ITransfer == iid)
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

    auto task = make_pair(*transferTaskID, 
                          shared_ptr<CancellationFlag>(new CancellationFlag()));
    thread_->message_loop()->PostTask(
        FROM_HERE, 
        Bind(&FileTransfer::TransferFileToApplicationPrivate, this, 
             udid, SysWideToUTF8(appid), pathValue.value(), task));

    tasks_.push_back(task);
    return S_OK;
}

HRESULT FileTransfer::CancelTransferTask(int transferTaskID)
{
    bool success = DeleteTaskByTransferTaskID(transferTaskID, true);
    return success ? S_OK : ios_transfer::E_TRANSFER_ID_NOT_FOUND;
}

void FileTransfer::TransferFileToApplicationPrivate(const string& udid, 
                                                    const string& applicationID, 
                                                    const wstring& filePath, 
                                                    const TransferTask& task)
{
    auto transferProcessNotify = 
        ClassProvider::GetInstance()->GetTransferProgressNotifition();
    if (transferProcessNotify)
        transferProcessNotify->NotifyTransferBeginning(task.first);

    auto flagDestroyer = 
        [task, transferProcessNotify] (const bool* needToNotify)
    {
        if (transferProcessNotify && (*needToNotify))
            transferProcessNotify->NotifyTransferError(
                task.first, 
                ITransferProgressNotifition::BUILD_CONNECT_ERROR);

        delete needToNotify;
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

    auto success = 
        CreateDeviceDirectory(afcc, applicationDocumentDirctory, false);
    if (success)
    {
        FilePath srcPath(filePath);
        FilePath dstPath(applicationDocumentDirctory);
        dstPath = dstPath.Append(srcPath.BaseName());
        success = TransferFileToIOSDevice(afcc, srcPath.value(), 
                                          dstPath.value(), task);
    }
    if (transferProcessNotify)
        transferProcessNotify->NotifyTransferFinished(task.first, success);  

    DeleteTaskByTransferTaskID(task.first, false);
}

bool FileTransfer::TransferFileToIOSDevice(afc_client_private* afcc,
                                           const wstring& srcPath, 
                                           const wstring& dstPath, 
                                           const TransferTask& task)
{
    if (!afcc)
        return false;

    uint64 fileHandle = 0;
    ITransferProgressNotifition::ErrorCode error = 
        ITransferProgressNotifition::UNKNOWN_ERROR;
    bool success = false;
    do {
        wstring path = dstPath;
        replace_all(path, L"\\", L"/");
        auto afcError = afc_file_open(afcc, 
                                      SysWideToUTF8(path.c_str()).c_str(), 
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
            if (task.second->IsSet())
                break;

            // 从文件中读取数据到缓存
            if (buffer.second.first == buffer.second.second)
            {
                int64 restSize = fileSize - static_cast<int64>(stream.tellg());
                int readSize = static_cast<int>(
                    std::min(static_cast<int64>(bufferSize), restSize));
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
                    task.first, 
                    static_cast<int>(sendedTotalSize * 100 / fileSize));

            if (task.second->IsSet())
                break;

        } while (sendedTotalSize < fileSize);

        if (error != ITransferProgressNotifition::UNKNOWN_ERROR)
            success = true;

    } while (false);
    if ((!success) && (!task.second->IsSet()))
    {
        auto transferProcessNotify = 
            ClassProvider::GetInstance()->GetTransferProgressNotifition();
        if (transferProcessNotify)
            transferProcessNotify->NotifyTransferError(task.first, error);
    }

    return success;
}

bool FileTransfer::CreateDeviceDirectory(afc_client_private* afcc, 
                                         const wstring& directory, 
                                         bool recursively)
{
    if (!afcc)
        return false;

    //此处要等多层目录创建成功后才能删除
//     list<wstring> dirs;
//     split(dirs, directory, [] (char c) { return (L'/' == c) || (L'\\' == c); });
// 
//     wstring destDirectory;
//     auto error = AFC_E_SUCCESS;
//     for (auto dir = dirs.begin(); dir != dirs.end(); ++dir)
//     {
//         wformat f("%s/%s");
//         f % destDirectory % (*dir)
//         destDirectory = f.str();
//         int fileHandle = 0;
//         error = afc_file_open(afcc, SysWideToUTF8(destDirectory).c_str(), 
//                               AFC_FOPEN_RDONLY, &fileHandle);
//         if (error != AFC_E_SUCCESS)
//             break;
// 
// 
//     }
    auto error = afc_make_directory(afcc, SysWideToUTF8(directory).c_str());
    return AFC_E_SUCCESS == error;
}

bool FileTransfer::DeleteTaskByTransferTaskID(int transferTaskID, 
                                              bool needCancel)
{
    AutoLock l(*lock_);
    auto condition = [transferTaskID] (const TransferTask& task) -> bool
    {
        return transferTaskID == task.first;
    };
    auto res = find_if(tasks_.begin(), tasks_.end(), condition);
    if (tasks_.end() == res)
        return false;

    if (needCancel)
        res->second->Set();

    tasks_.erase(res);
    return true;
}
