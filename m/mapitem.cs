using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.Serialization;
using System.ComponentModel;
using SpiffLib;

namespace m {

	public delegate void PropertyChangedHandler(IMapItem mi, string strProperty);

	public interface IMapItem : ICloneable {
		double tx {
			get;
			set;
		}
		double ty {
			get;
			set;
		}
		SizeF Grid {
			get;
		}
		int ctx {
			get;
		}
		int cty {
			get;
		}

		bool OnMouseMove(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd);
		bool OnMouseDown(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd);
		bool OnMouseUp(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd);
		Bitmap GetBitmap(Size sizTile, TemplateDoc tmpd);
		Point GetCenterPoint(Size sizTile);
		Rectangle GetBoundingRectAt(int x, int y, Size sizTile, TemplateDoc tmpd);
		bool HitTest(int x, int y, Size sizTile, TemplateDoc tmpd);
		void Draw(Graphics g, int x, int y, Size sizTile, TemplateDoc tmpd, LayerType layer, bool fSelected);
		Ini.Property GetIniProperty(int txOrigin, int tyOrigin);

		event PropertyChangedHandler PropertyChanged;
	}

	[Serializable]
	public abstract class MapItem : IMapItem, ISerializable {
		protected double m_tx;
		protected double m_ty;

		public event PropertyChangedHandler PropertyChanged;

		public MapItem() {
			m_tx = 0;
			m_ty = 0;
		}

		public MapItem(SerializationInfo info, StreamingContext ctx) {
			try {
				m_tx = info.GetDouble("TileX");
				m_ty = info.GetDouble("TileY");
			} catch (SerializationException) {
				try {
					m_tx = info.GetInt32("tx");
					m_ty = info.GetInt32("ty");
				} catch (SerializationException) {
					m_tx = info.GetInt32("X") / 16;
					m_ty = info.GetInt32("Y") / 16;
				}
			}
		}

		public virtual void GetObjectData(SerializationInfo info, StreamingContext context) {
			info.AddValue("TileX", m_tx);
			info.AddValue("TileY", m_ty);
		}

		public virtual void OnPropertyChanged(IMapItem mi, string strProperty) {
			if (PropertyChanged != null)
				PropertyChanged(mi, strProperty);
		}

		[Description("X Position (in tile coords)"), Category("Appearance")] 
		public virtual double tx {
			get {
				return m_tx;
			}
			set {
				if (value == m_tx)
					return;
				m_tx = value;
				if (PropertyChanged != null)
					PropertyChanged(this, "tx");
			}
		}

		[Description("Y Position (in tile coords)"), Category("Appearance")] 
		public virtual double ty {
			get {
				return m_ty;
			}
			set {
				if (value == m_ty)
					return;
				m_ty = value;
				if (PropertyChanged != null)
					PropertyChanged(this, "ty");
			}
		}

		public virtual int ctx {
			get {
				return 1;
			}
		}

		public virtual int cty {
			get {
				return 1;
			}
		}

		[Browsable(false)]
			public virtual SizeF Grid {
			get {
				return new SizeF(1.0f, 1.0f);
			}
		}

		public virtual bool OnMouseMove(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd) {
			return false;
		}

		public virtual bool OnMouseDown(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd) {
			return false;
		}

		public virtual bool OnMouseUp(System.Windows.Forms.MouseEventArgs e, Point ptMouse, Size sizTile, TemplateDoc tmpd) {
			return false;
		}

		public abstract Bitmap GetBitmap(Size sizTile, TemplateDoc tmpd);
		public abstract Point GetCenterPoint(Size sizTile);
		public abstract Rectangle GetBoundingRectAt(int x, int y, Size sizTile, TemplateDoc tmpd);
		public abstract bool HitTest(int x, int y, Size sizTile, TemplateDoc tmpd);
		public abstract void Draw(Graphics g, int x, int y, Size sizTile, TemplateDoc tmpd, LayerType layer, bool fSelected);
		public abstract Ini.Property GetIniProperty(int txOrigin, int tyOrigin);
		public abstract Object Clone();
	}
}
