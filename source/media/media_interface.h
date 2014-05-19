#ifndef _MEDIA_INTERFACE_H_
#define _MEDIA_INTERFACE_H_

#include <Unknwn.h>

namespace media
{
static const GUID CLSID_Media = 
    {0x8edaa2df, 0xb242, 0x4ecb, 0x93, 0x9e, 0x9d, 0xc4, 0x1d, 0xf7, 0x8a, 
     0xe5};

// {CCBCC94A-35EB-49DD-ABE4-930BF96F3082}
static const GUID IID_IAsyncMediaInfoProvider = 
    {0xccbcc94a, 0x35eb, 0x49dd, 0xab, 0xe4, 0x93, 0x0b, 0xf9, 0x6f, 0x30, 
     0x82};

// {C7638253-224D-4D24-92A6-B1797C8AB635}
static const GUID IID_IMediaFileConvertor = 
    {0xc7638253, 0x224d, 0x4d24, 0x92, 0xa6, 0xb1, 0x79, 0x7c, 0x8a, 0xb6, 
     0x35};

// {938B7E08-319A-4BF1-B21B-7CD23F975392}
static const GUID IID_IInitialzedConfigure = 
    {0x938b7e08, 0x319a, 0x4bf1, 0xb2, 0x1b, 0x7c, 0xd2, 0x3f, 0x97, 0x53, 
     0x92};

class IMediaInfoProviderNotify : public IUnknown
{
public:
    enum ErrorCode
    {
        NOT_MEDIA_FILE = 0
    };

    virtual HRESULT __stdcall StartLookForMediaFileInfo(int id) = 0;
    virtual HRESULT __stdcall FinishLookForMediaFileInfo(int id, 
                                                         bool success) = 0;
    virtual HRESULT __stdcall LookForMediaFileInfoError(int id, 
                                                        ErrorCode error) = 0;
};

class IMediaFileConvertProgressNotify : public IUnknown
{
public:
    virtual HRESULT __stdcall NotifyTransferBeginning(int convertTaskID) = 0;
    virtual HRESULT __stdcall NotifyTransferProgress(int convertTaskID, 
                                                     int precent) = 0;
    virtual HRESULT __stdcall NotifyTransferFinished(int convertTaskID, 
                                                     bool success) = 0;
    virtual HRESULT __stdcall NotifyTransferError(int convertTaskID, 
                                                  enum ErrorCode code) = 0;
};

class IAsyncMediaInfoProvider : public IUnknown
{
public:
    virtual HRESULT __stdcall LookForMediaInfo(const wchar_t* filePath, 
                                               int* id) = 0;
    virtual HRESULT __stdcall GetFileAudioTrackCount(int id, 
                                                     int* trackCount) = 0;
    virtual HRESULT __stdcall GetFileBitrate(int id, int* bitrate) = 0;
    virtual HRESULT __stdcall GetFileFormat(int id, wchar_t** format) = 0;

    virtual HRESULT __stdcall GetVideoHeightAndWidth(int id, int* height, 
                                                     int* width) = 0;
    virtual HRESULT __stdcall GetVideoBitrate(int id, int* bitrate) = 0;
    virtual HRESULT __stdcall GetVideoFormat(int id, wchar_t** format) = 0;

    virtual HRESULT __stdcall GetAudioBitrate(int id, int trackNum, 
                                              int* bitrate) = 0;
    virtual HRESULT __stdcall GetAudioFormat(int id, wchar_t** format) = 0;
    virtual HRESULT __stdcall GetAudioChannelCount(int id, 
                                                   int* channelCount) = 0;
    
};

class IMediaConvertor : public IUnknown
{
public:
    enum ConvertQuality
    {
        HIGH_QUALITY = 0,
        MIDDLE_QUALITY,
        LOW_QUALITY
    };

    virtual HRESULT __stdcall ConvertAudioToCiff(const wchar_t* sourceFileName, 
                                                 const wchar_t* destFileName,
                                                 __int64 beginTime, 
                                                 __int64 endTime,
                                                 bool convertedIfExist,
                                                 int* convertTaskID) = 0;

    virtual HRESULT __stdcall ConvertAudioToAAC(const wchar_t* sourceFileName, 
                                                const wchar_t* destFileName,
                                                __int64 beginTime, 
                                                __int64 endTime,
                                                bool convertedIfExist,
                                                ConvertQuality quality,
                                                int* convertTaskID) = 0;

    virtual HRESULT __stdcall ConvertVideoToMP4(const wchar_t* sourceFileName, 
                                                const wchar_t* destFileName,
                                                __int64 beginTime, 
                                                __int64 endTime,
                                                bool convertedIfExist,
                                                ConvertQuality quality,
                                                int* convertTaskID) = 0;
};

class IInitialzedConfigure : public IUnknown
{
public:
    virtual HRESULT __stdcall RegisterMediaInfoProviderNotifition(
        IMediaInfoProviderNotify* mediaInfoProviderNotifition) = 0;
    virtual HRESULT __stdcall RegisterMediaFileConvertProgress(
        IMediaFileConvertProgressNotify* transferProgressNotifition) = 0;
};

}

#endif