using System;
using System.IO;
using System.Collections;
using System.Diagnostics;

namespace SpiffLib {
#if false
	// These structs straight from Palm includes with minor editting

	struct RecordEntryType {
		uint localChunkID;		// local chunkID of a record
		byte attributes;			// record attributes;
		byte uniqueID;			// unique ID of record; should not be 0 for a legal record.
		byte uniqueID_byte1;
		byte uniqueID_byte2;
	}

    struct RsrcEntryType {
        uint type;
        ushort id;
        uint localChunkID;
    }
        
	struct RecordListType {
		uint nextRecordListID;	// local chunkID of next list
		ushort numRecords;			// number of records in this list
		ushort firstEntry;			// array of Record/Rsrc entries 
		// starts here
	}

	struct DatabaseHdrType {
		char[] name;	// name of database, 32 bytes
		ushort attributes;			// database attributes
		ushort version;				// version of database
		uint creationDate;		// creation date of database
		uint modificationDate;	// latest modification date
		uint lastBackupDate;		// latest backup date
		uint modificationNumber;	// modification number of database
		uint appInfoID;			// application specific info
		uint sortInfoID;			// app specific sorting info
		uint type;				// database type
		uint creator;				// database creator 
		uint uniqueIDSeed;		// used to generate unique IDs.
		//	Note that only the low order
		//	3 bytes of this is used (in
		//	RecordEntryType.uniqueID).
		//	We are keeping 4 bytes for 
		//	alignment purposes.
		RecordListType	recordList;	// first record list
	}

	// DatabaseHdrType - sizeof(ushort)
	// followed by RecordEntryType
	// followed by record data bytes
#endif

	public class PdbRecord {
		public PdbRecord() {
		}


		public byte[] Data {
			get {
				return m_ab;
			}
			set {
				m_ab = value;
			}
		}

		public uint Type {
			get 
			{
				return m_uiType;
			}
			set 
			{
				m_uiType = value;
			}
		}

		public ushort ID {
			get {
				return m_usID;
			} 
			set {
				m_usID = value;
			}
		}

		public uint IbRec {
			get {
				return m_ibRec;
			}
			set {
				m_ibRec = value;
			}
		}

		byte[] m_ab;
		uint m_uiType;
		ushort m_usID;
		uint m_ibRec;
	}

	public class PalmDatabase 
	{
		ArrayList m_alsPdbRecords = new ArrayList();
		string m_strName = "";

		const ushort dmHdrAttrBundle = 0x0800;
		const ushort dmHdrAttrBackup = 0x0008;
		const ushort dmHdrAttrResDB = 0x0001;

		ushort m_usAttributes = dmHdrAttrBundle | dmHdrAttrBackup;
		ushort m_usVersion = 1;
		uint m_uiCreationDate = GetCurrentTimePalmUnits();
		uint m_uiType;
		uint m_uiCreator;

		static int s_cbDbName = 32;

		public PalmDatabase() 
		{
		}

		public void SetRecordData(ArrayList alsRecordData) 
		{
			m_alsPdbRecords.Clear();
			foreach (byte[] abData in alsRecordData) {
				PdbRecord pdbr = new PdbRecord();
				pdbr.Data = (byte[])abData.Clone();
				m_alsPdbRecords.Add(pdbr);
			}
		}

		public ArrayList GetRecordData() 
		{
			ArrayList alsRecordData = new ArrayList();
			foreach (PdbRecord pdbr in m_alsPdbRecords)
				alsRecordData.Add((byte[])pdbr.Data.Clone());
			return alsRecordData;
		}

		public void Add(byte [] abData)
		{
			Add(abData, 0, 0);
		}

		public void Add(byte [] abData, ushort usID, uint uiType)
		{
			PdbRecord pdbr = new PdbRecord();
			pdbr.Data = (byte[])abData.Clone();
			pdbr.ID = usID;
			pdbr.Type = uiType;
			m_alsPdbRecords.Add(pdbr);
		}

		public PdbRecord this [int index] 
		{
			set {
				m_alsPdbRecords[index] = value;
			}
			get {
				return (PdbRecord)m_alsPdbRecords[index];
			}
		}

		public int Count 
		{
			get {
				return m_alsPdbRecords.Count;
			}
		}

		static uint GetCurrentTimePalmUnits() 
		{
			DateTime dt1904 = new DateTime(1904, 1, 1);
			TimeSpan ts = DateTime.Now - dt1904;
			return (uint)ts.TotalSeconds;
		}
	
		public void Load(string strFileName) 
		{
			Stream stm = new FileStream(strFileName, FileMode.Open, FileAccess.Read, FileShare.None);
			BinaryReader brdr = new BinaryReader(stm);

			// Name

			m_strName = "";
			for (int i = 0; i < s_cbDbName; i++) 
			{
				char ch = brdr.ReadChar();
				if (ch == 0)
					break;
				m_strName += ch;
			}
			brdr.BaseStream.Position = (long)s_cbDbName;

			// attributes
			m_usAttributes = Misc.SwapUShort(brdr.ReadUInt16());

			// version
			m_usVersion = Misc.SwapUShort(brdr.ReadUInt16());

			// creationDate
			m_uiCreationDate = Misc.SwapUInt(brdr.ReadUInt32());

			// modificationDate
			brdr.ReadUInt32();

			// lastBackupDate
			brdr.ReadUInt32();

			// modificationNumber
			brdr.ReadUInt32();

			// appInfoID
			uint uiAppInfoID = Misc.SwapUInt(brdr.ReadUInt32());
			Debug.Assert(uiAppInfoID == 0);

			// sortInfoID
			uint uiSortInfoID = Misc.SwapUInt(brdr.ReadUInt32());
			Debug.Assert(uiSortInfoID == 0);

			// type
			m_uiType = Misc.SwapUInt(brdr.ReadUInt32());

			// creator
			m_uiCreator = Misc.SwapUInt(brdr.ReadUInt32());

			// uniqueIDSeed
			brdr.ReadUInt32();

			// recordList.nextRecordListID
			brdr.ReadUInt32();

			// recordList.numRecords
			ushort crecs = Misc.SwapUShort((ushort)brdr.ReadUInt16());

			// Read in records

			m_alsPdbRecords = new ArrayList();

			for (int irec = 0; irec < crecs; irec++) {
				PdbRecord pdbr = new PdbRecord();
				m_alsPdbRecords.Add(pdbr);

				if ((m_usAttributes & dmHdrAttrResDB) != 0) {
					pdbr.Type = Misc.SwapUInt(brdr.ReadUInt32());
					pdbr.ID = Misc.SwapUShort(brdr.ReadUInt16());
					pdbr.IbRec = Misc.SwapUInt(brdr.ReadUInt32());
				}
				else {
					// localChunkId (actually an offset to the bytes for this record)		
					pdbr.IbRec = Misc.SwapUInt((uint)brdr.ReadUInt32());

					// attributes, unique id

					brdr.ReadByte();
					brdr.ReadByte();
					brdr.ReadByte();
					brdr.ReadByte();
				}
			}

			for (int irec = 0; irec < crecs; irec++) {
				uint ibrecNext;

				if (irec == crecs - 1)
					ibrecNext = (uint)brdr.BaseStream.Length;
				else
					ibrecNext = ((PdbRecord)m_alsPdbRecords[irec+1]).IbRec;

				PdbRecord pdbr = (PdbRecord)m_alsPdbRecords[irec];
				brdr.BaseStream.Position = pdbr.IbRec;
				pdbr.Data = brdr.ReadBytes((int)(ibrecNext - pdbr.IbRec));
			}

			// All done

			brdr.Close();
		}

		public bool Save(string strFileName) {
			Stream stm = new FileStream(strFileName, FileMode.Create, FileAccess.Write, FileShare.None);
			BinaryWriter bwtr = new BinaryWriter(stm);

			// dbName
			for (int i = 0; i < s_cbDbName; i++) {
				if (i < m_strName.Length) {
					bwtr.Write((byte)m_strName[i]);
					continue;
				}
				bwtr.Write((byte)0);
			}

			// attributes
			bwtr.Write(Misc.SwapUShort(m_usAttributes));

			// version
			bwtr.Write(Misc.SwapUShort(m_usVersion));

			// creationDate
			bwtr.Write(Misc.SwapUInt(m_uiCreationDate));

			// modificationDate
			uint uiDate = Misc.SwapUInt(GetCurrentTimePalmUnits());
			bwtr.Write(uiDate);

			// lastBackupDate
			bwtr.Write(uiDate);

			// modificationNumber
			bwtr.Write((uint)Misc.SwapUInt(1));

			// appInfoID
			bwtr.Write(Misc.SwapUInt(0));

			// sortInfoID
			bwtr.Write(Misc.SwapUInt(0));

			// type
			bwtr.Write(Misc.SwapUInt(m_uiType));

			// creator
			bwtr.Write(Misc.SwapUInt(m_uiCreator));

			// uniqueIDSeed
			bwtr.Write(Misc.SwapUInt((uint)(m_alsPdbRecords.Count + 1)));

			// recordList.nextRecordListID
			bwtr.Write((uint)0);

			// recordList.numRecords
			bwtr.Write(Misc.SwapUShort((ushort)m_alsPdbRecords.Count));

			// Bonus padding because the Tapwave signing tool fails unless it is there
			// It is also what buildprc inserts. Devices/simulators/emulators don't need it tho.

			int cbBonusPadding = 2;

			// Calc where the records begin, which is after the record headers
			int cbHdr = (int)bwtr.BaseStream.Position;
			int ibrecNext = cbHdr + m_alsPdbRecords.Count * ((m_usAttributes & dmHdrAttrResDB) != 0 ? 10 : 8) + cbBonusPadding;

			// Write out the record headers
			uint id = 1;
			foreach (PdbRecord pdbr in m_alsPdbRecords) {

				if ((m_usAttributes & dmHdrAttrResDB) != 0) {
					bwtr.Write(Misc.SwapUInt(pdbr.Type));
					bwtr.Write(Misc.SwapUShort(pdbr.ID));
					bwtr.Write(Misc.SwapUInt((uint)ibrecNext));
				}
				else {
					// localChunkId (actually an offset to the bytes for this record)
					bwtr.Write(Misc.SwapUInt((uint)ibrecNext));

					// attributes
					bwtr.Write((byte)0);
		
					// uniqueID
					bwtr.Write((byte)(id & 0xff));
					bwtr.Write((byte)((id >> 8) & 0xff));
					bwtr.Write((byte)((id >> 16) & 0xff));
				}

				ibrecNext += pdbr.Data.Length;
				id++;
			}

			// Write bonus padding

			while (cbBonusPadding-- != 0)
				bwtr.Write((byte)0);

			// Write out the record data
			foreach (PdbRecord pdbr in m_alsPdbRecords)
				bwtr.Write(pdbr.Data);

			// Close & stats.
			Console.WriteLine(Path.GetFileName(strFileName) + " written. " + bwtr.BaseStream.Length + " bytes.");
			bwtr.Close();
			return true;
		}

		public string Name {
			get {
				return m_strName;
			}
			set {
				string strT = value;
				if (strT.Length + 1 >= s_cbDbName)
					strT = strT.Substring(0, s_cbDbName - 1);
				m_strName = strT;
			}
		}

		public uint CreatorId {
			get {
				return m_uiCreator;
			}
			set {
				m_uiCreator = value;
			}
		}

		public uint TypeId {
			get {
				return m_uiType;
			}
			set {
				m_uiType = value;
			}
		}
	}
}
