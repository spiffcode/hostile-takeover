#ifndef __INI_H__
#define __INI_H__

#include "inc/basictypes.h"
#include "mpshared/packfile.h"
#include <stdarg.h>

// Don't leave this on all the time because IniScanf uses
// some non-standard format strings.
#if 0 // defined(__GNUC__) || defined(__clang__)
#define scanfFormat23 __attribute__((format(scanf, 2, 3)))
#define scanfFormat34 __attribute__((format(scanf, 3, 4)))
#define scanfFormat45 __attribute__((format(scanf, 4, 5)))
#else
#define scanfFormat23
#define scanfFormat34
#define scanfFormat45
#endif

namespace wi {

struct IniSection // sec
{
	word offNext;
	word cprops;
	// char szName[];
};

struct IniProperty // prop
{
	char *pszProp;
	char *pszValue;
};

class FindProp // find
{
public:
	FindProp()
	{
		m_psecFind = NULL;
		m_ipropFind = 0;
	}
	IniSection *m_psecFind;
	int m_ipropFind;

	void Assign(FindProp *pfind) {
		m_psecFind = pfind->m_psecFind;
		m_ipropFind = pfind->m_ipropFind;
	}
};

class IniReader // ini
{
public:
	IniReader(PackFileReader& pak);
	~IniReader();

	bool Init(const char *psz);
	bool GetPropertyValue(char *pszSec, char *pszProp, char *pszValue, int cbValue);
	bool FindNextProperty(FindProp *pfind, char *pszSec, char *pszProp, int cbProp);
	int GetPropertyValue(char *pszSec, char *pszProp, char *pszFmt, ...) scanfFormat45;
	int GetPropertyValue(FindProp *pfind, char *pszFmt, ...) scanfFormat34;

private:
	IniSection *FindSection(char *pszSec);
	bool FindProperty(IniSection *psec, char *pszProp, IniProperty *pprop);
	bool FindProperty(IniSection *psec, int iprop, IniProperty *pprop);

    PackFileReader& m_pak;
	IniSection *m_psecFirst;
	FileMap m_fmap;
};

IniReader *LoadIniFile(PackFileReader& pak, const char *pszFn);
int IniScanf(char *pszBuff, char *pszFmt, ...) scanfFormat23;

} // namespace wi;

#endif // __INI_H__
