using System;
using System.IO;
using System.Collections;
using System.Collections.Specialized;
using System.Drawing;
using System.Drawing.Imaging;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace SpiffLib {
	class TBitmapTools {
		public static Bitmap ScaleBitmap(Bitmap bm, double nScale, Palette palFixed) {
			Color[] aclrSpecial = new Color[] {
				// side colors
				Color.FromArgb(0, 116, 232),
				Color.FromArgb(0, 96, 196),
				Color.FromArgb(0, 64, 120),
				Color.FromArgb(0, 48, 92),
				Color.FromArgb(0, 32, 64),
				// transparent
				Color.FromArgb(255, 0, 255),
			    // shadow
				Color.FromArgb(156, 212, 248),
			};

			Bitmap bmNew = new Bitmap((int)Math.Floor(bm.Width * nScale + 0.5), (int)Math.Floor(bm.Height * nScale + 0.5));
			for (int y = 0; y < bmNew.Height; y++) {
				for (int x = 0; x < bmNew.Width; x++) {
					double nWidthRatio = (double)bm.Width / (double)bmNew.Width;
					double xLeft = (double)x * nWidthRatio;
					double nHeightRatio = (double)bm.Height / (double)bmNew.Height;
					double yTop = (double)y * nHeightRatio;
					double xRight = xLeft + 1.0 * nWidthRatio;
					if (xRight > bm.Width)
						xRight = bm.Width;
					double yBottom = yTop + 1.0 * nHeightRatio;
					if (yBottom > bm.Height)
						yBottom = bm.Height;
					DoubleRect drc = new DoubleRect(xLeft, yTop, xRight, yBottom);
					Color clrSample = SampleGobBitmap(bm, drc, aclrSpecial, 4);
					if (palFixed != null) {
						bool fSpecial = false;
						foreach (Color clrSpecial in aclrSpecial) {
							if (clrSample == clrSpecial) {
								fSpecial = true;
								break;
							}
						}
						if (!fSpecial)
							clrSample = palFixed[palFixed.FindClosestEntry(clrSample)];
					}

					bmNew.SetPixel(x, y, clrSample);
				}
			}

			return bmNew;		
		}

		static Color SampleGobBitmap(Bitmap bm, DoubleRect drc, Color[] aclrSpecial, int iclrSideLast) {
			int r = 0;
			int g = 0;
			int b = 0;

			// First figure out amount of special color

			double nAreaSide = 0.0;
			double nAreaSpecial = 0.0;
			double nAreaTotal = drc.Width * drc.Height;
			double[] anAreaColorSpecial = new double[aclrSpecial.Length];
			for (int y = (int)Math.Floor(drc.top); y < (int)Math.Ceiling(drc.bottom); y++) {
				for (int x = (int)Math.Floor(drc.left); x < (int)Math.Ceiling(drc.right); x++) {
					// Calc the area taken by this pixel fragment

					DoubleRect drcPixel = new DoubleRect(x, y, x + 1.0, y + 1.0);
					drcPixel.Intersect(drc);
					double nAreaPixel = drcPixel.Width * drcPixel.Height;

					// Is this is a special color? Remember which and area taken

					Color clr = bm.GetPixel(x, y);
					for (int iclr = 0; iclr < aclrSpecial.Length; iclr++) {
						if (clr == aclrSpecial[iclr]) {
							anAreaColorSpecial[iclr] += nAreaPixel;
							nAreaSpecial += nAreaPixel;
							if (iclr <= iclrSideLast)
								nAreaSide += nAreaPixel;
							break;
						}
					}
				}
			}

			// If percent of special color area is over a given threshold, return a special color

			if (nAreaSpecial / nAreaTotal >= 0.5) {
				// Which was most popular?

				double nAreaMax = -1.0;
				int iclrMax = -1;
				for (int iclr = 0; iclr < aclrSpecial.Length; iclr++) {
					if (anAreaColorSpecial[iclr] > nAreaMax) {
						nAreaMax = anAreaColorSpecial[iclr];
						iclrMax = iclr;
					}
				}

				// If not a side color, return it

				if (iclrMax > iclrSideLast && nAreaMax > nAreaSide)
					return aclrSpecial[iclrMax];

				// Otherwise blend and color match side colors.

				nAreaTotal = nAreaSide;
				for (int y = (int)Math.Floor(drc.top); y < (int)Math.Ceiling(drc.bottom); y++) {
					for (int x = (int)Math.Floor(drc.left); x < (int)Math.Ceiling(drc.right); x++) {
						// Is this is a special color? Remember which and area taken

						bool fSideColor = false;
						Color clr = bm.GetPixel(x, y);
						for (int iclr = 0; iclr < aclrSpecial.Length; iclr++) {
							if (clr == aclrSpecial[iclr]) {
								if (iclr <= iclrSideLast)
									fSideColor = true;
								break;
							}
						}
						if (!fSideColor)
							continue;

						// Calc the % of whole taken by this pixel fragment

						DoubleRect drcPixel = new DoubleRect(x, y, x + 1.0, y + 1.0);
						drcPixel.Intersect(drc);
						double nAreaPixel = drcPixel.Width * drcPixel.Height;
						double nPercentPixel = nAreaPixel / nAreaTotal;

						// Add in the color components

						r += (int)(clr.R * nPercentPixel);
						g += (int)(clr.G * nPercentPixel);
						b += (int)(clr.B * nPercentPixel);
					}
				}

				// Now color match to the closest side color

				int dMax = int.MaxValue;
				int iclrClosest = -1;
				for (int iclr = 0; iclr <= iclrSideLast; iclr++) {
					Color clrSide = aclrSpecial[iclr];
					int dr = r - clrSide.R;
					int dg = g - clrSide.G;
					int db = b - clrSide.B;
					int d = dr * dr + dg * dg + db * db;
					if (d < dMax) {
						dMax = d;
						iclrClosest = iclr;
					}
				}

				// Have our color...

				return aclrSpecial[iclrClosest];
			}

			// Otherwise add in the color components of each non-special color pixel fragment.
			// Subtract the special color area from the total area since we won't be
			// including it.

			nAreaTotal -= nAreaSpecial;
			for (int y = (int)Math.Floor(drc.top); y < (int)Math.Ceiling(drc.bottom); y++) {
				for (int x = (int)Math.Floor(drc.left); x < (int)Math.Ceiling(drc.right); x++) {
					// Is this is a special color? If so ignore it

					bool fSpecial = false;
					Color clr = bm.GetPixel(x, y);
					for (int iclr = 0; iclr < aclrSpecial.Length; iclr++) {
						if (clr == aclrSpecial[iclr]) {
							fSpecial = true;
							break;
						}
					}
					if (fSpecial)
						continue;
					
					// Calc the % of whole taken by this pixel fragment

					DoubleRect drcPixel = new DoubleRect(x, y, x + 1.0, y + 1.0);
					drcPixel.Intersect(drc);
					double nAreaPixel = drcPixel.Width * drcPixel.Height;
					double nPercentPixel = nAreaPixel / nAreaTotal;

					// Add in the color components

					r += (int)(clr.R * nPercentPixel);
					g += (int)(clr.G * nPercentPixel);
					b += (int)(clr.B * nPercentPixel);
				}
			}

			return Color.FromArgb(r, g, b);
		}
	}
}
