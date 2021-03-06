﻿#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

#include <utility>
#include <list>

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/unknown.h"

namespace base
{
class Thread;
class Lock;
class CancellationFlag;
}

namespace ios_transfer
{
enum ITunesCommonFolderType;
}

struct afc_client_private;

class FileTransfer : public Unknown
                   , public ios_transfer::ITransfer

{
public:
    FileTransfer();
    virtual ~FileTransfer();

    DECLARE_IUNKNOWN

    virtual HRESULT __stdcall NonDelegateQueryInterface(const GUID& iid, 
                                                        void** v) override;

protected:
    virtual HRESULT __stdcall TransferFileToITunes(
        ios_transfer::DeviceID id, const wchar_t* filePath,
        ios_transfer::ITunesCommonFolderType folderType, int* transferTaskID) 
        override;

    virtual HRESULT __stdcall TransferFileAsIOSRing(
        ios_transfer::DeviceID id, const wchar_t* filePath, 
        int* transferTaskID) override;

    virtual HRESULT __stdcall TransferFileToApplication(
        ios_transfer::DeviceID id, const wchar_t* filePath, 
        const wchar_t* applicationID, int* transferTaskID) override;

    virtual HRESULT __stdcall CancelTransferTask(int transferTaskID) override;

private:
    typedef std::pair<int, std::shared_ptr<base::CancellationFlag>> 
        TransferTask;

    void TransferFileToApplicationPrivate(const std::string& udid, 
                                          const std::string& applicationID, 
                                          const std::wstring& filePath, 
                                          const TransferTask& task);

    bool TransferFileToIOSDevice(afc_client_private* afcc,
                                 const std::wstring& srcPath, 
                                 const std::wstring& dstPath, 
                                 const TransferTask& transferTaskID);

    bool CreateDeviceDirectory(afc_client_private* afcc, 
                               const std::wstring& directory, bool recursively);

    bool DeleteTaskByTransferTaskID(int transferTaskID, bool needCancel);

    std::unique_ptr<base::Thread> thread_;
    int taskID_;
    std::list<TransferTask> tasks_;
    std::unique_ptr<base::Lock> lock_;
};

#endif