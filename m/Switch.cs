using System;
using System.Collections;
using SpiffLib;
using System.Runtime.Serialization;

namespace m {
	[Serializable]
	public class SwitchManager {
		ArrayList m_alsSwitches;
		bool m_fModified;

		public SwitchManager() {
			m_fModified = false;
			m_alsSwitches = new ArrayList();
		}

		public ArrayList Items {
			get {
				return m_alsSwitches;
			}
		}

		public Switch this[string strName] {
			get {
				foreach (Switch sw in m_alsSwitches) {
					if (sw.Name == strName)
						return sw;
				}
				return null;
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

		public Switch[] GetSwitchList() {
			return (Switch[])m_alsSwitches.ToArray(typeof(Switch));
		}

		public void AddSwitch(Switch sw) {
			m_alsSwitches.Add(sw);
			SetModified();
		}

		public void RemoveSwitch(Switch sw) {
			m_alsSwitches.Remove(sw);
			SetModified();
		}

		public void ModifySwitch(Switch swModify, Switch sw) {
			int n = m_alsSwitches.IndexOf(swModify);
			if (n >= 0)
				m_alsSwitches[n] = sw;
			SetModified();
		}
	}

	[Serializable]
	public class Switch : ISerializable {
		string m_strName;

		public Switch(string strName) {
			m_strName = strName;
		}

		// ISerializable methods for backwards compatibility

		private Switch(SerializationInfo info, StreamingContext context) {
			m_strName = info.GetString("m_strName");
		}

		public void GetObjectData(System.Runtime.Serialization.SerializationInfo info, System.Runtime.Serialization.StreamingContext context) {
			info.AddValue("m_strName", m_strName);
		}

		public virtual Switch Clone() {
			Switch sw = new Switch(m_strName);
			return sw;
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
}
