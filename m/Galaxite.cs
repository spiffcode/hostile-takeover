using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.Serialization;
using System.Text.RegularExpressions;
using SpiffLib;

namespace m {
	[Serializable]
	public class Galaxite : MapItem, ISerializable {
		private int m_nGx;
		private GobImage m_gimg;

		static private string[] s_astrBitmaps = {
			"Galaxite1a",
			"Galaxite1b",
			"Galaxite1c",
			"Galaxite2a",
			"Galaxite2b",
			"Galaxite2c",
			"Galaxite3a",
			"Galaxite3b",
			"Galaxite3c"
		};

		static private int[] s_anGxTranslate = { 5, 3, 1, 10, 8, 6, 15, 13, 11 };

		public Galaxite(int nGx) {
			Init(nGx, 0, 0);
		}

		public Galaxite(int nGx, int tx, int ty) {
			Init(nGx, tx, ty);
		}

		public Galaxite(string strName, string strValue, int txOrigin, int tyOrigin) {
			Regex re = new Regex(@"^(?<xl>\d+),(?<tx>\d+),(?<ty>\d+)$");
			Match m = re.Match(strValue);
			int xl = int.Parse(m.Groups["xl"].Value);
			int nGx = 0;
			for (int i = 0; i < s_anGxTranslate.Length; i++) {
				if (xl == s_anGxTranslate[i]) {
					nGx = i;
					break;
				}
			}
			Init(nGx, int.Parse(m.Groups["tx"].Value) + txOrigin,
					int.Parse(m.Groups["ty"].Value) + tyOrigin);
		}

		public Galaxite(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			m_nGx = info.GetInt32("nGx");
			m_gimg = Globals.GetGobImage(s_astrBitmaps[m_nGx], true);
		}

		public override void GetObjectData(SerializationInfo info, StreamingContext context) {
			base.GetObjectData(info, context);
			info.AddValue("nGx", m_nGx);
		}

		public override Ini.Property GetIniProperty(int txOrigin, int tyOrigin) {
			// For example: Gx=1,80,100
			return new Ini.Property("Gx", s_anGxTranslate[m_nGx] + "," + (m_tx - txOrigin).ToString() + "," + (m_ty - tyOrigin).ToString());
		}

		void Init(int nGx, int tx, int ty) {
			m_nGx = nGx;
			m_tx = tx;
			m_ty = ty;
			m_gimg = Globals.GetGobImage(s_astrBitmaps[m_nGx], true);
		}

		// IMapItem

		public override Bitmap GetBitmap(Size sizTile, TemplateDoc tmpd) {
			Bitmap[] abm = m_gimg.GetBitmapSides(sizTile);
			return abm[0];
		}

		public override Point GetCenterPoint(Size sizTile) {
			Size sizGob = m_gimg.GetSize(sizTile);
			return new Point((int)m_tx * sizTile.Width + sizGob.Width / 2, (int)m_ty * sizTile.Height + sizGob.Height / 2);
		}

		public override Rectangle GetBoundingRectAt(int x, int y, Size sizTile, TemplateDoc tmpd) {
			Size sizGob = m_gimg.GetSize(sizTile);
			return new Rectangle(x, y, sizGob.Width, sizGob.Height);
		}

		public override bool HitTest(int x, int y, Size sizTile, TemplateDoc tmpd) {
			int xT = x - (int)m_tx * sizTile.Width;
			int yT = y - (int)m_ty * sizTile.Height;
			Size sizGob = m_gimg.GetSize(sizTile);
			if (xT > 0 && xT < sizGob.Width && yT > 0 && yT < sizGob.Height) {
				Bitmap[] abm = m_gimg.GetBitmapSides(sizTile);
				return abm[0].GetPixel(xT, yT) != Color.Transparent;
			}
			return false;
		}

		public override Object Clone() {
			Object[] aobj = { m_nGx, (int)m_tx, (int)m_ty };
			return (Object)System.Activator.CreateInstance(GetType(), aobj);
		}

		public override void Draw(Graphics g, int x, int y, Size sizTile, TemplateDoc tmpd, LayerType layer, bool fSelected) {
			if (layer == LayerType.Galaxite) {
				Bitmap[] abm = m_gimg.GetBitmapSides(sizTile);
				Bitmap bm = abm[0];
				if (fSelected) {
					Rectangle rcDst = new Rectangle(x, y, bm.Width, bm.Height);
					ImageAttributes attr = new ImageAttributes();
					attr.SetGamma(0.5f);
					g.DrawImage(bm, rcDst, 0, 0, bm.Width, bm.Height, GraphicsUnit.Pixel, attr);
				} else {
					g.DrawImage(bm, x, y);
				}
			}
		}
	}
}
