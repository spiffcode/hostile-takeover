// stringtable.cpp
#include "ht.h"

namespace wi {

StringTable *gpstrtbl;

StringTable::StringTable() 
{
	m_pfil = NULL;
}

StringTable::~StringTable()
{
	if (m_pfil != NULL)
		gpakr.fclose(m_pfil);
}

bool StringTable::Init(char *pszFilename)
{
	m_pfil = gpakr.fopen(pszFilename, "rb");
	return m_pfil != NULL;
}

bool StringTable::GetString(int id, char *psz, int cb)
{
	Assert(m_pfil != NULL);

	// get index of string

	if (gpakr.fseek(m_pfil, 2 * id, SEEK_SET) != 0)
		return false;
	word off;
	if (gpakr.fread(&off, sizeof(word), 1, m_pfil) != 1)
		return false;
	off = BigWord(off);
	
	// get length
	if (gpakr.fseek(m_pfil, off, SEEK_SET) != 0)
		return false;
	byte cbT = 0;
	if (gpakr.fread(&cbT, sizeof(byte), 1, m_pfil) != 1)
		return false;

	if (cb < cbT)
		cbT = cb - 1;
	
	// read in bytes and zero terminate string
	if (gpakr.fread(psz, cbT, 1, m_pfil) != 1)
		return false;

	psz[cbT] = 0;
	return true;
}

} // namespace wi