#ifndef __PACKFILE_H__
#define __PACKFILE_H__

#include "inc/basictypes.h"
#include "mpshared/pdbreader.h"
#include "mpshared/enum.h"

namespace wi {

#if !defined(PIL)

// Pdb structs

#define dmDBNameLength 32

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned int UInt32;
typedef unsigned int LocalID;
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
	char name[dmDBNameLength];	// 0 name of database
	UInt16 attributes;			// 32 database attributes
	UInt16 version;				// 34 version of database
	UInt32 creationDate;		// 36 creation date of database
	UInt32 modificationDate;	// 40 latest modification date
	UInt32 lastBackupDate;		// 44 latest backup date
	UInt32 modificationNumber;	// 48 modification number of database
	LocalID appInfoID;			// 52 application specific info
	LocalID sortInfoID;			// 56 app specific sorting info
	UInt32 type;				// 60 database type
	UInt32 creator;				// 64 database creator 
	UInt32 uniqueIDSeed;		// 68 used to generate unique IDs.
								//	Note that only the low order
								//	3 bytes of this is used (in
								//	RecordEntryType.uniqueID).
								//	We are keeping 4 bytes for 
								//	alignment purposes.
	RecordListType	recordList;	// 72 first record list
                                // 80
};		
#endif // !defined(PIL)

// PackFileReader. Exposes c-runtime prototype apis, calls PdbReader for data
// access to pdbs created with packpdb.

#ifndef kcbFilename
#define kcbFilename 29
#endif

#if kcbFilename != 29
#error kcbFilename must be 29
#endif

// DirEntry is fixed at 32

struct DirEntry // dir
{
	char szFn[kcbFilename];
	byte cRecs;
	word nRecFirst;
};

struct ReaderInfo // rnfo
{
	PdbReader *ppdbReader;
	DirEntry *pdir;
	char *pszDir;
	char *pszFn;
	void *pvCookie;
	int cEntries;
	int cOpen;
};

// Passed to fopen, etc.

struct File
{
	ReaderInfo *prnfo;
	dword cbTotal;
	dword offRecStart;
	dword offStream;
	byte nRecOffStream;
	byte cRecs;
	word nRecFirst;
	word acb[1];
    void *pvData;
};

// Used for MapFile / UnmapFile

struct FileMap // fmap
{
	ReaderInfo *prnfo;
	void *pvCookie;
	dword nRec;
    dword dwPad;
    byte *pbAlloced;
};

#define PACKENUM_FIRST 0
#define PACKENUM_LAST 1

#ifndef USE_PALM_UNIX_HEADERS
class PackFileReader // pakr
{
public:
	PackFileReader();
	~PackFileReader();

	virtual File *fopen(const char *pszFn, const char *pszMode);
	virtual int fclose(File *pfil);
	virtual dword fread(void *pv, dword cb, int c, File *pfil);
	virtual int fseek(File *pfil, int off, int nOrigin);
	virtual dword ftell(File *pfil);
	virtual bool EnumFiles(Enum *penm, int key, char *pszFn, int cbFn);
	virtual void *MapFile(const char *pszFn, FileMap *pfmap, dword *pcb = NULL);
	virtual void UnmapFile(FileMap *pfmap);
    bool HashFile(const char *pszFn, byte *hash);
    bool GetPdbName(const char *pszFn, char *pszPdb, int cbPdb,
            char *pszDir = NULL, int cbDir = 0);

	virtual bool Push(const char *pszDir, const char *pszFn);
	virtual bool Pop();
	bool Delete(const char *pszDir, const char *pszFn);

private:
    virtual PdbReader *OpenPdb(const char *pszDir, const char *pszFn) = 0;
    virtual bool DeletePdb(const char *pszDir, const char *pszFn) = 0;

	bool Push(const char *pszDir, const char *pszFn, PdbReader *ppdbReader);
	bool FindDirEntry(const char *psz, DirEntry *pdir, ReaderInfo **pprnfo);
	void RemoveReader(int rnfo);

	int m_crnfo;
	int m_crnfoAlloc;
	ReaderInfo *m_arnfo;
};
#endif

#ifndef SEEK_CUR
#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0
#endif

} // namespace wi
    
#endif // __PACKFILE_H__
