using System;
using System.Collections;
using SpiffLib;
using System.Runtime.Serialization;
using System.Text.RegularExpressions;

namespace m {

	[Serializable]
	public class UnitTypeAndCount : ICloneable {
		public UnitType ut;
		public int c;

		public UnitTypeAndCount() {
			this.ut = UnitType.kutNone;
			this.c = 0;
		}

		public UnitTypeAndCount(UnitType ut, int c) {
			this.ut = ut;
			this.c = c;
		}

		override public string ToString() {
			string str = Helper.GetDisplayName(typeof(UnitType), ut.ToString());
			return str + ": " + c;
		}

		public string ToSaveString() {
			return ut.ToString() + "," + c;
		}

		public string FromSaveString(string strArg) {
			Regex re = new Regex(@"^(?<ut>\d+),(?<c>\d+)(?<end>.*)$");
			Match m = re.Match(strArg);
			this.ut = (UnitType)int.Parse(m.Groups["ut"].Value);
			this.c = int.Parse(m.Groups["c"].Value);
			return m.Groups["end"].Value;
		}

		public object Clone() {
			UnitTypeAndCount utc = new UnitTypeAndCount(ut, c);
			return utc;
		}
	}

	[Serializable]
	public class UnitGroup : ISerializable {
		bool m_fLoopForever;
		bool m_fRandomGroup;
		bool m_fSpawn;
		bool m_fCreateAtLevelLoad;
		bool m_fReplaceDestroyedGroup;
		int m_nHealth;
		string m_strSpawnArea;
		Side m_side;
		Aggressiveness m_aggr;
		string m_strName;
		ArrayList m_alsActions;
		ArrayList m_alsUnitTypeAndCounts;

		public UnitGroup(string strName) {
			m_strName = strName;
			m_side = Side.side2;
			m_aggr = Aggressiveness.Defender;
			m_alsActions = new ArrayList();
			m_alsUnitTypeAndCounts = new ArrayList();
			m_fLoopForever = false;
			m_fRandomGroup = false;
			m_fSpawn = false;
			m_fCreateAtLevelLoad = false;
			m_fReplaceDestroyedGroup = false;
			m_strSpawnArea = null;
			m_nHealth = 100;
		}

		// ISerializable methods for backwards compatibility

		private UnitGroup(SerializationInfo info, StreamingContext context) {
			m_strName = info.GetString("m_strName");
			m_side = (Side)info.GetValue("m_side", typeof(Side));
			m_alsActions = (ArrayList)info.GetValue("m_alsActions", typeof(ArrayList));
			m_alsUnitTypeAndCounts = (ArrayList)info.GetValue("m_alsUnitTypeAndCounts", typeof(ArrayList));

			try {
				m_fLoopForever = info.GetBoolean("m_fLoopForever");
			} catch (SerializationException) {
				m_fLoopForever = false;
			}

			try {
				m_aggr = (Aggressiveness)info.GetValue("m_aggr", typeof(Aggressiveness));
			} catch (SerializationException) {
				m_aggr = Aggressiveness.Defender;
			}

			try {
				m_fRandomGroup = info.GetBoolean("m_fRandomGroup");
				m_fSpawn = info.GetBoolean("m_fSpawn");
				m_fCreateAtLevelLoad = info.GetBoolean("m_fCreateAtLevelLoad");
			} catch (SerializationException) {
				m_fRandomGroup = false;
				m_fSpawn = false;
				m_fCreateAtLevelLoad = false;
			}

			try {
				m_fReplaceDestroyedGroup = info.GetBoolean("m_fReplaceDestroyedGroup");
			} catch (SerializationException) {
				try {
					m_fReplaceDestroyedGroup = info.GetBoolean("m_fReplaceDestroyedUnits");
				} catch (SerializationException) {
					m_fReplaceDestroyedGroup = false;
				}
			}

			try {
				m_strSpawnArea = info.GetString("m_strSpawnArea");
			} catch (SerializationException) {
				m_strSpawnArea = null;
			}

			try {
				m_nHealth = info.GetInt32("m_nHealth");
			} catch (SerializationException) {
				m_nHealth = 100;
			}
		}

		public void GetObjectData(System.Runtime.Serialization.SerializationInfo info, System.Runtime.Serialization.StreamingContext context) {
			info.AddValue("m_strName", m_strName);
			info.AddValue("m_side", m_side);
			info.AddValue("m_alsActions", m_alsActions);
			info.AddValue("m_alsUnitTypeAndCounts", m_alsUnitTypeAndCounts);
			info.AddValue("m_fLoopForever", m_fLoopForever);
			info.AddValue("m_aggr", m_aggr);
			info.AddValue("m_fRandomGroup", m_fRandomGroup);
			info.AddValue("m_fSpawn", m_fSpawn);
			info.AddValue("m_fCreateAtLevelLoad", m_fCreateAtLevelLoad);
			info.AddValue("m_fReplaceDestroyedGroup", m_fReplaceDestroyedGroup);
			info.AddValue("m_strSpawnArea", m_strSpawnArea);
			info.AddValue("m_nHealth", m_nHealth);
		}

		public virtual UnitGroup Clone() {
			UnitGroup ug = new UnitGroup(m_strName);
			ug.m_side = m_side;
			ug.m_aggr = m_aggr;
			ug.m_fLoopForever = m_fLoopForever;
			ug.m_fRandomGroup = m_fRandomGroup;
			ug.m_fSpawn = m_fSpawn;
			ug.m_fCreateAtLevelLoad = m_fCreateAtLevelLoad;
			ug.m_fReplaceDestroyedGroup = m_fReplaceDestroyedGroup;
			ug.m_strSpawnArea = (string)m_strSpawnArea.Clone();
			ug.m_nHealth = m_nHealth;
			foreach (UnitTypeAndCount utc in UnitTypeAndCounts)
				ug.UnitTypeAndCounts.Add(utc.Clone());
			foreach (CaBase cab in Actions)
				ug.Actions.Add(cab.Clone());
			return ug;
		}

		public string Name {
			get {
				return m_strName;
			}
			set {
				m_strName = value;
			}
		}

		public Side Side {
			get {
				return m_side;
			}
			set {
				m_side = value;
			}
		}

		public Aggressiveness Aggressiveness {
			get {
				return m_aggr;
			}
			set {
				m_aggr = value;
			}
		}

		public ArrayList Actions {
			get {
				return m_alsActions;
			}
		}

		public ArrayList UnitTypeAndCounts {
			get {
				return m_alsUnitTypeAndCounts;
			}
		}

		public bool LoopForever {
			get {
				return m_fLoopForever;
			}
			set {
				m_fLoopForever = value;
			}
		}

		public bool RandomGroup {
			get {
				return m_fRandomGroup;
			}
			set {
				m_fRandomGroup = value;
			}
		}

		public bool Spawn {
			get {
				return m_fSpawn;
			}
			set {
				m_fSpawn = value;
			}
		}

		public bool CreateAtLevelLoad {
			get {
				return m_fCreateAtLevelLoad;
			}
			set {
				m_fCreateAtLevelLoad = value;
			}
		}

		public bool ReplaceDestroyedGroup {
			get {
				return m_fReplaceDestroyedGroup;
			}
			set {
				m_fReplaceDestroyedGroup = value;
			}
		}

		public string SpawnArea {
			get {
				return m_strSpawnArea;
			}
			set {
				m_strSpawnArea = value;
			}
		}

		public int Health {
			get {
				return m_nHealth;
			}
			set {
				m_nHealth = value;
			}
		}

		public void AddIniProperties(Ini.Section sec) {
			// Save Name

			sec.Add(new Ini.Property("Name", Name));

			// Save Side

			sec.Add(new Ini.Property("Side", "k" + m_side.ToString()));

			// Save Aggressiveness

			sec.Add(new Ini.Property("Aggressiveness", "knAggressiveness" + m_aggr.ToString()));

			// Save flags

			sec.Add(new Ini.Property("LoopForever", m_fLoopForever ? "1" : "0"));
			sec.Add(new Ini.Property("CreateAtLevelLoad", m_fCreateAtLevelLoad ? "1" : "0"));
			sec.Add(new Ini.Property("RandomGroup", m_fRandomGroup ? "1" : "0"));
			sec.Add(new Ini.Property("Spawn", m_fSpawn ? "1" : "0"));
			sec.Add(new Ini.Property("ReplaceGroup", m_fReplaceDestroyedGroup ? "1" : "0"));

			// Save SpawnArea

			int nSpawnArea = CaTypeArea.GetArea(m_strSpawnArea);
			if (nSpawnArea != -1)
				sec.Add(new Ini.Property("SpawnArea", nSpawnArea.ToString()));

			// Save Health

			sec.Add(new Ini.Property("Health", Health.ToString()));

			// Save unit list

			if (m_alsUnitTypeAndCounts.Count > 0) {

				// Write total # of units

				int cTotalUnits = 0;
				foreach (UnitTypeAndCount utc in m_alsUnitTypeAndCounts)
					cTotalUnits += utc.c;
				string str = cTotalUnits.ToString();

				// Write unit/count pairs

				foreach (UnitTypeAndCount utc in m_alsUnitTypeAndCounts)
					str += "," + utc.ToSaveString();

				sec.Add(new Ini.Property("Units", str));
			}

			// Save actions

			foreach (CaBase cab in m_alsActions) {
				if (!(cab is CommentUnitGroupAction))
					sec.Add(new Ini.Property("A", cab.ToSaveString()));
			}
		}

		public static UnitGroup FromIniSection(Ini.Section sec) {
			UnitGroup ug = new UnitGroup(sec["Name"].Value);
			ug.Side = (Side)int.Parse(sec["Side"].Value);
			ug.Aggressiveness = (Aggressiveness)int.Parse(sec["Aggressiveness"].Value);
			ug.LoopForever = (int.Parse(sec["LoopForever"].Value) != 0);
			ug.CreateAtLevelLoad = (int.Parse(sec["CreateAtLevelLoad"].Value) != 0);
			ug.RandomGroup = (int.Parse(sec["RandomGroup"].Value) != 0);
			ug.Spawn = (int.Parse(sec["Spawn"].Value) != 0);
			ug.ReplaceDestroyedGroup = (int.Parse(sec["ReplaceGroup"].Value) != 0);
			if (sec["SpawnArea"] != null) {
				ug.SpawnArea = CaTypeArea.GetAreaNameFromIndex(int.Parse(sec["SpawnArea"].Value));
			}
			ug.Health = int.Parse(sec["Health"].Value);

			// Units

			string strUTC = null;
			if (sec["Units"] != null) {
				strUTC = sec["Units"].Value;
			}
			if (strUTC != null) {
				Regex re = new Regex(@"^(?<count>\d+),(?<end>.*)$");
				Match m = re.Match(strUTC);
				string strT = m.Groups["end"].Value;
				while (strT.Length != 0) {
					UnitTypeAndCount utc = new UnitTypeAndCount();
					strT = utc.FromSaveString(strT);
					ug.UnitTypeAndCounts.Add(utc);
					re = new Regex(@"^\s*,(?<end>.*)$");
					m = re.Match(strT);
					strT = m.Groups["end"].Value;
				}
			}

			// UnitGroup actions

			foreach (Ini.Property prop in sec.Properties) {
				if (prop.Name != "A") {
					continue;
				}
				CaBase cab = UnitGroupActionLoader.LoadIni(prop.Value);
				ug.Actions.Add(cab);
			}
			return ug;
		}
	}
}
