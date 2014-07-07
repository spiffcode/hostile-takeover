using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.Serialization;
using SpiffLib;

namespace m {
	[Serializable]
	public class Wall : MapItem, ISerializable {
		private int m_nHealth;
		private GobImage m_gimg;

		static private string[] s_astrBitmaps = {
			"Wall00",
			"Wall01",
			"Wall02",
			"Wall03",
			"Wall04",
			"Wall05",
			"Wall06",
			"Wall07",
			"Wall08",
			"Wall09",
			"Wall10",
			"Wall11",
			"Wall12",
			"Wall13",
			"Wall14",
			"Wall15",
		};

		public Wall(int nHealth) {
			m_nHealth = nHealth;
			m_gimg = Globals.GetGobImage(s_astrBitmaps[0], false);
		}

		public Wall(int nHealth, int tx, int ty) {
			m_nHealth = nHealth;
			m_tx = tx;
			m_ty = ty;
			m_gimg = Globals.GetGobImage(s_astrBitmaps[0], false);
		}

		public Wall(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			m_nHealth = info.GetInt32("nHealth");
			m_gimg = Globals.GetGobImage(s_astrBitmaps[0], false);
		}

		public override void GetObjectData(SerializationInfo info, StreamingContext context) {
			base.GetObjectData(info, context);
			info.AddValue("nHealth", m_nHealth);
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
			Object[] aobj = { m_nHealth, (int)m_tx, (int)m_ty };
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

		public override Ini.Property GetIniProperty(int txOrigin, int tyOrigin) {
			// For example: W=15,80,100
			return new Ini.Property("W", m_nHealth + "," + (m_tx - txOrigin).ToString() + "," + (m_ty - tyOrigin).ToString());
		}
	}
}
