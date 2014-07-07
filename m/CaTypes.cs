using System;
using System.Collections;
using System.Collections.Specialized;
using System.Windows.Forms;
using System.Runtime.Serialization;
using System.Text.RegularExpressions;

namespace m {
#if false
	QualifiedNumber,
	Player,
	Unit,
	Location,
	Resources,
	Points,
	Switch,
	Text,
	SoundEffect,
	Duration,
	ScoreType,
#endif

	[Serializable]
	public abstract class CaType {
		protected bool m_fInit = false;

		public bool IsInitialized() {
			return m_fInit;
		}

		public new abstract string ToString();
		public abstract string ToSaveString();
		public abstract string FromSaveString(string strArgs, bool fLast);

		public virtual bool EditProperties() {
			return CaPropForm.DoModal(this);
		}

		public virtual CaType Clone() {
			return (CaType)MemberwiseClone();
		}

		protected static string GetSaveString(object obj) {
			string str = obj.GetType().ToString();
			int ichDot = str.IndexOf(".");
			str = str.Substring(ichDot + 1, str.Length - ichDot - 1);
			str = "kn" + str + obj.ToString();
			return str;
		}
	}

	// Number

	[Serializable]
	public class CaTypeNumber : CaType {
		protected int m_nValue;

		public CaTypeNumber() {
			m_nValue = -1;
		}

		public override string ToString() {
			if (!m_fInit) {
				return "quantity";
			} else {
				return m_nValue.ToString();
			}
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<value>(-)?\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Value = int.Parse(m.Groups["value"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return ToString();
		}

		public int Value {
			get {
				return m_nValue;
			}
			set {
				m_nValue = value;
				m_fInit = true;
			}
		}
	}

	// QualifiedNumber

	public enum Qualifier { AtLeast, AtMost, Exactly };

	[Serializable]
	public class CaTypeQualifiedNumber : CaTypeNumber {
		Qualifier m_qfr;

		public CaTypeQualifiedNumber() {
			m_qfr = Qualifier.AtLeast;
		}

		public override string ToString() {
			if (m_fInit)
				return m_qfr.ToString() + " " + m_nValue.ToString();
			return base.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<qfr>\d+),(?<end>.*)$");
			Match m = re.Match(strArgs);
			Qualifier = (Qualifier)int.Parse(m.Groups["qfr"].Value);
			return base.FromSaveString(m.Groups["end"].Value, false);
		}

		public override string ToSaveString() {
			return GetSaveString(m_qfr) + "," + Value.ToString();
		}

		public Qualifier Qualifier {
			get {
				return m_qfr;
			}
			set {
				m_qfr = value;
			}
		}
	}

	[Flags]
	public enum CaSide { SideNeutral, Side1, Side2, Side3, Side4, Enemies, Allies, AllSides, CurrentSide };

	[Serializable]
	public class CaTypeSide : CaType {
		bool[] m_af;

		public CaTypeSide() {
			m_af = new bool[Enum.GetNames(typeof(CaSide)).Length];
		}

		public override CaType Clone() {
			CaTypeSide cat = (CaTypeSide)MemberwiseClone();
			cat.m_af = (bool[])m_af.Clone();
			return (CaType)cat;
		}

		public override string ToString() {
			if (!m_fInit) {
				return "side";
			} else {
				string str = "";
				for (int i = 0; i < m_af.Length; i++) {
					if (m_af[i]) {
						if (str.Length != 0)
							str += ", ";
						str += ((CaSide)i).ToString();
					}
				}
				return str;
			}
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<mask>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			uint dwMask = uint.Parse(m.Groups["mask"].Value);
			for (int i = 0; i < m_af.Length; i++) {
				m_af[i] = ((dwMask & (1 << i)) != 0);
			}
			m_fInit = true;
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			uint dwMask = 0;
			for (int i = 0; i < m_af.Length; i++) {
				if (m_af[i])
					dwMask |= (uint)(1 << i);
			}
			return dwMask.ToString();
		}

		// Is there a better way to expose flags to the property grid?

		public bool CurrentSide {
			get {
				return m_af[(int)CaSide.CurrentSide];
			}
			set {
				m_af[(int)CaSide.CurrentSide] = value;
				m_fInit = true;
			}
		}

		public bool Enemies {
			get {
				return m_af[(int)CaSide.Enemies];
			}
			set {
				m_af[(int)CaSide.Enemies] = value;
				m_fInit = true;
			}
		}

		public bool Allies {
			get {
				return m_af[(int)CaSide.Allies];
			}
			set {
				m_af[(int)CaSide.Allies] = value;
				m_fInit = true;
			}
		}

		public bool Side1 {
			get {
				return m_af[(int)CaSide.Side1];
			}
			set {
				m_af[(int)CaSide.Side1] = value;
				m_fInit = true;
			}
		}

		public bool Side2 {
			get {
				return m_af[(int)CaSide.Side2];
			}
			set {
				m_af[(int)CaSide.Side2] = value;
				m_fInit = true;
			}
		}

		public bool Side3 {
			get {
				return m_af[(int)CaSide.Side3];
			}
			set {
				m_af[(int)CaSide.Side3] = value;
				m_fInit = true;
			}
		}

		public bool Side4 {
			get {
				return m_af[(int)CaSide.Side4];
			}
			set {
				m_af[(int)CaSide.Side4] = value;
				m_fInit = true;
			}
		}

		public bool AllSides {
			get {
				return m_af[(int)CaSide.AllSides];
			}
			set {
				m_af[(int)CaSide.AllSides] = value;
				m_fInit = true;
			}
		}

		public bool SideNeutral {
			get {
				return m_af[(int)CaSide.SideNeutral];
			}
			set {
				m_af[(int)CaSide.SideNeutral] = value;
				m_fInit = true;
			}
		}
	}

	// WARNING: This enum must match exactly the one in ht.h
	// WARNING: these elements cannot be reordered without breaking existing levels

	public enum UnitType {	// ut
		kutNone = -1,

		// Infantry Units (NOTE: this is the order they will appear in the build form)
		[DisplayName("Security Guard")]
		kutShortRangeInfantry,
		[DisplayName("Rocket Trooper")]
		kutLongRangeInfantry,
		[DisplayName("Corporate Raider")]
		kutTakeoverSpecialist,

		// Vehicle Units (NOTE: this is the order they will appear in the build form)
		[DisplayName("Eagle")]
		kutMachineGunVehicle,
		[DisplayName("Broadsword")]
		kutLightTank,
		[DisplayName("Hydra")]
		kutRocketVehicle,
		[DisplayName("Liberator")]
		kutMediumTank,
		[DisplayName("Bullpup")]
		kutGalaxMiner,
		[DisplayName("Dominion")]
		kutMobileHeadquarters,
	
		// Structures (NOTE: this is the order they will appear in the build form)
		[DisplayName("Power Generator")]
		kutReactor,
		[DisplayName("Galaxite Processor")]
		kutProcessor,
		[DisplayName("Galaxite Warehouse")]
		kutWarehouse,
		[DisplayName("Human Resource Center")]
		kutHumanResourceCenter,
		[DisplayName("Vehicle Transport Station")]
		kutVehicleTransportStation,
		[DisplayName("Surveillance Center")]
		kutRadar,
		[DisplayName("Research Center")]
		kutResearchCenter,
		[DisplayName("Headquarters")]
		kutHeadquarters,
	
		// Special structures that like to kill
		[DisplayName("Gatling Tower")]
		kutMachineGunTower,
		[DisplayName("Rocket Tower")]
		kutRocketTower,

		[DisplayName("Andy")]
		kutAndy,
		[DisplayName("Cyclops")]
		kutArtillery,
		[DisplayName("Replicator")]
		kutReplicator,
		[DisplayName("Fox")]
		kutFox,

		kutMax = kutFox + 1,
	}

	// WARNING: This enum must match exactly the one in ht.h

	[Flags]
	public enum UnitMask { 
		kumShortRangeInfantry = 1 << UnitType.kutShortRangeInfantry,
		kumLongRangeInfantry = 1 << UnitType.kutLongRangeInfantry,
		kumTakeoverSpecialist = 1 << UnitType.kutTakeoverSpecialist,
		kumAndy = 1 << UnitType.kutAndy,
		kumFox = 1 << UnitType.kutFox,
		kumInfantry = kumShortRangeInfantry | kumLongRangeInfantry | kumTakeoverSpecialist,

		kumGalaxMiner = 1 << UnitType.kutGalaxMiner,
		kumLightTank = 1 << UnitType.kutLightTank,
		kumMediumTank = 1 << UnitType.kutMediumTank,
		kumMachineGunVehicle = 1 << UnitType.kutMachineGunVehicle,
		kumRocketVehicle = 1 << UnitType.kutRocketVehicle,
		kumMobileHeadquarters = 1 << UnitType.kutMobileHeadquarters,
		kumArtillery = 1 << UnitType.kutArtillery,
		kumVehicles = kumGalaxMiner | kumLightTank | kumMediumTank | kumMachineGunVehicle | 
				kumRocketVehicle | kumMobileHeadquarters | kumArtillery,
		kumMobileUnits = kumInfantry | kumVehicles,

		kumHumanResourceCenter = 1 << UnitType.kutHumanResourceCenter,
		kumReactor = 1 << UnitType.kutReactor,
		kumProcessor = 1 << UnitType.kutProcessor,
		kumHeadquarters = 1 << UnitType.kutHeadquarters,
		kumResearchCenter = 1 << UnitType.kutResearchCenter,
		kumVehicleTransportStation = 1 << UnitType.kutVehicleTransportStation,
		kumRadar = 1 << UnitType.kutRadar,
		kumWarehouse = 1 << UnitType.kutWarehouse,
		kumReplicator = 1 << UnitType.kutReplicator,

		kumMachineGunTower = 1 << UnitType.kutMachineGunTower,
		kumRocketTower = 1 << UnitType.kutRocketTower,
		kumTowers = kumMachineGunTower | kumRocketTower,

		kumStructures = kumHumanResourceCenter | kumReactor | kumProcessor | kumHeadquarters |
				kumResearchCenter | kumVehicleTransportStation | kumRadar | kumWarehouse | kumMachineGunTower | 
				kumRocketTower,
		kumFactories = kumHeadquarters | kumVehicleTransportStation | kumHumanResourceCenter,
		kumAll = kumInfantry | kumVehicles | kumStructures,
		kumNewAll = kumAll | kumAndy | kumFox | kumReplicator,
		kumNone = 0,
	}

#if false // nobody is using this
	[Serializable]
	public class CaTypeUnit : CaType {
		UnitType m_ut;
		string m_strName;

		public CaTypeUnit() {
			m_ut = UnitType.kutNone;
			m_strName = "";
			m_fInit = true;
		}

		public override string ToString() {
			if (m_strName != "")
				return m_strName;
			if (m_ut == UnitType.kutNone)
				return "unit";
			return m_ut.ToString();
		}

		public override string ToSaveString() {
			if (m_strName != "")
				return m_strName;
			return GetSaveString(m_ut);
		}

		public UnitType Unit {
			get {
				return m_ut;
			}
			set {
				m_ut = value;
			}
		}

		public string Name {
			get {
				return m_strName;
			}
			set {
				m_strName = value;
			}
		}
	}
#endif

	public enum ModifierType { Set, Add, Subtract };

	[Serializable]
	public class CaTypeModifier : CaType {
		ModifierType m_mod;

		public CaTypeModifier() {
			m_mod = ModifierType.Set;
			m_fInit = true;
		}

		public override string ToString() {
			return m_mod.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<mod>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Modifier = (ModifierType)int.Parse(m.Groups["mod"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_mod);
		}

		public ModifierType Modifier {
			get {
				return m_mod;
			}
			set {
				m_mod = value;
			}
		}
	}

	[Serializable]
	public class CaTypeOneSide : CaType {
		Side m_side;

		public CaTypeOneSide() {
			m_side = Side.sideNeutral;
			m_fInit = true;
		}

		public override string ToString() {
			return m_side.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<side>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Side = (Side)int.Parse(m.Groups["side"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_side);
		}

		public Side Side {
			get {
				return m_side;
			}
			set {
				m_side = value;
			}
		}
	}

#if false // nobody is using this
	public enum ResourceType { Credits };

	[Serializable]
	public class CaTypeResource : CaType {
		ResourceType m_res;

		public CaTypeResource() {
			m_res = ResourceType.Credits;
			m_fInit = true;
		}

		public override string ToString() {
			return m_res.ToString();
		}

		public override string ToSaveString() {
			return GetSaveString(m_res);
		}

		public ResourceType Modifier {
			get {
				return m_res;
			}
			set {
				m_res = value;
			}
		}
	}
#endif

	public enum WinLoseType { None, Win, Lose };

	[Serializable]
	public class CaTypeWinLose : CaType {
		WinLoseType m_wlt;

		public CaTypeWinLose() {
			m_wlt = WinLoseType.None;
		}

		public override string ToString() {
			if (!m_fInit)
				return "WinOrLose";
			return m_wlt.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<wlt>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Result = (WinLoseType)int.Parse(m.Groups["wlt"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_wlt);
		}

		public WinLoseType Result {
			get {
				return m_wlt;
			}
			set {
				m_wlt = value;
				m_fInit = (m_wlt != WinLoseType.None);
			}
		}
	}

	public enum OnOffType { None, On, Off };

	[Serializable]
	public class CaTypeOnOff : CaType {
		OnOffType m_oot;

		public CaTypeOnOff() {
			m_oot = OnOffType.None;
		}

		public override string ToString() {
			if (!m_fInit)
				return "OnOrOff";
			return m_oot.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<oot>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Result = (OnOffType)int.Parse(m.Groups["oot"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_oot);
		}

		public OnOffType Result {
			get {
				return m_oot;
			}
			set {
				m_oot = value;
				m_fInit = (m_oot != OnOffType.None);
			}
		}
	}
	public enum SmallLargeType { None, SmallBottom, Large, SmallTop };

	[Serializable]
	public class CaTypeSmallLarge : CaType {
		SmallLargeType m_slt;

		public CaTypeSmallLarge() {
			m_slt = SmallLargeType.None;
		}

		public override string ToString() {
			if (!m_fInit)
				return "SmallOrLarge";
			return m_slt.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<slt>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Result = (SmallLargeType)int.Parse(m.Groups["slt"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_slt);
		}

		public SmallLargeType Result {
			get {
				return m_slt;
			}
			set {
				m_slt = value;
				m_fInit = (m_slt != SmallLargeType.None);
			}
		}
	}

	public enum MoreCloseType { None, More, Close };

	[Serializable]
	public class CaTypeMoreClose : CaType {
		MoreCloseType m_mct;

		public CaTypeMoreClose() {
			m_mct = MoreCloseType.None;
		}

		public override string ToString() {
			if (!m_fInit)
				return "MoreOrCloseButton";
			return m_mct.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<mct>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Result = (MoreCloseType)int.Parse(m.Groups["mct"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_mct);
		}

		public MoreCloseType Result {
			get {
				return m_mct;
			}
			set {
				m_mct = value;
				m_fInit = (m_mct != MoreCloseType.None);
			}
		}
	}

	public enum ModifyCountdownType { None, Stop, Resume, Show, Hide };

	[Serializable]
	public class CaTypeModifyCountdown : CaType {
		ModifyCountdownType m_mct;

		public CaTypeModifyCountdown() {
			m_mct = ModifyCountdownType.None;
		}

		public override string ToString() {
			if (!m_fInit)
				return "Stop/Resume/Show/Hide";
			return m_mct.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<mct>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			int n = int.Parse(m.Groups["mct"].Value);
			// res.h switches around the last two values of the enum
			// unfortunately, so when importing it is necessary to switch
			// them back.
			if (n == 3) {
				n = 4;
			} else if (n == 4) {
				n = 3;
			}
			Result = (ModifyCountdownType)n;
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_mct);
		}

		public ModifyCountdownType Result {
			get {
				return m_mct;
			}
			set {
				m_mct = value;
				m_fInit = (m_mct != ModifyCountdownType.None);
			}
		}
	}

	public enum ModifyNumberType { None, Set, Add, Subtract };

	[Serializable]
	public class CaTypeModifyNumber : CaType {
		ModifyNumberType m_mnt;

		public CaTypeModifyNumber() {
			m_mnt = ModifyNumberType.None;
		}

		public override string ToString() {
			if (!m_fInit)
				return "Set/Add/Subtract";
			return m_mnt.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<mnt>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Result = (ModifyNumberType)int.Parse(m.Groups["mnt"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_mnt);
		}

		public ModifyNumberType Result {
			get {
				return m_mnt;
			}
			set {
				m_mnt = value;
				m_fInit = (m_mnt != ModifyNumberType.None);
			}
		}
	}

	public enum CreateType { None, Build, Spawn };

	[Serializable]
	public class CaTypeCreate : CaType {
		CreateType m_bs;

		public CaTypeCreate() {
			m_bs = CreateType.None;
		}

		public override string ToString() {
			if (!m_fInit)
				return "Build/Spawn";
			return m_bs.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<bs>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Result = (CreateType)int.Parse(m.Groups["bs"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_bs);
		}

		public CreateType Result {
			get {
				return m_bs;
			}
			set {
				m_bs = value;
				m_fInit = (m_bs != CreateType.None);
			}
		}
	}

	public enum CharacterType { None, Andy, Jana, Olstrom, Fox, ACME_Security, OMNI_Security, Anonymous, Blank };

	[Serializable]
	public class CaTypeCharacter : CaType {
		CharacterType m_cht;

		public CaTypeCharacter() {
			m_cht = CharacterType.None;
		}

		public override string ToString() {
			if (m_cht == CharacterType.None)
				return "character";
			return m_cht.ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<cht>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			Result = (CharacterType)int.Parse(m.Groups["cht"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return GetSaveString(m_cht);
		}

		public CharacterType Result {
			get {
				return m_cht;
			}
			set {
				m_cht = value;
				m_fInit = true;
			}
		}
	}

	[Serializable]
	public class CaTypeText : CaType {
		string m_str;

		public CaTypeText() {
			m_str = "";
		}

		public override string ToString() {
			if (m_str == "")
				return "text";
			return m_str;
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			if (fLast) {
				Text = strArgs;
				return "";
			} else {
				Text = strArgs.Split(',')[0];
				return strArgs.Substring(Text.Length);
			}
		}

		public override string ToSaveString() {
			return m_str;
		}

		public string Text {
			get {
				return m_str;
			}
			set {
				m_str = value;
				m_fInit = true;
			}
		}
	}

	[Serializable]
	public class CaTypeRichText : CaTypeText {
		public CaTypeRichText() {
		}

		public override bool EditProperties() {
			string str = EditRichTextForm.DoModal("Text", Text);
			if (str == null)
				return false;
			m_fInit = true;
			Text = str;
			return true;
		}
	}

	[Serializable]
	public class CaTypeUnitType : CaType {
		UnitType m_ut;

		public CaTypeUnitType() {
			m_ut = UnitType.kutNone;
		}

		public override string ToString() {
			if (!m_fInit)
				return "unit type";
			return Helper.GetDisplayName(typeof(UnitType), m_ut.ToString());
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<ut>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			UnitType = (UnitType)int.Parse(m.Groups["ut"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return m_ut.ToString();
		}

		public UnitType UnitType {
			get {
				return m_ut;
			}
			set {
				m_ut = value;
				m_fInit = m_ut != UnitType.kutNone;
			}
		}

		StringCollection GetUnitTypeNames() {
			StringCollection strc = new StringCollection();
			UnitMask um = UnitMask.kumNewAll;
			uint umT = 1;
			for (int i = 0; i < 32; i++) {
				if (((uint)um & umT) != 0) {
					UnitType ut = (UnitType)i;
					strc.Add(Helper.GetDisplayName(typeof(UnitType), ut.ToString()));
				}
				umT <<= 1;
			}

			return strc;
		}

		public override bool EditProperties() {
			StringCollection strc = GetUnitTypeNames();
			string strName = m_ut != UnitType.kutNone ? Helper.GetDisplayName(typeof(UnitType), m_ut.ToString()) : "";
			int n = PickListForm.DoModal("Unit Type", strName, strc);
			if (n == -1)
				return false;
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));			
			m_ut = (UnitType)n;
			m_fInit = true;
			return true;
		}
	}

	[Serializable]
	public class CaTypeUnitTypes : CaType, ISerializable {
		private UnitMask m_um;

		public CaTypeUnitTypes() {
			m_um = UnitMask.kumNone;
		}

		public CaTypeUnitTypes(UnitMask um) {
			m_um = um;
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<um>(-)?\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			UnitMask = (UnitMask)int.Parse(m.Groups["um"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return ((uint)m_um).ToString();
		}

		// Begin backward compatibility goo

		// ISerializable implementation

		private enum OldUnitType { 
			None, 
			AnyUnit, 
			Infantry, 
			Buildings, 
			Factories, 
			MachineGunInfantry, 
			RocketInfantry, 
			TakeoverSpecialist, 
			Reactor, 
			Processor, 
			HumanResourceCenter, 
			ResearchCenter, 
			Headquarters, 
			VehicleTransportationStation, 
			Radar, 
			Warehouse, 
			MachineGunTower, 
			RocketTower, 
			MobileHeadquarters, 
			LightTank, 
			MediumTank, 
			MachineGunVehicle, 
			RocketVehicle, 
			GalaxMiner
		}
		
		private static UnitMask[] s_aumFromOldUnitType = {
			UnitMask.kumNone,
			UnitMask.kumAll,
			UnitMask.kumInfantry,
			UnitMask.kumStructures,
			UnitMask.kumFactories,
			UnitMask.kumShortRangeInfantry,
			UnitMask.kumLongRangeInfantry,
			UnitMask.kumTakeoverSpecialist,
			UnitMask.kumReactor,
			UnitMask.kumProcessor,
			UnitMask.kumHumanResourceCenter,
			UnitMask.kumResearchCenter,
			UnitMask.kumHeadquarters,
			UnitMask.kumVehicleTransportStation,
			UnitMask.kumRadar,
			UnitMask.kumWarehouse,
			UnitMask.kumMachineGunTower,
			UnitMask.kumRocketTower,
			UnitMask.kumMobileHeadquarters,
			UnitMask.kumLightTank,
			UnitMask.kumMediumTank,
			UnitMask.kumMachineGunVehicle,
			UnitMask.kumRocketVehicle,
			UnitMask.kumGalaxMiner,
		};

		private CaTypeUnitTypes(SerializationInfo info, StreamingContext context) {
			try {
				m_um = 0;
				OldUnitType[] aut;
				aut = (OldUnitType[])info.GetValue("m_aut", typeof(OldUnitType[]));
				for (int i = 0; i < aut.Length; i++) {
					UnitMask umT = s_aumFromOldUnitType[(int)aut[i]];
					m_um = (UnitMask)(((int)m_um) | (int)umT);
				}
			} catch (SerializationException) {
				m_um = (UnitMask)info.GetValue("m_um", typeof(UnitMask));
			}
			m_fInit = true;
		}

		public void GetObjectData(System.Runtime.Serialization.SerializationInfo info, System.Runtime.Serialization.StreamingContext context) {
			info.AddValue("m_um", m_um);
		}
		// End backward compatibility goo


		public override CaType Clone() {
			return (CaType)MemberwiseClone();
		}

		public override string ToString() {
			if (!m_fInit) {
				return "unit types";
			} else {
				switch (m_um) {
				case UnitMask.kumAll:
					return "Any Unit";

				case UnitMask.kumInfantry:
					return "Any Infantry";

				case UnitMask.kumFactories:
					return "Any Builder";

				case UnitMask.kumMobileUnits:
					return "Any Mobile Unit";

				case UnitMask.kumVehicles:
					return "Any Vehicle";

				case UnitMask.kumStructures:
					return "Any Structure";

				case UnitMask.kumTowers:
					return "Any Tower";
				}

				string str = "";
				for (int i = 0; i < 31; i++) {
					if (((uint)m_um & (1 << i)) != 0) {
						if (str.Length != 0)
							str += ", ";
						str += Helper.GetDisplayName(typeof(UnitType), ((UnitType)i).ToString());
					}
				}
				return str;
			}
		}

		public override bool EditProperties() {
			CaTypeUnitTypesForm frm = new CaTypeUnitTypesForm(m_um);
			if (frm.ShowDialog() == DialogResult.OK) {
				UnitMask = frm.UnitMask;
			}
			return true;
		}

		public UnitMask UnitMask {
			get {
				return m_um;
			}
			set {
				m_um = value;
				m_fInit = (m_um != UnitMask.kumNone);
			}
		}

	}

	[Serializable]
	public class CaTypeSwitch : CaType, ISerializable {
		static ArrayList s_alsFixup;
		Switch m_sw;
		string m_strName;

		public CaTypeSwitch() {
			m_sw = null;
		}

		// Begin backward compatibility goo

		// ISerializable implementation

		private CaTypeSwitch(SerializationInfo info, StreamingContext context) {
			try {
				m_sw = (Switch)info.GetValue("m_sw", typeof(Switch));
			} catch (SerializationException) {
				m_strName = info.GetString("m_str");
				AddToFixupList(this);
			}
			m_fInit = true;
		}

		public void GetObjectData(System.Runtime.Serialization.SerializationInfo info, System.Runtime.Serialization.StreamingContext context) {
			info.AddValue("m_sw", m_sw);
		}

		// This sucks. There has to be a better way.

		public static void InitFixupList() {
			s_alsFixup = new ArrayList();
		}

		public static void AddToFixupList(CaTypeSwitch catSwitch) {
			s_alsFixup.Add(catSwitch);
		}

		public static void Fixup(LevelDoc lvld) {
			foreach (CaTypeSwitch catSwitch in s_alsFixup) {
				catSwitch.m_sw = lvld.SwitchManager[catSwitch.m_strName];
				if (catSwitch.m_sw == null) {
					catSwitch.m_sw = new Switch(catSwitch.m_strName);
					lvld.SwitchManager.AddSwitch(catSwitch.m_sw);
					catSwitch.m_strName = null;
				}
			}
			s_alsFixup = null;			
		}

		// End backward compatibility goo

		public override string ToString() {
			if (!m_fInit)
				return "switch";
			return m_sw.Name;
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<index>(-)?\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			int index = int.Parse(m.Groups["index"].Value);
			StringCollection strc = GetSwitchNames();
			if (index >= 0 && index < strc.Count) {
				LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
				m_sw = lvld.SwitchManager[strc[index]];
				m_fInit = true;
			}
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			return lvld.SwitchManager.Items.IndexOf(m_sw).ToString();
		}

		public static StringCollection GetSwitchNames() {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			StringCollection strc = new StringCollection();
			foreach (Switch sw in lvld.SwitchManager.Items)
				strc.Add(sw.Name);
			return strc;
		}

		public override bool EditProperties() {
			StringCollection strc = GetSwitchNames();
			string strName = m_sw != null ? m_sw.Name : "";
			Switch sw = SwitchesForm.DoModal("Switch", strName, strc);
			if (sw == null)
				return false;
			m_sw = sw;
			m_fInit = true;
			return true;
		}
	}

	[Serializable]
	public class CaTypeCounter : CaType {
		Counter m_ctr;

		public CaTypeCounter() {
			m_ctr = null;
		}

		public override string ToString() {
			if (!m_fInit)
				return "Counter";
			return m_ctr.Name;
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<index>(-)?\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			int index = int.Parse(m.Groups["index"].Value);
			StringCollection strc = GetCounterNames();
			if (index >= 0 && index < strc.Count) {
				m_ctr.Name = strc[index];
				m_fInit = true;
			}
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			return lvld.CounterManager.Items.IndexOf(m_ctr).ToString();
		}

		public static StringCollection GetCounterNames() {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			StringCollection strc = new StringCollection();
			foreach (Counter ctr in lvld.CounterManager.Items)
				strc.Add(ctr.Name);
			return strc;
		}

		public override bool EditProperties() {
			StringCollection strc = GetCounterNames();
			string strName = m_ctr != null ? m_ctr.Name : "";
			Counter ctr = CountersForm.DoModal("Counter", strName, strc);
			if (ctr == null)
				return false;
			m_ctr = ctr;
			m_fInit = true;
			return true;
		}
	}

	[Serializable]
	public class CaTypeArea : CaType {
		string m_strArea;

		public CaTypeArea() {
			m_strArea = "";
		}

		public CaTypeArea(string strArea) {
			Area = strArea;
		}

		public override string ToString() {
			if (!m_fInit)
				return "area";
			return m_strArea;
		}

		public override string ToSaveString() {
			return GetArea(m_strArea).ToString();
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<index>(-)?\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			int index = int.Parse(m.Groups["index"].Value);
			string strName = GetAreaNameFromIndex(index);
			if (strName != null) {
				Area = strName;
			}
			return m.Groups["end"].Value;
		}

		public string Area {
			get {
				return m_strArea;
			}
			set {
				m_strArea = value;
				m_fInit = (m_strArea != "");
			}
		}

		// UNDONE: Hmm.. looks like an AreaManager would be appropriate

		static public int VirtualAreaCount {
			get {
				return 1;
			}
		}

		static public string GetAreaNameFromIndex(int index) {
			StringCollection strc = GetAreaNames();
			int n = index + VirtualAreaCount;
			if (n < 0 || n > strc.Count) {
				return null;
			}
			return strc[n];
		}

		static public int GetArea(string strArea) {
			StringCollection strc = GetAreaNames();

			// Corrects for the 1 virtual area we have so far "[LastDiscovery]"

			return strc.IndexOf(strArea) - VirtualAreaCount;
		}

		static public StringCollection GetAreaNames() {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));			
			IMapItem[] ami = lvld.MapItems;
			ArrayList al = new ArrayList();
			foreach (IMapItem mi in ami) {
				if (!(mi is Area))
					continue;
				al.Add(((Area)mi).Name);
			}
			al.Add("[LastDiscovery]");
			al.Sort();

			StringCollection strc = new StringCollection();
			foreach (string str in al)
				strc.Add(str);
			return strc;
		}

		public override bool EditProperties() {
			StringCollection strc = GetAreaNames();
			int n = PickListForm.DoModal("Area", m_strArea, strc);
			if (n == -1)
				return false;
			m_strArea = strc[n];
			if (m_strArea == "")
				return false;
			m_fInit = true;
			return true;
		}
	}

	[Serializable]
	public class CaTypeUnitGroup : CaType {
		UnitGroup m_ug;

		public CaTypeUnitGroup() {
			m_ug = null;
		}

		public override string ToString() {
			if (!m_fInit)
				return "unit group";
			return m_ug.Name;
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<index>(-)?\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			int index = int.Parse(m.Groups["index"].Value);
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (index >= 0 && index < lvld.UnitGroupManager.Items.Count) {
				m_ug = (UnitGroup)lvld.UnitGroupManager.Items[index];
				m_fInit = true;
			}
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			return lvld.UnitGroupManager.Items.IndexOf(m_ug).ToString();
		}

		StringCollection GetUnitGroupNames() {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			StringCollection strc = new StringCollection();
			foreach (UnitGroup ug in lvld.UnitGroupManager.Items)
				strc.Add(ug.Name);
			return strc;
		}

		public override bool EditProperties() {
			StringCollection strc = GetUnitGroupNames();
			string strName = m_ug != null ? m_ug.Name : "";
			int n = PickListForm.DoModal("Unit Group", strName, strc);
			if (n == -1)
				return false;
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));			
			m_ug = (UnitGroup)lvld.UnitGroupManager.Items[n];
			m_fInit = true;
			return true;
		}

		public UnitGroup UnitGroup {
			get {
				return m_ug;
			}
		}
	}

	// WARNING: This enum must match exactly the one in ht.h
	// WARNING: these elements cannot be reordered without breaking existing levels

	public enum UpgradeType {	// upgt
		kupgtNone = -1,

		[DisplayName("Advanced HRC")]
		kupgtAdvancedHRC,
		[DisplayName("Advanced VTS")]
		kupgtAdvancedVTS,
		[DisplayName("Increaed Bullpup Speed")]
		kupgtIncreasedBullpupSpeed,

		kupgtMax = kupgtIncreasedBullpupSpeed + 1,
	}

	// WARNING: This enum must match exactly the one in ht.h

	[Flags]
	public enum UpgradeMask { 
		kupgmAdvancedHRC = 1 << UpgradeType.kupgtAdvancedHRC,
		kupgmAdvancedVTS = 1 << UpgradeType.kupgtAdvancedVTS,
		kupgmIncreasedBullpupSpeed = 1 << UpgradeType.kupgtIncreasedBullpupSpeed,
		kupgmAll = kupgmAdvancedHRC | kupgmAdvancedVTS | kupgmIncreasedBullpupSpeed,
		kupgmNone = 0,
	}

	[Serializable]
	public class CaTypeUpgradeTypes : CaType {
		private UpgradeMask m_upgm;

		public CaTypeUpgradeTypes() {
			m_upgm = UpgradeMask.kupgmNone;
		}

		public override string ToString() {
			if (!m_fInit) {
				return "upgrade types";
			} else {
				string str = "";
				for (int i = 0; i < 31; i++) {
					if (((uint)m_upgm & (1 << i)) != 0) {
						if (str.Length != 0)
							str += ", ";
						str += Helper.GetDisplayName(typeof(UpgradeType), ((UpgradeType)i).ToString());
					}
				}
				return str;
			}
		}

		public override string FromSaveString(string strArgs, bool fLast) {
			Regex re = new Regex(@"^(?<upgm>\d+)(?<end>.*)$");
			Match m = re.Match(strArgs);
			UpgradeMask = (UpgradeMask)uint.Parse(m.Groups["upgm"].Value);
			return m.Groups["end"].Value;
		}

		public override string ToSaveString() {
			return ((uint)m_upgm).ToString();
		}

		public override bool EditProperties() {
			CaTypeUpgradeTypesForm frm = new CaTypeUpgradeTypesForm(m_upgm);
			if (frm.ShowDialog() == DialogResult.OK) {
				UpgradeMask = frm.UpgradeMask;
			}
			return true;
		}

		public UpgradeMask UpgradeMask {
			get {
				return m_upgm;
			}
			set {
				m_upgm = value;
				m_fInit = (m_upgm != UpgradeMask.kupgmNone);
			}
		}
	}
}
