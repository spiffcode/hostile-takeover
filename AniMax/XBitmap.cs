using System;
using System.Collections;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.Serialization;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for XBitmap.
	/// </summary>

	// UNDONE: implement IDisposable and m_bm.Dispose()

	[Serializable]
	public class XBitmap : ISerializable {
		private Bitmap m_bm;
        private Bitmap m_bmBlack;
		private string m_strFileName;
		private bool m_fDirty = false;

		public XBitmap(string strFileName) {
			Load(strFileName, false);
		}

		public XBitmap(string strFileName, bool fFrame) {
			Load(strFileName, true);
		}

        private XBitmap(Bitmap bm, Bitmap bmBlack, string strFileName,
                bool fDirty) {
            m_bm = bm;
            m_bmBlack = bmBlack;
            m_strFileName = strFileName;
            m_fDirty = fDirty;
        }

        public XBitmap Clone() {
            return new XBitmap((Bitmap)m_bm.Clone(),
                    m_bmBlack == null ? null : (Bitmap)m_bmBlack.Clone(),
                    (string)m_strFileName.Clone(), m_fDirty);
        }

        public static string[] FilterFileNames(string[] astrFileNames) {
            ArrayList alFilenames = new ArrayList();
            foreach (string strFile in astrFileNames) {
                try {
                    HasBlackCompanion(strFile);
                    alFilenames.Add(strFile);
                } catch (Exception) {
                }
            }
            return (string[])alFilenames.ToArray(typeof(string));
        }

		// Public properties

		public Bitmap Bitmap {
			get {
				return m_bm;
			}
			set {
                if (m_bmBlack != null) {
                    throw new Exception("Setting bitmap on XBitmap with black counterpart!");
                }
				m_bm = value;
				m_fDirty = true;
			}
		}

		public string FileName {
			get {
				return m_strFileName;
			}
			set {
				m_strFileName = value;
			}
		}

		public int Width {
			get {
				return m_bm.Width;
			}
		}

		public int Height {
			get {
				return m_bm.Height;
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

        private static bool HasBlackCompanion(string strFile) {
            // 32 bit XBitmaps have a companion black_<filename>, which
            // is used for side color extraction. 8 bit XBitmaps
            // do not have this. Encapsulate this knowledge here in XBitmap.

            string strPath = Path.GetDirectoryName(strFile);
            string strFileName = Path.GetFileName(strFile);

            // Ensure the filename doesn't start with black_, otherwise raise
            // an exception.

            if (strFile.StartsWith("black_")) {
                throw new Exception("image file with black_: " + strFile);
            }

            // File is ok; see if there is a black_ counterpart. If there is,
            // this bitmap file is meant to be 32 bit
            if (File.Exists(Path.Combine(strPath, "black_" + strFileName))) {
                return true;
            }
            return false;
        }

		//

		private void Load(string strFile, bool fUseFirstPaletteEntryAsTransparentColor) {
            if (HasBlackCompanion(strFile)) {
                Load32(strFile);
            } else {
                Load8(strFile, fUseFirstPaletteEntryAsTransparentColor);
            }
        }

        private void Load32(string strFile) {
            if (!HasBlackCompanion(strFile)) {
                throw new Exception("doesn't have black_ counterpart: " +
                        strFile);
            }
            m_strFileName = strFile;
            using (var bmTemp = new Bitmap(strFile))
                m_bm = new Bitmap(bmTemp);
			string strPath = Path.GetDirectoryName(strFile);
			string strFileName = Path.GetFileName(strFile);
            using (var bmpTemp = new Bitmap(Path.Combine(strPath, "black_" + strFileName)))
                m_bmBlack = new Bitmap(bmTemp);
        }

		private void Load8(string strFileName, bool fUseFirstPaletteEntryAsTransparentColor) {
			m_strFileName = strFileName;
			m_bm = new Bitmap(strFileName);
            m_bmBlack = null;

			// All pixels the same color as the upper-left pixel get mapped to the
			// transparent color

			Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
			SolidBrush brTransparent = new SolidBrush(clrTransparent);

			m_bm.MakeTransparent(fUseFirstPaletteEntryAsTransparentColor ? m_bm.Palette.Entries[0] : m_bm.GetPixel(0, 0));

			Bitmap bmT = new Bitmap(m_bm.Width, m_bm.Height, PixelFormat.Format24bppRgb);
			using (Graphics g = Graphics.FromImage(bmT)) {

				// Prep the new image by filling with the transparent color

				g.FillRectangle(brTransparent, 0, 0, m_bm.Width, m_bm.Height);

				// Convert the Bitmap to 24-bpp while leaving transparent pixels behind

				g.DrawImageUnscaled(m_bm, 0, 0);
				m_bm.Dispose();
			}

			m_bm = bmT;
//			m_bm.MakeTransparent(clrTransparent);
		}

		public void Save(string strFileName) {
			if (strFileName == null) {
				strFileName = m_strFileName;
            }
            if (!System.IO.File.Exists(strFileName))
                m_bm.Save(strFileName);

            if (m_bmBlack != null) {
                string strPath = Path.GetDirectoryName(strFileName);
                string strFileT = Path.GetFileName(strFileName);
                string strBlackPath = Path.Combine(strPath, "black_" + strFileT);
                if (!System.IO.File.Exists(strBlackPath))
                    m_bmBlack.Save(strBlackPath);
            }

			m_fDirty = false;
		}

        /*
         * given h,s,l on [0..1],
         * return r,g,b on [0..1]
         * From Graphics Gems
         */
        unsafe static void Hsl2Rgb(double h, double sl, double l,
                double *r, double *g, double *b) {
            double v;

            v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
            if (v <= 0) {
                *r = *g = *b = 0.0;
            } else {
                double m;
                double sv;
                int sextant;
                double fract, vsf, mid1, mid2;

                m = l + l - v;
                sv = (v - m ) / v;
                h *= 6.0;
                sextant = (int)h;    
                fract = h - sextant;
                vsf = v * sv * fract;
                mid1 = m + vsf;
                mid2 = v - vsf;
                switch (sextant) {
                case 0: *r = v; *g = mid1; *b = m; break;
                case 1: *r = mid2; *g = v; *b = m; break;
                case 2: *r = m; *g = v; *b = mid1; break;
                case 3: *r = m; *g = mid2; *b = v; break;
                case 4: *r = mid1; *g = m; *b = v; break;
                case 5: *r = v; *g = m; *b = mid2; break;
                }
            }
        }

        unsafe static void Hsl2Rgb360(double h, double s, double l,
                RgbData *prgb) {
            // Convert 0..360 to 0..1
            double r, g, b;
            Hsl2Rgb(h / 360.0, s, l, &r, &g, &b);
            prgb->bRed = (byte)(r * 255.0);
            prgb->bGreen = (byte)(g * 255.0);
            prgb->bBlue = (byte)(b * 255.0);
        }

		struct RgbaData {
			public RgbaData(byte bAlpha, byte bRed, byte bGreen, byte bBlue) {
                this.bAlpha = bAlpha;
				this.bRed = bRed;
				this.bGreen = bGreen;
				this.bBlue = bBlue;
			}

			public byte bBlue;
			public byte bGreen;
			public byte bRed;
            public byte bAlpha;
		}

		struct RgbData {
			public RgbData(byte bRed, byte bGreen, byte bBlue) {
				this.bRed = bRed;
				this.bGreen = bGreen;
				this.bBlue = bBlue;
			}

			public byte bBlue;
			public byte bGreen;
			public byte bRed;
		}

		static RgbData[] argbSide = { 
                new RgbData(232, 32, 0),
                new RgbData(196, 28, 0),
                new RgbData(128, 8, 0),
                new RgbData(92, 8, 0),
                new RgbData(64, 8, 0),
        };

		public unsafe void SuperBlt(int xSrc, int ySrc,
                BitmapData bmdDst, int xDst, int yDst, int cx, int cy,
                bool fMapSideColors) {
            if (m_bmBlack == null) {
                SuperBlt8(xSrc, ySrc, bmdDst, xDst, yDst, cx, cy,
                        fMapSideColors);
            } else {
                SuperBlt32(xSrc, ySrc, bmdDst, xDst, yDst, cx, cy,
                        fMapSideColors);
            }
        }
    
		public unsafe void SuperBlt32(int xSrc, int ySrc,
                BitmapData bmdDst, int xDst, int yDst, int cx, int cy,
                bool fMapSideColors) {

			// If completely off dst bounds, just return.

			if ((xDst >= bmdDst.Width || xDst + cx < 0) ||
                    (yDst >= bmdDst.Height) || (yDst + cy < 0)) {
				return;
            }

			// Dst clip

			if (xDst + cx > bmdDst.Width)
				cx = bmdDst.Width - xDst;
			if (yDst + cy > bmdDst.Height)
				cy = bmdDst.Height - yDst;

			if (xDst < 0) {
				cx += xDst;
				xSrc -= xDst; 
				xDst = 0;
			}

			if (yDst < 0) {
				cy += yDst;
				ySrc -= yDst;
				yDst = 0;
			}

            BitmapData bmdSrc = m_bm.LockBits(
                    new Rectangle(0, 0, m_bm.Width, m_bm.Height), 
                    ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
            BitmapData bmdSrcBlack = m_bmBlack.LockBits(
                    new Rectangle(0, 0, m_bmBlack.Width, m_bmBlack.Height), 
                    ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);

			RgbaData* prgbSrc = (RgbaData*)((byte*)bmdSrc.Scan0 +
                    (ySrc * bmdSrc.Stride) + (xSrc * sizeof(RgbaData)));
			RgbaData* prgbSrcBlack = (RgbaData*)((byte*)bmdSrcBlack.Scan0 +
                    (ySrc * bmdSrcBlack.Stride) + (xSrc * sizeof(RgbaData)));
			RgbData* prgbDst = (RgbData*)((byte*)bmdDst.Scan0 +
                    (yDst * bmdDst.Stride) + (xDst * sizeof(RgbData)));

			while (cy-- > 0) {
				RgbaData* prgbSrcT = prgbSrc;
                RgbaData* prgbSrcBlackT = prgbSrcBlack;
				RgbData* prgbDstT = prgbDst;

				for (int x = 0; x < cx; x++) {
					RgbaData rgbSrc = *prgbSrcT++;
					RgbaData rgbSrcBlack = *prgbSrcBlackT++;

                    // rgbSrc = huemap(rgbSrc - rgbSrcBlack) + rgbSrcBlack

                    RgbData rgbT;
                    int v;
                    v = rgbSrc.bRed - rgbSrcBlack.bRed;
                    if (v < 0) {
                        v = 0;
                    }
                    rgbT.bRed = (byte)v;

                    v = rgbSrc.bGreen - rgbSrcBlack.bGreen;
                    if (v < 0) {
                        v = 0;
                    }
                    rgbT.bGreen = (byte)v;

                    v = rgbSrc.bBlue - rgbSrcBlack.bBlue;
                    if (v < 0) {
                        v = 0;
                    }
                    rgbT.bBlue = (byte)v;

                    Color clrT = Color.FromArgb(rgbT.bRed, rgbT.bGreen,
                            rgbT.bBlue);
                    double hue = clrT.GetHue();
                    if (fMapSideColors) {
                        hue -= 235.0;
                        if (hue < 0.0) {
                            hue += 360.0;
                        }
                    }
                    Hsl2Rgb360(hue, clrT.GetSaturation(), clrT.GetBrightness(),
                            &rgbT);

                    // Add new rgb back to black
                    v = rgbSrcBlack.bRed + rgbT.bRed;
                    if (v > 255) {
                        v = 255;
                    }
                    rgbSrc.bRed = (byte)v;

                    v = rgbSrcBlack.bGreen + rgbT.bGreen;
                    if (v > 255) {
                        v = 255;
                    }
                    rgbSrc.bGreen = (byte)v;

                    v = rgbSrcBlack.bBlue + rgbT.bBlue;
                    if (v > 255) {
                        v = 255;
                    }
                    rgbSrc.bBlue = (byte)v;

                    // Alpha blend into dest
                    double alpha = (double)rgbSrc.bAlpha / 255.0;
                    double d;
                    d = (double)rgbSrc.bRed * alpha +
                            (double)prgbDstT->bRed * (1.0 - alpha);
                    if (d > 255.0) {
                        d = 255.0;
                    }
                    prgbDstT->bRed = (byte)d;

                    d = (double)rgbSrc.bGreen * alpha +
                            (double)prgbDstT->bGreen * (1.0 - alpha);
                    if (d > 255.0) {
                        d = 255.0;
                    }
                    prgbDstT->bGreen = (byte)d;

                    d = (double)rgbSrc.bBlue * alpha +
                            (double)prgbDstT->bBlue * (1.0 - alpha);
                    if (d > 255.0) {
                        d = 255.0;
                    }
                    prgbDstT->bBlue = (byte)d;
                    prgbDstT++;
				}

				// Advance to next scan line

				prgbDst = (RgbData*)(((byte*)prgbDst) + bmdDst.Stride);
				prgbSrc = (RgbaData*)(((byte*)prgbSrc) + bmdSrc.Stride);
				prgbSrcBlack = (RgbaData*)(((byte*)prgbSrcBlack) +
                        bmdSrcBlack.Stride);
			}

            m_bm.UnlockBits(bmdSrc);
            m_bmBlack.UnlockBits(bmdSrcBlack);
        }

        // Skips dst where src has transparent color.
		// Darkens dst where src has shadow color.
		// Translates side colors.
		// NOTE: Performs dst but not src clipping!!!
		// NOTE: Assumes src and dst BitmapData are PixelFormat.Format24bppRgb

		public unsafe void SuperBlt8(int xSrc, int ySrc,
                BitmapData bmdDst, int xDst, int yDst, int cx, int cy,
                bool fMapSideColors) {

			// If completely off dst bounds, just return.

			if ((xDst >= bmdDst.Width || xDst + cx < 0) || (yDst >= bmdDst.Height) || (yDst + cy < 0))
				return;

			// Dst clip

			if (xDst + cx > bmdDst.Width)
				cx = bmdDst.Width - xDst;
			if (yDst + cy > bmdDst.Height)
				cy = bmdDst.Height - yDst;

			if (xDst < 0) {
				cx += xDst;
				xSrc -= xDst; 
				xDst = 0;
			}

			if (yDst < 0) {
				cy += yDst;
				ySrc -= yDst;
				yDst = 0;
			}

            BitmapData bmdSrc = m_bm.LockBits(
                    new Rectangle(0, 0, m_bm.Width, m_bm.Height), 
                    ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);

			RgbData* prgbSrc = (RgbData*)((byte*)bmdSrc.Scan0 + (ySrc * bmdSrc.Stride) + (xSrc * sizeof(RgbData)));
			RgbData* prgbDst = (RgbData*)((byte*)bmdDst.Scan0 + (yDst * bmdDst.Stride) + (xDst * sizeof(RgbData)));

			while (cy-- > 0) {
				RgbData* prgbDstT = prgbDst;
				RgbData* prgbSrcT = prgbSrc;

				for (int x = 0; x < cx; x++) {
					RgbData rgbSrc = *prgbSrcT++;

					// Handle shadow color

					if (rgbSrc.bRed == 156 && rgbSrc.bGreen == 212 & rgbSrc.bBlue == 248) {
						prgbDstT->bRed = (byte)((prgbDstT->bRed * 60) / 100);
						prgbDstT->bGreen = (byte)((prgbDstT->bGreen * 60) / 100);
						prgbDstT->bBlue = (byte)((prgbDstT->bBlue * 60) / 100);
						prgbDstT++;

						// Handle transparent color

					} else if (rgbSrc.bRed == 255 && rgbSrc.bGreen == 0 && rgbSrc.bBlue == 255) {
						prgbDstT++;

						// Handle side colors

					} else if (fMapSideColors) {
						if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 116 && rgbSrc.bBlue == 232) {
							*prgbDstT++ = argbSide[0];
						} else if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 96 && rgbSrc.bBlue == 196) {
							*prgbDstT++ = argbSide[1];
						} else if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 64 && rgbSrc.bBlue == 120) {
							*prgbDstT++ = argbSide[2];
						} else if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 48 && rgbSrc.bBlue == 92) {
							*prgbDstT++ = argbSide[3];
						} else if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 32 && rgbSrc.bBlue == 64) {
							*prgbDstT++ = argbSide[4];
						} else {
							*prgbDstT++ = rgbSrc;
						}

						// Just copy everything else unaltered

					} else {
						*prgbDstT++ = rgbSrc;
					}
				}

				// Advance to next scan line

				prgbDst = (RgbData*)(((byte*)prgbDst) + bmdDst.Stride);
				prgbSrc = (RgbData*)(((byte*)prgbSrc) + bmdSrc.Stride);
			}

            m_bm.UnlockBits(bmdSrc);
		}

		public static Rectangle CalcRealBounds(Bitmap bm) {
			Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
			SolidBrush brTransparent = new SolidBrush(clrTransparent);

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

#if false
			Bitmap bmT = new Bitmap(cx, cy, PixelFormat.Format24bppRgb);
			using (Graphics g = Graphics.FromImage(bmT)) {
				Rectangle rcT = new Rectangle(xL, yT, cx, cy);
				g.DrawImage(bm, 0, 0, rcT, GraphicsUnit.Pixel);
			}
#endif
			return new Rectangle(xL, yT, cx, cy);
		}

		public Bitmap MakeThumbnail(int cxThumb, int cyThumb, bool fFilter) {
			Rectangle rcSrc = XBitmap.CalcRealBounds(m_bm);
			if (rcSrc.Width <= 0 || rcSrc.Height <= 0)
				return m_bm;

			int cxy = Math.Max(rcSrc.Width, rcSrc.Height);
			Bitmap bmDst = new Bitmap(cxy, cxy);
			Graphics gDst = Graphics.FromImage(bmDst);
			gDst.Clear(Color.White);
			gDst.Dispose();
			int xDst = (bmDst.Width - rcSrc.Width) / 2;
			int yDst = (bmDst.Height - rcSrc.Height) / 2;

			BitmapData bmdDst = bmDst.LockBits(
                new Rectangle(0, 0, bmDst.Width, bmDst.Height), 
				ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);

			SuperBlt(rcSrc.X, rcSrc.Y, bmdDst, xDst, yDst,
                    rcSrc.Width, rcSrc.Height, false);

			bmDst.UnlockBits(bmdDst);

			Bitmap bmLarge = new Bitmap(cxThumb, cyThumb);
			Graphics g = Graphics.FromImage(bmLarge);
			InterpolationMode imOld = g.InterpolationMode;
			g.InterpolationMode = fFilter ? InterpolationMode.Bicubic : InterpolationMode.NearestNeighbor;
			PixelOffsetMode pomOld = g.PixelOffsetMode;
			g.PixelOffsetMode = PixelOffsetMode.Half;

			g.DrawImage(bmDst, 0, 0, cxThumb, cyThumb);

			g.PixelOffsetMode = pomOld;
			g.InterpolationMode = imOld;

			g.Dispose();
			bmDst.Dispose();

			return bmLarge;
		}

		// ISerializable interface implementation

		private XBitmap(SerializationInfo seri, StreamingContext stmc) {
			string strFileName = seri.GetString("FileName");
			strFileName = strFileName.Replace(@"\", Path.DirectorySeparatorChar.ToString());
			Load(strFileName, false);
		}

		void ISerializable.GetObjectData(SerializationInfo seri, StreamingContext stmc) {
			seri.AddValue("FileName", m_strFileName);
		}
	}
}
