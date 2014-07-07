// UNDONE: batch mode (batch from within script?)
// UNDONE: pull function names from compiled script to build menu, function call from menu
// UNDONE: events to script, e.g., anim import/load
// UNDONE: try creating new CodeItems instead of reinstancing the engine

using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Xml;
using System.Collections;
using System.Diagnostics;
using System.Threading;
using System.Net;
using System.Windows.Forms;
using System.Text;	// For ASCIIEncoding
using SpiffLib;

namespace AED {
	/// <summary>
	/// Summary description for AED.
	/// </summary>
	class AED {
		static bool gfVerbose = false;
		static bool gfSuperVerbose = false;

		public static int Main(string[] astrArgs)
		{
			bool fGui, fLoad = false, fImport = false, fSaveAs = false, fExport = false;
			bool fCrunch = false, fPalette = false, fValidateColors = false;
			string strLoadFileName = null, strImportFileSpec = null, strSaveAsFileName = null, strExportPath = null;
			string strPaletteFileName = null;

			fGui = astrArgs.Length == 0;

			// Parse command line arguments. Commands can be in any order.

			for (int i = 0; i < astrArgs.Length; i++) {
				switch (astrArgs[i]) {
				case "-g":
					fGui = true;
					break;

				case "-p":
					fPalette = true;
					if (i + 1 >= astrArgs.Length) {
						Console.WriteLine("Error: -p command requires a filename argument");
						return -1;
					}
					strPaletteFileName = astrArgs[++i];
					break;

				case "-v":
					gfVerbose = true;
					break;

				case "-v2":
					gfVerbose = true;
					gfSuperVerbose = true;
					break;

				case "-validatecolors":
					fValidateColors = true;
					break;

				case "-c":
					fCrunch = true;
					break;

				case "-l":
					fLoad = true;
					if (i + 1 >= astrArgs.Length) {
						Console.WriteLine("Error: -f command requires a filename argument");
						return -1;
					}
					strLoadFileName = astrArgs[++i];
					break;

				case "-i":
					fImport = true;
					if (i + 1 >= astrArgs.Length) {
						Console.WriteLine("Error: -i command requires a filespec argument");
						return -1;
					}
					strImportFileSpec = astrArgs[++i];
					break;

				case "-s":
					fSaveAs = true;
					if (i + 1 < astrArgs.Length && !astrArgs[i + 1].StartsWith("-"))
						strSaveAsFileName = astrArgs[++i];
					break;

				case "-x":
					fExport = true;
					if (i + 1 < astrArgs.Length && !astrArgs[i + 1].StartsWith("-"))
						strExportPath = astrArgs[++i];
					break;

				case "-?":
				case "-help":
				case "/?":
				case "/help":
					Console.WriteLine(
							"Usage:\nAED [-v] [-c] [-p palette.pal] <-l <filename> | -i <filespec>> [-s [filename]] [-x [dir]]\n" +
							"-v: verbose\n" +
							"-v2: super verbose!\n" +
							"-c: crunch (compile to runtime format) when exporting\n" +
							"-p palette.pal: specify the palette to be matched to when crunching to 8-bpp\n" +
							"-l filename: load a .ani animation\n" +
							"-s filename: save a .ani animation\n" +
							"-i filespec: import a set of bitmaps\n" +
							"-x dir: export a set of bitmaps to the specified directory\n" +
							"-validatecolors: during crunch, validate that all pixel colors are in\n" +
							"   the specified palette\n");
					return 0;

				default:
					Console.WriteLine("Error: Unknown command line argument \"{0}\"", astrArgs[i]);
					return -1;
				}
			}

			// Additional command validation

			if (fLoad && fImport) {
				Console.WriteLine("Error: Can't use -l and -i commands together, they're ambiguous");
				return -1;
			}

			if (!fGui && !fLoad && !fImport) {
				Console.WriteLine("Error: Must specify either -l or -i command so AED has something to work with");
				return -1;
			}

			// Execute the specified commands

			AnimSet anis = new AnimSet();

			if (fLoad) {
				if (gfVerbose)
					Console.WriteLine("Loading {0}...", strLoadFileName);
				if (!Load(anis, strLoadFileName))
					return -1;
			}

			if (fImport) {
				if (gfVerbose)
					Console.WriteLine("Importing {0}...", strImportFileSpec);
				if (!Import(anis, strImportFileSpec))
					return -1;
			}

			if (fGui) {
				// This is modal and won't return until the MainForm is closed

				Application.Run(new Gui(anis.Items.Count != 0 ? anis : null));
			}

			if (fSaveAs) {
				if (gfVerbose)
					Console.WriteLine("Saving {0}...", strSaveAsFileName);
				if (!SaveAs(anis, strSaveAsFileName))
					return -1;
			}

			if (fExport) {
				if (gfVerbose)
					Console.WriteLine("Exporting {1} to {0}...", strExportPath, anis.Name);
				Palette pal = null;
				if (fPalette)
					pal = new Palette(strPaletteFileName);
				if (!Export(anis, strExportPath, pal, fCrunch, fValidateColors))
					return -1;
			}

			return 0;
		}

		// Expand the filespec into a list of filenames and pass them to the real Import()

		static public bool Import(AnimSet anis, string strFileSpec) {
			string strDir = Path.GetDirectoryName(strFileSpec);
			if (strDir == "")
				strDir = ".";
			string[] astrFiles = Directory.GetFiles(strDir, Path.GetFileName(strFileSpec));

			return Import(anis, astrFiles);
		}

		static public bool Import(AnimSet anis, string[] astrFileNames) {

			// By sorting the filenames we introduce a useful bit of determinism.

			Array.Sort(astrFileNames);

			// Enumerate all the filenames and for each one:

			Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
			SolidBrush brTransparent = new SolidBrush(clrTransparent);

			foreach (string strFile in astrFileNames) {

				// 1. Read the bitmap from it.

				Bitmap bm;
				try {
					bm = new Bitmap(strFile);
				} catch {
					Console.WriteLine("Error: Can't load \"{0}\"", strFile);
					return false;
				}

				// 2. 'Normalize' the bitmap. Normalized bitmaps are 24-bit, 'tight',
				//    have the proper transparent color, and an origin.

				// All pixels the same color as the upper-left pixel get mapped to the
				// transparent color

				bm.MakeTransparent(bm.GetPixel(0, 0));

				Bitmap bmT = new Bitmap(bm.Width, bm.Height, PixelFormat.Format24bppRgb);
				using (Graphics g = Graphics.FromImage(bmT)) {

					// Prep the new image by filling with the transparent color

					g.FillRectangle(brTransparent, 0, 0, bm.Width, bm.Height);

					// Convert the Bitmap to 24-bpp while leaving transparent pixels behind

					g.DrawImageUnscaled(bm, 0, 0);
				}

				bm = bmT;

				// UNDONE: any color mapping

				// Find the tight boundary of the image and create a new Bitmap with just
				// that portion of the Bitmap
				// OPT: this could be made faster by doing four independent edge scans

				int xL = bm.Width;
				int xR = 0;
				int yT = bm.Height;
				int yB = 0;
				for (int y = 0; y < bm.Height; y++) {
					for (int x = 0; x < bm.Width; x++) {
						Color clr = bm.GetPixel(x, y);
						if (clr != clrTransparent) {
							xL = Math.Min(xL, x);
							xR = Math.Max(xR, x);
							yT = Math.Min(yT, y);
							yB = Math.Max(yB, y);
						}
					}
				}
				int cx = xR - xL + 1;
				int cy = yB - yT + 1;
				int xOrigin = bm.Width / 2 - xL;
				int yOrigin = bm.Height / 2 - yT;

				bmT = new Bitmap(cx, cy, PixelFormat.Format24bppRgb);
				using (Graphics g = Graphics.FromImage(bmT)) {
					Rectangle rcT = new Rectangle(xL, yT, cx, cy);
					g.DrawImage(bm, 0, 0, rcT, GraphicsUnit.Pixel);
				}

				bm = bmT;

				// 3. Create a Frame object which references the normalized bitmap.

				Frame frm = new Frame(bm, xOrigin, yOrigin);

				// 4. Assign it to the proper Anim and FrameSet by using info parsed
				//    from the bitmap's original filename. If the proper Anim/FrameSet
				//    does not yet exist, create it.

				string[] astr = strFile.Substring(strFile.LastIndexOf('\\') + 1).Split('_', '.');
				if (astr.Length != 5) {
					Console.WriteLine("Warning: file {0} does not match the requisite naming pattern", strFile);
					continue;
				}
				string strAnimSet = astr[0];
				string strAnim = astr[1];
				string strFrameSet = astr[2];
				string strFrame = astr[3];

				anis.Name = strAnimSet;
				anis.AddFrame(strAnim, strFrameSet, strFrame, frm);
			}

			return true;
		}

		static bool Load(AnimSet anis, string strFileName) {
			Console.WriteLine("Loading not implemented yet.");
			return true;
		}

		// Write the .ani file and create a _ani subdirectory populated with one
		// .png for each frame

		public static bool SaveAs(AnimSet anis, string strFileName) {
			if (strFileName == null)
				strFileName = anis.Name + ".ani";
			XmlTextWriter xwtr = new XmlTextWriter(strFileName, null);
			xwtr.Formatting = Formatting.Indented;
			anis.Serialize(xwtr);
			xwtr.Close();

			// Delete/create _ani subdirectory

			string strDir = strFileName.Replace(".ani", "_ani");
			if (Directory.Exists(strDir))
				Directory.Delete(strDir, true);
			Directory.CreateDirectory(strDir);

			// Write all the frames as .png files into the _ani subdir

			foreach (DictionaryEntry deAnimSet in anis.Items) {
				Anim ani = (Anim)deAnimSet.Value;
				foreach (DictionaryEntry deAnim in ani.Items) {
					FrameSet frms = (FrameSet)deAnim.Value;
					foreach (Frame frm in frms) {
						frm.Bitmap.Save(strDir + @"\" + anis.Name + "_" + ani.Name + "_" + frms.Name + "_" + frm.Index + ".png",
							ImageFormat.Png);
					}
				}
			}

			return true;
		}

		// Write the .anir file and .bmps or .tbms, depending on fCrunch

		public static bool Export(AnimSet anis, string strExportPath, Palette pal, bool fCrunch, bool fValidateColors) {
			Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
			SolidBrush brTransparent = new SolidBrush(clrTransparent);

			if (strExportPath == null)
				strExportPath = ".";

			ASCIIEncoding enc = new ASCIIEncoding();

			FileStream stm = new FileStream(strExportPath + @"\" + anis.Name + ".anir", FileMode.Create, FileAccess.Write);
			BinaryWriter stmw = new BinaryWriter(stm);

			// Count the number of FrameSets (aka Strips)

			ushort cstpd = 0;
			foreach (DictionaryEntry deAnimSet in anis.Items) {
				Anim ani = (Anim)deAnimSet.Value;
				cstpd += (ushort)ani.Items.Count;
			}

			// Write AnimationFileHeader.cstpd

			stmw.Write(Misc.SwapUShort(cstpd));

			// Write array of offsets to StripDatas (AnimationFileHeader.aoffStpd)

			ushort offStpd = (ushort)(2 + (2 * cstpd));
			byte ibm = 0;
			ArrayList albm = new ArrayList();

			foreach (DictionaryEntry deAnimSet in anis.Items) {
				Anim ani = (Anim)deAnimSet.Value;
				foreach (DictionaryEntry deAnim in ani.Items) {
					FrameSet frms = (FrameSet)deAnim.Value;
					stmw.Write(Misc.SwapUShort(offStpd));

					// Advance offset to where the next StripData will be

					offStpd += (ushort)((26+1+1+2) /* sizeof(StripData) - sizeof(FrameData) */ +
							((1+1+1+1+1+1) /* sizeof(FrameData) */ * frms.Count));
				}
			}

			// Write array of StripDatas

			foreach (DictionaryEntry deAnimSet in anis.Items) {
				Anim ani = (Anim)deAnimSet.Value;
				foreach (DictionaryEntry deAnim in ani.Items) {
					FrameSet frms = (FrameSet)deAnim.Value;

					// Write StripData.Name

					string strName = ani.Name + " " + frms.Name;
					byte[] abT = new byte[26];
					enc.GetBytes(strName, 0, Math.Min(strName.Length, 25), abT, 0);
					abT[25] = 0;
					stmw.Write(abT);

					// Write StripData.cDelay

					stmw.Write((byte)0);

					// Write StripData.bfFlags

					stmw.Write((byte)0);

					// Write StripData.cfrmd

					ushort cfrmd = (ushort)frms.Count;
					stmw.Write(Misc.SwapUShort(cfrmd));

					// Write array of FrameDatas

					foreach (Frame frm in frms) {

						// Write FrameData.ibm (the index of the Bitmap as it will be in the Bitmap array)

						stmw.Write((byte)ibm);
						ibm++;

						// Add the Frame's Bitmap for output

						Bitmap bm = frm.Bitmap;
						if (fCrunch) {
							albm.Add(bm);
						} else {
							// If not crunching then we need to go through some special work to preserve
							// the origin point. Since the origin point is determined by finding the center
							// of the imported bitmap we preserve it at export by creating a new bitmap
							// sized so that he origin will be in its center.


							// The '+2' at the end gives us a pixel of transparent space padding the left
							// and right sides of the bitmap. We don't require this for any particular
							// reason but it comes in handy.

							int cxNew = (Math.Max(frm.OriginX, bm.Width - frm.OriginX) * 2) + 2;

							// The '+2' at the end is to ensure a single blank scanline at the top
							// which Import relies on to determine the transparent color.

							int cyNew = (Math.Max(frm.OriginY, bm.Height - frm.OriginY) * 2) + 2;

							using (Bitmap bmNew = new Bitmap(cxNew, cyNew, bm.PixelFormat)) {
								using (Graphics g = Graphics.FromImage(bmNew)) {
									g.FillRectangle(brTransparent, 0, 0, cxNew, cyNew);
									g.DrawImage(bm, cxNew / 2 - frm.OriginX, cyNew / 2 - frm.OriginY);
								}
								strName = anis.Name + "_" + ani.Name + "_" + frms.Name + "_" + frm.Index.ToString();
								bmNew.Save(strExportPath + @"\" + strName + ".png", ImageFormat.Png);
							}
						}

						// Write FrameData.xOrigin, FrameData.yOrigin

						stmw.Write((byte)frm.OriginX);
						stmw.Write((byte)frm.OriginY);

						// Write FrameData.bCustomData1, FrameData.bCustomData2, FrameData.bCustomData3

						stmw.Write((byte)0);
						stmw.Write((byte)0);
						stmw.Write((byte)0);
					}
				}
			}

			stmw.Close();

			// Write out .tbm

			if (albm.Count != 0) {
				string strFileName = strExportPath + @"\" + anis.Name + ".tbm";
				if (gfSuperVerbose)
					Console.WriteLine("Crunching and writing " + strFileName);
				TBitmap.Save((Bitmap[])albm.ToArray(typeof(Bitmap)), pal, strFileName);
			}

			return true;
		}
	}
}
