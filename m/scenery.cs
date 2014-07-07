using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.Serialization;
using System.Text.RegularExpressions;
using SpiffLib;

namespace m {
	[Serializable]
	public class Scenery : MapItem, ISerializable {
		private String m_strName;
		private GobImage m_gimg;

		public Scenery(String strName) {
			m_strName = strName;
			m_gimg = Globals.GetGobImage(strName, false);
		}

		public Scenery(String strName, int tx, int ty) {
			m_strName = strName;
			m_tx = tx;
			m_ty = ty;
			m_gimg = Globals.GetGobImage(strName, false);
		}

		public Scenery(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			m_strName = info.GetString("Name");
			m_gimg = Globals.GetGobImage(m_strName, false);
		}

		public Scenery(string strName, string strValue, int txOrigin, int tyOrigin) {
			Regex re = new Regex(@"^(?<gt>\d+),(?<name>\w+).tbm,(?<tx>\d+),(?<ty>\d+)$");
			Match m = re.Match(strValue);
			m_strName = m.Groups["name"].Value;
			m_tx = int.Parse(m.Groups["tx"].Value) + txOrigin;
			m_ty = int.Parse(m.Groups["ty"].Value) + tyOrigin;
			m_gimg = Globals.GetGobImage(m_strName, false);
		}

		public override  Ini.Property GetIniProperty(int txOrigin, int tyOrigin) {
			// For example: nil=scenery,image.tbm,80,100
			return new Ini.Property("nil", "kgt" + GetType().Name + "," + m_strName + ".tbm," + (m_tx - txOrigin).ToString() + "," + (m_ty - tyOrigin).ToString());
		}

		public override void GetObjectData(SerializationInfo info, StreamingContext context) {
			base.GetObjectData(info, context);
			info.AddValue("Name", m_strName);
		}

		// IMapItem

		public override Bitmap GetBitmap(Size sizTile, TemplateDoc tmpd) {
			Bitmap[] abm = m_gimg.GetBitmapSides(sizTile);
			return abm[0];
		}

		public override Point GetCenterPoint(Size sizTile) {
			Size sizGob = m_gimg.GetSize(sizTile);
			return new Point((int)m_tx * sizTile.Width + sizGob.Width / 2, 
					(int)m_ty * sizTile.Height + sizGob.Height / 2);
		}

		public override Rectangle GetBoundingRectAt(int x, int y, Size sizTile, TemplateDoc tmpd) {
			Size sizGob = m_gimg.GetSize(sizTile);
			return new Rectangle(x, y, sizGob.Width, sizGob.Height);
		}

		public override bool HitTest(int x, int y, Size sizTile, TemplateDoc tmpd) {
			int xT = x - ((int)m_tx * sizTile.Width);
			int yT = y - ((int)m_ty * sizTile.Height);
			Size sizGob = m_gimg.GetSize(sizTile);
			if (xT > 0 && xT < sizGob.Width && yT > 0 && yT < sizGob.Height) {
				Bitmap[] abm = m_gimg.GetBitmapSides(sizTile);
				return abm[0].GetPixel(xT, yT) != Color.Transparent;
			}
			return false;
		}

		public override Object Clone() {
			Object[] aobj = { m_strName, (int)m_tx, (int)m_ty };
			return (Object)System.Activator.CreateInstance(GetType(), aobj);
		}

		public override void Draw(Graphics g, int x, int y, Size sizTile, TemplateDoc tmpd, LayerType layer, bool fSelected) {
			if (layer == LayerType.Scenery) {
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
