using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Runtime.Serialization;
using System.Collections;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for Frame.
	/// </summary>
	[Serializable]
	public class Frame : ISerializable, ICloneable
	{
		private int m_cHold = 0;
		private Point m_ptSpecial;
		private BitmapPlacerList m_plcl;

		public Frame() {
			m_plcl = new BitmapPlacerList();
		}

		public int HoldCount {
			set {
				m_cHold = value;
			}
			get {
				return m_cHold;
			}
		}

		public Point SpecialPoint {
			set {
				m_ptSpecial = value;
			}
			get {
				return m_ptSpecial;
			}
		}

		public BitmapPlacerList BitmapPlacers {
			get {
				return m_plcl;
			}
		}

		// Deep copy

		public object Clone() {
			Frame fr = new Frame();
			fr.m_cHold = m_cHold;
			fr.m_ptSpecial = new Point(m_ptSpecial.X, m_ptSpecial.Y);
			fr.m_plcl = (BitmapPlacerList)m_plcl.Clone();
			return fr;
		}

		public struct DrawArgs { // drwa
			public int nScale;
			public bool fShowGrid;
			public bool fShowOrigin;
			public bool fShowSpecialPoint;
			public bool fMapSideColors;
			public bool fDrawBackground;
			public Color clrBackground;
			public int cxGrid;
			public int cyGrid;
		}

		public void Draw(Graphics g, Rectangle rcClient, DrawArgs drwa, Point ptOffset) {
			int xCenter = rcClient.Width / 2;
			int yCenter = rcClient.Height / 2;

			int cxT = ((rcClient.Width + drwa.nScale - 1) / drwa.nScale) + 2;
			int cyT = ((rcClient.Height + drwa.nScale - 1) / drwa.nScale) + 2;
			int xCenterT = cxT / 2;
			int yCenterT = cyT / 2;

			// NOTE: these 'using' statements (a 'shortcut' for calling .Dispose()) are 
			// absolutely necessary or we chew up all virtual memory while animating

			// Create a temporary bitmap for compositing the grid, frames, origin indicator, etc into

			using (Bitmap bmT = new Bitmap(cxT, cyT)) {

				// Draw the frame and its indicators (grid, center point, special point, etc)

				DrawUnscaled(bmT, cxT, cyT, drwa, ptOffset);

				// Force a nice simple fast old-school stretchblt

				InterpolationMode imOld = g.InterpolationMode;
				g.InterpolationMode = InterpolationMode.NearestNeighbor;

				// NOTE: _without_ this the first row and column are only scaled by half!

				PixelOffsetMode pomOld = g.PixelOffsetMode;
				g.PixelOffsetMode = PixelOffsetMode.Half;

				// StretchBlt the temporary composite to the passed-in Graphic
				
				g.DrawImage(bmT, rcClient.Left - ((xCenterT * drwa.nScale) - xCenter), 
						rcClient.Top - ((yCenterT * drwa.nScale) - yCenter), 
						cxT * drwa.nScale, cyT * drwa.nScale);

				g.PixelOffsetMode = pomOld;
				g.InterpolationMode = imOld;
			}
		}

		public void DrawUnscaled(Bitmap bmDst, int cx, int cy, DrawArgs drwa, Point ptOffset) {
			Graphics gDst = Graphics.FromImage(bmDst);

			int xCenter = cx / 2;
			int yCenter = cy / 2;

			// Draw background (if enabled)

			if (drwa.fDrawBackground)
				gDst.Clear(drwa.clrBackground);

#if false
			// Draw background bitmap, if any

			if (m_bmBackground != null)
				gT.DrawImage(m_bmBackground, xCenter - (m_bmBackground.Width / 2) + m_ptBackgroundOffset.X,
						yCenter - (m_bmBackground.Height / 2) + m_ptBackgroundOffset.Y, 
						m_bmBackground.Width, m_bmBackground.Height);
#endif
			// Draw grid (if enabled)
			// UNDONE: use alpha to draw grid (e.g., brighten or darken)

			if (drwa.fShowGrid) {
				int cxGrid = drwa.cxGrid;
				int cyGrid = drwa.cyGrid;
//						Brush br = new SolidBrush(Color.FromKnownColor(KnownColor.LightGray));
				Brush br = new SolidBrush(Color.FromArgb(256 / 3, 255, 255, 255));
				for (int x = (xCenter + ptOffset.X) % cxGrid; x < cx; x += cxGrid)
					gDst.FillRectangle(br, x, 0, 1, cy);

				for (int y = (yCenter + ptOffset.Y) % cyGrid; y < cy; y += cyGrid)
					gDst.FillRectangle(br, 0, y, cx, 1);
			}

			BitmapData bmdDst = bmDst.LockBits(
                    new Rectangle(0, 0, bmDst.Width, bmDst.Height), 
					ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);

			// Draw bitmaps from bottom up

			for (int i = BitmapPlacers.Count - 1; i >= 0; i--) {
                BitmapPlacer plc = BitmapPlacers[i];
                XBitmap xbm = plc.XBitmap;
                xbm.SuperBlt(0, 0, bmdDst,
                        xCenter - plc.X + ptOffset.X,
                        yCenter - plc.Y + ptOffset.Y,
                        xbm.Bitmap.Width, xbm.Bitmap.Height,
                        drwa.fMapSideColors);
			}

			bmDst.UnlockBits(bmdDst);

			// Draw origin point (if enabled)

			if (drwa.fShowOrigin) {

				// This is really weird but if we don't do our own clipping then SetPixel will
				// raise an exception!

				int x = xCenter + ptOffset.X;
				int y = yCenter + ptOffset.Y;
				if (x >= 0 && y >= 0 && x < bmDst.Width && y < bmDst.Height)
					bmDst.SetPixel(x, y, Color.FromKnownColor(KnownColor.Orange));
			}

			// Draw special point (if enabled)

			if (drwa.fShowSpecialPoint) {

				// This is really weird but if we don't do our own clipping then SetPixel will
				// raise an exception!

				int x = xCenter + ptOffset.X + m_ptSpecial.X;
				int y = yCenter + ptOffset.Y + m_ptSpecial.Y;
				if (x >= 0 && y >= 0 && x < bmDst.Width && y < bmDst.Height)
					bmDst.SetPixel(x, y, Color.FromArgb(0, 255, 0));
			}

			gDst.Dispose();
		}

		// ISerializable interface implementation

		private Frame(SerializationInfo seri, StreamingContext stmc) : this() {
			try {
				m_plcl = (BitmapPlacerList)seri.GetValue("BitmapPlacers", typeof(BitmapPlacerList));
			} catch {
				m_plcl = new BitmapPlacerList();
				m_plcl.Add(new BitmapPlacer());
				m_plcl[0].X = seri.GetInt32("BitmapX");
				m_plcl[0].Y = seri.GetInt32("BitmapY");
				m_plcl[0].XBitmap = (XBitmap)seri.GetValue("XBitmap", typeof(XBitmap));
			}

			try {
				m_cHold = seri.GetInt32("HoldCount");
			} catch {}

			try {
				m_ptSpecial = (Point)seri.GetValue("SpecialPoint", typeof(Point));
			} catch {}
		}

		void ISerializable.GetObjectData(SerializationInfo seri, StreamingContext stmc) {
			seri.AddValue("BitmapPlacers", m_plcl);
			seri.AddValue("HoldCount", m_cHold);
			seri.AddValue("SpecialPoint", m_ptSpecial);
		}
	}

	[Serializable]
	public class BitmapPlacerList : CollectionBase, ISerializable, ICloneable {
		public BitmapPlacerList() {
		}

		// Automatically expand the collection to include as many
		// elements as the caller is expecting.

		public BitmapPlacer this[int i] {
			get {
//				while (i >= InnerList.Count)
//					InnerList.Add(new BitmapPlacer());
				return (BitmapPlacer)InnerList[i];
			}
			set {
//				while (i >= InnerList.Count)
//					InnerList.Add(new BitmapPlacer());
				InnerList[i] = value;
			}
		}

		public int Add(BitmapPlacer plc) {
			return ((IList)this).Add(plc);
		}

		public void Insert(int iplc, BitmapPlacer plc) {
			InnerList.Insert(iplc, plc);
		}

        public int Index(BitmapPlacer plc) {
			for (int i = 0; i < InnerList.Count; i++) {
				if (plc == (BitmapPlacer)InnerList[i]) {
                    return i;
                }
			}
            return -1;
        }

		// Deep copy

		public object Clone() {
			BitmapPlacerList plcl = new BitmapPlacerList();
			for (int i = 0; i < InnerList.Count; i++) {
				plcl.Add((BitmapPlacer)((BitmapPlacer)InnerList[i]).Clone());
			}
			return plcl;
		}

		// ISerializable interface implementation

		private BitmapPlacerList(SerializationInfo seri, StreamingContext stmc) : this() {
			for (int i = 0; i < seri.MemberCount; i++) {
				BitmapPlacer plc = (BitmapPlacer)seri.GetValue(i.ToString(), typeof(BitmapPlacer));
				InnerList.Add(plc);
			}
		}

		void ISerializable.GetObjectData(SerializationInfo seri, StreamingContext stmc) {
			int i = 0;
			foreach (BitmapPlacer plc in InnerList) {
				seri.AddValue(i.ToString(), plc);
				i++;
			}
		}
	}

	[Serializable]
	public class BitmapPlacer : ISerializable, ICloneable {
		private Point m_pt;
		private XBitmap m_xbm;

		public BitmapPlacer() {
		}

		public Point Point {
			get {
				return m_pt;
			}
			set {
				m_pt = value;
			}
		}

		public int X {
			get {
				return m_pt.X;
			}
			set {
				m_pt.X = value;
			}
		}

		public int Y {
			get {
				return m_pt.Y;
			}
			set {
				m_pt.Y = value;
			}
		}

		public XBitmap XBitmap {
			get {
				return m_xbm;
			}
			set {
				m_xbm = value;
			}
		}

		public object Clone() {
			BitmapPlacer plc = new BitmapPlacer();
			plc.m_pt = new Point(m_pt.X, m_pt.Y);
			plc.m_xbm = m_xbm.Clone();
			return plc;
		}

		private BitmapPlacer(SerializationInfo seri, StreamingContext stmc) : this() {
			m_pt = (Point)seri.GetValue("Point", typeof(Point));
			m_xbm = (XBitmap)seri.GetValue("XBitmap", typeof(XBitmap));
		}

		void ISerializable.GetObjectData(SerializationInfo seri, StreamingContext stmc) {
			seri.AddValue("Point", m_pt);
			seri.AddValue("XBitmap", XBitmap);
		}
	}
}
