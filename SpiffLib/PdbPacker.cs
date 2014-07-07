using System;
using System.IO;
using System.Collections;
using System.Diagnostics;

namespace SpiffLib {
	public class PdbPacker {
		static int s_cbRecordMax = 32000;
		static int s_cbFilenameMax = 29;
		ArrayList m_alsFiles = new ArrayList();

		public class File {
			public string str;
			public byte[] ab;
			public bool fCompress;
			int m_cbCompressed;

			public File() {
			}

			public File(string strT, byte[] abT, bool fCompressT, int cbCompressed) {
				str = strT;
				ab = abT;
				fCompress = fCompressT;
				m_cbCompressed = cbCompressed;
			}

			public File(string strT, byte[] abT, bool fCompressT) {
				str = strT;
				ab = abT;
				fCompress = fCompressT;
			}

			public File(string strT, byte[] abT) {
				str = strT;
				ab = abT;
				fCompress = true;
			}

			public int GetCompressedSize() {
				return m_cbCompressed;
			}
		}

		public PdbPacker() {
		}

		public PdbPacker(string strFile) {
			Load(strFile);
		}

		public int Count {
			get {
				return m_alsFiles.Count;
			}
		}

		public void Add(File file) {
			m_alsFiles.Add(file);
		}

		public File this[string strFile] {
			get {
				foreach (File file in m_alsFiles) {
					if (file.str.ToLower() == strFile.ToLower())
						return file;
				}
				return null;
			}
		}

		public File this[int iFile] {
			get {
				return (File)m_alsFiles[iFile];
			}
		}

		struct DirEntry {
			public string strFile;
			public byte crec;
			public ushort irec;
		};

		public void Load(string strFile) {
			// Open palm database

			PalmDatabase pdb = new PalmDatabase();
			pdb.Load(strFile);

			// Load directory entries

			ArrayList alsRecordData = pdb.GetRecordData();
			CompressionHeader coh;
			DirEntry[] ade = ParseDirectoryRecord(UnpackRecord((byte[])alsRecordData[0], out coh));

			// Read in the file data

			m_alsFiles.Clear();
			foreach (DirEntry de in ade) {
				// Decompress and write out bytes

				ArrayList alsFileBytes = new ArrayList();
				for (int i = 0; i < de.crec; i++) {
					byte[] ab = UnpackRecord((byte[])alsRecordData[i + de.irec], out coh);
					alsFileBytes.AddRange(ab);
				}

				File file = new File(de.strFile, (byte[])alsFileBytes.ToArray(typeof(byte)), coh.fCompressed, coh.cbCompressed);
				m_alsFiles.Add(file);
			}
		}

		// Straight ascii order comparer, since at runtime we use a strcmp based binary search

		public class FilenameComparer : IComparer {
			public int Compare(object objA, object objB) {
				string strA = (string)objA;
				string strB = (string)objB;
				int cch = Math.Min(strA.Length, strB.Length);
				for (int ich = 0; ich < cch; ich++) {
					if (strA[ich] < strB[ich])
						return -1;
					if (strA[ich] > strB[ich])
						return 1;
				}

				if (strA.Length < strB.Length)
					return -1;
				if (strA.Length > strB.Length)
					return 1;

				return 0;
			}
		}

		public void Save(string strFilePdb, string strCreatorId, string strTypeId) {
			// First sort all the files by name

			string[] astrFileNames = new string[m_alsFiles.Count];
			for (int iFile = 0; iFile < m_alsFiles.Count; iFile++) {
				File file = (File)m_alsFiles[iFile];
				astrFileNames[iFile] = file.str.ToLower();
			}
			File[] afile = (File[])m_alsFiles.ToArray(typeof(File));
			Array.Sort(astrFileNames, afile, new FilenameComparer());

			// Create the record bytes

			ArrayList alsDirEntries = new ArrayList();
			ArrayList alsRecords = new ArrayList();
			ushort irec = 1;
			foreach (File file in afile) {
				// Get filename only, check length

				string strFile = file.str.ToLower();
				if (strFile.Length >= s_cbFilenameMax)
					throw new Exception("The file " + strFile + " is too long. Must be " + (s_cbFilenameMax - 1) + "chars max.");

				// Read the file into chunks

				BinaryReader brdr = new BinaryReader(new MemoryStream(file.ab));
				byte crec = 0;
				while (brdr.BaseStream.Position < brdr.BaseStream.Length) {
					int cbLeft = (int)(brdr.BaseStream.Length - brdr.BaseStream.Position);
					int cbRead = cbLeft < s_cbRecordMax ? cbLeft : s_cbRecordMax;
					byte[] abChunk = brdr.ReadBytes(cbRead);
					alsRecords.Add(PackRecord(abChunk, file.fCompress));
					crec++;
				}
				brdr.Close();

				// Make a directory entry for this file

				DirEntry de = new DirEntry();
				de.strFile = strFile;
				de.irec = irec;
				de.crec = crec;
				alsDirEntries.Add(de);
				irec += de.crec;

#if false
				Console.WriteLine(de.strFile + ", " + de.crec + " recs");
#endif
			}

			// Insert the record for the directory entries

			alsRecords.Insert(0, PackRecord(MakeDirectoryRecord(alsDirEntries), false));

			// Create and save a .pdb around these records

			PalmDatabase pdb = new PalmDatabase();
			pdb.SetRecordData(alsRecords);
			pdb.Name = Path.GetFileName(Path.GetFullPath(strFilePdb));
			pdb.CreatorId = IdFromString(strCreatorId);
			pdb.TypeId = IdFromString(strTypeId);
			pdb.Save(strFilePdb);
		}

		static byte[] PackRecord(byte[] abChunk, bool fCompress) {
			ArrayList alsRecordBytes = new ArrayList();
			byte[] abCompressed = null;
			if (fCompress) {
				abCompressed = Compressor.CompressChunk(abChunk);
				if (abCompressed.Length >= abChunk.Length)
					abCompressed = null;
			}
			if (abCompressed != null) {
				alsRecordBytes.Add((byte)0);
				alsRecordBytes.Add((byte)1);
				alsRecordBytes.Add((byte)((abChunk.Length >> 8) & 0xff));
				alsRecordBytes.Add((byte)(abChunk.Length & 0xff));
				alsRecordBytes.Add((byte)((abCompressed.Length >> 8) & 0xff));
				alsRecordBytes.Add((byte)(abCompressed.Length & 0xff));
				alsRecordBytes.AddRange(abCompressed);
			} else {
				alsRecordBytes.Add((byte)0);
				alsRecordBytes.Add((byte)0);
				alsRecordBytes.Add((byte)((abChunk.Length >> 8) & 0xff));
				alsRecordBytes.Add((byte)(abChunk.Length & 0xff));
				alsRecordBytes.Add((byte)((abChunk.Length >> 8) & 0xff));
				alsRecordBytes.Add((byte)(abChunk.Length & 0xff));
				alsRecordBytes.AddRange(abChunk);
			}
			return (byte[])alsRecordBytes.ToArray(typeof(byte));
		}

		struct CompressionHeader {
			public bool fCompressed;
			public ushort cbUncompressed;
			public ushort cbCompressed;
		}

		static byte[] UnpackRecord(byte[] ab, out CompressionHeader coh) {
			coh.fCompressed = (ab[1] == 1);
			coh.cbUncompressed = (ushort)((ab[2] << 8) + ab[3]);
			coh.cbCompressed = (ushort)((ab[4] << 8) + ab[5]);

			byte[] abT = new byte[ab.Length - 6];
			Array.Copy(ab, 6, abT, 0, abT.Length);
			if (coh.fCompressed) {
				ab = Compressor.DecompressChunk(abT);
			} else {
				ab = abT;
			}

			return ab;
		}

		static uint IdFromString(string str) {
			str.PadLeft(4, ' ');
			uint ui = 0;
			for (int i = 0; i < 4; i++) {
				ui <<= 8;
				ui |= (uint)str[i];
			}
			return ui;
		}

		static byte[] MakeDirectoryRecord(ArrayList alsDirEntries) {
			// Each entry is 32 bytes. Serialize into a byte array.
			//
			// struct DirEntry {
			//	char szFn[kcbFilename];
			//	byte crec;
			//	word irec;
			// };

			ArrayList alsPdbDirEntries = new ArrayList();
			foreach (DirEntry de in alsDirEntries) {
				byte[] abFilename = new byte[s_cbFilenameMax];
				for (int ich = 0; ich < de.strFile.Length; ich++)
					abFilename[ich] = (byte)de.strFile[ich];
				alsPdbDirEntries.AddRange(abFilename);
				alsPdbDirEntries.Add((byte)de.crec);
				alsPdbDirEntries.Add((byte)((de.irec >> 8) & 0xff));
				alsPdbDirEntries.Add((byte)(de.irec & 0xff));
			}
			return (byte[])alsPdbDirEntries.ToArray(typeof(byte));
		}

		static DirEntry[] ParseDirectoryRecord(byte[] ab) {
			ArrayList alsPdbDirEntries = new ArrayList();
			for (int ib = 0; ib < ab.Length; ib += 32) {
				DirEntry de;
				de.strFile = "";
				for (int ich = 0; ich < s_cbFilenameMax; ich++) {
					char ch = (char)ab[ib + ich];
					if (ch == 0)
						break;
					de.strFile += ch;
				}
				de.crec = (byte)ab[ib + s_cbFilenameMax];
				de.irec = (ushort)((ab[ib + s_cbFilenameMax + 1] << 8) + ab[ib + s_cbFilenameMax + 2]);
				alsPdbDirEntries.Add(de);
			}
			return (DirEntry[])alsPdbDirEntries.ToArray(typeof(DirEntry));
		}

		public static void MergePdbs(PalmDatabase pdb, PalmDatabase [] apdb)
		{
			// Create directory entry.

			int irec = pdb.Count + 1; // plus one for directory

			ArrayList alsPdbDirEntries = new ArrayList();
			for (int ipdb = 0; ipdb < apdb.Length; ipdb++) {

				byte [] abFilename = new byte[28];
				for (int ich = 0; ich < apdb[ipdb].Name.Length; ich++)
					abFilename[ich] = (byte)apdb[ipdb].Name[ich];

				alsPdbDirEntries.AddRange(abFilename);
				alsPdbDirEntries.Add((byte)((irec >> 8) & 0xff));
				alsPdbDirEntries.Add((byte)(irec & 0xff));
				alsPdbDirEntries.Add((byte)((apdb[ipdb].Count >> 8) & 0xff));
				alsPdbDirEntries.Add((byte)(apdb[ipdb].Count & 0xff));
				
				irec += apdb[ipdb].Count;
			}

			alsPdbDirEntries.Add((byte)0); // end of directory marker
			
			uint uiType = 0x5041434b; // PACK

			irec = 0;
			pdb.Add((byte[])alsPdbDirEntries.ToArray(typeof(byte)), (ushort)irec, uiType);
			
			for (int ipdb = 0; ipdb < apdb.Length; ipdb++) {
				for (int irecAdd = 0; irecAdd < apdb[ipdb].Count; irecAdd++) {
					irec += 1;
					pdb.Add(apdb[ipdb][irecAdd].Data, (ushort)irec, uiType);
				}
			}
		}	
	}
}
