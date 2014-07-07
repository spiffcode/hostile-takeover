#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include "ini.h"

// IniScanf / VIniScanf
//
// Supports:
// %s : "trimmed string", matches a string with interleaving spaces. Removes
//		leading and trailing white space, with ini-friendly delimiters
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

bool NewSection(char *pszSec, dword nch, IniSection **ppsec, int *pcsec)
{
	IniSection *psec = new IniSection[(*pcsec) + 1];
	if (psec == NULL)
		return false;
	if (*ppsec != NULL) {
		memcpy(psec, *ppsec, sizeof(IniSection) * (*pcsec));
		delete *ppsec;
	} 
	*ppsec = psec;
	psec = &(*ppsec)[*pcsec];
	(*pcsec)++;

	psec->nchSec = nch;
	psec->cchSec = strlen(pszSec);
	psec->cprop = 0;
	psec->pprop = NULL;
	return true;
}
	
bool NewProperty(IniSection *psec, char *pszProp, dword nchProp, char *pszValue, dword nchValue)
{
	IniProperty *pprop = new IniProperty[psec->cprop + 1];
	if (pprop == NULL)
		return false;
	if (psec->pprop != NULL) {
		memcpy(pprop, psec->pprop, sizeof(IniProperty) * psec->cprop);
		delete psec->pprop;
	}
	psec->pprop = pprop;
	pprop = &psec->pprop[psec->cprop];
	psec->cprop++;

	pprop->nchProp = nchProp;
	pprop->cchProp = strlen(pszProp);
	pprop->nchValue = nchValue;
	pprop->cchValue = strlen(pszValue);
	return true;
}

IniSection *LoadIniFile(char *pszFn, int *pcSections)
{
	IniSection *psec = NULL;
	int csec = 0;

	FILE *pf = fopen(pszFn, "rb");
	if (pf == NULL)
		return false;

	while (true) {
		// Get the next line

		dword nchLine = ftell(pf);
		char szLine[50 * 1024];
		if (fgets(szLine, sizeof(szLine) - 1, pf) == NULL)
			break;

		// Is it a section header? If so, add it

		char szT[1024];
		int nchSec;
		if (IniScanf(szLine, "\t[%s+]\t", szT, &nchSec) == 2) {
			if (!NewSection(szT, nchLine + nchSec, &psec, &csec))
				return NULL;
			continue;
		}

		// Is it just whitespace? Allow leading ; for comments

		bool fAllWhiteSpace = true;
		int cch = strlen(szLine);
		for (int n = 0; n < cch; n++) {
			if (!IsWhiteSpace(szLine[n])) {
				if (szLine[n] == ';')
					break;
				fAllWhiteSpace = false;
				break;
			}
		}
		if (fAllWhiteSpace)
			continue;

		// See if it is property = value

		int nchProp;
		int nchValue;
		char szValue[50 * 1024];
		if (IniScanf(szLine, "%s+=%s+", szT, &nchProp, szValue, &nchValue) == 4) {
			if (!NewProperty(&psec[csec - 1], szT, nchLine + nchProp, szValue, nchLine + nchValue))
				return NULL;
			continue;
		}

		// Otherwise ignore it
	}

	*pcSections = csec;
	return psec;
}

