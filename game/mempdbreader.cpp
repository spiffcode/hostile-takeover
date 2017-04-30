#include "game/mempdbreader.h"

namespace wi {

MemPdbReader::MemPdbReader()
{
	m_cb = 0;
	m_pb = NULL;
	m_cRecs = 0;
	m_cMapped = 0;
	m_aphcRecordData = NULL;
}

MemPdbReader::~MemPdbReader()
{
	Assert(m_pb == NULL);
	Assert(m_cMapped == 0);
	Assert(m_aphcRecordData == NULL);
}

bool MemPdbReader::Open(char *pszFn)
{
	// Attempt to open

	Assert(m_pb == NULL);
    FileHandle hf = HostOpenFile(pszFn, kfOfRead);
	if (hf == NULL) {
        Trace("fopen(\"%s\", \"rb\"); failed", pszFn);
		return false;
    }

	// Read in the entire thing

    HostSeekFile(hf, 0, kfSeekEnd);
    m_cb = HostTellFile(hf);
    HostSeekFile(hf, 0, kfSeekSet);

	m_pb = new byte[m_cb];
	if (m_pb == NULL) {
		HostCloseFile(hf);
		return false;
	}
	if (HostReadFile(m_pb, m_cb, 1, hf) != 1) {
		HostCloseFile(hf);
		return false;
	}
	HostCloseFile(hf);

	// Alloc cache handle array

	DatabaseHdrType *phdr = (DatabaseHdrType *)m_pb;
	m_cRecs = BigWord(phdr->recordList.numRecords);
	m_aphcRecordData = new CacheHandle[m_cRecs];
	if (m_aphcRecordData == NULL)
		return false;
	memset(m_aphcRecordData, 0, sizeof(CacheHandle) * m_cRecs);

	return true;
}

void MemPdbReader::Close()
{
	Assert(m_cMapped == 0);
	Assert(m_pb != NULL);
	delete[] m_pb;
	m_pb = NULL;
	delete[] m_aphcRecordData;
	m_aphcRecordData = NULL;
}

bool MemPdbReader::GetRecordSize(word nRec, word *pcb)
{
	CompressionHeader coh;
	if (GetCompressionHeader(nRec, &coh) == 0)
		return false;
	*pcb = BigWord(coh.cbUncompressed);
	return true;
}

bool MemPdbReader::GetRecordEntry(word nRec, int cRec, RecordEntryType *prece, dword *pcb)
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
		if (pcb != NULL)
			*pcb = m_cb - BigDword(prece[cRec - 1].localChunkID);
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
	if (pcb != NULL)
		*pcb = BigDword(prece[cRec - 1].localChunkID) - BigDword(prece[0].localChunkID);
	return true;
}

dword MemPdbReader::GetCompressionHeader(word nRec, CompressionHeader *pcoh)
{
	dword cbRecord;
	RecordEntryType rece;
	if (!GetRecordEntry(nRec, 1, &rece, &cbRecord))
		return 0;
	memcpy(pcoh, &m_pb[BigDword(rece.localChunkID)], kcbCompressionHeader);
	return BigDword(rece.localChunkID) + kcbCompressionHeader;
}

bool MemPdbReader::ReadRecord(word nRec, word n, word cb, void *pv)
{
	// See if the data is cached

	Assert(nRec < m_cRecs);
	CacheHandle hc = m_aphcRecordData[nRec];
	if (!gcam.IsValid(hc)) {
		// Not cached, get the compression header

		CompressionHeader coh;
		dword offBytes = GetCompressionHeader(nRec, &coh);
		if (offBytes == 0)
			return false;

		// If not compressed, read data straight from file

		if (!BigWord(coh.fCompressed)) {
			if (n + cb > BigWord(coh.cbUncompressed))
				return false;
			memcpy(pv, &m_pb[offBytes + n], cb);
			return true;
		}

		// It is compressed, but not cached.

		hc = gcam.NewObject(NULL, BigWord(coh.cbUncompressed));
		if (hc == 0)
			return false;
		m_aphcRecordData[nRec] = hc;
		DecompressToCache(&m_pb[offBytes], hc, &coh);
	}

	// Read from cached data

	word cbUncompressed = gcam.GetSize(hc);
	byte *pbUncompressed = (byte *)gcam.GetPtr(hc);
	if (n + cb > cbUncompressed)
		return false;
	memcpy(pv, pbUncompressed + n, cb);
	return true;
}

byte *MemPdbReader::MapRecord(word nRec, void **ppvCookie, word *pcb)
{
	// First see if the record is cached

	Assert(m_pb != NULL);
	Assert(nRec < m_cRecs);
	CacheHandle hc = m_aphcRecordData[nRec];
	if (!gcam.IsValid(hc)) {
		// Not cached, get the compression header

		CompressionHeader coh;
		dword offBytes = GetCompressionHeader(nRec, &coh);
		if (offBytes == 0)
			return NULL;

		// If it is not compressed, we can't just point to it; it may
		// be unaligned

		if (!BigWord(coh.fCompressed)) {
			word cbT = BigWord(coh.cbUncompressed);
			byte *pbT = new byte[cbT];
			if (pbT == NULL)
				return NULL;
			memcpy(pbT, &m_pb[offBytes], cbT);
			m_cMapped++;
			*ppvCookie = pbT;
			*pcb = cbT;
			return pbT;
		}

		// It is compressed, decompress it and lock it

		hc = gcam.NewObject(NULL, BigWord(coh.cbUncompressed), kfHintWillLock);
		if (hc == 0)
			return NULL;
		m_aphcRecordData[nRec] = hc;
		DecompressToCache(&m_pb[offBytes], hc, &coh);
	}

	// We have cache access to this record and it is locked. Return a pointer.

	byte *pbUncompressed = (byte *)gcam.Lock(hc);
	if (pcb != NULL)
		*pcb = gcam.GetSize(hc);
	*ppvCookie = 0;
	m_cMapped++;
	return pbUncompressed;
}

void MemPdbReader::UnmapRecord(word nRec, void *pvCookie)
{
	Assert(m_pb != NULL);
	Assert(nRec < m_cRecs);
	Assert(m_cMapped > 0);
	m_cMapped--;
	if (pvCookie == NULL) {
		// Pointed to cache object; unlock it

		if (m_aphcRecordData[nRec] != 0) {
			// It was compressed and cached so unlock the cache handle

			gcam.Unlock(m_aphcRecordData[nRec]);
		}
	} else {
		// Pointed to alloced object; unlock it

		byte *pb = (byte *)pvCookie;
		delete[] pb;
	}
}

} // namespace wi
