using System;
using System.Collections;
using SpiffLib;
using System.Runtime.Serialization;

namespace m {
	[Serializable]
	public class CounterManager {
		ArrayList m_alsCounters;
		bool m_fModified;

		public CounterManager() {
			m_fModified = false;
			m_alsCounters = new ArrayList();
		}

		public ArrayList Items {
			get {
				return m_alsCounters;
			}
		}

		public Counter this[string strName] {
			get {
				foreach (Counter ctr in m_alsCounters) {
					if (ctr.Name == strName)
						return ctr;
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

		public Counter[] GetCounterList() {
			return (Counter[])m_alsCounters.ToArray(typeof(Counter));
		}

		public void AddCounter(Counter ctr) {
			m_alsCounters.Add(ctr);
			SetModified();
		}

		public void RemoveCounter(Counter ctr) {
			m_alsCounters.Remove(ctr);
			SetModified();
		}

		public void ModifyCounter(Counter ctrModify, Counter ctr) {
			int n = m_alsCounters.IndexOf(ctrModify);
			if (n >= 0)
				m_alsCounters[n] = ctr;
			SetModified();
		}
	}

	[Serializable]
	public class Counter : ISerializable {
		string m_strName;

		public Counter(string strName) {
			m_strName = strName;
		}

		// ISerializable methods for backwards compatibility

		private Counter(SerializationInfo info, StreamingContext context) {
			m_strName = info.GetString("m_strName");
		}

		public void GetObjectData(System.Runtime.Serialization.SerializationInfo info, System.Runtime.Serialization.StreamingContext context) {
			info.AddValue("m_strName", m_strName);
		}

		public virtual Counter Clone() {
			Counter ctr = new Counter(m_strName);
			return ctr;
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
