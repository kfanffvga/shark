#include <string>
#include <Windows.h>

#include "third_party/chromium/base/at_exit.h"

#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/com_factory.h"

using std::wstring;
using std::move;

namespace {
const wchar_t* classID = L"{7B5ED7CA-D01B-4A06-8C5D-F61B1F3B3A20}";
const wchar_t* directoryName = L"InprocServer32";

wstring GetRegisterPath()
{
    wstring registerPath;
    registerPath = wstring(classID) + L"\\" + wstring(directoryName);
    return move(registerPath);
}

}

HMODULE module_;

int __stdcall DllMain(HMODULE module, uint32 reasonForCall, void* reserved)
{
    static base::AtExitManager* atExit = NULL;
    switch (reasonForCall)
    {
    case DLL_PROCESS_ATTACH:
        atExit = new base::AtExitManager;
        module_ = static_cast<HMODULE>(module);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        delete atExit;
        break;
    }

    return TRUE;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    return S_OK;
}

extern "C" HRESULT __stdcall DllGetClassObject(const GUID& classID, 
                                               const GUID& interfaceID, 
                                               void** value)
{
    if (ios_transfer::CLSID_IOSTransfer == classID)
    {
        ComFactory* factory = new ComFactory();
        HRESULT hr = factory->QueryInterface(interfaceID, value);
        if (SUCCEEDED(hr))
            return hr;

        delete factory;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
    HKEY root, newKey;
    RegOpenKey(HKEY_CLASSES_ROOT,L"CLSID",&root);
    RegCreateKey(root, GetRegisterPath().c_str(), &newKey);
    wchar_t moduleFilePath[MAX_PATH];
    GetModuleFileName(module_, moduleFilePath, MAX_PATH);
    RegSetValue(newKey, NULL,REG_SZ, moduleFilePath, MAX_PATH);
    RegCloseKey(root);
    return S_OK;
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
    HKEY root;
    RegOpenKey(HKEY_CLASSES_ROOT, L"CLSID", &root);
    RegDeleteKey(root, classID);
    RegDeleteKey(root, GetRegisterPath().c_str());
    RegCloseKey(root);
    return S_OK;
}