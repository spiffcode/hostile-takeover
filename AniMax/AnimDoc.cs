using System;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Soap;
using System.IO;
using System.Collections;
using System.Windows.Forms;
using System.Drawing;
using System.Text;	// For ASCIIEncoding
using SpiffLib;
using ICSharpCode.SharpZipLib.Zip;
using System.Collections.Specialized;
using System.Text.RegularExpressions;
using System.Diagnostics;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for AnimDoc.
	/// </summary>
	[Serializable] 
	public class AnimDoc : ISerializable
	{
		// Persistable state

		private int m_nTileSize;
		private XBitmapSet m_xbms;
		private StripSet m_stps;
		private int m_msFrameRate;

		//

		private bool m_fDirty = false;
		private bool m_fHires = false;
		private string m_strFileName = "untitled.amx";
		private Strip m_stpActive;
        
		// Public properties

		public XBitmapSet XBitmapSet {
			get {
				return m_xbms;
			}
		}

		public StripSet StripSet {
			get {
				return m_stps;
			}
		}

		public bool Hires {
			get {
				return m_fHires;
			}
			set {
				m_fHires = value;
			}
		}

		// Exposed for anyone who wants to keep track of this AnimDoc's ActiveStrip

		public event EventHandler ActiveStripChanged;

		public Strip ActiveStrip {
			get {
				return m_stpActive;
			}
			set {
				m_stpActive = value;
				if (ActiveStripChanged != null)
					ActiveStripChanged(this, EventArgs.Empty);
			}
		}

		public bool Dirty {
			get {
				return m_fDirty;
			}
			set {
				m_fDirty = value;
			}
		}

		public string FileName {
			get {
				return m_strFileName;
			}
			set {
				m_strFileName = value;
				Dirty = true;
			}
		}

		public int FrameRate {
			get {
				return m_msFrameRate;
			}
			set {
				m_msFrameRate = value;
			}
		}

		public int TileSize {
			get {
				return m_nTileSize;
			}
			set {
                m_nTileSize = value;
				Dirty = true;
			}
		}

		//

		public AnimDoc(int nTileSize, int cmsFrameRate) {
			m_nTileSize = nTileSize;
			m_msFrameRate = cmsFrameRate;
			m_xbms = new XBitmapSet();
			m_stps = new StripSet();
			m_fDirty = true;
		}

		public static AnimDoc Load(string strFileName) {
			if (!File.Exists(strFileName))
				throw new FileNotFoundException("File Not found", strFileName);

			return Load(strFileName, null);
		}

		public static AnimDoc Load(string strFileName, Stream stmZamx) {
			string strFileNameOrig = strFileName;
			string strExt = Path.GetExtension(strFileName).ToLower();
			bool fZip = strExt == ".zip" || strExt == ".zamx";

			string strCurrentDirSav = null;
			string strTempDir = null;

			// Remember current dir

			strCurrentDirSav = Directory.GetCurrentDirectory();

			if (fZip) {
				// Change current dir to temp dir

				strTempDir = Path.Combine(Path.GetTempPath(), "AniMax_temp_extract_dir");
				Directory.CreateDirectory(strTempDir);
				Directory.SetCurrentDirectory(strTempDir);

				// Extract the .zip to the temp dir

				ZipInputStream zipi = new ZipInputStream(stmZamx != null ? stmZamx : File.OpenRead(strFileName));
            		
				ZipEntry zipe;
				while ((zipe = zipi.GetNextEntry()) != null) {

					string strDir = Path.GetDirectoryName(zipe.Name);
					if (Path.GetExtension(zipe.Name).ToLower() == ".amx")
						strFileName = zipe.Name;

					if (strDir != null && strDir != "") {
						if (!Directory.Exists(strDir))
							Directory.CreateDirectory(strDir);
					}

					FileStream stm = File.Create(zipe.Name);
					byte[] abT = new byte[zipe.Size];

					// For some reason due to its implementation, ZipInputStream.Read can return
					// fewer than the requested number of bytes. Loop until we have them all.

					while (true) {
						int cbRead = zipi.Read(abT, 0, abT.Length);
						if (cbRead <= 0)
							break;
						stm.Write(abT, 0, cbRead);
					}

					stm.Close();
				}

				zipi.Close();

				// Convert filename from, say, c:\ht\data\foo.zip or foo.zamx to foo.amx

				strFileName = Path.GetFileNameWithoutExtension(strFileName) + ".amx";
			}

			FileStream stmAmx = File.Open(strFileName, FileMode.Open, FileAccess.Read);
			SoapFormatter spfmt = new SoapFormatter();
			spfmt.AssemblyFormat = System.Runtime.Serialization.Formatters.FormatterAssemblyStyle.Simple;
			spfmt.Binder = new RelaxedSerializationBinder();

			if (!fZip) {
				// If .amx being loaded is in a directory other than the current one, 
				// change to it so deserialization will find the contained bitmaps in
				// their proper place.

				string strPath = Path.GetDirectoryName(strFileName);
				if (strPath != null && strPath != "")
					Directory.SetCurrentDirectory(strPath);
			}			
			
			AnimDoc doc = null;
			try {
				doc = (AnimDoc)spfmt.Deserialize(stmAmx);
			} catch (Exception ex) {
				MessageBox.Show(ex.ToString());
				Console.WriteLine(ex);
			}
			stmAmx.Close();

			// Restore current dir (NOTE: can't delete temp dir until it isn't current)

			Directory.SetCurrentDirectory(strCurrentDirSav);

			if (fZip) {

				// Delete temp extraction dir and its contents

				Directory.Delete(strTempDir, true);
			}

			if (doc == null)
				return null;

			doc.m_strFileName = strFileNameOrig;
			return doc;
		}

		public void Save(string strFileName) {
			string strExt = Path.GetExtension(strFileName).ToLower();
			bool fZip = strExt == ".zip" || strExt == ".zamx";

			// Update the XBitmaps to have paths relative to the specified file
			// in a subdirectory named after the file.

			m_strFileName = strFileName;

			// Create a sub-directory for all the Bitmaps
			// UNDONE: clean it out?

			string strName = Path.GetFileNameWithoutExtension(strFileName);
			string strBitmapDir = Path.Combine(Path.GetDirectoryName(strFileName), strName);

			foreach (XBitmap xbm in XBitmapSet)
				xbm.FileName = Path.Combine(strName, Path.GetFileName(xbm.FileName));

			string strAmxFileName = fZip ? strFileName + ".temporary_file" : strFileName;

			FileStream stm = File.Open(strAmxFileName, FileMode.Create, FileAccess.Write);
			SoapFormatter spfmt = new SoapFormatter();

			// This cuts down on the verbosity of the generated XML file

			spfmt.AssemblyFormat = System.Runtime.Serialization.Formatters.FormatterAssemblyStyle.Simple;

			spfmt.Serialize(stm, this);
			stm.Close();

			if (fZip) {
				ZipOutputStream zipo = null;
				zipo = new ZipOutputStream(File.Create(strFileName));
 				zipo.SetLevel(9); // maximum compression

				// Read temporary .amx file

				stm = File.OpenRead(strAmxFileName);
				byte[] abT = new byte[stm.Length];
				stm.Read(abT, 0, abT.Length);
				stm.Close();

				// Delete temporary .amx file

				File.Delete(strAmxFileName);

				// Write .amx file to .zip

           		ZipEntry zipe = new ZipEntry(strName + ".amx");
				zipo.PutNextEntry(zipe);
				zipo.Write(abT, 0, abT.Length);

				// Write the bitmaps too

				foreach (XBitmap xbm in m_xbms) {

					// Write temporary bitmap file

					xbm.Save(strAmxFileName);

					// Read temporary bitmap file

					stm = File.OpenRead(strAmxFileName);
					abT = new byte[stm.Length];
					stm.Read(abT, 0, abT.Length);
					stm.Close();

					// Delete the temporary bitmap file

					File.Delete(strAmxFileName);

					// Write bitmap to .zip

           			zipe = new ZipEntry(Path.Combine(strName, Path.GetFileName(xbm.FileName)));
					zipo.PutNextEntry(zipe);
					zipo.Write(abT, 0, abT.Length);
				}

				zipo.Finish();
				zipo.Close();
			} else {

				// Save the Bitmaps too

				Directory.CreateDirectory(strBitmapDir);

				foreach (XBitmap xbm in m_xbms)
					xbm.Save(Path.Combine(strBitmapDir, Path.GetFileName(xbm.FileName)));
			}
			m_fDirty = false;
		}

		public static bool ParseNameValueString(string str, out string strName, out string strValue) {
			int ichEquals = str.IndexOf('=');
			if (ichEquals == -1) {
				strName = null;
				strValue = null;
				return false;
			}
			strName = str.Substring(0, ichEquals).Trim();
			strValue = str.Substring(ichEquals + 1);
			return true;
		}

		public bool Import(string[] astrFileNames) {

			// Is this a SideWinder framedata.txt file?

			if (astrFileNames.Length == 1 && Path.GetFileName(astrFileNames[0]).ToLower() == "framedata.txt") {

				// Yep, open it up and parse it

				StreamReader stmr = new StreamReader(astrFileNames[0]);
				int iLine = 0;
				string str;
				
				do {
					iLine++;
					str = stmr.ReadLine();
				} while (str == "");	// skip blank lines

				if (str == null) {
					MessageBox.Show(null, "Reached the end of the file before it was expected", "Error");
					return false;
				}

				string strName, strValue;
				if (!ParseNameValueString(str, out strName, out strValue)) {
					MessageBox.Show(null, String.Format("Syntax error on line %d: %s", iLine, str), "Error");
					return false;
				}

				if (strName != "cfrm") {
					MessageBox.Show(null, "Expected a 'cfrm =' statement but didn't find it", "Error");
					return false;
				}

				// Find a unique name for this strip

				int iStrip = 0;
				while (StripSet["strip" + iStrip] != null)
					iStrip++;
				Strip stp = new Strip("strip" + iStrip);
				StripSet.Add(stp);

				int cfr = int.Parse(strValue);
				for (int ifr = 0; ifr < cfr; ifr++) {

					// 1. Read the bitmap from it and add it to the Document's XBitmapSet

					XBitmap xbm;
					string strBitmap = "frame" + ifr + ".bmp";
					try {
						xbm = new XBitmap(strBitmap, true);
					} catch {
						MessageBox.Show(null, String.Format("Can't load \"{0}\"", strBitmap), "Error");
						return false;
					}
					XBitmapSet.Add(xbm);

					// 2. Create a Frame to go with the Bitmap and add it to the appropriate
					// Strip. If no strip exists, create one.

					Frame fr = new Frame();
					fr.BitmapPlacers.Add(new BitmapPlacer());
					fr.BitmapPlacers[0].XBitmap = xbm;
					stp[ifr] = fr;

					bool fDone = false;
					while (!fDone) {
						do {
							iLine++;
							str = stmr.ReadLine();
						} while (str == "");	// skip blank lines

						if (!ParseNameValueString(str, out strName, out strValue)) {
							MessageBox.Show(null, String.Format("Syntax error on line %d: %s", iLine, str), "Error");
							return false;
						}
						switch (strName) {
						case "flags":
							Debug.Assert(strValue.Trim() == "0");
							break;

						case "xCenter":
							fr.BitmapPlacers[0].X = int.Parse(strValue);
							break;

						case "yCenter":
							fr.BitmapPlacers[0].Y = int.Parse(strValue);
							break;

						case "xGrab":
							fr.SpecialPoint = new Point(int.Parse(strValue) - fr.BitmapPlacers[0].X, fr.SpecialPoint.Y);
							break;

						case "yGrab":
							fr.SpecialPoint = new Point(fr.SpecialPoint.X, int.Parse(strValue) - fr.BitmapPlacers[0].Y);
							break;

						case "xWidth":
							Debug.Assert(int.Parse(strValue.Trim()) == xbm.Width);
							break;

						case "yHeight":
							Debug.Assert(int.Parse(strValue.Trim()) == xbm.Height);
							fDone = true;
							break;
						}
					}
				}

			} else {
                // XBitmap encapsulates special filename rules

                astrFileNames = XBitmap.FilterFileNames(astrFileNames);

                // By sorting the filenames we introduce a useful bit of
                // determinism.

				Array.Sort(astrFileNames);

				// Enumerate all the filenames and for each one:

				foreach (string strFile in astrFileNames) {

					// 0. Verify the filename fits the pattern we're expecting

					string[] astr = strFile.Substring(strFile.LastIndexOf('\\') + 1).Split('_', '.');
					if (astr.Length != 5) {
						MessageBox.Show(null, String.Format("File {0} does not match the requisite naming pattern. Skipping and continuing.", 
							strFile), "Error");
						continue;
					}
					string strAnimDoc = astr[0];
					string strStripA = astr[1];
					string strStripB = astr[2];
					int ifr = Convert.ToInt32(astr[3]);

					// 1. Read the bitmap from it and add it to the Document's XBitmapSet

					XBitmap xbm;
					try {
						xbm = new XBitmap(strFile);
					} catch {
						MessageBox.Show(null, String.Format("Can't load \"{0}\"", strFile), "Error");
						return false;
					}
					XBitmapSet.Add(xbm);
					
					// 2. Create a Frame to go with the Bitmap and add it to the appropriate
					// Strip. If no strip exists, create one.

					Frame fr = new Frame();
					fr.BitmapPlacers.Add(new BitmapPlacer());
					fr.BitmapPlacers[0].XBitmap = xbm;
					fr.BitmapPlacers[0].X = xbm.Width / 2;
					fr.BitmapPlacers[0].Y = xbm.Height / 2;

					string strStripName = strStripA + " " + strStripB;
					Strip stp = StripSet[strStripName];
					if (stp == null) {
						stp = new Strip(strStripName);
						StripSet.Add(stp);
					}
					stp[ifr] = fr;
				}
			}

			Dirty = true;
			return true;
		}

		public bool WriteAnir(string strExportPath, string strAnimName) {
			Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
			SolidBrush brTransparent = new SolidBrush(clrTransparent);

			ASCIIEncoding enc = new ASCIIEncoding();

			FileStream stm = new FileStream(strExportPath + Path.DirectorySeparatorChar + strAnimName + ".anir", FileMode.Create, FileAccess.Write);
			BinaryWriter stmw = new BinaryWriter(stm);

			// Count the number of Strips

			uint cstpd = (uint)StripSet.Count;

			// Write AnimationFileHeader.cstpd

			stmw.Write(Misc.SwapUInt(cstpd));

			// Write array of offsets to StripDatas (AnimationFileHeader.aoffStpd)

			uint offStpd = (uint)(4 + (4 * cstpd));
			ArrayList albm = new ArrayList();

			foreach (Strip stp in StripSet) {
				stmw.Write(Misc.SwapUInt(offStpd));

				// Advance offset to where the next StripData will be

				offStpd += (uint)((26+1+1+2) /* sizeof(StripData) - sizeof(FrameData) */ + 
					((64+64+2+2+1+1+1+1+1+1+1) /* sizeof(FrameData) */ * stp.Count));

				// Force word alignment of StripDatas

				if ((offStpd & 1) == 1)
					offStpd++;
			}

			// Write array of StripDatas

			foreach (Strip stp in StripSet) {

				// Write StripData.Name

				byte[] abT = new byte[26];
				enc.GetBytes(stp.Name, 0, Math.Min(stp.Name.Length, 25), abT, 0);
				abT[25] = 0;
				stmw.Write(abT);

				// Write StripData.cHold

				stmw.Write((byte)stp.DefHoldCount);

				// Write StripData.bfFlags

				stmw.Write((byte)0);

				// Write StripData.cfrmd

				ushort cfrmd = (ushort)stp.Count;
				stmw.Write(Misc.SwapUShort(cfrmd));

				// Write array of FrameDatas

				foreach (Frame fr in stp) {

					// Add the Frame's Bitmap for output
					
					int ibm = -1;
					Bitmap bm;
					if (fr.BitmapPlacers.Count > 0) {
						bm = fr.BitmapPlacers[0].XBitmap.Bitmap;
						ibm = albm.IndexOf(bm);
						if (ibm == -1)
							ibm = albm.Add(bm);
					}

                    // Write paths for the frame images

                    String sfn = String.Empty;
                    byte[] abTitle = new byte[64];

                    for (int i = 0; i < 2; i++) {
                        sfn = String.Empty;
                        if (fr.BitmapPlacers.Count > i) {
                            sfn = fr.BitmapPlacers[i].XBitmap.FileName;
                            sfn = sfn.Replace(@"\", "/");
                        }

                        Array.Clear(abTitle, 0, abTitle.Length);
                        enc.GetBytes(sfn, 0, Math.Min(sfn.Length, 63), abTitle, 0);
                        abTitle[63] = 0;
                        stmw.Write(abTitle);
                    }

					// Write FrameData.ibm (the index of the Bitmap as it will be in the Bitmap array)

                    stmw.Write(Misc.SwapUShort((ushort)ibm));

					ibm = -1;
					if (fr.BitmapPlacers.Count > 1) {
						// Add the Frame's Bitmap for output
					
						bm = fr.BitmapPlacers[1].XBitmap.Bitmap;
						ibm = albm.IndexOf(bm);
						if (ibm == -1)
							ibm = albm.Add(bm);
					}

					// Write FrameData.ibm2 (the index of the Bitmap as it will be in the Bitmap array)

                    stmw.Write(Misc.SwapUShort((ushort)ibm));

					// Write FrameData.cHold

					stmw.Write((byte)fr.HoldCount);

					// Write FrameData.xOrigin, FrameData.yOrigin

					if (fr.BitmapPlacers.Count > 0) {
						stmw.Write((byte)fr.BitmapPlacers[0].X);
						stmw.Write((byte)fr.BitmapPlacers[0].Y);
					} else {
						stmw.Write((byte)0);
						stmw.Write((byte)0);
					}

					if (fr.BitmapPlacers.Count > 1) {
						stmw.Write((byte)fr.BitmapPlacers[1].X);
						stmw.Write((byte)fr.BitmapPlacers[1].Y);
					} else {
						stmw.Write((byte)0);
						stmw.Write((byte)0);
					}

					// Write FrameData.bCustomData1, FrameData.bCustomData2

					stmw.Write((byte)fr.SpecialPoint.X);
					stmw.Write((byte)fr.SpecialPoint.Y);

#if false
					// Write FrameData.bCustomData3

					stmw.Write((byte)0);
#endif
				}

				// Force word alignment of StripDatas given that FrameDatas are an odd
				// number of bytes long and there may be an odd number of frames.

				if ((cfrmd & 1) == 1)
					stmw.Write((byte)0);
			}

			stmw.Close();

#if false
			// Write out .tbm

			if (albm.Count != 0) {
				string strFileName = strExportPath + Path.DirectorySeparatorChar + strAnimName + ".tbm";
//				if (gfSuperVerbose)
//					Console.WriteLine("Crunching and writing " + strFileName); 
				TBitmap.Save((Bitmap[])albm.ToArray(typeof(Bitmap)), pal, strFileName);
			}
#endif

			return true;
		}

		// ISerializable interface implementation

		private AnimDoc(SerializationInfo seri, StreamingContext stmc) {
			m_xbms = (XBitmapSet)seri.GetValue("Bitmaps", typeof(XBitmapSet));
			m_stps = (StripSet)seri.GetValue("Strips", typeof(StripSet));
			m_msFrameRate = seri.GetInt32("FrameRate");

			try {
				bool fHires = seri.GetBoolean("Hires");
                if (fHires) {
                    m_nTileSize = 24;
                } else {
                    m_nTileSize = 16;
                }
			} catch {
                m_nTileSize = -1;
			}

            if (m_nTileSize == -1) {
                m_nTileSize = seri.GetInt32("TileSize");
            }
		}

		void ISerializable.GetObjectData(SerializationInfo seri, StreamingContext stmc) {
			seri.AddValue("Bitmaps", m_xbms);
			seri.AddValue("Strips", m_stps);
			seri.AddValue("FrameRate", m_msFrameRate);
            seri.AddValue("TileSize", m_nTileSize);
		}
	}

	// This class is implemented to allow one Assembly read a .amx file written by a
	// different Assembly -- what a concept!

	public class RelaxedSerializationBinder : SerializationBinder {
		public override Type BindToType(string strAssemblyName, string strTypeName) {
			return Type.GetType(strTypeName);
		}
	}
}
