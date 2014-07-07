using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.Resources;
using System.IO;
using SpiffLib;

namespace bcr2 {
	class Class1 {
		static void Usage() {
			Console.WriteLine("bcr2 [-scale N] [-font] [-code] [-colorkey] [-savescaled] palette outputdir imagefiles");		}

		[STAThread]
		static void Main(string[] astr)	{
			// Special cases
			if (astr.Length == 0) {
				Usage();
				return;
			}

			bool fRaw = false;
			bool fFont = false;
			double nScale = 1.0;
			bool fSaveScaled = false;

			int istr = 0;
			for (; istr < astr.Length; istr++) {
				if (astr[istr][0] != '-')
					break;
				switch (astr[istr]) {
				case "-raw":
					fRaw = true;
					break;

				case "-font":
					fFont = true;
					break;

				case "-scale":
					istr++;
					nScale = double.Parse(astr[istr]);
					break;

				case "-savescaled":
					fSaveScaled = true;
					break;
				}
			}

			// Params: palette outputdir filespecs
			// Get palette
			Palette pal = new Palette(astr[istr++]);

			// Get directory
			string strDir = astr[istr++];

			// Expand filespecs
			ArrayList alsFiles = new ArrayList();
			for (; istr < astr.Length; istr++) {
				string strFileT = Path.GetFileName(astr[istr]);
				string strDirT = Path.GetDirectoryName(astr[istr]);
				if (strDirT == "")
					strDirT = ".";
				string[] astrFiles = Directory.GetFiles(strDirT, strFileT);
				alsFiles.AddRange(astrFiles);
			}

			// Output
			if (fFont) {
				foreach (string strFile in alsFiles) {
					string strFileFnt = strDir + Path.DirectorySeparatorChar + Path.GetFileName(strFile.Substring(0, strFile.Length - 4) + ".fnt");
					string strFileAscii = Path.GetDirectoryName(strFile) + Path.DirectorySeparatorChar + Path.GetFileName(strFile.Substring(0, strFile.Length - 4) + ".txt");
					Console.WriteLine(strFile + " -> " + strFileFnt);
					TBitmap.SaveFont(strFile, pal, strFileAscii, strFileFnt);
				}
			} else {
				if (fRaw) {
					foreach (string strFile in alsFiles) {
						string strFileRaw = strDir + Path.DirectorySeparatorChar + Path.GetFileName(strFile.Substring(0, strFile.Length - 4) + ".rbm");
						BitmapRaw.Save(strFile, pal, strFileRaw);
					}
				} else {
					foreach (string strFile in alsFiles) {
						string strFileTbm = strDir + Path.DirectorySeparatorChar + Path.GetFileName(strFile.Substring(0, strFile.Length - 4) + ".tbm");
						Console.WriteLine(strFile + " -> " + strFileTbm);
						Bitmap[] abm = new Bitmap[1];
						abm[0] = new Bitmap(strFile);
						if (nScale != 1.0)
							abm[0] = TBitmapTools.ScaleBitmap(abm[0], nScale, pal);
						if (fSaveScaled)
							abm[0].Save(Path.Combine(strDir, Path.GetFileName(strFile)), ImageFormat.Bmp);
						else
							TBitmap.Save(abm, pal, strFileTbm);
					}
				}
			}
		}
	}
}
