#pragma once

#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#endif // 

#include "XComCli.h"

template<class T, const XComGUID& clsid>
class XComLoader
{

    typedef XCOMRESULT(XCOMCALL* CREATE_COM_OBJECT_FUNC)(const XComGUID&, const XComGUID&, void**);
    typedef XCOMRESULT(XCOMCALL* DLL_CAN_UNLOAD_FUNC)();

public:
    XComLoader() {

    }

    ~XComLoader()
    {
        Unload();
    }

    bool Load(const wchar_t* szModule) 
    {
        if (m_hModule)
        {
            return true;
        }
#ifdef _WIN32
        m_hModule = ::LoadLibraryW(szModule);
#elif __linux__
        char libPath[1024] = {0};
        size_t converted = wcstombs(libPath, szModule, sizeof(libPath));
        if (converted == (size_t)-1) {
            return false;
        }
        m_hModule = dlopen(libPath, RTLD_LAZY);
#endif // 

        if (m_hModule == nullptr)
        {
            return false;
        }
        
#ifdef _WIN32
        CREATE_COM_OBJECT_FUNC pCreateFunc = reinterpret_cast<CREATE_COM_OBJECT_FUNC>(
            GetProcAddress(m_hModule, "XComGetClassObject"));
        m_pfDllCanUnload = reinterpret_cast<DLL_CAN_UNLOAD_FUNC>(
            GetProcAddress(m_hModule, "XComCanUnloadNow"));
#elif __linux__
        CREATE_COM_OBJECT_FUNC pCreateFunc = reinterpret_cast<CREATE_COM_OBJECT_FUNC>(
            dlsym(m_hModule, "XComGetClassObject"));
        m_pfDllCanUnload = reinterpret_cast<DLL_CAN_UNLOAD_FUNC>(
            dlsym(m_hModule, "XComCanUnloadNow"));
#endif // 

        if (pCreateFunc == nullptr)
        {
            Unload();
            return false;
        }

        XCOMRESULT hr = pCreateFunc(
            clsid, 
            XComGuidOf<IXComClassFactory>(),
            reinterpret_cast<void**>(&m_pClassFactory));

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
        
        if (m_pfDllCanUnload &&
            XCOM_FAILED_STRICT(m_pfDllCanUnload()))
        {
            return;
        }

        if (m_hModule)
        {
#ifdef _WIN32
            ::FreeLibrary(m_hModule);
#elif __linux__
            dlclose(m_hModule);
#endif // 
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
            reinterpret_cast<void**>(&pInstance));

        return XCOM_SUCCEEDED_STRICT(hr);
    }

private:
    XComLoader(const XComLoader&) = delete;
    XComLoader& operator=(const XComLoader&) = delete;

private:
#ifdef _WIN32
    HMODULE m_hModule = nullptr;
#elif __linux__
    void* m_hModule = nullptr;
#endif // 
    XComPtr<IXComClassFactory> m_pClassFactory = nullptr;
    DLL_CAN_UNLOAD_FUNC m_pfDllCanUnload = nullptr;
};
