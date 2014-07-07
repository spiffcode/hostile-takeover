using System;
using System.IO;
using System.Collections;
using System.Collections.Specialized;
using System.Drawing;
using System.Drawing.Imaging;
using SpiffLib;

namespace BmpProof
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class App
	{
		static StringCollection gstrcFileNames = new StringCollection();
		static string gstrPalette;
		static Palette gpal;
		static bool gf6bitRGB = false;

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static unsafe int Main(string[] astrArgs)
		{
			// Command-line argument processing

			if (astrArgs.Length == 0) {
				PrintHelp();
				return 0;
			}

			for (int i = 0; i < astrArgs.Length; i++) {
				switch (astrArgs[i]) {
				case "-?":
					PrintHelp();
					return 0;

				case "-p":
					gstrPalette = astrArgs[++i];
					gpal = new Palette(gstrPalette);
					break;

				case "-6":
					gf6bitRGB = true;
					break;

				default:
					if (astrArgs[i][0] == '-') {
						Console.WriteLine("Error: invalid flag '{0}'", astrArgs[i]);
						return -1;
					}

					// Assume all 'unassociated' arguments are input filenames (potentially wildcarded)

					string strDir = Path.GetDirectoryName(astrArgs[i]);
					if (strDir == "")
						strDir = ".";
					string[] astrFileNames = Directory.GetFiles(strDir, Path.GetFileName(astrArgs[i]));

					if (astrFileNames.Length == 0) {
						gstrcFileNames.Add(astrArgs[i]);
					} else {
						foreach (string strFileName in astrFileNames) {
							if (strFileName.ToLower().EndsWith(".ani")) {
								string strT = Path.GetDirectoryName(strFileName) + @"\" + Path.GetFileNameWithoutExtension(strFileName) + @"\*.png";
								string[] astrT = Directory.GetFiles(Path.GetDirectoryName(strT), Path.GetFileName(strT));
								gstrcFileNames.AddRange(astrT);
							} else {
								gstrcFileNames.Add(strFileName);
							}
						}
					}
					break;
				}
			}

			if (gpal == null) {
				Console.WriteLine("A valid palette must be specified via the '-p' switch");
				return -1;
			}

			if (gstrcFileNames.Count == 0) {
				Console.WriteLine("Error: no files specified");
				return -1;
			}

			int nReturnValue = 0;
			Color clrShadow = Color.FromArgb(156, 212, 248);

			Console.Write("Verifying bitmap colors...");

			foreach (string strFileName in gstrcFileNames) {
				Hashtable htInvalidColors = new Hashtable();
				Bitmap bm = null;
				try {
					bm = new Bitmap(strFileName);
				} catch {
					Console.WriteLine("Error: {0} is not a recognized bitmap or palette file", strFileName);
					continue;
				}
		
				Color clrTransparent = bm.GetPixel(0, 0);
				if (gf6bitRGB)
					clrTransparent = Color.FromArgb(clrTransparent.R & 0xfc, clrTransparent.G & 0xfc, clrTransparent.B & 0xfc);

				// Lock down bits for speed

				Rectangle rc = new Rectangle(0, 0, bm.Width, bm.Height);
				BitmapData bmd = bm.LockBits(rc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
				byte *pbBase = (byte *)bmd.Scan0.ToPointer();

				for (int y = 0; y < bm.Height; y++) {
					for (int x = 0; x < bm.Width; x++) {
						byte *pb = pbBase + y * bmd.Stride + x * 3;
						Color clr;
						if (gf6bitRGB)
							clr = Color.FromArgb(pb[2] & 0xfc, pb[1] & 0xfc, pb[0] & 0xfc);
						else
							clr = Color.FromArgb(pb[2], pb[1], pb[0]);

						int i = gpal.FindClosestEntry(clr);
						if (gpal[i] != clr && clr != clrShadow && clr != clrTransparent && !htInvalidColors.ContainsKey(clr))
								htInvalidColors.Add(clr, clr);
					}
				}
				bm.UnlockBits(bmd);

				// Report any invalid colors

				if (htInvalidColors.Count != 0) {
					if (nReturnValue == 0)
						Console.WriteLine();
					nReturnValue = -1;

					int cclr = htInvalidColors.Count;

					Color[] aclr = new Color[cclr];
					htInvalidColors.Values.CopyTo(aclr, 0);
					Console.Write("{0} contains {1} invalid color{2} (",
							Path.GetFileName(strFileName), cclr, cclr == 1 ? "" : "s");
					for (int i = 0; i < aclr.Length; i++) {
						Color clr = aclr[i];
						Console.Write("{0},{1},{2}", clr.R, clr.G, clr.B);
						if (i != aclr.Length - 1)
							Console.Write(", ");
					}
					Console.WriteLine(")");
				}
			}

			if (nReturnValue == 0)
				Console.WriteLine("done");
			return nReturnValue;
		}

		//

		static void PrintHelp() {
			Console.WriteLine(
					"Usage: BmpProof [-6] <-p filename> file[s]\n" +
					"-6: convert colors to 6-bit precision before comparing\n" +
					"-p: specify palette of valid colors\n" +
					"files[s]: bitmap files to be processed. Wildcards allowed.");
		}
	}
}
