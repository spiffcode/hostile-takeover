using System;
using System.IO;
using System.Collections;
using SpiffLib;

namespace wavcrunch
{
	class Class1
	{
		[STAThread]
		static void Main(string[] args)
		{
			if (args[0] == "-g") {
				GenTables();
				return;
			}

			// Get source

			ArrayList alsFiles = new ArrayList();
			for (int n = 0; n < args.Length; n++) {
				string strFileT = Path.GetFileName(args[n]);
				string strDirT = Path.GetDirectoryName(args[n]);
				if (strDirT == "")
					strDirT = ".";
				string[] astrFiles = Directory.GetFiles(strDirT, strFileT);
				alsFiles.AddRange(astrFiles);
			}

			foreach (string strFileWav in alsFiles) {
				// Read in wav

				Pcm pcm = new Pcm(strFileWav);

				// Write out .snd file

				string strFileSnd = Path.ChangeExtension(strFileWav, ".snd");
				Console.WriteLine(Path.GetFileName(strFileWav) + " -> " + Path.GetFileName(strFileSnd));
				BinaryWriter bwtr = new BinaryWriter(new FileStream(strFileSnd, FileMode.Create, FileAccess.Write, FileShare.None));
				bwtr.Write(pcm.GetSndEncoding());
				bwtr.Close();

			}
		}

		static void GenTables() {
			Console.WriteLine("gmp2SumAverage:");
			for (int n = 0; n < 512; n++) {
				if (n % 16 == 0)
					Console.Write(".byte ");
				int m = (int)((float)n / 2.0 + 0.5);
				if (m > 255)
					m = 255;
				if (n % 16 == 15) {
					Console.WriteLine(m);
				} else {
					Console.Write(m + ", ");
				}
			}
			Console.WriteLine("");

			Console.WriteLine("gmp3SumAverage:");
			for (int n = 0; n < 768; n++) {
				if (n % 16 == 0)
					Console.Write(".byte ");
				int m = (int)((float)n / 3.0 + 0.5);
				if (m > 255)
					m = 255;
				if (n % 16 == 15) {
					Console.WriteLine(m);
				} else {
					Console.Write(m + ", ");
				}
			}
			Console.WriteLine("");

			Console.WriteLine("gmp4SumAverage:");
			for (int n = 0; n < 1024; n++) {
				if (n % 16 == 0)
					Console.Write(".byte ");
				int m = (int)((float)n / 4.0 + 0.5);
				if (m > 255)
					m = 255;
				if (n % 16 == 15) {
					Console.WriteLine(m);
				} else {
					Console.Write(m + ", ");
				}
			}
		}
	}
}
