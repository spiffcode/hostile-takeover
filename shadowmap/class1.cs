using System;
using System.Drawing;
using System.IO;
using SpiffLib;

namespace shadowmap
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
		static unsafe void Main(string[] args)
		{
			// Get parameters
			Palette palIn = new Palette(args[0]);
			string strFileOut = args[1];
			double dAlpha = Double.Parse(args[2]);

			// Create mapping
			byte[] ab = new byte[palIn.Length];
			Palette palInHSB = new Palette(palIn.Length);
			for (int iclr = 0; iclr < palIn.Length; iclr++) {
				Color clr = palIn[iclr];
				double h = clr.GetHue();
				double s = clr.GetSaturation();
				double l = clr.GetBrightness();
				double r;
				double g;
				double b;
				MyHSLtoRGB(h, s, l * dAlpha, &r, &g, &b);
				Color clrShadow = Color.FromArgb((int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
				ab[iclr] = (byte)palIn.FindClosestEntry(clrShadow);
			}

			// Write palette mapping
			Stream stm = new FileStream(strFileOut, FileMode.Create, FileAccess.Write, FileShare.None);
			BinaryWriter bwtr = new BinaryWriter(stm);
			bwtr.Write(ab);
			bwtr.Close();

#if false
			// Check it
			Palette palCheck = new Palette(palIn.Length);
			for (int iclr = 0; iclr < palIn.Length; iclr++)
				palCheck[iclr] = palIn[ab[iclr]];
			palCheck.SaveJasc("shadow.pal");
#endif
		}

		// .NET doesn't seem to have HSL->RGB mapping, only RGB->HSL.

		/*
		 * given h,s,l on [0..1],
		 * return r,g,b on [0..1]
		 */
		unsafe static void
			HSL_to_RGB(double h, double sl, double l, double *r, double *g, double *b) {
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

		unsafe static void MyHSLtoRGB(double h, double s, double l, double *pr, double *pg, double *pb) {
			// From Graphics Gems. Convert Foley's 0..360 to 0..1

			HSL_to_RGB(h / 360.0, s, l, pr, pg, pb);
		}
	}
}
