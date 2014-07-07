using System;
using System.IO;
using System.Collections;
using System.Drawing;
using System.Drawing.Imaging;
using SpiffLib;

namespace SpiffLib {
	public class BitmapRaw {
		public static unsafe void Save(string strFile, Palette pal, string strFileOut) {
			Bitmap bm = new Bitmap(strFile);

			// Find the palette index for black
			int iclrBlack = pal.FindClosestEntry(Color.FromArgb(0, 0, 0));

			// Lock down bits for speed
			Rectangle rc = new Rectangle(0, 0, bm.Width, bm.Height);
			BitmapData bmd = bm.LockBits(rc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
			byte *pbBase = (byte *)bmd.Scan0.ToPointer();

			// Map all the pixels in the bitmap to 'nearest' color palette indices
			byte[] ab = new byte[((bm.Width + 1) & ~1) * bm.Height];
			int i = 0;
			for (int y = 0; y < bm.Height; y++) {
				for (int x = 0; x < bm.Width; x++) {
					byte *pb = pbBase + y * bmd.Stride + x * 3;
					Color clr = Color.FromArgb(pb[2], pb[1], pb[0]);
					if (clr == Color.FromArgb(156, 212, 248))
						clr = Color.FromArgb(0, 0, 0);
					ab[i++] = (byte)pal.FindClosestEntry(clr);
				}
				if ((bm.Width & 1) == 1)
					ab[i++] = (byte)iclrBlack;
			}
			bm.UnlockBits(bmd);

			// Write bitmap header, bits
			BinaryWriter bwtr = new BinaryWriter(new FileStream(strFileOut, FileMode.Create, FileAccess.Write));
			bwtr.Write(Misc.SwapUShort((ushort)((bm.Width + 1) & ~1)));
			bwtr.Write(Misc.SwapUShort((ushort)bm.Height));
			bwtr.Write(ab);
			bwtr.Close();

			// Done
			bm.Dispose();
		}
	}
}
