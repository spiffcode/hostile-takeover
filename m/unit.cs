using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.Serialization;
using SpiffLib;
using System.Text.RegularExpressions;
using System.Collections.Specialized;
using System.Drawing.Design;
using System.ComponentModel;

namespace m {
	// Wrappers for graceful future versioning
	[Serializable]
	public class GalaxMiner : MobileUnit {
		public GalaxMiner(Side side, int tx, int ty) : base(side, tx, ty) {
			m_aggr = Aggressiveness.Coward;
		}

		public GalaxMiner(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
			m_aggr = Aggressiveness.Coward;
		}

		public GalaxMiner(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public GalaxMiner(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			try {
				m_aggr = (Aggressiveness)info.GetInt32("Aggressiveness");
			} catch (SerializationException) {
				m_aggr = Aggressiveness.Coward;
			}
		}
	}
	
	[Serializable]
	public class ShortRangeInfantry : MobileUnit {
		public ShortRangeInfantry(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public ShortRangeInfantry(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public ShortRangeInfantry(string strName, string strValue, int txOrigin, int tyOrigin) :
			base(strName, strValue, txOrigin, tyOrigin) {
		}

		public ShortRangeInfantry(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class LongRangeInfantry : MobileUnit {
		public LongRangeInfantry(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public LongRangeInfantry(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public LongRangeInfantry(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public LongRangeInfantry(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class TakeoverSpecialist : MobileUnit {
		public TakeoverSpecialist(Side side, int tx, int ty) : base(side, tx, ty) {
			m_aggr = Aggressiveness.Coward;
		}

		public TakeoverSpecialist(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
			m_aggr = Aggressiveness.Coward;
		}

		public TakeoverSpecialist(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public TakeoverSpecialist(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			try {
				m_aggr = (Aggressiveness)info.GetInt32("Aggressiveness");
			} catch (SerializationException) {
				m_aggr = Aggressiveness.Coward;
			}
		}
	}

	[Serializable]
	public class LightTank : MobileUnit {
		public LightTank(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public LightTank(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public LightTank(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public LightTank(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class MediumTank : MobileUnit {
		public MediumTank(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public MediumTank(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public MediumTank(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public MediumTank(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class MachineGunVehicle : MobileUnit {
		public MachineGunVehicle(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public MachineGunVehicle(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public MachineGunVehicle(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public MachineGunVehicle(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class RocketVehicle : MobileUnit {
		public RocketVehicle(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public RocketVehicle(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public RocketVehicle(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public RocketVehicle(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class MobileHeadquarters : MobileUnit {
		public MobileHeadquarters(Side side, int tx, int ty) : base(side, tx, ty) {
			m_aggr = Aggressiveness.Coward;
		}

		public MobileHeadquarters(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
			m_aggr = Aggressiveness.Coward;
		}

		public MobileHeadquarters(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public MobileHeadquarters(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			try {
				m_aggr = (Aggressiveness)info.GetInt32("Aggressiveness");
			} catch (SerializationException) {
				m_aggr = Aggressiveness.Coward;
			}
		}
	}

	[Serializable]
	public class Artillery : MobileUnit {
		public Artillery(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public Artillery(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public Artillery(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Artillery(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class Andy : MobileUnit {
		public Andy(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public Andy(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public Andy(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Andy(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class Fox : MobileUnit {
		public Fox(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public Fox(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty, aggr, cab) {
		}

		public Fox(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Fox(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	public enum Aggressiveness {
		Coward,
		Pacifist,
		SelfDefense,
		Defender,
		Pitbull
	}

	[Serializable]
	public class MobileUnit : Unit {
		protected Aggressiveness m_aggr;

		public MobileUnit(Side side, int tx, int ty) : base(side, tx, ty) {
			m_aggr = Aggressiveness.Defender;
			m_cab = new GuardUnitAction();
		}

		public MobileUnit(Side side, int tx, int ty, Aggressiveness aggr, CaBase cab) : base(side, tx, ty) {
			m_aggr = aggr;
			m_cab = cab;
		}

		public MobileUnit(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
			try {
				m_aggr = (Aggressiveness)info.GetInt32("Aggressiveness");
			} catch (SerializationException) {
				m_aggr = Aggressiveness.Defender;
			}

			try {
				m_cab = (CaBase)info.GetValue("Action", typeof(CaBase));
			} catch (SerializationException) {
				m_cab = new GuardUnitAction();
			}
			Init();
		}

		public MobileUnit(string strName, string strValue, int txOrigin,
				int tyOrigin) : base(strName, strValue, txOrigin, tyOrigin) {
			Regex re = new Regex(@"^(?<gt>\d+),(?<side>\d+),(?<tx>\d+),(?<ty>\d+),(?<wf>\d+),(?<health>\d+),(?<aggr>\d+)(,\{(?<cab>.*)\})?$");
			Match m = re.Match(strValue);
			m_aggr = (Aggressiveness)int.Parse(m.Groups["aggr"].Value);
			if (m.Groups["cab"] != null) {
				m_cab = UnitActionLoader.LoadIni(m.Groups["cab"].Value);
			}
		}

		public override Ini.Property GetIniProperty(int txOrigin, int tyOrigin) {
			// For example: nil=kgtRocketVehicle,side,38,120,0,100,knAggressivenessPitbull,{knGuardAreaAction,2}
			Ini.Property prp = base.GetIniProperty(txOrigin, tyOrigin);
			prp.Value += ",knAggressiveness" + m_aggr.ToString();
			if (m_cab != null)
				prp.Value += ",{" + m_cab.ToSaveString() + "}";
			return prp;
		}

		public override void GetObjectData(SerializationInfo info, StreamingContext context) {
			base.GetObjectData(info, context);
			info.AddValue("Aggressiveness", m_aggr);
			info.AddValue("Action", m_cab);
		}

		public override Object Clone() {
			Object[] aobj = { m_side, (int)m_tx, (int)m_ty, m_aggr, m_cab.Clone() };
			return (Object)System.Activator.CreateInstance(GetType(), aobj);
		}

		[Category("Behavior")]
		public Aggressiveness Aggressiveness {
			get {
				return m_aggr;
			}
			set {
				if (m_aggr != value) {
					m_aggr = value;
					OnPropertyChanged(this, "Aggressiveness");
				}
			}
		}

		[EditorAttribute(typeof(ActionEditor), typeof(UITypeEditor)), Category("Behavior")]
		public CaBase Action {
			get {
				return m_cab;
			}
			set {
				if (m_cab != value) {
					m_cab = value;
					OnPropertyChanged(this, "Action");
				}
			}
		}
	}

	public class ActionEditor : UITypeEditor {
		// Indicates that the UITypeEditor provides a form-based (modal) dialog.

		public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context) {
			return UITypeEditorEditStyle.Modal;
		}

		// Displays the UI for value selection.

		public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value) {            
			CaBase cab = (CaBase)value;
			cab = CaNew.DoModal(cab.Clone(), "Modify Action", "UnitAction");
			return cab == null ? value : cab;
		}
	}

	[Serializable]
	public class Unit : MapItem, ISerializable {
		protected Side m_side;
		protected GobImage m_gimg;
		protected CaBase m_cab;
		protected int m_nHealth;

		public Unit(Side side) {
			m_side = side;
			Init();
		}

		public Unit(Side side, int tx, int ty) {
			m_tx = tx;
			m_ty = ty;
			m_side = side;
			Init();
		}

		public Unit(SerializationInfo info, StreamingContext ctx) : base (info, ctx) {
			m_side = (Side)info.GetInt32("Side");
			try {
				m_nHealth = info.GetInt32("Health");
			} catch (SerializationException) {
				m_nHealth = 100;
			}
			Init();
		}

		public Unit(string strName, string strValue, int txOrigin, int tyOrigin) {
			Regex re = new Regex(@"^(?<gt>\d+),(?<side>\d+),(?<tx>\d+),(?<ty>\d+),(?<wf>\d+),(?<health>\d+).*$");
			Match m = re.Match(strValue);
			m_side = (Side)int.Parse(m.Groups["side"].Value);
			m_tx = int.Parse(m.Groups["tx"].Value) + txOrigin;
			m_ty = int.Parse(m.Groups["ty"].Value) + tyOrigin;
			m_nHealth = int.Parse(m.Groups["health"].Value);
			Init();
		}

		public override Ini.Property GetIniProperty(int txOrigin, int tyOrigin) {
			// For example: nil=gobtype,side,38,120,0,100
			string strValue = "kgt" + GetType().Name + ",";
			strValue += "k" + m_side.ToString() + ",";
			strValue += (m_tx - txOrigin).ToString() + "," + (m_ty - tyOrigin).ToString();
			strValue += ",0," + m_nHealth;
			return new Ini.Property("nil", strValue);
		}

		public override void GetObjectData(SerializationInfo info, StreamingContext context) {
			base.GetObjectData(info, context);
			info.AddValue("Side", m_side);
			info.AddValue("Health", m_nHealth);
		}

		protected virtual void Init() {
			m_gimg = Globals.GetGobImage(GetType().Name, true);
			if (m_gimg == null)
				throw new Exception("Cannot find image for gob " + GetType().Name);
		}

		public Side Side {
			get {
				return m_side;
			}
			set {
				if (m_side != value) {
					m_side = value;
					OnPropertyChanged(this, "Side");
				}
			}
		}

		public int Health {
			get {
				return m_nHealth;
			}
			set {
				m_nHealth = value;
				OnPropertyChanged(this, "Health");
			}
		}

		public virtual Point GetTileOrigin(Size sizTile) {
			// Units have an offset of mid-tile (to place their origin in the tile center, as per game code)

			return new Point(sizTile.Width / 2, sizTile.Height / 2);
		}

		public virtual Size GetSize(Size sizTile) {
			return m_gimg.GetSize(sizTile);
		}

		// IMapItem

		public override Bitmap GetBitmap(Size sizTile, TemplateDoc tmpd) {
			Bitmap[] abm = m_gimg.GetBitmapSides(sizTile);
			return abm[(int)m_side];
		}

		public override Point GetCenterPoint(Size sizTile) {
			Point ptTOrigin = GetTileOrigin(sizTile);
			Point ptGobOrigin = m_gimg.GetOrigin(sizTile);
			Size sizGob = m_gimg.GetSize(sizTile);
			int x = (int)m_tx * sizTile.Width + ptTOrigin.X - ptGobOrigin.X;
			int y = (int)m_ty * sizTile.Height + ptTOrigin.Y - ptGobOrigin.Y;
			return new Point(x + sizGob.Width / 2, y + sizGob.Height / 2);
		}

		public override Rectangle GetBoundingRectAt(int x, int y, Size sizTile, TemplateDoc tmpd) {
			Point ptTOrigin = GetTileOrigin(sizTile);
			Point ptGobOrigin = m_gimg.GetOrigin(sizTile);
			int xT = x + ptTOrigin.X - ptGobOrigin.X;
			int yT = y + ptTOrigin.Y - ptGobOrigin.Y;
			Size siz = GetSize(sizTile);
			return new Rectangle(xT, yT, siz.Width, siz.Height);
		}

		public override bool HitTest(int x, int y, Size sizTile, TemplateDoc tmpd) {
			Point ptTOrigin = GetTileOrigin(sizTile);
			Point ptGobOrigin = m_gimg.GetOrigin(sizTile);
			Size sizGob = m_gimg.GetSize(sizTile);
			int xT = x - ((int)m_tx * sizTile.Width + ptTOrigin.X - ptGobOrigin.X);
			int yT = y - ((int)m_ty * sizTile.Height + ptTOrigin.Y - ptGobOrigin.Y);
			if (xT > 0 && xT < sizGob.Width && yT > 0 && yT < sizGob.Height) {
				Bitmap[] abmGob = m_gimg.GetBitmapSides(sizTile);
				return abmGob[(int)m_side].GetPixel(xT, yT) != Color.Transparent;
			}
			return false;
		}

		public override Object Clone() {
			Object[] aobj = { m_side, (int)m_tx, (int)m_ty };
			return (Object)System.Activator.CreateInstance(GetType(), aobj);
		}

		public override void Draw(Graphics g, int x, int y, Size sizTile, TemplateDoc tmpd, LayerType layer, bool fSelected) {
			Point ptTOrigin = GetTileOrigin(sizTile);
			Point ptGobOrigin = m_gimg.GetOrigin(sizTile);
			x += ptTOrigin.X - ptGobOrigin.X;
			y += ptTOrigin.Y - ptGobOrigin.Y;

			if (layer == LayerType.DepthSorted) {
				Bitmap bm = m_gimg.GetBitmapSides(sizTile)[(int)m_side];
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

