using System;
using System.Collections;
using SpiffLib;

namespace m
{
	[Serializable]
	public class UnitGroupManager
	{
		ArrayList m_alsUnitGroups;
		bool m_fModified;

		public UnitGroupManager()
		{
			m_fModified = false;
			m_alsUnitGroups = new ArrayList();
		}

		public ArrayList Items {
			get {
				return m_alsUnitGroups;
			}
		}

		public void SetModified() {
			m_fModified = true;
		}

		public void ClearModified() {
			m_fModified = false;
		}

		public bool IsModified() {
			return m_fModified;
		}

		public UnitGroup[] GetUnitGroupList() {
			return (UnitGroup[])m_alsUnitGroups.ToArray(typeof(UnitGroup));
		}

		public void AddUnitGroup(UnitGroup ug) {
			m_alsUnitGroups.Add(ug);
			SetModified();
		}

		public void RemoveUnitGroup(UnitGroup ug) {
			m_alsUnitGroups.Remove(ug);
			SetModified();
		}

		public void ModifyUnitGroup(UnitGroup ugModify, UnitGroup ug) {
			int n = m_alsUnitGroups.IndexOf(ugModify);
			if (n >= 0)
				m_alsUnitGroups[n] = ug;
			SetModified();
		}

		public void SaveIni(Ini ini) {
			for (int i = 0; i < m_alsUnitGroups.Count; i++) {
				Ini.Section sec = new Ini.Section("UnitGroup " + i);
				((UnitGroup)m_alsUnitGroups[i]).AddIniProperties(sec);
				ini.Add(sec);
			}
		}

		public void LoadIni(Ini ini) {
			for (int index = 0; true; index++) {
				Ini.Section sec = ini["UnitGroup " + index];
				if (sec == null) {
					break;
				}
				UnitGroup ug = UnitGroup.FromIniSection(sec);
				AddUnitGroup(ug);
			}
		}
	}
}
