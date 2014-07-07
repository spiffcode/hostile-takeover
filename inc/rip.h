#ifndef __RIP_H__
#define __RIP_H__

namespace wi {

#if defined(_DEBUG) || defined(DEBUG)

// For Palm we need to specify the section these functions will end up in
// or bad things will happen when Assert is used in inline functions/methods.

#if defined(__GNUC__) && defined(__CPU_68K)
#define secRip __attribute__((section("code2")))
#define secCode14 __attribute__((section("code14")))
typedef char TCHAR;
#define TEXT(str) str
#else
#define secRip
#define secCode14
#endif

// These are macroized solely so they can be redefined. This is useful
// if the below macros are going to be used in helper functions, where you
// want the file and line of the caller and not the helper function.

#define __XYZFILE __FILE__
#define __XYZLINE __LINE__

//
// The below macros constitute system support for asserts, warnings,
// bad param reporting, and rips.
//
// The macros are:
//
//   Assert(f, ...)
//      Standard assertion macro.  If present, the second parameter is a
//      sprintf format string.

// These globals need to be defined for rip to work.
    
extern char *gpszRipFile;
extern int giRipLine;

void DoAssertRip(int fAssert, char *psz, ...) secRip;
void DoAssertRip(int fAssert) secRip;
void DoAssertRip(char *psz, ...) secRip;
void DoAssertRip() secRip;
void Break() secRip;
void dprintf(const char *psz, ...);

#define Assert   wi::gpszRipFile = __XYZFILE, wi::giRipLine = __XYZLINE, ::wi::DoAssertRip

#else // !defined(_DEBUG) && !defined(DEBUG)

// This hack allows us to create varargs macros that compile to nothing
// on release.

inline void DoConditionalRip(int, ...) { }
inline void DoConditionalRip(char *psz, ...) { }
inline void DoConditionalRip() { }
inline void Break() { }
inline void dprintf(const char *psz, ...) { }

#define Assert 1 ? (void)0 : ::wi::DoConditionalRip

#endif // defined(_DEBUG) || defined(DEBUG)

#define CompileAssert(expr) typedef char __COMPILE_ASSERT__[(expr) ? 1 : -1]

} // namespace wi
    
#endif // __RIP_H__
