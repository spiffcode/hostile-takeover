using System;
using System.IO;
using System.Collections;
using SpiffCode;

namespace amx2zamx
{
	/// <summary>
	/// Summary description for App.
	/// </summary>
	class App
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static int Main(string[] astrArgs)
		{
			string strOutputDir = null;
			ArrayList alFiles = new ArrayList();
			bool fForce = false;

			if (astrArgs.Length < 1) {
				Console.WriteLine("amx2zamx usage:\namx2zamx <file.amx ...> [-o output dir]");
				return -1;
			}

			for (int i = 0; i < astrArgs.Length; i++) {
				string str = astrArgs[i];

				if (str == "-o") {
					strOutputDir = astrArgs[++i];
					continue;
				} else if (str == "-f") {
					fForce = true;
					continue;
				}

				string strRoot = Path.GetPathRoot(str);
				if (strRoot == "")
					strRoot = ".";
				string strFile = Path.GetFileName(str);
				string[] astrFiles = Directory.GetFiles(strRoot, strFile);
				if (astrFiles.Length == 0) {
					Console.WriteLine("Warning: nothing matches {0}", str);
					continue;
				}

				alFiles.AddRange(astrFiles);
			}

			foreach (string strFile in alFiles) {
				if (Path.GetExtension(strFile).ToLower() != ".amx") {
					Console.WriteLine("Warning: ignoring {0}", strFile);
					continue;
				}

				AnimDoc doc = AnimDoc.Load(strFile);

				string strOutFile = Path.GetFileName(strFile);
				strOutFile = Path.ChangeExtension(strOutFile, ".zamx");
				if (strOutputDir != null)
					strOutFile = Path.Combine(strOutputDir, strOutFile);

				if (File.Exists(strOutFile) && !fForce) {
					Console.WriteLine("Warning: {0} already exists, skipping", strOutFile);
					continue;
				}

				Console.WriteLine("writing {0}", strOutFile);
				doc.Save(strOutFile);
			}
			
			return 0;
		}
	}
}
