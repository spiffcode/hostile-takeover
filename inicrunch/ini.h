typedef unsigned long dword;
typedef unsigned char byte;
typedef unsigned short word;
#define NULL 0

#include <stdarg.h>

struct IniProperty // prop
{
	dword nchProp;
	int cchProp;
	word hashProp;
	dword nchValue;
	int cchValue;
};

struct IniSection // sec
{
	dword nchSec;
	int cchSec;
	word hashSec;
	int cprop;
	IniProperty *pprop;
};

IniSection *LoadIniFile(char *pszFn, int *pcSections);
int IniScanf(char *pszBuff, char *pszFmt, ...);
int VIniScanf(char *pszBuff, char *pszFmt, va_list va);
