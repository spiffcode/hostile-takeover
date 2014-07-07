#ifndef __NCPDBREADER_H__
#define __NCPDBREADER_H__

#include "inc/basictypes.h"
#include "mpshared/pdbreader.h"
#include "mpshared/decompress.h"
#include "mpshared/packfile.h"

namespace wi {

//
// Reads pdbs from memory
//

class NoCachePdbReader : public PdbReader
{
public:
	NoCachePdbReader();
	~NoCachePdbReader();
	bool Open(const char *pszFn);

	virtual void Close();
	virtual bool GetRecordSize(word nRec, word *pcb);
	virtual bool ReadRecord(word nRec, word n, word cb, void *pv);
	virtual byte *MapRecord(word nRec, dword *pdwCookie, word *pcb = NULL);
	virtual void UnmapRecord(word nRec, dword dwCookie);

private:
	bool GetRecordEntry(word nRec, int cRec, RecordEntryType *prece,
            dword *pcb);
	dword GetCompressionHeader(word nRec, CompressionHeader *pcoh);

	dword m_cb;
	byte *m_pb;
	word m_cRecs;
	word m_cMapped;
};

} // namespace wi

#endif // __NCPDBREADER_H__
