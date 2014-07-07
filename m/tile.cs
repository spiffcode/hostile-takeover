using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Runtime.Serialization;
using SpiffLib;

namespace m {
	[Serializable]
	public class Tile : MapItem, ISerializable {
		string m_strName = null;
		bool[,] m_afVisible = null;
		bool[,] m_afOccupancy = null;
		Bitmap m_bmCache = null;
		TemplateDoc m_tmpdCache = null;
		Template m_tmplCache = null;

		public Tile(TemplateDoc tmpd, string strName, int tx, int ty) {
			m_strName = strName;
			m_tx = tx;
			m_ty = ty;
			Template tmpl = GetTemplate(tmpd);
			m_afOccupancy = tmpl.OccupancyMap;
			m_afVisible = null;
			InitCommon();
		}

		public Tile(string strName, int tx, int ty, bool[,] afVisible, bool[,] afOccupancy) {
			m_tx = tx;
			m_ty = ty;
			m_afVisible = afVisible;
			m_strName = strName;
			m_afOccupancy = afOccupancy;
			InitCommon();
		}

		public Tile(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			m_strName = null;
			try {
				m_strName = info.GetString("Name");
			} catch {
				m_strName = info.GetInt32("Cookie").ToString();
			}

			m_afVisible = (bool[,])info.GetValue("Visibility", typeof(bool[,]));

			try {
				m_afOccupancy = (bool[,])info.GetValue("Occupancy", typeof(bool[,]));
			} catch {
				TemplateDoc tmpd = (TemplateDoc)DocManager.GetActiveDocument(typeof(TemplateDoc));
				Template tmpl = tmpd.FindTemplate(m_strName);
				if (tmpl != null) {
					m_afOccupancy = tmpl.OccupancyMap;
				} else {
					m_afOccupancy = new bool[1, 1];
				}
			}

			InitCommon();
		}

		public override void GetObjectData(SerializationInfo info, StreamingContext context) {
			base.GetObjectData(info, context);
			info.AddValue("Name", m_strName);
			info.AddValue("Visibility", m_afVisible);
			info.AddValue("Occupancy", m_afOccupancy);
		}

		void InitCommon() {
			TemplateDocTemplate doct = (TemplateDocTemplate)DocManager.FindDocTemplate(typeof(TemplateDoc));
			doct.TemplateChanged += new TemplateDocTemplate.TemplateChangedHandler(TemplateDocTemplate_TemplateChanged);
		}

		void TemplateDocTemplate_TemplateChanged(TemplateDoc tmpd, string strProperty, string strName, string strParam) {
			if (m_strName != strName)
				return;

			// Flush cached items

			GetTemplate(null);

			// Handle changes

			switch (strProperty) {
			case "Name":
				m_strName = strParam;
				OnPropertyChanged(this, "Name");
				break;

			case "Bitmap":
				OnPropertyChanged(this, "Bitmap");
				break;
			}
		}

		public string Name {
			get {
				return m_strName;
			}
		}

		public Template GetTemplate(TemplateDoc tmpd) {
			if (tmpd == m_tmpdCache)
				return m_tmplCache;
			m_tmpdCache = null;
			m_tmplCache = null;
			m_bmCache = null;
			if (tmpd == null)
				return null;
			Template tmpl = tmpd.FindTemplate(m_strName);
			if (tmpl != null) {
				m_tmpdCache = tmpd;
				m_tmplCache = tmpl;
				m_afOccupancy = tmpl.OccupancyMap;
				return tmpl;
			}
			return null;
		}

		[BrowsableAttribute(false)]
		public bool[,] Visibility {
			get {
				return m_afVisible;
			}
			set {
				// Setting to null?

				m_bmCache = null;
				if (value == null) {
					m_afVisible = null;
					return;
				}

				// Same as occupancy map? Then no visibility map

				bool fEqual = true;
				for (int ty = 0; ty < value.GetLength(0); ty++) {
					for (int tx = 0; tx < value.GetLength(1); tx++) {
						if (value[ty, tx] != m_afOccupancy[ty, tx])
							fEqual = false;
					}
				}

				if (fEqual) {
					m_afVisible = null;
				} else {
					m_afVisible = value;
				}
			}
		}


		public bool IsVisible(int tx, int ty) {
			if (m_afVisible == null) {
				return true;
			}
			if (tx < m_afVisible.GetLength(1) && ty < m_afVisible.GetLength(0)) {
				return m_afVisible[ty, tx];
			}
			return false;
		}

		// IMapItem

		public override Bitmap GetBitmap(Size sizTile, TemplateDoc tmpd) {
			// Get the template. This'll invalidate m_bmCache if necessary

			Template tmpl = GetTemplate(tmpd);
			
			// If already cached, use it

			if (m_bmCache != null)
				return m_bmCache;

			// If we haven't mapped to a template, create a correctly sized bitmap as a placeholder

			bool fDispose = false;
			Bitmap bm;
			if (tmpl != null) {
				bm = tmpl.Bitmap;

				// Dont use the bitmap directly, it might have knobbies on that'll create
				// problems at compile time.

#if false
				if (m_afVisible == null)
					return bm;
#endif
			} else {
				fDispose = true;
				bm = new Bitmap(m_afOccupancy.GetLength(1) * sizTile.Width, m_afOccupancy.GetLength(0) * sizTile.Height);
				using (Graphics gT = Graphics.FromImage(bm))
					gT.Clear(Color.Firebrick);
			}

			// Fill in the chunks that are visible

			int cxT = m_afOccupancy.GetLength(1) * sizTile.Width;
			int cyT = m_afOccupancy.GetLength(0) * sizTile.Height;
			m_bmCache = new Bitmap(cxT, cyT);
			using (Graphics g = Graphics.FromImage(m_bmCache)) {
				g.Clear(Color.FromArgb(255, 0, 255));
				for (int ty = 0; ty < m_afOccupancy.GetLength(0); ty++) {
					for (int tx = 0; tx < m_afOccupancy.GetLength(1); tx++) {
						if (!m_afOccupancy[ty, tx])
							continue;
						if (!IsVisible(tx, ty)) {
							continue;
						}
						Rectangle rcDst = new Rectangle(tx * sizTile.Width, ty * sizTile.Height, sizTile.Width, sizTile.Height);
						g.DrawImage(bm, rcDst, tx * sizTile.Width, ty * sizTile.Height, sizTile.Width, sizTile.Height, GraphicsUnit.Pixel);
					}
				}
			}
			if (fDispose)
				bm.Dispose();

			// Hide the areas that are invisible

			TemplateTools.MakeTransparent(m_bmCache);
			return m_bmCache;
		}


		public override Point GetCenterPoint(Size sizTile) {
			int x = (int)m_tx * sizTile.Width + m_afOccupancy.GetLength(1) * sizTile.Width / 2;
			int y = (int)m_ty * sizTile.Height + m_afOccupancy.GetLength(0) * sizTile.Height / 2;
			return new Point(x, y);
		}

		public override Rectangle GetBoundingRectAt(int x, int y, Size sizTile, TemplateDoc tmpd) {
			Bitmap bm = GetBitmap(sizTile, tmpd);
			return new Rectangle(x, y, bm.Width, bm.Height);
		}

		public override bool HitTest(int x, int y, Size sizTile, TemplateDoc tmpd) {
			int xOffset = x - (int)m_tx * sizTile.Width;
			int yOffset = y - (int)m_ty * sizTile.Height;
			Rectangle rc = new Rectangle(new Point(0, 0), GetBitmap(sizTile, tmpd).Size);
			if (!rc.Contains(xOffset, yOffset))
				return false;
			int tx = xOffset / sizTile.Width;
			int ty = yOffset / sizTile.Height;
			try {
				if (m_afVisible != null)
					return IsVisible(tx, ty);
				return m_afOccupancy[ty, tx];
			} catch {
				return false;
			}
		}

		public override Object Clone() {
			Tile tile = new Tile(m_strName, (int)m_tx, (int)m_ty, m_afVisible, m_afOccupancy);
			return (Object)tile;
		}

		public override void Draw(Graphics g, int x, int y, Size sizTile, TemplateDoc tmpd, LayerType layer, bool fSelected) {
			if (layer == LayerType.TileMap) {
				Bitmap bm = GetBitmap(sizTile, tmpd);
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

		public override Ini.Property GetIniProperty(int xOrigin, int yOrigin) {
			return null;
		}
	}
}
