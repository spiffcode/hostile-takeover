#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "ini.h"

#ifndef WINDOWS
#include <limits.h>
#define _MAX_PATH PATH_MAX
#define _MAX_FNAME NAME_MAX
#define _MAX_DRIVE NAME_MAX
#define _MAX_DIR (PATH_MAX-NAME_MAX)
#define _MAX_EXT NAME_MAX
#define _unlink remove
#define _fullpath(resolved_path, path, length) realpath(path, resolved_path)

void _splitpath(const char* path, char* drv, char* dir, char* name, char* ext)
{
    const char* end; /* end of processed string */
	const char* p;	 /* search pointer */
	const char* s;	 /* copy pointer */

	/* extract drive name */
	if (path[0] && path[1]==':') {
		if (drv) {
			*drv++ = *path++;
			*drv++ = *path++;
			*drv = '\0';
		}
	} else if (drv)
		*drv = '\0';

	/* search for end of string or stream separator */
	for(end=path; *end && *end!=':'; )
		end++;

	/* search for begin of file extension */
	for(p=end; p>path && *--p!='\\' && *p!='/'; )
		if (*p == '.') {
			end = p;
			break;
		}

	if (ext)
		for(s=end; (*ext=*s++); )
			ext++;

	/* search for end of directory name */
	for(p=end; p>path; )
		if (*--p=='\\' || *p=='/') {
			p++;
			break;
		}

	if (name) {
		for(s=p; s<end; )
			*name++ = *s++;

		*name = '\0';
	}

	if (dir) {
		for(s=path; s<p; )
			*dir++ = *s++;

		*dir = '\0';
	}
}
#endif


#define kcbRecordMax 0x8000
#define SwapWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))
#define SwapDword(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | (((x)&0xFF000000)>>24))

struct RecordChunk // rck
{
	byte *pb;
	byte *pbMax;
};

struct SecChunk // sck
{
	word offSecNext;
	short cprop;
	// char szSecName[];
};

struct PropChunk // pck
{
	// char szProp[];
	// char szPropValue[];
};

bool WriteDatabase(char *pszFn, RecordChunk *arck, int cChunks, long *pcbFile);
bool WriteBinary(char *pszFn, RecordChunk *arck, int cChunks, long *pcbFile);
int CalcSectionSize(IniSection *psec);
bool ReadString(FILE *pf, int nch, int cch, byte *pb);

void Usage()
{
	printf("\n");
	printf("Usage: inicrunch infile outfile\n");
	printf("\n");
	printf("This will read the infile and\n");
	printf("output outfile which is a straight image binary\n");
	printf("\n");
	printf("Usage: inicrunch -pdb infile outfile\n");
	printf("\n");
	printf("This will read the infile and output outfile\n");
	printf("in .pdb format.\n");
	printf("\n");
	exit(1);
}

int main(int argc, char **argv)
{
	bool fPdbOnly = false;
	char *pszFnIn;
	char *pszFnOut;
	if (argc == 4 && strcmp(argv[1], "-pdb") == 0) {
		fPdbOnly = true;
		pszFnIn = argv[2];
		pszFnOut = argv[3];
	} else if (argc == 3) {
		pszFnIn = argv[1];
		pszFnOut = argv[2];
	} else {
		Usage();
	}

	// File exists?

	FILE *pf = fopen(pszFnIn, "rb");
	if (pf == NULL) {
		printf("Error opening %s for reading.\n", pszFnIn);
		exit(1);
	}

	// Parse .ini file

	int cSections;
	IniSection *psec = LoadIniFile(pszFnIn, &cSections);
	if (psec == NULL) {
		printf("Error parsing .ini file %s.\n", pszFnIn);
		fclose(pf);
		exit(1);
	}

	// Build in memory format

	RecordChunk arck[256];
	memset(arck, 0, sizeof(arck));
	int cChunks = 0;

	IniSection *psecT = psec;
	IniSection *psecMax = &psec[cSections];

	bool fDone = false;
	while (!fDone) {
		RecordChunk *prck = &arck[cChunks++];
		prck->pb = new byte[kcbRecordMax];
		byte *pbT = prck->pb;

		while (!fDone) {
			// If this section does not fit in this chunk, start a new chunk	

			if (&pbT[CalcSectionSize(psecT)] > &prck->pb[kcbRecordMax])
				break;

			// Add this section to this chunk

			SecChunk *psck = (SecChunk *)pbT;
			psck->cprop = SwapWord(psecT->cprop);

			// Copy in the section name and zero extend

			pbT = (byte *)(psck + 1);
			if (!ReadString(pf, psecT->nchSec, psecT->cchSec, pbT)) {
				printf("Error reading %s.\n", pszFnIn);
				fclose(pf);
				exit(1);
			}
			pbT += psecT->cchSec + 1;

			// Copy in the property / value pairs

			IniProperty *ppropT = psecT->pprop;
			for (int n = 0; n < psecT->cprop; n++) {
				if (!ReadString(pf, ppropT->nchProp, ppropT->cchProp, pbT)) {
					printf("Error reading %s.\n", pszFnIn);
					fclose(pf);
					exit(1);
				}
				pbT += ppropT->cchProp + 1;
				if (!ReadString(pf, ppropT->nchValue, ppropT->cchValue, pbT)) {
					printf("Error reading %s.\n", pszFnIn);
					fclose(pf);
					exit(1);
				}				
				pbT += ppropT->cchValue + 1;
				ppropT++;
			}

			// Even align

			if ((pbT - prck->pb) & 1)
				*pbT++ = 0;

			// Backfill the "next section" offset

			psck->offSecNext = SwapWord(pbT - (byte *)psck);

			// Next section...

			psecT++;
			if (psecT >= psecMax) {
				psck->offSecNext = 0;
				fDone = true;
			}
		}

		// Remember how much of this chunk we wrote to

		prck->pbMax = pbT;
	}
	fclose(pf);

	// Write the chunks out to the database

	long cbFile;
	bool fSuccess;
	if (fPdbOnly) {
		fSuccess = WriteDatabase(pszFnOut, arck, cChunks, &cbFile);
	} else {
		fSuccess = WriteBinary(pszFnOut, arck, cChunks, &cbFile);
	}

	// Free chunks

	int n;
	for (n = 0; n < cChunks; n++)
		delete arck[n].pb;

	// Free property sections

	for (n = 0; n < cSections; n++)
		delete psec[n].pprop;
	delete psec;

	return fSuccess ? 0 : 1;
}

int CalcSectionSize(IniSection *psec)
{
	int cb = sizeof(SecChunk);
	cb += psec->cchSec + 1;
	for (int n = 0; n < psec->cprop; n++) {
		cb += psec->pprop[n].cchProp + 1;
		cb += psec->pprop[n].cchValue + 1;
	}
	cb = (cb + 1) & ~1;
	return cb;
}

bool ReadString(FILE *pf, int nch, int cch, byte *pb)
{
	*pb = 0;
	if (cch == 0)
		return true;
	fseek(pf, nch, SEEK_SET);
	if (fread(pb, cch, 1, pf) != 1)
		return false;
	pb[cch] = 0;
	return true;
}

bool WriteBinary(char *pszFn, RecordChunk *arck, int cChunks, long *pcbFile)
{
	// Create bin file

	FILE *pf = fopen(pszFn, "wb");
	if (pf == NULL) {
		printf("Unable to create \"%s\".\n", pszFn);
		return false;
	}

	// Write out the chunks

	for (int n = 0; n < cChunks; n++) {
		if (fwrite(arck[n].pb, arck[n].pbMax - arck[n].pb, 1, pf) != 1) {
			printf("Error writing record data.\n");
			fclose(pf);
			_unlink(pszFn);
			return false;
		}
	}

	// Close & success

	*pcbFile = ftell(pf);
	fclose(pf);
	return true;
}

#define dmDBNameLength 32

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
typedef unsigned long LocalID;
typedef char Char;

struct RecordEntryType {
	LocalID localChunkID;		// local chunkID of a record
	UInt8 attributes;			// record attributes;
	UInt8 uniqueID[3];			// unique ID of record; should
								//	not be 0 for a legal record.
};

struct RecordListType {
	LocalID nextRecordListID;	// local chunkID of next list
	UInt16 numRecords;			// number of records in this list
	UInt16 firstEntry;			// array of Record/Rsrc entries 
								// starts here
};

struct DatabaseHdrType {
	char name[dmDBNameLength];	// name of database
	UInt16 attributes;			// database attributes
	UInt16 version;				// version of database
	UInt32 creationDate;		// creation date of database
	UInt32 modificationDate;	// latest modification date
	UInt32 lastBackupDate;		// latest backup date
	UInt32 modificationNumber;	// modification number of database
	LocalID appInfoID;			// application specific info
	LocalID sortInfoID;			// app specific sorting info
	UInt32 type;				// database type
	UInt32 creator;				// database creator 
	UInt32 uniqueIDSeed;		// used to generate unique IDs.
								//	Note that only the low order
								//	3 bytes of this is used (in
								//	RecordEntryType.uniqueID).
								//	We are keeping 4 bytes for 
								//	alignment purposes.
	RecordListType	recordList;	// first record list
};		

#ifdef WINDOWS
#include <windows.h>
dword GetCurrentTimePalmUnits()
{
	// Palm keeps time as a count of seconds from Jan 1, 1904.

	FILETIME timCurrent;
	GetSystemTimeAsFileTime(&timCurrent);

	SYSTEMTIME timsys1904;
	timsys1904.wYear = 1904;
	timsys1904.wMonth = 1;
	timsys1904.wDayOfWeek = 6;
	timsys1904.wDay = 1;
	timsys1904.wHour = 0;
	timsys1904.wMinute = 0;
	timsys1904.wSecond = 0;
	timsys1904.wMilliseconds = 0;

	FILETIME tim1904;
	SystemTimeToFileTime(&timsys1904, &tim1904);

	// Now we have 2 64 bit unsigned ints that specifies time in terms of 100 nanosecond
	// units. Subtract the two and we have the time between now and Jan 1 1904 in 100 nanosecond
	// units. 1 s == 1e9 ns, so there are 1e7 filetime units in 1 second.

	unsigned __int64 tCurrent;
	tCurrent = ((unsigned __int64)timCurrent.dwHighDateTime << 32) | timCurrent.dwLowDateTime;
	unsigned __int64 t1904;
	t1904 = ((unsigned __int64)tim1904.dwHighDateTime << 32) | tim1904.dwLowDateTime;
	unsigned __int64 tDiff = tCurrent - t1904;

	// Divide by 1024*1024 so it fits in a dword

	dword dw = (dword)(tDiff >> 20);

	// Divide by 1e9 / (1024*1024)

	return (dword)((double)dw / (10000000.0 / (double)(1024 * 1024)));

#else // TODO(darrinm): maybe this is fine on Windows too?
#include <ctime>
dword GetCurrentTimePalmUnits()
{
	// Palm keeps time as a count of seconds from Jan 1, 1904.
	struct tm tm1904;
	tm1904.tm_year = 1904 - 1900; // tm_year 0=1900
	tm1904.tm_mon = 1;
	tm1904.tm_wday = 6;
	tm1904.tm_mday = 1;
	tm1904.tm_yday = 0;
	tm1904.tm_hour = 0;
	tm1904.tm_min = 0;
	tm1904.tm_sec = 0;
	tm1904.tm_isdst = -1;
	time_t time1904 = mktime(&tm1904);
	time_t timeCurrent = time(NULL);
	return (dword)difftime(timeCurrent, time1904);
}
#endif

bool WriteDatabase(char *pszFnPdb, RecordChunk *arck, int cChunks, long *pcbFile)
{
	// Construct the new filename, which is same as the current with an extension of ".pdb"
	
	char szFnT[_MAX_PATH];
	_fullpath(szFnT, pszFnPdb, _MAX_PATH);
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	char szFname[_MAX_FNAME];
	char szExt[_MAX_EXT];
	_splitpath(szFnT, szDrive, szDir, szFname, szExt);

	// Create pdb file

	FILE *pf = fopen(pszFnPdb, "wb");
	if (pf == NULL) {
		printf("Unable to create \"%s\".\n", pszFnPdb);
		return false;
	}

	// Figure out the creator

	//dword dwCreator = (pszCreator[0] << 24) | (pszCreator[1] << 16) | (pszCreator[2] << 8) | pszCreator[3];
	// TODO(darrinm): huh? Is this right for WI?
	dword dwCreator = 'Vexd';

	// Init the .pdb header

	DatabaseHdrType hdr;
	memset(&hdr, 0, sizeof(hdr));

	// Use the filename as the database name

	strcpy(hdr.name, szFname);
	if (strlen(hdr.name) > dmDBNameLength - 1) {
		printf("Pdb filename too long.\n");
Error:
		fclose(pf);
		_unlink(pszFnPdb);
		return false;
	}

	// Hardwire a bunch of stuff about this .pdb.

	hdr.attributes = SwapWord(0);
	hdr.version = SwapWord(1);
	dword dwDate = GetCurrentTimePalmUnits();
	hdr.creationDate = SwapDword(dwDate);
	hdr.modificationDate = SwapDword(dwDate);
	hdr.lastBackupDate = SwapDword(dwDate);
	hdr.modificationNumber = SwapDword(1);
	hdr.appInfoID = SwapDword(0);
	hdr.sortInfoID = SwapDword(0);
	hdr.type = SwapDword('DATA');
	hdr.creator = SwapDword(dwCreator);
	hdr.uniqueIDSeed = SwapDword(cChunks + 1);
	hdr.recordList.nextRecordListID = SwapDword(0);
	hdr.recordList.numRecords = SwapWord((word)cChunks);

	// Write the header out

	int cbHdr = sizeof(hdr) - sizeof(hdr.recordList.firstEntry);	
	if (fwrite(&hdr, cbHdr, 1, pf) != 1) {
		printf("Error writing database header.\n");
		goto Error;
	}

	// Calculate where the actual records begin, which is after the record headers

	int offNext = cbHdr + cChunks * sizeof(RecordEntryType);

	// Write out the record headers.

	int n;
	for (n = 0; n < cChunks; n++) {
		RecordEntryType entry;
		memset(&entry, 0, sizeof(entry));
		entry.localChunkID	= SwapDword(offNext);
		entry.attributes = 0;
		dword id = n + 1;
		entry.uniqueID[2] = (byte)(id & 0xff);
		entry.uniqueID[1] = (byte)((id >> 8) & 0xff);
		entry.uniqueID[0] = (byte)((id >> 16) & 0xff);
		id++;
		offNext += arck[n].pbMax - arck[n].pb;
		if (fwrite(&entry, sizeof(entry), 1, pf) != 1) {
			printf("Error writing database record header.\n");
			goto Error;
		}
	}

	// Write out the chunks

	for (n = 0; n < cChunks; n++) {
		if (fwrite(arck[n].pb, arck[n].pbMax - arck[n].pb, 1, pf) != 1) {
			printf("Error writing record data.\n");
			goto Error;
		}
	}

	*pcbFile = ftell(pf);
	fclose(pf);

	return true;
}
