#include "mpshared/ini.h"
#include "mpshared/misc.h"
#include "inc/rip.h"
#include <stdlib.h>
#include <string.h>

namespace wi {

#undef BigWord
#define BigWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))

// IniScanf / VIniScanf
//
// Supports:
// %s : "trimmed string", matches a string with interleaving spaces. Removes
//		leading and trailing white space, with ini-friendly delimiters
// %d : signed integer in the range of +32,767 / -32,768. Leading '.' & trailing
//      '%' ignored for convenience of percentages like .xx and xx%.
// %ld : signed integer in the range of +2,147,483,647 / -2,147,483,648. Leading '.' 
//      & trailing '%' ignored for convenience of percentages like .xx and xx%.
// %a : "argument group", matches a curly-brace enclosed string that can include
//      any character (e.g., commas)
// %s+: means %s plus store index of matching string
// %+ : means store current buffer index
// '*' after % is supported and means match but don't store
// ' ', '\t', '\n' : match for optional white space (space, tab, newline)

#define IsWhiteSpace(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r')
#ifdef IsDigit
#undef IsDigit
#endif
#define IsDigit(ch) ((ch) >= '0' && (ch) <= '9')

int IniScanf(char *pszBuff, char *pszFmt, ...)
{
	va_list va;
	va_start(va, pszFmt);
	int c = VIniScanf(pszBuff, pszFmt, va);
	va_end(va);
	return c;
}

int VIniScanf(char *pszBuff, char *pszFmt, va_list va)
{
	bool fLong;
	char *pszBuffT = pszBuff;
	char *pszFmtT = pszFmt;
	int cArgs = 0;

	while (*pszFmtT != 0) {
		// Make this special check before checking for end
		// of buffer

		if (pszFmtT[0] == '%' && pszFmtT[1] == '+') {
			pszFmtT += 2;
			*va_arg(va, int *) = pszBuffT - pszBuff;
			cArgs++;
			continue;
		}

		switch (*pszFmtT) {
		case ' ':
		case '\n':
		case '\t':
			// Eat white space (space, tab, newline)

			while (IsWhiteSpace(*pszBuffT))
				pszBuffT++;
			pszFmtT++;
			continue;
		}

		// End of buffer? Bail.

		if (*pszBuffT == 0)
			break;

		bool fSuppress;
		switch (*pszFmtT) {
		case '%':
			// Assume parsed ints are not longs

			fLong = false;

			// Suppress?

			pszFmtT++;
			fSuppress = false;
			if (*pszFmtT == '*') {
				fSuppress = true;
				pszFmtT++;
			}

			// Format specification

			switch (*pszFmtT) {
			case 's':
				// Trimmed string.
				{
					pszFmtT++;

					// If %s+, it means save the location of substring

					bool fSaveLocation = false;
					if (*pszFmtT == '+') {
						fSaveLocation = true;
						pszFmtT++;
					}

					// Go past initial whitespace

					int nchStart = pszBuffT - pszBuff;
					while (IsWhiteSpace(*pszBuffT)) {
						nchStart++;
						pszBuffT++;
					}

					// Copy the string into pszT. Figure out when to stop
					// and remember trimming information.

					char chStop = *pszFmtT;	
					char *pszT = va_arg(va, char *);
					char *pszWhiteLast = NULL;
					while (*pszBuffT != chStop) {
						if (*pszBuffT == 0)
							break;
						if (!IsWhiteSpace(*pszBuffT)) {
							pszWhiteLast = NULL;
						} else if (pszWhiteLast == NULL) {
							pszWhiteLast = pszT;
						}
						if (!fSuppress)
							*pszT++ = *pszBuffT++;
					}
					if (!fSuppress) {
						*pszT = 0;
						if (pszWhiteLast != NULL)
							*pszWhiteLast = 0;
						cArgs++;
					}

					// %s+ means save location string started

					if (!fSuppress && fSaveLocation) {
						*va_arg(va, int *) = nchStart;
						cArgs++;
					}
				}
				continue;

			case 'a':
				// Argument group.
				{
					pszFmtT++;

					// Go past opening curly brace

					Assert(*pszBuffT == '{');
					pszBuffT++;

					// Copy the string into pszT. Figure out when to stop
					// and remember trimming information.

					char *pszT = va_arg(va, char *);
					while (*pszBuffT != '}') {
						if (*pszBuffT == 0)
							break;
						if (!fSuppress)
							*pszT++ = *pszBuffT++;
					}
					if (!fSuppress) {
						*pszT = 0;
						cArgs++;
					}

					// Go past closing curly brace

					pszBuffT++;
				}
				continue;

			case 'l':
				fLong = true;
				pszFmtT++;
				Assert(*pszFmtT == 'd');

				// ... FALL THROUGH ...

			case 'd':
                // Integer between 32767 / -32768. Leading . and trailing %
                // ignored.  NOTE: this precision limitation is not enforced.
                // Parsing a number greater than 32767 will succeed on Win32
                // but fail on Palm due to differing int sizes.
                { 
                    pszFmtT++;

					while (IsWhiteSpace(*pszBuffT))
						pszBuffT++;
					if (*pszBuffT == '.')
						pszBuffT++;
					char szT[11];
					int n = 0;
					if (*pszBuffT == '-') {
						szT[0] = '-';
						n = 1;
						pszBuffT++;
					}
					bool fHaveInt = false;
					for (; n < sizeof(szT) - 1; n++) {
						if (!IsDigit(*pszBuffT))
							break;
						szT[n] = *pszBuffT++;
						fHaveInt = true;
					}
					if (fHaveInt) {
						szT[n] = 0;
						if (*pszBuffT == '%')
							pszBuffT++;
						if (!fSuppress) {
							if (fLong)
								*va_arg(va, long *) = atol(szT);
							else
								*va_arg(va, int *) = atoi(szT);
							cArgs++;
						}
					}
				}
				continue;

			default:
				// Non-recognized % command

				return cArgs;
			}
			break;

		default:
			// Match character

			if (*pszBuffT == *pszFmtT) {
				pszBuffT++;
				pszFmtT++;
				continue;
			}
			break;
		}

		// If we're here it means stop

		break;
	}

	return cArgs;
}

IniReader *LoadIniFile(PackFileReader& pak, const char *pszFn)
{
	IniReader *pini = new IniReader(pak);
	if (pini == NULL)
		return NULL;
	if (!pini->Init(pszFn)) {
		delete pini;
		return NULL;
	}
	return pini;
}

// IniReader

IniReader::IniReader(PackFileReader& pak) : m_pak(pak), m_psecFirst(NULL)
{
}

IniReader::~IniReader()
{
	if (m_psecFirst != NULL)
		m_pak.UnmapFile(&m_fmap);
}

bool IniReader::Init(const char *psz)
{
	m_psecFirst = (IniSection *)m_pak.MapFile(psz, &m_fmap);
	return m_psecFirst != NULL;
}

bool IniReader::GetPropertyValue(char *pszSec, char *pszProp, char *pszValue, int cbValue)
{
	IniSection *psec = FindSection(pszSec);
	if (psec == NULL)
		return false;
	IniProperty prop;
	if (!FindProperty(psec, pszProp, &prop))
		return false;
	strncpyz(pszValue, prop.pszValue, cbValue);
	return true;
}

int IniReader::GetPropertyValue(char *pszSec, char *pszProp, char *pszFmt, ...)
{
	IniSection *psec = FindSection(pszSec);
	if (psec == NULL)
		return 0;
	IniProperty prop;
	if (!FindProperty(psec, pszProp, &prop))
		return 0;
	va_list va;
	va_start(va, pszFmt);
	int c = VIniScanf(prop.pszValue, pszFmt, va);
	va_end(va);
	return c;
}

bool IniReader::FindNextProperty(FindProp *pfind, char *pszSec, char *pszProp, int cbProp)
{
	if (pfind->m_psecFind == NULL) {
		pfind->m_psecFind = FindSection(pszSec);
		if (pfind->m_psecFind == NULL)
			return false;
		pfind->m_ipropFind = 0;
	} else {
		pfind->m_ipropFind++;		
		if (pfind->m_ipropFind >= BigWord(pfind->m_psecFind->cprops))
			return false;
	}

	IniProperty prop;
	if (!FindProperty(pfind->m_psecFind, pfind->m_ipropFind, &prop))
		return false;
	strncpyz(pszProp, prop.pszProp, cbProp);
	return true;
}

int IniReader::GetPropertyValue(FindProp *pfind, char *pszFmt, ...)
{
	IniProperty prop;
	if (!FindProperty(pfind->m_psecFind, pfind->m_ipropFind, &prop))
		return 0;
	va_list va;
	va_start(va, pszFmt);
	int c = VIniScanf(prop.pszValue, pszFmt, va);
	va_end(va);
	return c;
}

IniSection *IniReader::FindSection(char *pszSec)
{
	IniSection *psec = m_psecFirst;
	while (psec != NULL) {
		if (strcmp(pszSec, (char *)(psec + 1)) == 0)
			return psec;
		if (psec->offNext == 0)
			return NULL;
		psec = (IniSection *)((byte *)psec + BigWord(psec->offNext));
	}
	return NULL;
}

bool IniReader::FindProperty(IniSection *psec, char *pszProp, IniProperty *pprop)
{
	char *pszPropT = (char *)(psec + 1) + strlen((char *)(psec + 1)) + 1;
	for (int iprop = 0; iprop < BigWord(psec->cprops); iprop++) {
		if (strcmp(pszPropT, pszProp) == 0) {
			pprop->pszProp = pszPropT;
			pprop->pszValue = pszPropT + strlen(pszPropT) + 1;
			return true;
		}

		pszPropT += strlen(pszPropT) + 1;
		pszPropT += strlen(pszPropT) + 1;
	}
	return false;
}

bool IniReader::FindProperty(IniSection *psec, int iprop, IniProperty *pprop)
{
	char *pszPropT = (char *)(psec + 1) + strlen((char *)(psec + 1)) + 1;
	for (int ipropT = 0; ipropT < BigWord(psec->cprops); ipropT++) {
		if (ipropT == iprop) {
			pprop->pszProp = pszPropT;
			pprop->pszValue = pszPropT + strlen(pszPropT) + 1;
			return true;
		}

		pszPropT += strlen(pszPropT) + 1;
		pszPropT += strlen(pszPropT) + 1;
	}
	return false;
}

} // namespace wi
