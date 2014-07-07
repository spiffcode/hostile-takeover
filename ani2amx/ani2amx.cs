using System;
using System.IO;
using SpiffCode;

namespace acrunch
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class App
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static int Main(string[] astrArgs) {
			foreach (string strFileName in Directory.GetFiles(".", "*.ani")) {
				string strImportDir = Path.GetFileNameWithoutExtension(strFileName);
				Console.WriteLine("Importing files from {0}", strImportDir);

				AnimDoc doc = new AnimDoc();
				if (!doc.Import(Directory.GetFiles(strImportDir, "*.png"))) {
					Console.WriteLine("Error: couldn't import files from dir {0}", strImportDir);
					return -1;
				}

				Console.WriteLine("Writing {0}.amx", strImportDir);
				doc.Save(strImportDir + ".amx");
			}
			return 0;
		}
	}
}
