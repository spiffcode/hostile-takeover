//
// Reads pdbs from a filesystem in a streamed manner
//
// Note this implementation has issues on removeable media on PocketPCs. Remove the media and
// PocketPC can't require a mount sometimes, causing the game to crash.
//

class FilePdbReader : public PdbReader
{
public:
	FilePdbReader() secPackFile;
	~FilePdbReader() secPackFile;
	bool Open(char *pszFn) secPackFile;

	virtual void Close() secPackFile;
	virtual bool GetRecordSize(word nRec, word *pcb) secPackFile;
	virtual bool ReadRecord(word nRec, word n, word cb, void *pv) secPackFile;
	virtual byte *MapRecord(word nRec, dword *pdwCookie, word *pcb = NULL) secPackFile;
	virtual void UnmapRecord(word nRec, dword dwCookie) secPackFile;

private:
	bool GetRecordEntry(word nRec, int cRec, RecordEntryType *prece, dword *pcb) secPackFile;
	bool GetCompressionHeader(word nRec, CompressionHeader *pcoh) secPackFile;

	FILE *m_pfil;
	word m_cRecs;
	word m_cMapped;
	CacheHandle *m_aphcRecordData;
};

FilePdbReader::FilePdbReader()
{
	m_pfil = NULL;
	m_cRecs = 0;
	m_cMapped = 0;
	m_aphcRecordData = NULL;
}

FilePdbReader::~FilePdbReader()
{
	Assert(m_cMapped == 0);
	Assert(m_pfil == NULL);
	Assert(m_aphcRecordData == NULL);
}

bool FilePdbReader::Open(char *pszFn)
{
	// Attempt to open

	Assert(m_pfil == NULL);
	m_pfil = fopen(pszFn, "rb");
	if (m_pfil == NULL)
		return false;

	// First read in the header

	DatabaseHdrType hdr;
	int cbHdr = sizeof(hdr) - sizeof(hdr.recordList.firstEntry);
	if (fread(&hdr, cbHdr, 1, m_pfil) != 1) {
		fclose(m_pfil);
		m_pfil = NULL;
		return false;
	}
	m_cRecs = BigWord(hdr.recordList.numRecords);

	// Alloc cache handle array

	m_aphcRecordData = new CacheHandle[m_cRecs];
	if (m_aphcRecordData == NULL) {
		fclose(m_pfil);
		m_pfil = NULL;
		return false;
	}
	memset(m_aphcRecordData, 0, sizeof(CacheHandle) * m_cRecs);

	return true;
}

void FilePdbReader::Close()
{
	Assert(m_cMapped == 0);
	Assert(m_pfil != NULL);
	fclose(m_pfil);
	m_pfil = NULL;
	delete m_aphcRecordData;
}

bool FilePdbReader::GetRecordSize(word nRec, word *pcb)
{
	CompressionHeader coh;
	if (!GetCompressionHeader(nRec, &coh))
		return false;
	*pcb = BigWord(coh.cbUncompressed);
	return true;
}

bool FilePdbReader::GetRecordEntry(word nRec, int cRec, RecordEntryType *prece, dword *pcb)
{
	// Valid?

	Assert(cRec != 0);
	Assert(nRec + cRec <= m_cRecs);
	if (nRec + cRec > m_cRecs)
		return false;

	// Seek to record header offset

	DatabaseHdrType hdr;
	hdr;
	dword off = (sizeof(hdr) - sizeof(hdr.recordList.firstEntry)) + nRec * sizeof(RecordEntryType);
	if (fseek(m_pfil, off, SEEK_SET) != 0)
		return false;

	// End of records case?
	
	if (nRec + cRec == m_cRecs) {
		if (fread(prece, sizeof(RecordEntryType) * cRec, 1, m_pfil) != 1)
			return false;
		fseek(m_pfil, 0, SEEK_END);
		if (pcb != NULL)
			*pcb = ftell(m_pfil) - BigDword(prece[cRec - 1].localChunkID);
		return true;
	}

	// 1 record case and not end of records

	if (cRec == 1) {
		RecordEntryType arece[2];
		if (fread(arece, sizeof(RecordEntryType) * 2, 1, m_pfil) != 1)
			return false;
		*prece =  arece[0];
		if (pcb != NULL)
			*pcb = BigDword(arece[1].localChunkID) - BigDword(arece[0].localChunkID);
		return true;
	}

	// N record case and not end of records

	if (fread(prece, sizeof(RecordEntryType) * cRec, 1, m_pfil) != 1)
		return false;
	if (pcb != NULL)
		*pcb = BigDword(prece[cRec - 1].localChunkID) - BigDword(prece[0].localChunkID);
	return true;
}

bool FilePdbReader::GetCompressionHeader(word nRec, CompressionHeader *pcoh)
{
	dword cbRecord;
	RecordEntryType rece;
	if (!GetRecordEntry(nRec, 1, &rece, &cbRecord))
		return false;
	if (fseek(m_pfil, BigDword(rece.localChunkID), SEEK_SET) != 0)
		return false;
	if (fread(pcoh, sizeof(*pcoh), 1, m_pfil) != 1)
		return false;
	return true;
}

bool FilePdbReader::ReadRecord(word nRec, word n, word cb, void *pv)
{
	// See if the data is cached

	Assert(nRec < m_cRecs);
	CacheHandle hc = m_aphcRecordData[nRec];
	if (!gcam.IsValid(hc)) {
		// Not cached, get the compression header

		CompressionHeader coh;
		if (!GetCompressionHeader(nRec, &coh))
			return false;

		// If not compressed, read data straight from file

		if (!BigWord(coh.fCompressed)) {
			if (n + cb > BigWord(coh.cbUncompressed))
				return false;
			if (fseek(m_pfil, n, SEEK_CUR) != 0)
				return false;
			if (fread(pv, cb, 1, m_pfil) != 1)
				return false;
			return true;
		}

		// It is compressed, but not cached.

		byte *pbCompressed = new byte[BigWord(coh.cbCompressed)];
		if (pbCompressed == NULL)
			return false;
		if (fread(pbCompressed, BigWord(coh.cbCompressed), 1, m_pfil) != 1) {
			delete pbCompressed;
			return false;
		}

		hc = gcam.NewObject(NULL, BigWord(coh.cbUncompressed));
		if (hc == NULL) {
			delete pbCompressed;
			return false;
		}
		m_aphcRecordData[nRec] = hc;
		DecompressToCache(pbCompressed, hc, &coh);
		delete pbCompressed;
	}

	// Read from cached data

	word cbUncompressed = gcam.GetSize(hc);
	byte *pbUncompressed = (byte *)gcam.GetPtr(hc);
	if (n + cb > cbUncompressed)
		return false;
	memcpy(pv, pbUncompressed + n, cb);
	return true;
}

byte *FilePdbReader::MapRecord(word nRec, dword *pdwCookie, word *pcb)
{
	word cbT;
	if (!GetRecordSize(nRec, &cbT))
		return NULL;
	if (pcb != NULL)
		*pcb = cbT;
	byte *pb = new byte[*pcb];
	if (pb == NULL)
		return NULL;
	if (!ReadRecord(nRec, 0, *pcb, pb)) {
		delete pb;
		return NULL;
	}
	*pdwCookie = (dword)pb;
	m_cMapped++;
	return pb;
}

void FilePdbReader::UnmapRecord(word nRec, dword dwCookie)
{
	Assert(m_cMapped > 0);
	m_cMapped--;
	byte *pb = (byte *)dwCookie;
	delete pb;
}

