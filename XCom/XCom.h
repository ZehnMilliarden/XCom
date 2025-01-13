#ifndef __XCOM_H__
#define __XCOM_H__

#include <atomic>
#include <mutex>
#include <cassert>
#include <limits>

#include "XComCore.h"
#include "XComCli.h"

#define XCOM_BEGIN_OBJECT_MAP(x) static _XCOM_OBJMAP_ENTRY x[] = {
#define XCOM_END_OBJECT_MAP() {nullptr, nullptr, nullptr, nullptr}};
#define XCOM_OBJECT_ENTRY(clsid, cls) {&clsid, cls::_ClassFactoryCreatorClass::CreateInstance, cls::_CreatorClass::CreateInstance, nullptr},

#define XCOM_BEGIN_MAP(x) public:\
    typedef x _XComMapClass;\
    static int __stdcall _Cache(void* pv, const XComGUID& riid, void** ppvObject, void* pCacheData) {\
        _XComMapClass* p = (_XComMapClass*)pv;\
        p->Lock(); \
        bool bRet = false; \
        try\
        {\
            bRet = XCComObjectRootBase::_Cache(pv, riid, ppvObject, pCacheData);\
        }\
        catch (...) \
        {\
           \
        }\
        p->Unlock();\
        return bRet;\
    }\
    IXComUnknown* _GetRawUnknown() noexcept {\
        return nullptr;\
    }\
    int _InternalQueryInterface(const XComGUID& riid, void** ppvObject) noexcept {\
        return InternalQueryInterface(this, _GetEntries(), riid, ppvObject);\
    }\
    const static _XCOM_INTMAP_ENTRY* __stdcall _GetEntries() {\
    static const _XCOM_INTMAP_ENTRY _entries[] = {


#define XCOM_INTERFACE_ENTRY(x)\
    { &XComGuidOf<x>(), xcomoffsetofclass(x, _XComMapClass), _XCOM_SIMPLEMAPENTRY },
    
#define XCOM_END_MAP() \
    {nullptr, 0, 0}}; \
    return _entries; \
    } \
    virtual int __stdcall AddRef() = 0;\
    virtual int __stdcall Release() = 0;\
    virtual int __stdcall QueryInterface(const XComGUID& riid, void** ppvObject) = 0;

#define XCOM_DECLARE_GET_CONTROLLING_UNKNOWN() public:\
	virtual IXComUnknown* GetControllingUnknown() noexcept { return GetUnknown(); }

class XCComFakeCriticalSection
{
public:
	bool Lock() noexcept {
		return true;
	}
	bool Unlock() noexcept {
		return true;
	}
};

class XCComCriticalSection
{
public:
    bool Lock() {
        m_mtx.lock();
        return true;
    }

    bool UnLock() {
        m_mtx.unlock();
        return true;
    }

    std::mutex m_mtx;
};


class XCComMultiThreadModelNoCS
{
public:
	typedef XCComFakeCriticalSection AutoCriticalSection;
	typedef XCComFakeCriticalSection AutoDeleteCriticalSection;
	typedef XCComFakeCriticalSection CriticalSection;
	typedef XCComMultiThreadModelNoCS ThreadModelNoCS;
};

class XCComSingleThreadModel
{
public:
    typedef XCComFakeCriticalSection AutoCriticalSection;
    typedef XCComFakeCriticalSection AutoDeleteCriticalSection;
    typedef XCComFakeCriticalSection CriticalSection;
    typedef XCComSingleThreadModel ThreadModelNoCS;
};

class XCComMultiThreadModel
{
public:
    typedef XCComCriticalSection AutoCriticalSection;
    typedef XCComCriticalSection AutoDeleteCriticalSection;
    typedef XCComCriticalSection CriticalSection;
    typedef XCComMultiThreadModelNoCS ThreadModelNoCS;
};

class XCComObjectRootBase
{
public:
    XCComObjectRootBase() 
        : m_nRef(0)
        , m_pOuterUnknown(nullptr)
    {

    }

    ~XCComObjectRootBase() {}

    bool FinalConstruct() { 
        return true; 
    }

    bool _FinalConstruct() {
        return true;
    }

    void FinalRelease() {

    }

    void _FinalRelease() {

    }

    int OuterAddRef() {
        return m_pOuterUnknown->AddRef();
    }

    int OuterRelease() {
        return m_pOuterUnknown->Release();
    }

    int OuterQueryInterface(const XComGUID& guid, void** ppInterface) {
        return m_pOuterUnknown->QueryInterface(guid, ppInterface);
    }

    void SetVoid(void*)
    {
    }

    void InternalFinalConstructAddRef()
    {
    }

    void InternalFinalConstructRelease()
    {
        assert(0 == m_nRef.load());
    }

    static int __stdcall InternalQueryInterface(void* pThis, const _XCOM_INTMAP_ENTRY* pEntries, const XComGUID& riid, void** ppvObject) {
        assert(nullptr != ppvObject);
        assert(nullptr != pThis);
        assert(_XCOM_SIMPLEMAPENTRY == pEntries->pFunc);
        int nRet = XComInternalQueryInterface(pThis, (_XCOM_INTMAP_ENTRY*)pEntries, riid, ppvObject);
        return nRet;
    }

    int InternalAddRef() {
        assert(-1L != m_nRef.load(), "-1L != m_nRef.load()");
        return m_nRef.fetch_add(1, std::memory_order_acq_rel) + 1;
    }

    int InternalRelease() {
        unsigned long l = m_nRef.fetch_sub(1, std::memory_order_acq_rel) - 1;
        assert(-1L != m_nRef.load(), "-1L != m_nRef.load()");
        return l;
    }

    static int __stdcall _Cache(void* pv, const XComGUID& guid, void** ppvObject, void* pCacheData)
    {
        bool bRet = false;
        _XCOM_CACHEDATA* pCache = (_XCOM_CACHEDATA*)pCacheData;
        IXComUnknown** ppUnk = (IXComUnknown**)((unsigned long*)pv + pCache->nOffsetVar);
        *ppvObject = nullptr;
        if (nullptr == *ppUnk) 
        {
            bRet = pCache->pCreatorFunc(pv, guid, ppvObject);
        }
        else 
        {
            bRet = (*ppUnk)->QueryInterface(guid, ppvObject);
        }
        return bRet;
    }

protected:
    std::atomic<long> m_nRef;
    IXComUnknown* m_pOuterUnknown;
};

#define XCOM_DECLARE_PROTECT_FINAL_CONSTRUCT()\
	void InternalFinalConstructAddRef() {InternalAddRef();}\
	void InternalFinalConstructRelease() {InternalRelease();}

template<class XComThreadModel>
class XCComObjectRootEx;

template<class XComThreadModel>
class XCComObjectLockT
{
public:
    XCComObjectLockT(XCComObjectRootEx<XComThreadModel>* p) {
        if (p)
            p->Lock();
        m_p = p;
    }

    ~XCComObjectLockT() {
        if (m_p)
            m_p->Unlock();
    }

    XCComObjectRootEx<XComThreadModel>* m_p;
};

template<>
class XCComObjectLockT<XCComSingleThreadModel> {
public:
    XCComObjectLockT(XCComObjectRootEx<XCComSingleThreadModel>* p) {
    }

    ~XCComObjectLockT() {
    }
};

template<>
class XCComObjectRootEx<XCComSingleThreadModel> 
    : public XCComObjectRootBase
{
public:
    typedef XCComSingleThreadModel _XComThreadModel;
    typedef typename _XComThreadModel::AutoCriticalSection _XComCritSec;
    typedef typename _XComThreadModel::AutoDeleteCriticalSection _XComAutoDelCritSec;
    typedef XCComObjectLockT<_XComThreadModel> _XComObjectLock;

    void Lock() {
    }

    void Unlock() {
    }
};

template<class XComThreadModel>
class XCComObjectRootEx : public XCComObjectRootBase
{
public:
    typedef XComThreadModel _XComThreadModel;
    typedef typename _XComThreadModel::AutoCriticalSection _XComCritSec;
    typedef typename _XComThreadModel::AutoDeleteCriticalSection _XComAutoDelCritSec;
    typedef XCComObjectLockT<_XComThreadModel> _XComObjectLock;

    void Lock() {
        m_critsec.Lock();
    }

    void Unlock() {
        m_critsec.UnLock();
    }

private:
    _XComAutoDelCritSec m_critsec;
};

typedef XCComObjectRootEx<XCComMultiThreadModel> XCComObjectRoot;

template<class Base>
class XCComObject : public Base
{
public:
    typedef Base _BaseClass;
    XCComObject(void* pv = nullptr) {
        XComModuleManger::GetInstance()->Lock();
    }

    virtual ~XCComObject() 
    {
        this->m_nRef.store(-(LONG_MAX / 2));
        this->FinalRelease();
        XComModuleManger::GetInstance()->Unlock();
    }

    int __stdcall AddRef() {
        return this->InternalAddRef();
    }

    int __stdcall Release() {
        unsigned long l = this->InternalRelease();
        if (0 == l)
        {
            XComModuleLockHelper lock;
            delete this;
        }
        return l;
    }

    int __stdcall QueryInterface(const XComGUID& riid, void** ppvObject) {
        return this->_InternalQueryInterface(riid, ppvObject);
    }

    template<class Q>
    int QueryInterface(Q** ppvObject) {
        return this->QueryInterface(XComGuidOf<Q>(), ppvObject);
    }

    static int __stdcall CreateInstance(XCComObject<Base>** ppvObject) {
        assert(nullptr != ppvObject);
        if (nullptr == ppvObject)
            return false;

        *ppvObject = nullptr;
        bool bRet = false;
        XCComObject<Base>* p = nullptr;
        try
        {
            p = new(std::nothrow) XCComObject<Base>();
        }
        catch(...)
        {
        }
        
        if (nullptr != p)
        {
            p->SetVoid(nullptr);
            p->InternalFinalConstructAddRef();
            bRet = p->FinalConstruct();
            if (bRet)
            {
                p->_FinalConstruct();
            }
            p->InternalFinalConstructRelease();

            if (!bRet)
            {
                delete p;
                p = nullptr;
            }
        }

        *ppvObject = p;
        return bRet;
    }
};

template<class Base>
class XCComContainedObject
    : public Base
{
public:
    typedef Base _BaseClass;
    XCComContainedObject(void* pv) {
        this->m_pOuterUnknown = (IXComUnknown*)pv;
    }

    int __stdcall AddRef() {
        return this->OuterAddRef();
    }

    int __stdcall Release() {
        return this->OuterRelease();
    }

    int __stdcall QueryInterface(const XComGUID& guid, void** ppvObject) {
        return this->OuterQueryInterface(guid, ppvObject);
    }

    IXComUnknown* GetControllingUnknown() {
        return this->m_pOuterUnknown;
    }
};

template <class contained>
class XCComAggObject
    : public IXComUnknown
    , public XCComObjectRootEx<typename contained::_XComThreadModel::ThreadModelNoCS>
{
private:
    typedef XCComObjectRootEx<typename contained::_XComThreadModel::ThreadModelNoCS> _MyXCComObjectRootEx;  

public:
    typedef contained _BaseClass;
    XCComAggObject(void* pv = nullptr) 
        : m_contained(pv)
    {
        XComModuleManger::GetInstance()->Lock();
    }

    virtual ~XCComAggObject() {
        this->m_nRef.store(-(LONG_MAX / 2));
        FinalRelease();
        XComModuleManger::GetInstance()->Unlock();
    }

    bool FinalConstruct() {
        _MyXCComObjectRootEx::FinalConstruct();
        return m_contained.FinalConstruct();
    }

    void FinalRelease() {
        _MyXCComObjectRootEx::FinalRelease();
        m_contained.FinalRelease();
    }

    int __stdcall AddRef() {
        return this->InternalAddRef();
    }

    int __stdcall Release() {
        unsigned long l = this->InternalRelease();
        if (0 == l)
        {
            XComModuleLockHelper lock;
            delete this;
        }
        return l;
    }

    int __stdcall QueryInterface(const XComGUID& guid, void** ppvObject) {
        assert(nullptr != ppvObject);
        if (nullptr == ppvObject)
            return false;
        *ppvObject = nullptr;
        bool bRet = true;
        if (IsEqualUnknownIID(guid))
        {
            *ppvObject = (void*)(IXComUnknown*)this;
            AddRef();
        }
        else
        {
            bRet = m_contained._InternalQueryInterface(guid, ppvObject);
        }        
        return bRet;
    }

    template<class Q>
    bool __stdcall QueryInterface(Q** ppvObject) {
        return this->QueryInterface(XComGuidOf<Q>(), (void**)ppvObject);
    }

    static int __stdcall CreateInstance(
        IXComUnknown* pUnkOuter,
        XCComAggObject<contained>** ppvObject) 
    {
        assert(nullptr != ppvObject);
        if (nullptr == ppvObject)
            return false;

        *ppvObject = nullptr;
        bool bRet = false;
        XCComAggObject<contained>* p = nullptr;
        try
        {
            p = new(std::nothrow) XCComAggObject<contained>(pUnkOuter);
        }   
        catch(...)
        {
        }
        
        if (nullptr != p)
        {
            p->SetVoid(nullptr);
            p->InternalFinalConstructAddRef();
            bRet = p->FinalConstruct();
            if (bRet)
            {
                p->_FinalConstruct();
            }
            p->InternalFinalConstructRelease();

            if (!bRet)
            {
                delete p;
                p = nullptr;
            }
        }

        *ppvObject = p;
        return bRet;
    }

    XCComContainedObject<contained> m_contained;
};

template<class Base>
class XCComObjectCached
    : public Base
{
public:
    typedef Base _BaseClass;
    XCComObjectCached(void* pv = nullptr) {
        
    }

    virtual ~XCComObjectCached() {
        this->m_nRef.store(-(LONG_MAX / 2));
        this->FinalRelease();
    }

    int __stdcall AddRef() {
        unsigned long l = this->InternalAddRef();
        if (2 == l)
        {
            XComModuleManger::GetInstance()->Lock();
        }
        return l;
    }

    int __stdcall Release() {
        unsigned long l = this->InternalRelease();
        if (0 == l)
        {
            delete this;
        }
        else if (1 == l)
        {
            XComModuleManger::GetInstance()->Unlock();
        }
        return l;
    }

    int __stdcall QueryInterface(const XComGUID& guid, void** ppvObject) {
        return this->_InternalQueryInterface(guid, ppvObject);
    }

    static int __stdcall CreateInstance(XCComObjectCached<Base>** ppvObject) {
        assert(nullptr != ppvObject);
        if (nullptr == ppvObject)
            return false;

        *ppvObject = nullptr;
        bool bRet = false;
        XCComObjectCached<Base>* p = nullptr;
        try
        {
            p = new(std::nothrow) XCComObjectCached<Base>();
        }   
        catch(...)
        {
        }
        
        if (nullptr != p)
        {
            p->SetVoid(nullptr);
            p->InternalFinalConstructAddRef();
            bRet = p->FinalConstruct();
            if (bRet)
            {
                p->_FinalConstruct();
            }
            p->InternalFinalConstructRelease();
        }    
        *ppvObject = p;
        return bRet;     
    }
};

class XCComClassFactory
    : public IXComClassFactory
    , public XCComObjectRootEx<XCComMultiThreadModel>
{
public:
    XCOM_BEGIN_MAP(XCComClassFactory)
        XCOM_INTERFACE_ENTRY(IXComClassFactory)
    XCOM_END_MAP()

    virtual ~XCComClassFactory() {
        
    }

    int __stdcall CreateInstance(IXComUnknown* pUnkOuter, const XComGUID& riid, void** ppvObject) {
        assert(nullptr != ppvObject);
        bool bRet = false;
        if (nullptr != ppvObject)
        {
            *ppvObject = nullptr;
            if ((nullptr == pUnkOuter) || IsEqualUnknownIID(riid))
            {
                bRet = m_pfnCreateInstance(pUnkOuter, riid, ppvObject);
            }
        }
        return bRet;
    }

    int __stdcall LockServer(bool bLock) {
        if (bLock)
            XComModuleManger::GetInstance()->Lock();
        else
            XComModuleManger::GetInstance()->Unlock();
        return true;
    }

    void SetVoid(void* pv) {
        m_pfnCreateInstance = (_XCOM_CREATORFUNC*)pv;
    }

    _XCOM_CREATORFUNC* m_pfnCreateInstance;
};

template <class T>
class XCComClassFactorySingleton
    : public XCComClassFactory
{
public:
    XCComClassFactorySingleton() {
        
    }

    virtual ~XCComClassFactorySingleton() {
        
    }

    int __stdcall CreateInstance(IXComUnknown* pUnkOuter, const XComGUID& riid, void** ppvObject) {
        bool bRet = false;
        if (nullptr != ppvObject)
        {
            assert(nullptr != ppvObject);
            if (nullptr == pUnkOuter)
            {
                if (m_bCreate && nullptr == m_spObj)
                {
                    Lock();
                    if (m_bCreate && nullptr == m_spObj)
                    {
                        XCComObjectCached<T> *p;
                        m_bCreate = XCComObjectCached<T>::CreateInstance(&p);
                        if (m_bCreate)
                        {
                            m_bCreate = p->QueryInterface(XComGuidOf<IXComUnknown>(), (void**)&m_spObj);
                            if (!m_bCreate)
                            {
                                delete p;
                            }
                        }
                    }
                    Unlock();
                }
            }

            if (m_bCreate)
            {
                bRet = m_spObj->QueryInterface(riid, ppvObject);
            }
            else
            {
                bRet = m_bCreate;
            }
        }

        return bRet;
    }

    bool m_bCreate = true;
    XComPtr<IXComUnknown> m_spObj;
};

template<class T, const XComGUID* pclsid = &XCOM_GUID_NULL>
class XCComCoClass
{
public:
    static const XComGUID& GetClassID() {
        return *pclsid;
    }

    template<class Q>
    static int __stdcall CreateInstance(IXComUnknown* pUnkOuter, Q** ppvObject) {
        return T::_CreatorClass::CreateInstance(pUnkOuter, XComGuidOf<Q>(), (void**)ppvObject);
    }

    template<class Q>
    static int __stdcall CreateInstance(Q** ppvObject) {
        return T::_CreatorClass::CreateInstance(nullptr,  XComGuidOf<Q>(), (void**)ppvObject);
    }
};

template <class T1>
class XCComCreator
{
public:
    static int __stdcall CreateInstance(void* pCreaterInstance, const XComGUID& riid, void** ppvObject) {
        assert(nullptr != ppvObject);
        if (nullptr == ppvObject)
        {
            return  false;
        }

        bool bRet = false;
        *ppvObject = nullptr;
        T1* p = new(std::nothrow) T1(pCreaterInstance);
        if (nullptr != p)
        {
            p->SetVoid(pCreaterInstance);
            p->InternalFinalConstructAddRef();
            bRet = p->FinalConstruct();
            if (bRet)
            {
                bRet =p->_FinalConstruct();
            }
            p->InternalFinalConstructRelease();

            if (bRet)
            {
                bRet = p->QueryInterface(riid, ppvObject);
            }

            if (!bRet)
            {
                delete p;
            }
        }
        return bRet;
    }
};

template <int ret>
class XCComFailCreater
{
public:
    static int __stdcall CreateInstance(void* pCreaterInstance, const XComGUID &riid, void **ppvObject)
    {
        if (nullptr == ppvObject)
        {
            return false;
        }
        return ret;
    }
};

template <class T1, class T2>
class XCComCreator2
{
public:
    static int __stdcall CreateInstance(void* pCreaterInstance, const XComGUID& riid, void** ppvObject) {
        assert(nullptr != ppvObject);
        return (nullptr == pCreaterInstance) ? T1::CreateInstance(nullptr, riid, ppvObject) : T2::CreateInstance(pCreaterInstance, riid, ppvObject);
    }
};

#define XCOM_DECLARE_CLASSFACTORY_EX(cf) typedef XCComCreator<XCComObjectCached<cf>> _ClassFactoryCreatorClass;
#define XCOM_DECLARE_CLASSFACTORY() XCOM_DECLARE_CLASSFACTORY_EX(XCComClassFactory)
#define XCOM_DECLARE_CLASSFACTORY_SINGLETON(obj) XCOM_DECLARE_CLASSFACTORY_EX(XCComClassFactorySingleton<obj>);

#define XCOM_DECLARE_NOT_AGGREGATABLE(x) public:\
    typedef XCComCreator2<XCComCreator<XCComObject<x>>, XCComFailCreater<false>> _CreatorClass;
#define XCOM_DECLARE_AGGREGATABLE(x) public:\
    typedef XCComCreator2<XCComCreator<XCComObject<x>>, XCComCreator<XCComAggObject<x>>> _CreatorClass;
#define XCOM_DECLARE_ONLY_AGGREGATABLE(x) public:\
    typedef XCComCreator2<XCComFailCreater<false>, XCComCreator<XCComAggObject<x>>> _CreatorClass;

#define XCOM_DECLARE_CLS_INSTANCE_CREATER(cls) public:\
    static bool __stdcall CreateClsInstance(XComPtr<XCComObject<cls>>& pObj)\
    {\
        XCComObject<cls>* _pTarget = nullptr;\
        bool bRet = XCComObject<cls>::CreateInstance(&_pTarget);\
        if (bRet)\
        {\
            pObj = _pTarget;\
        }\
        return hr;\
    }\
    static bool __stdcall CreateClsInstance(_Inout_opt_ IXComUnknown* pUnkOuter, XComPtr<XCComAggObject<cls>>& pObj)\
    {\
        XCComAggObject<cls>* _pTarget = nullptr;\
        bool bRet = XCComAggObject<cls>::CreateInstance(pUnkOuter,&_pTarget);\
        if (bRet)\
        {\
            pObj = _pTarget;\
        }\
        return hr;\
    }

#endif // __XCOM_H__