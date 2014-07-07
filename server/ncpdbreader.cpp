#include "server/ncpdbreader.h"
#include "inc/rip.h"
#include "mpshared/misc.h"
#include "base/log.h"
#include <stdio.h>
#include <string.h>

namespace wi {

NoCachePdbReader::NoCachePdbReader()
{
	m_cb = 0;
	m_pb = NULL;
	m_cRecs = 0;
	m_cMapped = 0;
}

NoCachePdbReader::~NoCachePdbReader()
{
	Assert(m_pb == NULL);
	Assert(m_cMapped == 0);
}

bool NoCachePdbReader::Open(const char *pszFn)
{
	// Attempt to open

	Assert(m_pb == NULL);
	FILE *pfil = fopen(pszFn, "rb");
	if (pfil == NULL) {
        RLOG() << "fopen failed: " << pszFn;
		return false;
    }

	// Read in the entire thing

	fseek(pfil, 0, SEEK_END);
	m_cb = ftell(pfil);
	fseek(pfil, 0, SEEK_SET);

	m_pb = new byte[m_cb];
	if (m_pb == NULL) {
		fclose(pfil);
		return false;
	}
	if (fread(m_pb, m_cb, 1, pfil) != 1) {
		fclose(pfil);
		return false;
	}
	fclose(pfil);

	DatabaseHdrType *phdr = (DatabaseHdrType *)m_pb;
	m_cRecs = BigWord(phdr->recordList.numRecords);

	return true;
}

void NoCachePdbReader::Close()
{
	Assert(m_cMapped == 0);
	Assert(m_pb != NULL);
	delete[] m_pb;
	m_pb = NULL;
}

bool NoCachePdbReader::GetRecordSize(word nRec, word *pcb)
{
	CompressionHeader coh;
	if (GetCompressionHeader(nRec, &coh) == 0)
		return false;
	*pcb = BigWord(coh.cbUncompressed);
	return true;
}

bool NoCachePdbReader::GetRecordEntry(word nRec, int cRec,
        RecordEntryType *prece, dword *pcb)
{
	// Valid?

	Assert(cRec != 0);
	Assert(nRec + cRec <= m_cRecs);
	if (nRec + cRec > m_cRecs)
		return false;

	// Seek to record header offset

	DatabaseHdrType hdr;
	dword off = (sizeof(hdr) - sizeof(hdr.recordList.firstEntry)) + nRec * sizeof(RecordEntryType);

	// End of records case?
	
	if (nRec + cRec == m_cRecs) {
		memcpy(prece, &m_pb[off], sizeof(RecordEntryType) * cRec);
		if (pcb != NULL) {
			*pcb = m_cb - BigDword(prece[cRec - 1].localChunkID);
        }
		return true;
	}

	// 1 record case and not end of records

	if (cRec == 1) {
		RecordEntryType arece[2];
		memcpy(arece, &m_pb[off], sizeof(RecordEntryType) * 2);
		*prece =  arece[0];
		if (pcb != NULL)
			*pcb = BigDword(arece[1].localChunkID) - BigDword(arece[0].localChunkID);
		return true;
	}

	// N record case and not end of records

	memcpy(prece, &m_pb[off], sizeof(RecordEntryType) * cRec);
	if (pcb != NULL) {
		*pcb = BigDword(prece[cRec - 1].localChunkID) -
                BigDword(prece[0].localChunkID);
    }
	return true;
}

dword NoCachePdbReader::GetCompressionHeader(word nRec, CompressionHeader *pcoh)
{
	dword cbRecord;
	RecordEntryType rece;
	if (!GetRecordEntry(nRec, 1, &rece, &cbRecord))
		return 0;
	memcpy(pcoh, &m_pb[BigDword(rece.localChunkID)], kcbCompressionHeader);
	return BigDword(rece.localChunkID) + kcbCompressionHeader;
}

bool NoCachePdbReader::ReadRecord(word nRec, word n, word cb, void *pv)
{
	// See if the data is cached
	Assert(nRec < m_cRecs);
    CompressionHeader coh;
    dword offBytes = GetCompressionHeader(nRec, &coh);
    if (offBytes == 0) {
        return false;
    }

    if (n + cb > BigWord(coh.cbUncompressed)) {
        return false;
    }

    // If not compressed, read data straight from file
    if (!BigWord(coh.fCompressed)) {
        memcpy(pv, &m_pb[offBytes + n], cb);
    } else {
        byte *pbT = new byte[BigWord(coh.cbUncompressed)];
        if (pbT == NULL) {
            return false;
        }
        DecompressToMemory(&m_pb[offBytes], pbT, &coh);
        memcpy(pv, &pbT[n], cb);
        delete[] pbT;
    }
    return true;
}

byte *NoCachePdbReader::MapRecord(word nRec, dword *pdwCookie, word *pcb)
{
	// First see if the record is cached

	Assert(m_pb != NULL);
	Assert(nRec < m_cRecs);

    CompressionHeader coh;
    dword offBytes = GetCompressionHeader(nRec, &coh);
    if (offBytes == 0)
        return NULL;

    word cbT = BigWord(coh.cbUncompressed);
    byte *pbT = new byte[cbT];
    if (pbT == NULL) {
        return NULL;
    }

    if (BigWord(coh.fCompressed)) {
        DecompressToMemory(&m_pb[offBytes], pbT, &coh);
    } else {
        memcpy(pbT, &m_pb[offBytes], cbT);
    }

    m_cMapped++;
    *pdwCookie = (dword)pbT;
    *pcb = cbT;
    return pbT;
}

void NoCachePdbReader::UnmapRecord(word nRec, dword dwCookie)
{
	Assert(m_pb != NULL);
	Assert(nRec < m_cRecs);
	Assert(m_cMapped > 0);
	m_cMapped--;
    byte *pb = (byte *)dwCookie;
    delete[] pb;
}

} // namespace wi
