using System;
using System.IO;
using System.Collections;
using SpiffLib;

namespace packpdb2 {
	class Class1 {
		static int s_cbFilenameMax = 29;

		static void Usage() {
			Console.WriteLine("Usage:");
			Console.WriteLine("   packpdb2 -p <CRID> <pdb file> [filespec]");
			Console.WriteLine("		Pack [filespec] into <pdb file> with creator id <CRID>.");
			Console.WriteLine("		Optional [filespec] defaults to .\\*.*");
			Console.WriteLine("");
			Console.WriteLine("   packpdb2 -u <pdb file>");
			Console.WriteLine("		Unpacks files from <pdb file>.");
			Console.WriteLine("");
			Console.WriteLine("   packpdb2 -v <pdb file>");
			Console.WriteLine("		View contents of <pdb file>.");
			Console.WriteLine("");
			Console.WriteLine("   packpdb2 -m <prc file out> <prc file in> <pdb file>...");
			Console.WriteLine("		Merge a prc with one or more pdb files.");
			Console.WriteLine("");
		}
		
		[STAThread]
		static void Main(string[] args) {
			bool fSuccess = false;
			if (args.Length > 0) {
				switch (args[0]) {
				case "-p":
					fSuccess = PackFiles(args);
					break;

				case "-u":
					fSuccess = UnpackFiles(args[1]);
					break;

				case "-v":
					fSuccess = ViewFiles(args[1]);
					break;

				case "-m":
					fSuccess = MergeFiles(args);
					break;
				}
			}
			if (!fSuccess)
				Usage();
		}

		static bool ViewFiles(string strFilePdb) {
			// Open pack pdb

			PdbPacker pdbp = new PdbPacker(strFilePdb);

			// For each file...

			for (int iFile = 0; iFile < pdbp.Count; iFile++) {
				PdbPacker.File file = pdbp[iFile];
				int cbCompressed = file.GetCompressedSize();
				int cbUncompressed = (int)file.ab.Length;
				int nPercentSavings = ((cbUncompressed - cbCompressed) * 100 + cbCompressed / 2) / cbCompressed;
				Console.WriteLine(file.str + ", " + cbUncompressed + " bytes, " + cbCompressed + " bytes compressed, " + nPercentSavings + "% savings");
			}

			return true;
		}

		static bool UnpackFiles(string strFilePdb) {
			// Open pack pdb

			PdbPacker pdbp = new PdbPacker(strFilePdb);

			// For each file...

			for (int iFile = 0; iFile < pdbp.Count; iFile++) {
				PdbPacker.File file = pdbp[iFile];
				BinaryWriter bwtr = new BinaryWriter(new FileStream(file.str, FileMode.Create, FileAccess.Write, FileShare.None));
				bwtr.Write(file.ab);
				bwtr.Close();
				Console.WriteLine("Wrote " + file.str + ", " + file.ab.Length + " bytes.");
			}

			return true;
		}

		static bool PackFiles(string[] args) {
			// Parse parameters, validate.

			if (args[1].Length > 4) {
				Console.WriteLine("Creator id " + args[1] + " must be 4 chars or less.");
				return false;
			}
			string strCreatorId = args[1];
			string strFullPathPdb = Path.GetFullPath(args[2]);
			string strFilePdb = Path.GetFileName(strFullPathPdb);
			string strDirPdb = Path.GetDirectoryName(strFullPathPdb);
			if (strDirPdb == "")
				strDirPdb = ".";
			if (strFilePdb.Length > 31) {
				Console.WriteLine("Pdb filename " + strFilePdb + " must be 31 characters or less.");
				return false;
			}
			string strFileSpec;
			if (args.Length == 3) {
				strFileSpec = ".\\*.*";
			} else {
				strFileSpec = args[3];
			}

			// Get list of files to add

			string strFileFileSpecAdd = Path.GetFileName(strFileSpec);
			string strDirFileSpecAdd = Path.GetDirectoryName(strFileSpec);
			if (strDirFileSpecAdd == "")
				strDirFileSpecAdd = ".";
			string[] astrFilesAdd = Directory.GetFiles(strDirFileSpecAdd, strFileFileSpecAdd);

			// File types not to compress

			ArrayList alsStrFilesNoCompress = new ArrayList();
			if (args.Length > 4) {
				if (args[4] == "-nocompress") {
					for (int i = 5; i < args.Length; i++) {
						string[] astrFiles = Directory.GetFiles(strDirFileSpecAdd, args[i]);
						foreach (string str in astrFiles)
							alsStrFilesNoCompress.Add(Path.GetFullPath(str));
					}
				}
			}

			// Print status

			Console.Write("Packing files... ");
			PdbPacker pdbp = new PdbPacker();
			foreach (string strFileAdd in astrFilesAdd) {
				// Don't add the .pdb we're building

				string strFullPathAdd = Path.GetFullPath(strFileAdd);
				if (strFullPathPdb.ToLower() == strFullPathAdd.ToLower())
					continue;

				// Get filename only, check length

				string strFile = Path.GetFileName(strFullPathAdd).ToLower();
				if (strFile.Length >= s_cbFilenameMax) {
					Console.WriteLine("The file " + strFile + " is too long. Must be " + (s_cbFilenameMax - 1) + "chars max.");
					return false;
				}

				// Compress or not?

				bool fCompress = true;
				foreach (string strFileNoCompress in alsStrFilesNoCompress) {
					if (strFullPathAdd.ToLower() == strFileNoCompress.ToLower()) {
						fCompress = false;
						break;
					}
				}

				// Read the file

				Stream stm = new FileStream(strFullPathAdd, FileMode.Open, FileAccess.Read);
				BinaryReader brdr = new BinaryReader(stm);
				PdbPacker.File file = new PdbPacker.File(strFile, brdr.ReadBytes((int)brdr.BaseStream.Length), fCompress);
				brdr.Close();
				pdbp.Add(file);
			}

			// Save out

			pdbp.Save(strFullPathPdb, strCreatorId);
			return true;
		}

		static bool MergeFiles(string [] args)
		{
			if (args.Length < 4) {
				Console.WriteLine("Must specify input prc, output prc and at least one pdb.");
				return false;
			}
			
			// Read input prc.

			PalmDatabase pdb = new PalmDatabase();
			pdb.Load(args[2]);

			// Read pdbs.

			PalmDatabase [] apdb = new PalmDatabase[args.Length - 3];

			for (int iarg = 3; iarg < args.Length; iarg++) {
				apdb[iarg - 3] = new PalmDatabase();
				apdb[iarg - 3].Load(args[iarg]);
			}

			// Merge.

			PdbPacker.MergePdbs(pdb, apdb);
			
			// Write the output.

			pdb.Save(args[1]);

			return true;
		}
	}
}
