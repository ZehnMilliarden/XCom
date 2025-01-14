#ifndef __XCOMERROR_H__
#define __XCOMERROR_H__

typedef long XCOMRESULT;

#define XCOM_HRESULT_TYPEDEF(_sc)           ((XCOMRESULT)_sc)
#define XCOM_SUCCEEDED(hr)                  (((XCOMRESULT)(hr)) >= 0L)
#define XCOM_FAILED(hr)                     (((XCOMRESULT)(hr)) < 0L)
#define XCOM_SUCCEEDED_STRICT(hr)           (0L == ((XCOMRESULT)(hr)))
#define XCOM_FAILED_STRICT(hr)              (0L != ((XCOMRESULT)(hr)))

// SUCCESSED
#define XCOM_S_OK                           XCOM_HRESULT_TYPEDEF(0x0L)
#define XCOM_S_FALSE                        XCOM_HRESULT_TYPEDEF(0x1L)

// FAILED
#define XCOM_E_UNEXPECTED                   XCOM_HRESULT_TYPEDEF(0x8000FFFFL)
#define XCOM_E_NOTIMPL                      XCOM_HRESULT_TYPEDEF(0x80004001L)
#define XCOM_E_OUTOFMEMORY                  XCOM_HRESULT_TYPEDEF(0x8007000EL)
#define XCOM_E_INVALIDARG                   XCOM_HRESULT_TYPEDEF(0x80070057L)
#define XCOM_E_NOINTERFACE                  XCOM_HRESULT_TYPEDEF(0x80004002L)
#define XCOM_E_POINTER                      XCOM_HRESULT_TYPEDEF(0x80004003L)
#define XCOM_E_ABORT                        XCOM_HRESULT_TYPEDEF(0x80004004L)
#define XCOM_E_FAIL                         XCOM_HRESULT_TYPEDEF(0x80004005L)
#define XCOM_E_NOAGGREGATION                XCOM_HRESULT_TYPEDEF(0x80040110L)

#endif // __XCOMERROR_H__