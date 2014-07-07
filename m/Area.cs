using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.Serialization;
using System.Text.RegularExpressions;
using SpiffLib;

namespace m
{
	[Serializable]
	public class Area : MapItem, ISerializable, IComparable {
		int m_nBonusSortKey = -1;
		int m_ctx;
		int m_cty;
		int m_nHandleDragging;
		string m_strName;
		int m_xDeltaMouse;
		int m_yDeltaMouse;
		static int s_nAlpha = 96;
		static SolidBrush s_brArea = new SolidBrush(Color.FromArgb(s_nAlpha, 32, 150, 204));
		static SolidBrush s_brAreaSelected = new SolidBrush(Color.FromArgb(s_nAlpha, 152, 212, 240));
		static SolidBrush s_brWhite = new SolidBrush(Color.FromArgb(255, 255, 255));
		static Pen s_penWhite = new Pen(Color.FromArgb(255, 255, 255));
		static Pen s_penBlack = new Pen(Color.FromArgb(0, 0, 0));
		static int s_cpSizeMargin = 5;

		public Area(int ctx, int cty)
		{
			m_strName = "New Area";
			m_nHandleDragging = -1;
			m_ctx = ctx;
			m_cty = cty;
		}

		// ISerializable implementation

		public Area(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			m_ctx = info.GetInt32("Ctx");
			m_cty = info.GetInt32("Cty");
			m_strName = info.GetString("Name");
			m_nHandleDragging = -1;
		}

		public Area(string strName, string strValue, int txOrigin, int tyOrigin) {
			Regex re = new Regex(@"^(?<tx>\d+),(?<ty>\d+),(?<ctx>\d+),(?<cty>\d+)$");
			Match m = re.Match(strValue);
			m_tx = int.Parse(m.Groups["tx"].Value) + txOrigin;
			m_ty = int.Parse(m.Groups["ty"].Value) + tyOrigin;
			m_ctx = int.Parse(m.Groups["ctx"].Value);
			m_cty = int.Parse(m.Groups["cty"].Value);
			m_strName = strName;
			m_nHandleDragging = -1;
		}

		public override Ini.Property GetIniProperty(int txOrigin, int tyOrigin) {
			return new Ini.Property(m_strName, (m_tx - txOrigin) + "," + (m_ty - tyOrigin) + "," + m_ctx + "," + m_cty);
		}

		public override void GetObjectData(SerializationInfo info, StreamingContext context) {
			base.GetObjectData(info, context);
			info.AddValue("Ctx", m_ctx);
			info.AddValue("Cty", m_cty);
			info.AddValue("Name", m_strName);
		}

		// IComparable implementation

		public int CompareTo(object ob) {
			Area ar = (Area)ob;
			int n = String.Compare(m_strName, ar.m_strName);
			if (n != 0) {
				return n;
			}
			if (m_nBonusSortKey < ar.m_nBonusSortKey) {
				return -1;
			}
			if (m_nBonusSortKey > ar.m_nBonusSortKey) {
				return 1;
			}
			return 0;
		}

		public int BonusSortKey {
			get {
				return m_nBonusSortKey;
			}
			set {
				m_nBonusSortKey = value;
			}
		}

		void DrawHandle(Graphics g, Rectangle rc) {
			g.FillRectangle(s_brWhite, rc);
		}

		Rectangle GetHandleRect(int n, int x, int y, Size sizT) {
			// 0 is top left, go clockwise

			int yT = y;
			if (n == 2 || n == 3)
				yT = y + sizT.Height - s_cpSizeMargin;
			int xT = x;
			if (n == 1 || n == 2)
				xT = x + sizT.Width - s_cpSizeMargin;
			return new Rectangle(xT, yT, s_cpSizeMargin, s_cpSizeMargin);
		}

		void DrawArea(Graphics g, int x, int y, Size sizTile, bool fSelected) {
			Size sizT = new Size(sizTile.Width * m_ctx, sizTile.Height * m_cty);
			g.FillRectangle(fSelected ? s_brAreaSelected : s_brArea, x, y, sizT.Width, sizT.Height);
			for (int n = 0; n < 4; n++) {
				Rectangle rc = GetHandleRect(n, x, y, sizT);
				DrawHandle(g, rc);
			}
			RectangleF rcfClipSav = g.ClipBounds;
			g.SetClip(new Rectangle(x, y, sizT.Width, sizT.Height));
			g.DrawString(m_strName, new Font("Arial", 8), s_brWhite, x + s_cpSizeMargin, y + 1);
			g.SetClip(rcfClipSav);
			Pen pen = fSelected ? s_penWhite : s_penBlack;
			g.DrawLine(pen, x, y, x + sizT.Width - 1, y);
			g.DrawLine(pen, x + sizT.Width - 1, y, x + sizT.Width - 1, y + sizT.Height - 1);
			g.DrawLine(pen, x + sizT.Width - 1, y + sizT.Height - 1, x, y + sizT.Height - 1);
			g.DrawLine(pen, x, y + sizT.Height - 1, x, y);
		}

		// IMapItem

		public override int ctx {
			get {
				return m_ctx;
			}
		}

		public override int cty {
			get {
				return m_cty;
			}
		}

		public override bool OnMouseDown(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd) {
			int nHandle = HittestHandles(ptMouse, sizTile);
			if (nHandle != -1) {
				m_nHandleDragging = nHandle;
				Point ptCorner = GetCornerPoint(m_nHandleDragging, sizTile);
				m_xDeltaMouse = ptMouse.X - ptCorner.X;
				m_yDeltaMouse = ptMouse.Y - ptCorner.Y;
				return true;
			}
			return false;
		}

		public override bool OnMouseMove(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd) {
			if (m_nHandleDragging == -1)
				return false;
			Point ptCorner = GetCornerPoint(m_nHandleDragging, sizTile);
			int txOld = ptCorner.X / sizTile.Width;
			int tyOld = ptCorner.Y / sizTile.Height;
			int txNew = (ptMouse.X - m_xDeltaMouse) / sizTile.Width;
			int tyNew = (ptMouse.Y - m_yDeltaMouse) / sizTile.Height;
			if (txOld == txNew && tyOld == tyNew)
				return true;

			switch (m_nHandleDragging) {
			case 0:
				m_tx = txNew;
				m_ty = tyNew;
				m_ctx += txOld - txNew;
				m_cty += tyOld - tyNew;
				break;

			case 1:
				m_ty = tyNew;
				m_cty += tyOld - tyNew;
				m_ctx += txNew - txOld;
				break;

			case 2:
				m_ctx += txNew - txOld;
				m_cty += tyNew - tyOld;
				break;

			case 3:
				m_tx = txNew;
				m_ctx += txOld - txNew;
				m_cty += tyNew - tyOld;
				break;
			}

			m_ctx = Math.Max(1, m_ctx);
			m_cty = Math.Max(1, m_cty);

			OnPropertyChanged(this, "Bitmap");
			return true;
		}

		Point GetCornerPoint(int nHandleDragging, Size sizTile) {
			int x = (int)m_tx * sizTile.Width;
			int y = (int)m_ty * sizTile.Height;
			int cx = m_ctx * sizTile.Width;
			int cy = m_cty * sizTile.Height;

			switch (nHandleDragging) {
			case 0:
				return new Point(x, y);

			case 1:
				return new Point(x + cx, y);

			case 2:
				return new Point(x + cx, y + cy);

			case 3:
				return new Point(x, y + cy);
			}

			return new Point(-1, -1);
		}

		public override bool OnMouseUp(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd) {
			if (m_nHandleDragging != -1) {
				m_nHandleDragging = -1;
				return true;
			}
			return false;
		}

		int HittestHandles(Point ptMouse, Size sizTile) {
			int x = (int)m_tx * sizTile.Width;
			int y = (int)m_ty * sizTile.Height;
			Size sizT = new Size(sizTile.Width * m_ctx, sizTile.Height * m_cty);
			for (int n = 0; n < 4; n++) {
				Rectangle rc = GetHandleRect(n, x, y, sizT);
				if (rc.Contains(ptMouse))
					return n;
			}
			return -1;
		}

		public override Bitmap GetBitmap(Size sizTile, TemplateDoc tmpd) {
			Bitmap bm = new Bitmap(sizTile.Width * m_ctx, sizTile.Height * m_cty);
			Graphics g = Graphics.FromImage(bm);
			DrawArea(g, 0, 0, sizTile, false);
			g.Dispose();
			return bm;
		}

		public override Point GetCenterPoint(Size sizTile) {
			Size sizT = new Size(sizTile.Width * m_ctx, sizTile.Height * m_cty);
			return new Point((int)m_tx * sizTile.Width + sizT.Width / 2, (int)m_ty * sizTile.Height + sizT.Height / 2);
		}

		public override Rectangle GetBoundingRectAt(int x, int y, Size sizTile, TemplateDoc tmpd) {
			Size sizT = new Size(sizTile.Width * m_ctx, sizTile.Height * m_cty);
			return new Rectangle(x, y, sizT.Width, sizT.Height);
		}

		public override bool HitTest(int x, int y, Size sizTile, TemplateDoc tmpd) {
			int xT = x - (int)m_tx * sizTile.Width;
			int yT = y - (int)m_ty * sizTile.Height;
			Size sizT = new Size(sizTile.Width * m_ctx, sizTile.Height * m_cty);
			if (xT >= 0 && xT < sizT.Width && yT >= 0 && yT < sizT.Height)
				return true;
			return false;
		}

		public override Object Clone() {
			return MemberwiseClone();
		}

		public override void Draw(Graphics g, int x, int y, Size sizTile, TemplateDoc tmpd, LayerType layer, bool fSelected) {
			if (layer == LayerType.Area)
				DrawArea(g, x, y, sizTile, fSelected);
		}

		public string Name {
			get {
				return m_strName;
			}
			set {
				m_strName = value;
				OnPropertyChanged(this, "Bitmap");
			}
		}
	}
}
