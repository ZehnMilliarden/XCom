#ifndef __XCOMCLI_H__
#define __XCOMCLI_H__

#include "XComBase.h"

template <class T>
class _NoAddRefReleaseOnXCComPtr 
    : public T
{
private:
    int XCOMCALL AddRef() = 0;
    int XCOMCALL Release() = 0;
};

template<class T>
class XComPtrBase
{
protected:
    XComPtrBase() noexcept {
        m_pObject = nullptr;
    }
    XComPtrBase(T * pObject) noexcept {
        m_pObject = pObject;
        if (m_pObject)
            m_pObject->AddRef();
    }

    void Swap(XComPtrBase& other) noexcept {
        T* pTemp = m_pObject;
        m_pObject = other.m_pObject;
        other.m_pObject = pTemp;
    }

public:
    typedef T _PtrClass;
    virtual ~XComPtrBase() { 
        if (m_pObject) m_pObject->Release(); 
    }

    operator T*() const noexcept{ 
        return m_pObject; 
    }

    T& operator*() const{ 
        assert(m_pObject != nullptr);
        return *m_pObject; 
    }

    T** operator&() noexcept
    {
        assert(m_pObject == NULL);
        return &m_pObject;
    }

    _NoAddRefReleaseOnXCComPtr<T>* operator->() const {
        assert(m_pObject != nullptr);
        return (_NoAddRefReleaseOnXCComPtr<T>*)m_pObject;
    }

    bool operator!() const noexcept {
        return m_pObject == nullptr;
    }

    bool operator < (T* pObject) const noexcept {
        return m_pObject < pObject;
    }

    bool operator == (T* pObject) const noexcept {
        return m_pObject == pObject;
    }

    void Release() {
        T* pTemp = m_pObject;
        if (pTemp) {
            m_pObject = nullptr;
            pTemp->Release();
        }
    }

    inline bool IsEqualObject(IXComUnknown* pObject) const noexcept;

    void Attach(T* pObject) {
        if (m_pObject)
        {
            unsigned long l = m_pObject->Release();
            assert(0L != l || m_pObject != pObject);
        }
        m_pObject = pObject;
    }

    T* Detach() {
        T* pTemp = m_pObject;
        m_pObject = nullptr;
        return pTemp;
    }

    bool CopyTo(T** ppObject) {
        assert(ppObject != nullptr);
        if (ppObject == nullptr) 
            return false;
        *ppObject = m_pObject;
        if (m_pObject)
            m_pObject->AddRef();
        return true;
    }

    template<class Q>
    XCOMRESULT QueryInterface(Q** ppInterface) {
        assert(ppInterface != nullptr);
        if (ppInterface == nullptr) 
            return XCOM_E_INVALIDARG;
        return m_pObject->QueryInterface(ppInterface);
    }

    T* m_pObject;
};

template<class T>
class XComPtr : public XComPtrBase<T>
{
public:
    XComPtr() {}
    XComPtr(T *pObject) : XComPtrBase<T>(pObject)
    {
    }
    XComPtr(const XComPtr<T>& other) : XComPtrBase<T>(other.m_pObject) 
    {
    }
    XComPtr(XComPtr<T>&& other) : XComPtrBase<T>()
    {
        other.Swap(*this);
    }

    T* operator=(T* pObject) {
        if (this->m_pObject != pObject)
        {
            XComPtr(pObject).Swap(*this);
        }
        return *this;
    }
    T* operator=(const XComPtr<T>& other) {
        if (this->m_pObject != other.m_pObject)
        {
            XComPtr(other).Swap(*this);
        }
        return *this;
    }
    T* operator=(XComPtr<T>&& other) {
        if (this->m_pObject != other.m_pObject)
        {
            CComPtr(static_cast<XComPtr&&>(other)).Swap(*this);
        }
        return *this;
    }
    template <class Q>
    T* operator=(XComPtr<Q>& other) {
        if (!this->IsEqualObject(other))
        {
            IXComUnknown* pTemp = this->m_pObject;
            if ((other == NULL) || XCOM_FAILED_STRICT(other->QueryInterface(&this->m_pObject))) {
                this->m_pObject = NULL;

                if (pTemp)
                    pTemp->Release();
            }
        }
        return *this;
    }    
};

template <class T>
bool XComPtrBase<T>::IsEqualObject(IXComUnknown *pObject) const noexcept
{
    if (!pObject && !m_pObject)
        return true;
    if (!pObject || !m_pObject)
        return false;

    XComPtr<IXComUnknown> pUnk1;
    XComPtr<IXComUnknown> pUnk2;

    pObject->QueryInterface(XComGuidOf<IXComUnknown>(), (void **)&pUnk1);
    m_pObject->QueryInterface(XComGuidOf<IXComUnknown>(), (void **)&pUnk2);

    return pUnk1 == pUnk2;
}


template <class T>
class XComCAdapt
{
public:
    XComCAdapt()
    {
    }

    XComCAdapt(const T& rSrc) :
        m_T(rSrc)
    {
    }

    XComCAdapt(const XComCAdapt<T>& rSrCA) :
        m_T(rSrCA.m_T)
    {
    }

    XComCAdapt<T>& operator=(const T& rSrc)
    {
        m_T = rSrc;

        return *this;
    }

    XComCAdapt<T>& operator=(const XComCAdapt<T>& rSrc)
    {
        if (this != &rSrc)
        {
            m_T = rSrc.m_T;
        }
        return *this;
    }

    XComCAdapt(T&& rSrc) :
        m_T(static_cast<T&&>(rSrc))
    {
    }

    XComCAdapt(XComCAdapt<T>&& rSrCA) :
        m_T(static_cast<T&&>(rSrCA.m_T))
    {
    }

    XComCAdapt<T>& operator=(T&& rSrc)
    {
        m_T = static_cast<T&&>(rSrc);

        return *this;
    }

    XComCAdapt<T>& operator=(XComCAdapt<T>&& rSrc)
    {
        if (this != &rSrc)
        {
            m_T = static_cast<T&&>(rSrc.m_T);
        }
        return *this;
    }

    bool operator<(const T& rSrc) const
    {
        return m_T < rSrc;
    }

    bool operator==(const T& rSrc) const
    {
        return m_T == rSrc;
    }

    operator T& ()
    {
        return m_T;
    }

    operator const T& () const
    {
        return m_T;
    }

    T& operator->()
    {
        return m_T;
    }

    const T& operator->() const
    {
        return m_T;
    }

    T m_T;
};

#endif // __XCOMCLI_H__