using System;
using System.Collections;
using SpiffLib;

namespace m
{
	// Capitalized this way on purpose for ToString() niceness

	public enum Side { 
		[DisplayName("neutral")] sideNeutral = 0, 
		[DisplayName("side 1")] side1, 
		[DisplayName("side 2")] side2,
		[DisplayName("side 3")] side3, 
		[DisplayName("side 4")] side4 
	};

	[AttributeUsage(AttributeTargets.All, Inherited = true, AllowMultiple = false)]
	class DisplayNameAttribute : Attribute {
		private string m_strDisplayName;

		public DisplayNameAttribute(string strName) {
			m_strDisplayName = strName;
		}

		public string DisplayName {
			get {
				return m_strDisplayName;
			}
			set {
				m_strDisplayName = value;
			}
		}
	}
	
	[AttributeUsage(AttributeTargets.All, Inherited = true, AllowMultiple = false)]
	class DescriptionAttribute : Attribute {
		private string m_strDescription;

		public DescriptionAttribute(string strDescription) {
			m_strDescription = strDescription;
		}

		public string Description {
			get {
				return m_strDescription;
			}
			set {
				m_strDescription = value;
			}
		}
	}
	
	[Serializable]
	public class TriggerManager
	{
		ArrayList m_alsTriggers;
		ArrayList[] m_aalsSideTriggers;
		bool m_fModified;

		public TriggerManager()
		{
			m_fModified = false;
			m_alsTriggers = new ArrayList();
			m_aalsSideTriggers = new ArrayList[Enum.GetValues(typeof(Side)).Length];
			for (int n = 0; n < m_aalsSideTriggers.Length; n++)
				m_aalsSideTriggers[n] = new ArrayList();
		}

		public ArrayList Triggers {
			get {
				return m_alsTriggers;
			}
		}

		void SetModified() {
			m_fModified = true;
		}

		public void ClearModified() {
			m_fModified = false;
		}

		public bool IsModified() {
			return m_fModified;
		}

		public Trigger[] GetTriggerList(Side side) {
			ArrayList als = (ArrayList)m_aalsSideTriggers[(int)side];
			return (Trigger[])als.ToArray(typeof(Trigger));
		}

		public void AddTrigger(Trigger tgr) {
			m_alsTriggers.Add(tgr);
			foreach(Side side in Enum.GetValues(typeof(Side))) {
				if ((tgr.Sides & SideToMask(side)) != 0) {
					ArrayList als = (ArrayList)m_aalsSideTriggers[(int)side];
					als.Add(tgr);
				}
			}
			SetModified();
		}

		public void RemoveTrigger(Trigger tgr) {
			m_alsTriggers.Remove(tgr);
			foreach (Side side in Enum.GetValues(typeof(Side))) {
				if ((tgr.Sides & SideToMask(side)) != 0) {
					ArrayList als = (ArrayList)m_aalsSideTriggers[(int)side];
					als.Remove(tgr);
				}
			}
			SetModified();
		}

		public void ModifyTrigger(Trigger tgrModify, Trigger tgr) {
			int n = m_alsTriggers.IndexOf(tgrModify);
			if (n >= 0)
				m_alsTriggers[n] = tgr;
			foreach(Side side in Enum.GetValues(typeof(Side))) {
				ArrayList als = (ArrayList)m_aalsSideTriggers[(int)side];
				if ((SideToMask(side) & tgr.Sides) != 0) {
					n = als.IndexOf(tgrModify);
					if (n >= 0) {
						als[n] = tgr;
					} else {
						als.Add(tgr);
					}
				} else {
					als.Remove(tgrModify);
				}
			}
			SetModified();
		}

		public int MoveUpTrigger(Side side, Trigger tgr) {
			ArrayList als = (ArrayList)m_aalsSideTriggers[(int)side];
			int n = als.IndexOf(tgr);
			if (n > 0) {
				als.Remove(tgr);
				als.Insert(n - 1, tgr);
				SetModified();
				return n - 1;
			}
			return -1;
		}

		public int MoveDownTrigger(Side side, Trigger tgr) {
			ArrayList als = (ArrayList)m_aalsSideTriggers[(int)side];
			int n = als.IndexOf(tgr);
			if (n != -1 && n < als.Count - 1) {
				als.Remove(tgr);
				als.Insert(n + 1, tgr);
				SetModified();
				return n + 1;
			}
			return -1;
		}

		public int SideToMask(Side side) {
			return (1 << (int)side);
		}

		public Side[] GetTriggerSides() {
			int nfMask = GetSidesMask();
			ArrayList alsSides = new ArrayList();
			foreach (Side side in Enum.GetValues(typeof(Side))) {
				if ((nfMask & SideToMask(side)) != 0)
					alsSides.Add(side);
			}
			return (Side[])alsSides.ToArray(typeof(Side));
		}

		public int GetSidesMask() {
			int nfMask = 0;
			foreach (Trigger tgr in m_alsTriggers) {
				nfMask |= tgr.Sides;
			}
			return nfMask;
		}

		public Ini.Section GetIniSection(bool fDemoCheckTrigger) {
			// If asked create a trigger causes mission failure if running on
			// demo version side1

			bool fModifiedSave = m_fModified;
			Trigger tgrDemo = new Trigger();
			if (fDemoCheckTrigger) {
				// condition: persistent variable $demo is exactly 1
				// action: end mission: lose

				// Condition

				tgrDemo.Sides = SideToMask(Side.side1);
				TestPvarCondition cdn = new TestPvarCondition();
				cdn.Active = true;
				CaTypeText catText = (CaTypeText)cdn.GetTypes()[0];
				catText.Text = "$demo";
				CaTypeQualifiedNumber catQualNum = (CaTypeQualifiedNumber)cdn.GetTypes()[1];
				catQualNum.Qualifier = Qualifier.Exactly;
				catQualNum.Value = 1;
				tgrDemo.Conditions.Add(cdn);

				// Action

				EndMissionTriggerAction acn = new EndMissionTriggerAction();
				acn.Active = true;
				CaTypeWinLose catWinLose = (CaTypeWinLose)acn.GetTypes()[0];
				catWinLose.Result = WinLoseType.Lose;
				tgrDemo.Actions.Add(acn);

				// Add this trigger temporarily
				// Move it up to first place

				AddTrigger(tgrDemo);
				while (MoveUpTrigger(Side.side1, tgrDemo) != -1)
					;
			}

			// Save triggers

			Ini.Section sec = new Ini.Section("Triggers");
			sec.Add(new Ini.Property("Count", m_alsTriggers.Count.ToString()));
			foreach (Trigger tgr in m_alsTriggers) {
				// Calc per side indexes

				string strT = "";
				for (int n = 0; n < m_aalsSideTriggers.Length; n++) {
					ArrayList als = (ArrayList)m_aalsSideTriggers[n];
					int j = als.IndexOf(tgr);
					if (j != -1) {
						if (strT != "")
							strT += ",";
						string strType = "k" + ((Side)n).ToString();
						strT += strType + ":" + j.ToString();
					}
				}
				sec.Add(new Ini.Property("T", strT));

				// Save trigger contents

				tgr.AddIniProperties(sec);
			}

			// Restore order

			if (fDemoCheckTrigger) {
				m_fModified = fModifiedSave;
				RemoveTrigger(tgrDemo);
			}
		
			return sec;
		}

		public void LoadIni(Ini ini) {
			Hashtable map = new Hashtable();
			Trigger tgrCurrent = null;
			Ini.Section sec = ini["Triggers"];
			foreach (Ini.Property prop in sec.Properties) {
				if (prop.Name == "Count") {
					continue;
				}
				if (prop.Name == "T") {
					tgrCurrent = new Trigger();
					int nfSides = 0;
					foreach (string key in prop.Value.Split(',')) {
						Side side = (Side)int.Parse(key.Split(':')[0]);
						nfSides |= SideToMask(side);
						map.Add(key, tgrCurrent);
					}
					tgrCurrent.Sides = nfSides;
					m_alsTriggers.Add(tgrCurrent);
					continue;
				}
				if (prop.Name == "C") {
					tgrCurrent.Conditions.Add(TriggerConditionLoader.LoadIni(prop.Value));
					continue;
				}
				if (prop.Name == "A") {
					tgrCurrent.Actions.Add(TriggerActionLoader.LoadIni(prop.Value));
					continue;
				}
			}

			// Add the triggers for each side in proper order

			for (int side = 0; side < m_aalsSideTriggers.Length; side++) {
				int index = 0;
				while (true) {
					bool fFound = false;
					foreach (string key in map.Keys) {
						int sideT = int.Parse(key.Split(':')[0]);
						if (sideT != side) {
							continue;
						}
						int indexT = int.Parse(key.Split(':')[1]);
						if (indexT != index) {
							continue;
						}
						fFound = true;
						m_aalsSideTriggers[side].Add(map[key]);
					}
					if (!fFound) {
						break;
					}
					index = index + 1;
				}
			}

			// Go through all the triggers and search for demo check trigger.
			// There should be only one, but check them all.

			ArrayList alsRemove = new ArrayList();
			foreach (Trigger tgr in m_alsTriggers) {
				foreach (CaBase cab in tgr.Conditions) {
					if (cab.GetType() == typeof(TestPvarCondition)) {
						TestPvarCondition cdn = (TestPvarCondition)cab;
						if (cdn.GetVariableString() == "$demo") {
							alsRemove.Add(tgr);
							break;
						}
					}

				}
			}
			foreach (Trigger tgr in alsRemove) {
				RemoveTrigger(tgr);
			}

			// Triggers have been modified

			SetModified();
		}
	}
}
