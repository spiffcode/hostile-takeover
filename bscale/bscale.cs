using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
//using SpiffCode;
using SpiffLib;

namespace bscale
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static int Main(string[] astrArgs)
		{
			if (astrArgs.Length < 3) {
				Console.WriteLine("Usage:\nbscale -scale <scale> -pal <palette.pal> <file(s) ...> -out <outdir>\n");
				return -1;
			}

			double nScale = 1.0;
			string strOutDir = null, strPalette = null;
			ArrayList alFileSpecs = new ArrayList();

			for (int iarg = 0; iarg < astrArgs.Length; iarg++) {
				if (astrArgs[iarg][0] == '-') {
					switch (astrArgs[iarg]) {
					case "-scale":
						nScale = double.Parse(astrArgs[++iarg]);
						break;

					case "-out":
						strOutDir = astrArgs[++iarg];
						break;

					case "-pal":
						strPalette = astrArgs[++iarg];
						break;
					}
				} else {
					alFileSpecs.Add(astrArgs[iarg]);
				}
			}

			// Read in the palette

			Palette pal = new Palette(strPalette);
			if (pal == null) {
				Console.WriteLine("Error: unable to read the palette file {0}\n", strPalette);
				return -1;
			}

			foreach (string strFileSpec in alFileSpecs) {
				Console.WriteLine("dir = " + Path.GetDirectoryName(strFileSpec) + ", file = " + Path.GetFileName(strFileSpec));
				string[] astrFiles = Directory.GetFiles(Path.GetDirectoryName(strFileSpec), Path.GetFileName(strFileSpec));

				foreach (string strFile in astrFiles) {
					Console.WriteLine(strFile);
					Bitmap bm = new Bitmap(strFile);
					Bitmap bmScaled = TBitmapTools.ScaleBitmap(bm, nScale, pal);
					if (!Directory.Exists(strOutDir))
						Directory.CreateDirectory(strOutDir);

					bmScaled.Save(strOutDir + Path.DirectorySeparatorChar + 
							Path.GetFileName(strFile), bm.RawFormat);
				}
			}

			return 0;
		}
	}
}
