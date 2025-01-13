#ifndef __XCOM_BASE_H__
#define __XCOM_BASE_H__

#include <stdio.h>
#include <string.h>
#include <cassert>

struct XComGUID
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];

    XComGUID() {
        Data1 = 0;
        Data2 = 0;
        Data3 = 0;
        memset(Data4, 0, sizeof(Data4));
    }

    explicit XComGUID(const char* szGuidString) {
        sscanf_s(szGuidString,
            "%8lx-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
            &Data1, &Data2, &Data3,
            &Data4[0], &Data4[1], &Data4[2], &Data4[3],
            &Data4[4], &Data4[5], &Data4[6], &Data4[7], sizeof(Data4[0]), sizeof(Data4[1]),
            sizeof(Data4[2]), sizeof(Data4[3]), sizeof(Data4[4]), sizeof(Data4[5]),
            sizeof(Data4[6]), sizeof(Data4[7]));
    }

    XComGUID(const XComGUID& other) {
        Data1 = other.Data1;
        Data2 = other.Data2;
        Data3 = other.Data3;
        ::memcpy_s(Data4, sizeof(Data4), other.Data4, sizeof(Data4));
    }

    XComGUID& operator=(const XComGUID& other) {
        Data1 = other.Data1;
        Data2 = other.Data2;
        Data3 = other.Data3;
        ::memcpy_s(Data4, sizeof(Data4), other.Data4, sizeof(Data4));
        return *this;
    }

    bool operator==(const XComGUID& other) const {
        return Data1 == other.Data1 &&
               Data2 == other.Data2 &&
               Data3 == other.Data3 &&
               memcmp(Data4, other.Data4, sizeof(Data4)) == 0;
    }
};

static const XComGUID XCOM_GUID_NULL;
static const XComGUID XCOM_IID_NULL = XCOM_GUID_NULL;

__inline int IsEqualGUID(const XComGUID& guid1, const XComGUID& guid2) {
    return 0 == memcmp(&guid1, &guid2, sizeof(XComGUID));
}

template<typename T>
struct TypeGUID {
    static const XComGUID& get() {
        return XComGUID();
    }
};

template<typename T>
inline const XComGUID& XComGuidOf() {
    return TypeGUID<T>::get();
}

/**
#define XCOM_DEFINE_GUID_FOR_TYPE(TYPE, DATA1, DATA2, DATA3, D4_0, D4_1, D4_2, D4_3, D4_4, D4_5, D4_6, D4_7) \
template<> \
struct TypeGUID<TYPE> { \
    static XComGUID get() { \
        return {DATA1, DATA2, DATA3, {D4_0, D4_1, D4_2, D4_3, D4_4, D4_5, D4_6, D4_7}}; \
    } \
};
*/

#define XCOM_DEFINE_GUID_FOR_TYPE(TYPE, GUID_STRING) \
template<> \
struct TypeGUID<TYPE> { \
    static const XComGUID& get() { \
        static const XComGUID _guid(GUID_STRING); \
        return _guid; \
    } \
};

struct IXComUnknown
{
    virtual int __stdcall QueryInterface(const XComGUID& guid, void** ppInterface) = 0;
    virtual int __stdcall AddRef() = 0;
    virtual int __stdcall Release() = 0;

    template<typename T>
    int __stdcall QueryInterface(T** ppInterface) {
        return QueryInterface(XComGuidOf<T>(), (void**)ppInterface);
    }
};
XCOM_DEFINE_GUID_FOR_TYPE(IXComUnknown, "00000000-0000-0000-C000-000000000046");

struct IXComClassFactory : public IXComUnknown {
    virtual int __stdcall CreateInstance(
        IXComUnknown* pUnkOuter,
        const XComGUID& iid,
        void** ppvObject) = 0;
    virtual int __stdcall LockServer(bool bLock) = 0;
};
XCOM_DEFINE_GUID_FOR_TYPE(IXComClassFactory, "00000001-0000-0000-C000-000000000046");

__inline bool IsEqualUnknownIID(const XComGUID& iid) {
    return IsEqualGUID(iid, XComGuidOf<IXComUnknown>());
}

#endif // __XCOM_BASE_H__