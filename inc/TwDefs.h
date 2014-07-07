/* Copyright (c) 2002-2003 Tapwave, Inc. All rights reserved. */

#ifndef __TWDEFS_H__
#define __TWDEFS_H__

/*
 * Define common preprocessor variables for existing 68K code.
 */
#ifndef EMULATION_NONE
#define EMULATION_NONE          0       /* native environment   */
#define EMULATION_WINDOWS       1       /* emulate on Windows   */
#define EMULATION_DOS           2       /* emulate on DOS       */
#define EMULATION_MAC           3       /* emulate on Macintosh */
#define EMULATION_UNIX          4       /* emulate on Linux     */
#endif

#ifndef EMULATION_LEVEL
#define EMULATION_LEVEL         EMULATION_NONE
#endif

#ifndef CPU_x86
#define CPU_x86                 1       /* x86 type             */
#endif

#ifndef CPU_ARM
#define CPU_ARM                 3       /* ARM type             */
#endif

#ifdef  __arm
#undef  CPU_TYPE
#define CPU_TYPE                CPU_ARM
#endif

#ifdef  _MSC_VER
#undef  CPU_TYPE
#define CPU_TYPE                CPU_x86
#endif

/*
 * Special processor variable for cross compilation 16-bit/32-bit app.
 */
#ifndef __palm__
#if CPU_TYPE == CPU_68K
#define __palm__                0x0400
#else
#define __palm__                0x0500
#endif
#endif

/*
 * Special compiler option for armlet callback from PalmOS.
 */
#if defined(__PALMOS_ARMLET__)
  #if defined(__MWERKS__) && defined(__arm)
    #define ARMLET_CALLBACK     __declspec(armlet_callback)
  #elif defined(_MSC_VER) && defined(_WIN32)
    #define ARMLET_CALLBACK     __declspec(dllexport)
  #endif
#endif /* __PALMOS_ARMLET__ */

#ifndef ARMLET_CALLBACK
#define ARMLET_CALLBACK         /* nothing */
#endif

/*
 * For unkonwn reason, this resource type is missing from ARM headers.
 */
#ifndef iconType
#define iconType                'tAIB'
#endif

#if !defined(inline) && !defined(__cplusplus)

#if defined(__arm)
    #define inline              __inline
#elif defined(_MSC_VER)
    #define inline              __inline
#elif defined(__MWERKS__)
     #define inline             inline
#else
     #define inline             static
#endif

#endif /* !defined(inline) && !defined(__cplusplus) */

#ifndef UNUSED
#define UNUSED(v)               if (v) {}
#endif

/*
 * Additional data types for writing portable code.
 */
#if defined(_MSC_VER)
typedef signed __int64          Int64;
typedef unsigned __int64        UInt64;
#else 
typedef signed long long        Int64;
typedef unsigned long long      UInt64;
#endif

/*
 * This data type is necessary for casting between data pointers and
 * function pointers, and it is also more generic than void*.
 */
typedef signed long             IntPtr;
typedef unsigned long           UIntPtr;

#ifdef __MWERKS__
typedef signed long             ssize_t;
typedef unsigned long           size_t;
#endif

/*
 * This is the generic creator for Tapwave products, assigned by PalmSource
 */
#define twCreatorID             'Tpwv'
#define twFtrCreator            twCreatorID

/*
 * This is the PalmOS feature number for TapWave API version.
 */
#define twFtrAPIVersion         0x0000

/*
 * This is the PalmOS feature number for TapWave ARMlet Glue.
 */
#define twFtrAPIGlue            0x0001

/*
 * This is the compile-time version for TapWave API.
 */
#ifndef TAPWAVE_API_VERSION
#define TAPWAVE_API_VERSION     0x0100
#endif

#endif /* __TWDEFS_H__ */
