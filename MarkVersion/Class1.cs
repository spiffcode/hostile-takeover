using System;
using System.IO;

namespace MarkVersion {
	class Class1 {
		[STAThread]
		static void Main(string[] args) {
			int cMarks;
			BinaryReader br;
			BinaryWriter bw;
			FileStream stm;
			string strMark;
			string strPattern;
			try {
				cMarks = Int32.Parse(args[0]);
				stm = new FileStream(args[1], FileMode.Open, FileAccess.ReadWrite);
				br = new BinaryReader(stm);
				bw = new BinaryWriter(stm);
				strPattern = args[2];
				strMark = null;
				switch (args.Length) {
				case 4:
					strMark = args[3];
					break;

				case 5:
					if (args[3] == "-f") {
						TextReader tr = new StreamReader(args[4]);
						strMark = tr.ReadLine();
						tr.Close();
					}
					break;

				case 3:
					strMark = Console.In.ReadLine();
					break;
				}
				strMark.Trim();
			} catch {
				Console.WriteLine("Usage:");
				Console.WriteLine("MarkVersion count file pattern text");
				Console.WriteLine("MarkVersion count file pattern -f markfile");
				Console.WriteLine("MarkVersion count file pattern");
				Console.WriteLine("If the mark is left off, stdin will be used.");
				Console.WriteLine("If the mark count doesn't match count, it is an error. -1 means unlimited.");
				Console.WriteLine("ex: date \"+%m.%d.%y, %l:%M%p\" | markversion 1 file.pdb \"+++DATEDATEDATE+++\"");
				return;
			}

			int c = 0;
			long pos = 0;
			while (pos < br.BaseStream.Length) {
				pos = FindPattern(br, pos, strPattern);
				if (pos == -1)
					break;
				pos = Mark(bw, pos, strMark, strPattern);
				c++;
			}

			br.Close();
			bw.Close();
			stm.Close();
			Console.WriteLine("Marked " + c + " \"" + strPattern + "\" with \"" + strMark + "\"");

			// Match mark count or exception

			if (cMarks != -1 && c != cMarks)
				throw new Exception("Marked " + c + " times, you asked for " + cMarks);
		}

		static long FindPattern(BinaryReader br, long pos, string strPattern) {
			br.BaseStream.Position = pos;
			while (br.BaseStream.Position < br.BaseStream.Length - strPattern.Length) {
				bool fMatch = true;
				long posPattern = br.BaseStream.Position;
				for (int i = 0; i < strPattern.Length; i++) {
					if ((byte)strPattern[i] != br.ReadByte()) {
						fMatch = false;
						break;
					}
				}
				if (fMatch)
					return posPattern;
			}
			return -1;
		}

		static long Mark(BinaryWriter bw, long pos, string strMark, string strPattern) {
			bw.BaseStream.Position = pos;
			int cch = Math.Min(strMark.Length, strPattern.Length);
			for (int n = 0; n < cch; n++)
				bw.Write((byte)strMark[n]);

			// Zero terminate if the mark is shorter than strPattern

			if (strMark.Length < strPattern.Length)
				bw.Write((byte)0);

			return bw.BaseStream.Position;
		}
	}
}
