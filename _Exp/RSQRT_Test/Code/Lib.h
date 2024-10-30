// RSQRT_Test

#ifndef __RSQRT_LIB_H__
#define __RSQRT_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER >= 1300 && !defined(_WIN64)
#define LIB_INCLUDE_ASM 1
#endif

#if !defined(LIB_INCLUDE_ASM)
#define LIB_INCLUDE_ASM 0
#endif

typedef float (_cdecl * LibFnPtr)(float);

float _cdecl Q_rsqrt_clib( float number );
float _cdecl Q_rsqrt_newton( float number );

#if LIB_INCLUDE_ASM
float _cdecl Q_rsqrt_VS6_clib(float number);
float _cdecl Q_rsqrt_VS6_newton(float number);
float _cdecl Q_rsqrt_VS2003_clib(float number);
float _cdecl Q_rsqrt_VS2003_newton(float number);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif __RSQRT_LIB_H__
