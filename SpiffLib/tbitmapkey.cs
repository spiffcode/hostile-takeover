using System;
using System.IO;
using System.Collections;
using System.Drawing;
using System.Drawing.Imaging;

namespace SpiffLib {
	/// <summary>
	/// 
	/// </summary>
	public class TBitmap {
		private Bitmap m_bm;
		private Palette m_pal;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFile"></param>
		/// <param name="pal"></param>
		public TBitmap(string strFile, Palette pal) {
			m_bm = new Bitmap(strFile);
			m_pal = pal;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="bm"></param>
		/// <param name="pal"></param>
		public TBitmap(Bitmap bm, Palette pal) {
			m_bm = new Bitmap(bm);
			m_pal = pal;
		}

		/// <summary>
		/// 
		/// </summary>
		public void Dispose() {
			m_bm.Dispose();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="clrMismatch"></param>
		/// <returns></returns>
		public unsafe bool CheckColorsMatch(out Color clrMismatch) {
			// Lock down bits for speed
			Rectangle rc = new Rectangle(0, 0, m_bm.Width, m_bm.Height);
			BitmapData bmd = m_bm.LockBits(rc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
			byte *pbBase = (byte *)bmd.Scan0.ToPointer();

			// Make sure each pixel is an exact match
			clrMismatch = Color.FromArgb(0, 0, 0);
			for (int y = 0; y < m_bm.Height; y++) {
				for (int x = 0; x < m_bm.Width; x++) {
					byte *pb = pbBase + y * bmd.Stride + x * 3;
					Color clr = Color.FromArgb(pb[2], pb[1], pb[0]);
					if (m_pal[m_pal.FindClosestEntry(clr)] != clr) {
						m_bm.UnlockBits(bmd);
						clrMismatch = clr;
						return false;
					}
				}
			}
			m_bm.UnlockBits(bmd);
			return true;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFile"></param>
		public unsafe void Save(string strFile) {
			// Find the palette index for the transparent color
			Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
			int iclrTransparent = m_pal.FindClosestEntry(clrTransparent);

			// Lock down bits for speed
			Rectangle rc = new Rectangle(0, 0, m_bm.Width, m_bm.Height);
			BitmapData bmd = m_bm.LockBits(rc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
			byte *pbBase = (byte *)bmd.Scan0.ToPointer();

			// Map all the pixels in the bitmap to 'nearest' color palette indices
			byte[] ab = new byte[(m_bm.Width + 1 & ~1) * m_bm.Height];
			int i = 0;
			for (int y = 0; y < m_bm.Height; y++) {
				for (int x = 0; x < m_bm.Width; x++) {
					byte *pb = pbBase + y * bmd.Stride + x * 3;
					Color clr = Color.FromArgb(pb[2], pb[1], pb[0]);
					if (clr == Color.FromArgb(156, 212, 248))
						clr = Color.FromArgb(0, 0, 0);
					ab[i++] = (byte)m_pal.FindClosestEntry(clr);
				}
				if ((m_bm.Width & 1) == 1)
					ab[i++] = (byte)iclrTransparent;
			}
			m_bm.UnlockBits(bmd);

			// Write bitmap header, bits
			BinaryWriter bwtr = new BinaryWriter(new FileStream(strFile, FileMode.Create, FileAccess.Write));
			bwtr.Write(Misc.SwapUShort((ushort)TbmType.ColorKey));
			bwtr.Write(Misc.SwapUShort((ushort)m_bm.Width));
			bwtr.Write(Misc.SwapUShort((ushort)m_bm.Height));
			bwtr.Write((ushort)(((byte)iclrTransparent << 8) | (byte)iclrTransparent));
			bwtr.Write(ab);
			bwtr.Close();
		}
	}
}
