#ifndef __MEMPDBREADER_H__
#define __MEMPDBREADER_H__

#include "game/ht.h"
#include "mpshared/pdbreader.h"

namespace wi {

//
// Reads pdbs from memory
//

class MemPdbReader : public PdbReader
{
public:
	MemPdbReader() secPackFile;
	~MemPdbReader() secPackFile;
	bool Open(char *pszFn) secPackFile;

	virtual void Close() secPackFile;
	virtual bool GetRecordSize(word nRec, word *pcb) secPackFile;
	virtual bool ReadRecord(word nRec, word n, word cb, void *pv) secPackFile;
	virtual byte *MapRecord(word nRec, void **ppvCookie, word *pcb = NULL) secPackFile;
	virtual void UnmapRecord(word nRec, void *pvCookie) secPackFile;

private:
	bool GetRecordEntry(word nRec, int cRec, RecordEntryType *prece, dword *pcb) secPackFile;
	dword GetCompressionHeader(word nRec, CompressionHeader *pcoh) secPackFile;

	dword m_cb;
	byte *m_pb;
	word m_cRecs;
	word m_cMapped;
	CacheHandle *m_aphcRecordData;
};

} // namespace wi

#endif // __MEMPDBREADER_H__
