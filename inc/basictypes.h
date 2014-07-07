#ifndef __BASICTYPES_H__
#define __BASICTYPES_H__

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned long long ddword;
typedef unsigned long long dword64;
typedef long long long64;
#undef NULL
#define NULL 0

#define ELEMENTSIZE(a) (sizeof(a[0]))
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#define OFFSETOF(t, m) ((int)&(((t *)0)->m))
    
#endif // __BASICTYPES_H__
