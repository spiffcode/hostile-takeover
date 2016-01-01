#ifndef __PDBREADER_H__
#define __PDBREADER_H__

#include "inc/basictypes.h"

namespace wi {

// Used to read palm databases. They can be stored in various ways for
// example in the Palm database manager (includes Springboard modules),
// in the Palm DS card (streamed FAT filesystem), or in the Win32
// filesystem (streamed or mapped).

class PdbReader
{
public:
	virtual ~PdbReader() {}
	virtual void Close() = 0;
	virtual bool GetRecordSize(word nRec, word *pcb) = 0;
	virtual bool ReadRecord(word nRec, word n, word cb, void *pv) = 0;
	virtual byte *MapRecord(word nRec, void **ppvCookie, word *pcb = NULL) = 0;
	virtual void UnmapRecord(word nRec, void *pvCookie) = 0;
};

} // namespace wi

#endif // __PDBREADER_H__
