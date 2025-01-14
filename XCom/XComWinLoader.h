#pragma once

#include "XComCli.h"
#include <Windows.h>

template<class T, const XComGUID& clsid>
class XComWinLoader
{

    typedef XCOMRESULT(__stdcall* CREATE_COM_OBJECT_FUNC)(const XComGUID&, const XComGUID&, void**);
    typedef XCOMRESULT(__stdcall* DLL_CAN_UNLOAD_FUNC)();

public:
    XComWinLoader() {

    }

    ~XComWinLoader()
    {
        Unload();
    }

    bool Load(const wchar_t* szModule) 
    {
        if (m_hModule)
        {
            return true;
        }

        m_hModule = ::LoadLibraryW(szModule);

        if (m_hModule == nullptr)
        {
            return false;
        }
        
        CREATE_COM_OBJECT_FUNC pCreateFunc = reinterpret_cast<CREATE_COM_OBJECT_FUNC>(
            GetProcAddress(m_hModule, "XComDllGetClassObject"));

        m_pfDllCanUnload = reinterpret_cast<DLL_CAN_UNLOAD_FUNC>(
            GetProcAddress(m_hModule, "XComDllCanUnloadNow"));

        if (pCreateFunc == nullptr)
        {
            Unload();
            return false;
        }

        XCOMRESULT hr = pCreateFunc(
            clsid, 
            XComGuidOf<IXComClassFactory>(),
            reinterpret_cast<LPVOID*>(&m_pClassFactory));

        if (XCOM_FAILED_STRICT(hr))
        {
            Unload();
            return false;
        }

        return true;
    }

    void Unload() 
    {
        if (m_pClassFactory)
        {
            m_pClassFactory.Release();
            m_pClassFactory = nullptr;
        }
        
        if (XCOM_FAILED_STRICT(m_pfDllCanUnload()))
        {
            return;
        }

        if (m_hModule)
        {
            ::FreeLibrary(m_hModule);
            m_hModule = nullptr;
        }
    }

    bool CreateInstance(XComPtr<T>& pInstance) {
        if (m_pClassFactory == nullptr) {
            return false;
        }

        XCOMRESULT hr = m_pClassFactory->CreateInstance(
            nullptr, 
            XComGuidOf<T>(),
            reinterpret_cast<LPVOID*>(&pInstance));

        return XCOM_SUCCEEDED_STRICT(hr);
    }

private:
    XComWinLoader(const XComWinLoader&) = delete;
    XComWinLoader& operator=(const XComWinLoader&) = delete;

private:
    HMODULE m_hModule = nullptr;
    XComPtr<IXComClassFactory> m_pClassFactory = nullptr;
    DLL_CAN_UNLOAD_FUNC m_pfDllCanUnload = nullptr;
};
