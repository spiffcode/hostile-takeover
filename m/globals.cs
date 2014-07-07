using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Collections;
using System.Collections.Specialized;
using System.Windows.Forms;
using System.Reflection;
using SpiffLib;

namespace m {
	public class Globals {
		public static Ini GobTemplatesIni = null;
		public static PropertyGrid PropertyGrid;
		public static StatusBar StatusBar;
		private static ArrayList m_alsGobImages = new ArrayList();
		public static Font LabelFont;
		public static ArrayList Plugins = new ArrayList();
		private static bool m_fKit = true;

		public Globals() {
#if false
			// For the time being, we can execute without gob templates.
			// Fix this approach.
			OpenFileDialog frmOpen = new OpenFileDialog();
			frmOpen.Filter = "pp (*.*)|*.*";
			frmOpen.Title = "Templates File";
			if (frmOpen.ShowDialog() != DialogResult.Cancel)
				GobTemplatesIni = new Ini(frmOpen.FileName);
#endif
			LabelFont = new Font("Arial", 8);
		}

		public static void InitKit() {
			try {
				Ini ini = new Ini(Application.ExecutablePath.Replace(".exe", ".ini"));
				Globals.SetKit(ini["General"]["Kit"].Value == bool.TrueString);
			} catch {
				Globals.SetKit(true);
			}
		}

		public static bool IsKit() {
			return m_fKit;
		}

		public static void SetKit(bool fKit) {
			m_fKit = fKit;
		}

		public static GobImage GetGobImage(String strName, bool fTight) {
			// See if it already exists
			foreach (GobImage gimg in m_alsGobImages) {
				if (strName == gimg.Name)
					return gimg;
			}

			// See if it can be loaded
			GobImage gimgT = GobImage.Load(strName, fTight);
			if (gimgT != null)
				m_alsGobImages.Add(gimgT);
			return gimgT;
		}
	}

	public class GobImage {
		public String Name;
		ArrayList m_alsBitmapSides = new ArrayList();
		Point m_ptOriginUnscaled;
		Bitmap m_bmUnscaled;

		public static GobImage Load(String strName, bool fTight) {
			// Load in from embedded resource
			System.Reflection.Assembly ass = typeof(GobImage).Module.Assembly;
			Stream stm = ass.GetManifestResourceStream("m.GobImages." + strName + ".png");
			if (stm == null)
				stm = ass.GetManifestResourceStream("m.GobImages." + strName + ".bmp");
			if (stm == null)
				throw new Exception("Cannot load image for " + strName);
			Bitmap bm = new Bitmap(stm);

			// Extract image and calc origin (this code is from aed)

			// 2. 'Normalize' the bitmap. Normalized bitmaps are 24-bit, 'tight',
			//    have the proper transparent color, and an origin.

			// All pixels the same color as the upper-left pixel get mapped to the
			// transparent color

			bm.MakeTransparent(bm.GetPixel(0, 0));

			Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
			SolidBrush brTransparent = new SolidBrush(clrTransparent);

			Bitmap bmT = new Bitmap(bm.Width, bm.Height, PixelFormat.Format24bppRgb);
			using (Graphics g = Graphics.FromImage(bmT)) {

				// Prep the new image by filling with the transparent color

				g.FillRectangle(brTransparent, 0, 0, bm.Width, bm.Height);

				// Convert the Bitmap to 24-bpp while leaving transparent pixels behind

				g.DrawImageUnscaled(bm, 0, 0);
			}

			bm.Dispose();
			bm = bmT;
			int xOrigin = 0;
			int yOrigin = 0;

			if (fTight) {
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
				xOrigin = bm.Width / 2 - xL;
				yOrigin = bm.Height / 2 - yT;

				bmT = new Bitmap(cx, cy);
				using (Graphics g = Graphics.FromImage(bmT)) {
					Rectangle rcT = new Rectangle(xL, yT, cx, cy);
					g.DrawImage(bm, 0, 0, rcT, GraphicsUnit.Pixel);
				}

				bm.Dispose();
				bm = bmT;
			}

			return new GobImage(strName, bm, new Point(xOrigin, yOrigin));
		}

		public GobImage(String strName, Bitmap bmUnscaled, Point ptOriginUnscaled) {
			Name = strName;
			m_bmUnscaled = bmUnscaled;
			m_ptOriginUnscaled = ptOriginUnscaled;
		}

		BitmapSides FindBitmapSides(Size sizTile) {
			BitmapSides bmsFound = null;
			foreach(BitmapSides bms in m_alsBitmapSides) {
				if (bms.m_sizTile == sizTile) {
					bmsFound = bms;
					break;
				}
			}
			if (bmsFound == null) {
				bmsFound = new BitmapSides(m_bmUnscaled, m_ptOriginUnscaled, sizTile);
				m_alsBitmapSides.Add(bmsFound);
			}
			return bmsFound;
		}

		public Bitmap[] GetBitmapSides(Size sizTile) {
			BitmapSides bms = FindBitmapSides(sizTile);
			return bms.m_abmSides;
		}

		public Size GetSize(Size sizTile) {
			Bitmap[] abmSides = GetBitmapSides(sizTile);
			return abmSides[0].Size;
		}

		public Point GetOrigin(Size sizTile) {
			BitmapSides bms = FindBitmapSides(sizTile);
			return bms.m_ptOrigin;
		}
	}

	public class BitmapSides {
		public Size m_sizTile;
		public Bitmap[] m_abmSides;
		public Point m_ptOrigin;

		public BitmapSides(Bitmap bmUnscaled, Point ptOriginUnscaled, Size sizTile) {
			m_sizTile = sizTile;
			m_abmSides = GetBitmapSides(bmUnscaled, sizTile);
			float xScale = (float)sizTile.Width / 16.0f;
			float yScale = (float)sizTile.Height / 16.0f;
			int xNew = (int)((float)ptOriginUnscaled.X * xScale + 0.5f);
			int yNew = (int)((float)ptOriginUnscaled.Y * yScale + 0.5f);
			m_ptOrigin = new Point(xNew, yNew);
		}

		Bitmap[] GetBitmapSides(Bitmap bmUnscaled, Size sizTile) {
			// Maps the blues into a new hue

			Color[][] aaclr = {
								  new Color[] {
												  // gray
												  Color.FromArgb(116, 116, 116),
												  Color.FromArgb(96, 96, 96),
												  Color.FromArgb(64, 64, 64),
												  Color.FromArgb(48, 48, 48),
												  Color.FromArgb(32, 32, 32)
											  },
								  new Color[] {
												  // blue
												  Color.FromArgb(0, 116, 232),	// (h: 149, s: 255, l: 116)
												  Color.FromArgb(0, 96, 196),		// (h: 149, s: 255, l: 98)
												  Color.FromArgb(0, 64, 120),		// (h: 147, s: 255, l: 60)
												  Color.FromArgb(0, 48, 92),		// (h: 148, s: 255, l: 46)
												  Color.FromArgb(0, 32, 64)		// (h: 149, s: 255, l: 32)
											  },
								  new Color[] {
												  // red (h: 6)
												  Color.FromArgb(232, 32, 0),
												  Color.FromArgb(196, 28, 0),
												  Color.FromArgb(128, 8, 0),
												  Color.FromArgb(92, 8, 0),
												  Color.FromArgb(64, 8, 0)
											  },
								  new Color[] {
												  // yellow (h: 42)
												  Color.FromArgb(232, 229, 0),
												  Color.FromArgb(196, 194, 0),
												  Color.FromArgb(120, 119, 0),
												  Color.FromArgb(92, 91, 0),
												  Color.FromArgb(64, 63, 0)
											  },
								  new Color[] {
												  // cyan
												  Color.FromArgb(0, 229, 232),	// (h: 128, s: 255, l: 116)
												  Color.FromArgb(0, 194, 196),	// (h: 128, s: 255, l: 98)
												  Color.FromArgb(0, 119, 120),	// (h: 128, s: 255, l: 60)
												  Color.FromArgb(0, 91, 92),		// (h: 128, s: 255, l: 46)
												  Color.FromArgb(0, 63, 64)		// (h: 128, s: 255, l: 32)
											  },
								  new Color[] {
												  // green (h: 104)
												  Color.FromArgb(0, 232, 104),
												  Color.FromArgb(0, 196, 88),
												  Color.FromArgb(0, 120, 54),
												  Color.FromArgb(0, 92, 41),
												  Color.FromArgb(0, 64, 29)
											  },
			};

			// Make the side bitmaps
			m_abmSides = new Bitmap[6];
			double nScale = (double)sizTile.Width / 16.0;
			Bitmap bmScaled = TBitmapTools.ScaleBitmap(bmUnscaled, nScale, null);
			for (int i = 0; i < 6; i++) {
				Bitmap bmSide = new Bitmap(bmScaled);
				MapColors(bmSide, aaclr[1], aaclr[i]);
				MapColors(bmSide, new Color[] { Color.FromArgb(156, 212, 248) }, new Color[] { Color.FromArgb(75, 0, 0, 0) });

				// HACK: Map the transparent color manually because Mono's MakeTransparent doesn't get the job done.
				MapColors(bmSide, new Color[] { Color.FromArgb(255, 0, 255) }, new Color[] { Color.FromArgb(0, 0, 0, 0) });
				// bmSide.MakeTransparent(Color.FromArgb(255, 255, 0, 255));
				m_abmSides[i] = bmSide;
			}

			return m_abmSides;
		}

		void MapColors(Bitmap bm, Color[] aclrFrom, Color[] aclrTo) {
			for (int y = 0; y < bm.Height; y++) {
				for (int x = 0; x < bm.Width; x++) {
					Color clr = bm.GetPixel(x, y);
					for (int n = 0; n < aclrFrom.Length; n++) {
						if (clr == aclrFrom[n]) {
							bm.SetPixel(x, y, aclrTo[n]);
							break;
						}
					}
				}
			}
		}
	}

	public class Helper {
		static public string[] GetDisplayNames(Type typ) {
			ArrayList al = new ArrayList();
			FieldInfo[] afldi = typ.GetFields();
			foreach (FieldInfo fldi in afldi) {
				DisplayNameAttribute dna = (DisplayNameAttribute)Attribute.GetCustomAttribute(fldi, typeof(DisplayNameAttribute));
				if (dna != null)
					al.Add(dna.DisplayName);
			}

			return (string[])al.ToArray(typeof(string));
		}

		static public string GetDisplayName(Type typ, string strField) {
			FieldInfo fldi = typ.GetField(strField);
			DisplayNameAttribute dna = (DisplayNameAttribute)Attribute.GetCustomAttribute(fldi, typeof(DisplayNameAttribute));
			if (dna != null)
				return dna.DisplayName;
			else
				return strField;
		}

		static public string GetDisplayName(Type typ) {
			DisplayNameAttribute dna = (DisplayNameAttribute)Attribute.GetCustomAttribute(typ, typeof(DisplayNameAttribute));
			if (dna == null)
				return null;
			return dna.DisplayName;
		}

		static public string GetDescription(Type typ) {
			DescriptionAttribute desa = (DescriptionAttribute)Attribute.GetCustomAttribute(typ, typeof(DescriptionAttribute));
			if (desa == null)
				return null;
			return desa.Description;
		}
	}
}


