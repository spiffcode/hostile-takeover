using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using SpiffLib;
using System.Collections;
using System.Collections.Specialized;

// need limit argument (so fixed pal doesn't go above a certain limit. 128 on 8 bit, 16 on 4
// need overall palette size argument (can use -p)

namespace PalTool
{
	/// <summary>
	/// Summary description for PalToolApp.
	/// </summary>
	class PalToolApp
	{
		static StringCollection gstrcFileNames = new StringCollection();
		static bool gfPrintHistogram = false;
		static bool gf6bitRGB = false;
		static string gstrOutputFileName = null;
		static bool gfVerbose = false;
		static bool gfEliminateShadowColor = false;
		static bool gfEliminateTransparentColor = false;
		static bool gfPrintColorUsers = false;
		static bool gfAnalyse = false;
		static int giclrInsert = -1;
		static int gcPaletteEntries = 256;
		static int gcColorEntries = 256;
		static bool gfPhotoshopPad = false;

		struct BitmapColorInfo {
			public string strFileName;
			public Hashtable htColorCount;
		}

		class ColorCounter : IComparable {
			public int cclr;
			public ArrayList alBitmaps = new ArrayList();

			// IComparable implementation

			public int CompareTo(object ob) {
				return cclr - ((ColorCounter)ob).cclr;
			}
		}

		static void AddFilesFromFile(string strFile) {
			char[] achDelimiter = new char[1];
			achDelimiter[0] = ' ';
			StreamReader sr = new StreamReader(strFile);
			String strLine;
			while ((strLine = sr.ReadLine()) != null) {
				if (strLine.Trim() != "") {
					string[] astrFiles = strLine.Split(achDelimiter);
					for (int i = 0; i < astrFiles.Length; i++) {
						AddFiles(astrFiles[i]);
					}
				}
			}
		}

		static void AddFiles(string strFileArg) {
			string strDir = Path.GetDirectoryName(strFileArg);
			if (strDir == "")
				strDir = ".";
			string[] astrFileNames = Directory.GetFiles(strDir, Path.GetFileName(strFileArg));

			if (astrFileNames.Length == 0) {
				gstrcFileNames.Add(strFileArg);
			} else {
				foreach (string strFileName in astrFileNames) {
					if (strFileName.ToLower().EndsWith(".ani") || strFileName.ToLower().EndsWith(".amx")) {
						string strT = Path.GetDirectoryName(strFileName) + @"\" + Path.GetFileNameWithoutExtension(strFileName) + @"\*.png";
						string[] astrT = Directory.GetFiles(Path.GetDirectoryName(strT), Path.GetFileName(strT));
						gstrcFileNames.AddRange(astrT);
					} else {
						gstrcFileNames.Add(strFileName);
					}
				}
			}
		}

		static unsafe int Main(string[] astrArgs) {
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

				case "-i":
					giclrInsert = Int32.Parse(astrArgs[++i]);
					break;

				case "-p":
					gcPaletteEntries = Int32.Parse(astrArgs[++i]);
					break;

				case "-c":
					gcColorEntries = Int32.Parse(astrArgs[++i]);
					break;

				case "-t":
					gfEliminateTransparentColor = true;
					break;

				case "-s":
					gfEliminateShadowColor = true;
					break;

				case "-v":
					gfVerbose = true;
					break;

				case "-u":
					gfPrintColorUsers = true;
					break;

				case "-6":
					gf6bitRGB = true;
					break;

				case "-h":
					gfPrintHistogram = true;
					break;

				case "-a":
					gfAnalyse = true;
					break;

				case "-n":
					gfPhotoshopPad = true;
					break;

				case "-f":
					AddFilesFromFile(astrArgs[++i]);
					break;

				case "-o":
					if (i + 1 >= astrArgs.Length) {
						Console.WriteLine("Error: -o command requires a filename argument");
						return -1;
					}
					gstrOutputFileName = astrArgs[++i];
					break;

				default:
					if (astrArgs[i][0] == '-') {
						Console.WriteLine("Error: invalid flag '{0}'", astrArgs[i]);
						return -1;
					}

					// Assume all 'unassociated' arguments are input filenames (potentially wildcarded)

					AddFiles(astrArgs[i]);
					break;
				}
			}

			if (gstrcFileNames.Count == 0) {
				Console.WriteLine("Error: no files specified");
				return -1;
			}

			// Build a list of the colors used and count of uses for each bitmap

			ArrayList alstBitmapColorInfos = new ArrayList();

			foreach (string strFileName in gstrcFileNames) {

				BitmapColorInfo bci = new BitmapColorInfo();
				bci.strFileName = strFileName;
				bci.htColorCount = new Hashtable();

				// Handle .PALs

				if (strFileName.ToLower().EndsWith(".pal")) {
					Palette pal = new Palette(strFileName);

					int i = 0;
					foreach (Color clr in pal.Colors) {
						Color clrT = clr;
						if (gf6bitRGB)
							clrT = Color.FromArgb(clr.R & 0xfc, clr.G & 0xfc, clr.B & 0xfc);

						// This hack causes the .PAL colors to be sorted at the head of the
						// combined palette while retaining the order they were found in the .PAL.

						if (!bci.htColorCount.Contains(clrT))
							bci.htColorCount[clrT] = (Int32.MaxValue / 2) - i++;
					}

				// Handle everything else (bitmaps)

				} else {
					Bitmap bm = null;
					try {
						bm = new Bitmap(strFileName);
					} catch {
						Console.WriteLine("Error: {0} is not a recognized bitmap or palette file", strFileName);
						continue;
					}

					// Prep to filter out the transparent color

					Color clrTransparent = Color.GhostWhite;
					if (gfEliminateTransparentColor)
						clrTransparent = bm.GetPixel(0, 0);

					// Prep to filter out the shadow color

					Color clrShadow = Color.GhostWhite;
					if (gfEliminateShadowColor)
						clrShadow = Color.FromArgb(156, 212, 248);

					// Keep a per-bitmap list of unique colors and how many times they're used

					Hashtable ht = bci.htColorCount;
			
					// Lock down bits for speed

					Rectangle rc = new Rectangle(0, 0, bm.Width, bm.Height);
					BitmapData bmd = bm.LockBits(rc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
					byte *pbBase = (byte *)bmd.Scan0.ToPointer();

					for (int y = 0; y < bm.Height; y++) {
						for (int x = 0; x < bm.Width; x++) {
							byte *pb = pbBase + y * bmd.Stride + x * 3;
							Color clr = Color.FromArgb(pb[2], pb[1], pb[0]);
							if (gfEliminateTransparentColor && clr == clrTransparent)
								continue;

							if (gfEliminateShadowColor && clr == clrShadow)
								continue;

							if (gf6bitRGB)
								clr = Color.FromArgb(clr.R & 0xfc, clr.G & 0xfc, clr.B & 0xfc);

							object obT = ht[clr];
							if (obT == null)
								ht[clr] = 1;
							else
								ht[clr] = 1 + (int)obT;
						}
					}
					bm.UnlockBits(bmd);
				}

				if (gfVerbose)
					Console.WriteLine("{0} uses {1} colors", strFileName, bci.htColorCount.Count);

				if (gfPrintHistogram && gfVerbose) {
					foreach (DictionaryEntry de in bci.htColorCount) {
						Color clr = (Color)de.Key;
						Console.WriteLine("{0},{1},{2} : {3} occurances", clr.R, clr.G, clr.B, (int)de.Value);
					}
					Console.WriteLine();
				}

				alstBitmapColorInfos.Add(bci);
			}

			if (alstBitmapColorInfos.Count == 0) {
				Console.WriteLine("Error: no valid bitmap files to process, terminating");
				return -1;
			}

			// Combine all the color tables and count data

			Hashtable htCombined = new Hashtable();

			foreach (BitmapColorInfo bci in alstBitmapColorInfos) {
				foreach (DictionaryEntry de in bci.htColorCount) {
					Color clr = (Color)de.Key;
					ColorCounter clrc = (ColorCounter)htCombined[clr];
					if (clrc == null) {
						clrc = new ColorCounter();
						clrc.cclr = (int)de.Value;
						htCombined[clr] = clrc;
					} else {
						int nAdd = (int)de.Value;
						if (nAdd > Int32.MaxValue / 3)
							clrc.cclr = (int)de.Value;
						else if (clrc.cclr < Int32.MaxValue / 3)
							clrc.cclr += nAdd;
					}
					clrc.alBitmaps.Add(bci.strFileName);
				}
			}

			int cclrCombined = htCombined.Count;
			Console.WriteLine("Combined palette has {0} unique colors", cclrCombined);

			// Sort everything by # colors used

			ColorCounter[] aclrcSorted = new ColorCounter[cclrCombined];
//			int i = 0;
//			foreach (ColorCounter clrc in htCombined.Values)
//				acOccurancesSorted[i++] = clrc.cclr;
			htCombined.Values.CopyTo(aclrcSorted, 0);
			Color[] aclrSorted = new Color[cclrCombined];
			htCombined.Keys.CopyTo(aclrSorted, 0);

			Array.Sort(aclrcSorted, aclrSorted);

			// Reverse so most-used colors come first
			// OPT: could do this inside the Sort above by specifying a custom IComparer

			Array.Reverse(aclrcSorted);
			Array.Reverse(aclrSorted);

			if (gfPrintHistogram || gfPrintColorUsers) {
				for (int i = 0; i < cclrCombined; i++) {
					Color clr = aclrSorted[i];
					int cOccurances = aclrcSorted[i].cclr;
					if (cOccurances >= Int32.MaxValue / 3)
						Console.WriteLine("{0},{1},{2} : preloaded", clr.R, clr.G, clr.B);
					else
						Console.WriteLine("{0},{1},{2} : {3} occurances", clr.R, clr.G, clr.B, cOccurances);

					if (gfPrintColorUsers) {
						foreach (string strFileName in aclrcSorted[i].alBitmaps) {
							Console.WriteLine("    {0}", strFileName);
						}
					}
				}
			}

			// Print warning if # of unique colors is greater than the desired palette size
			// Truncate to match the requested size since other tools depend on this

			if (cclrCombined > gcColorEntries) {
				Console.WriteLine("Warning! {0} unique colors, {1} palette entries reserved. Truncating...",  cclrCombined, gcColorEntries);
				Color[] aclrSortedT = new Color[gcColorEntries];
				Array.Copy(aclrSorted, 0, aclrSortedT, 0, gcColorEntries);
				aclrSorted = aclrSortedT;
				ColorCounter[] aclrcSortedT = new ColorCounter[gcColorEntries];
				Array.Copy(aclrcSorted, 0, aclrcSortedT, 0, gcColorEntries);
				aclrcSorted = aclrcSortedT;
				cclrCombined = gcColorEntries;
			}

			// Create the palette. Presorted colors start at 0. New colors start at giclrInsert.
			// giclrInsert == -1 means new colors are simply appended to the presorted colors.

			Color[] aclrPalette = aclrSorted;

			if (giclrInsert != -1) {
				// Init to transparent

				aclrPalette = new Color[gcColorEntries];
				for (int i = 0; i < aclrPalette.Length; i++)
					aclrPalette[i] = Color.FromArgb(255, 0, 255);

				// Insert new colors appropriately

				int iclrBase = -1;
				for (int i = 0; i < cclrCombined; i++) {
					if (aclrcSorted[i].cclr >= Int32.MaxValue / 3) {
						aclrPalette[i] = aclrSorted[i];
						continue;
					}
					if (iclrBase == -1)
						iclrBase = i;
					int iclrNew = giclrInsert + (i - iclrBase);
					if (iclrNew < aclrPalette.Length)
						aclrPalette[iclrNew] = aclrSorted[i];
				}
			}

			// Write the output palette file, if requested

			if (gstrOutputFileName != null) {
				Palette pal = new Palette(aclrPalette);
				if (gfPhotoshopPad)
					pal.Pad(gcPaletteEntries, pal[pal.Length - 1]);
				else
					pal.Pad(gcPaletteEntries, Color.FromArgb(255, 0, 255));
				pal.SaveJasc(gstrOutputFileName);
			}

			if (gfAnalyse) {
				Palette pal = new Palette(aclrPalette);

				// For each color find the nearest color in RGB space and print
				// the pair as well as the distance between them.

				for (int iclrA = 0; iclrA < aclrPalette.Length; iclrA++) {
					Color clrA = aclrPalette[iclrA];

					// Find the entry, the long way

					int nLowest = 256 * 256 * 3;
					int iLowest = 0;
					for (int iclr = 0; iclr < aclrPalette.Length; iclr++) {
						if (iclr == iclrA)
							continue;

						Color clrPal = aclrPalette[iclr];
						int dR = clrPal.R - clrA.R;
						int dG = clrPal.G - clrA.G;
						int dB = clrPal.B - clrA.B;
						int nD = dR * dR + dG * dG + dB * dB;
						if (nD < nLowest) {
							nLowest = nD;
							iLowest = iclr;
						}
					}

					Color clrB = aclrPalette[iLowest];
					double n = Math.Sqrt(nLowest);
					Console.WriteLine("{8:#.##}\t[{3}] {0},{1},{2} \t[{7}] {4},{5},{6}",
							clrA.R, clrA.G, clrA.B, iclrA, clrB.R, clrB.G, clrB.B, iLowest, n);
				}
			}

			return 0;
		}

		//

		static void PrintHelp() {
			Console.WriteLine(
					"PalTool usage:\n" +
					"PalTool [-v] [-t] [-s] [-h] [-6] [-o <filename>] file[s]\n" +
					"-v: verbose\n" +
					"-c <count>: color entries that aren't padding\n" +
					"-p <count>: total palette entry count\n" +
					"-i <index>: color index to insert new colors, can be -1\n" +
					"-t: remove transparent color(s) from combined palette\n" +
					"-s: remove shadow color from combined palette\n" +
					"-h: print histogram for combined palette\n" +
					"-u: list which bitmaps use each color\n" +
					"-6: reduce colors to 6-bit RGB before using\n" +
					"-a: analyse the resulting palette and print the results\n" +
					"-n: repeat last entry to pad out to total (good for Photoshop import)\n" +
					"-o <filename>: output combined palette to filename\n" +
					"files[s]: palette, bitmap, ani, and amx files to be processed. Wildcards allowed.");
		}
	}
}
