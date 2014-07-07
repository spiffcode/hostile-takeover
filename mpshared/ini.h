#ifndef __INI_H__
#define __INI_H__

#include "inc/basictypes.h"
#include "mpshared/packfile.h"
#include <stdarg.h>

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
	int GetPropertyValue(char *pszSec, char *pszProp, char *pszFmt, ...);
	bool FindNextProperty(FindProp *pfind, char *pszSec, char *pszProp, int cbProp);
	int GetPropertyValue(FindProp *pfind, char *pszFmt, ...);

private:
	IniSection *FindSection(char *pszSec);
	bool FindProperty(IniSection *psec, char *pszProp, IniProperty *pprop);
	bool FindProperty(IniSection *psec, int iprop, IniProperty *pprop);

    PackFileReader& m_pak;
	IniSection *m_psecFirst;
	FileMap m_fmap;
};
IniReader *LoadIniFile(PackFileReader& pak, const char *pszFn);
int IniScanf(char *pszBuff, char *pszFmt, ...);
int VIniScanf(char *pszBuff, char *pszFmt, va_list va);

} // namespace wi;

#endif // __INI_H__
