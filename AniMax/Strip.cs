using System;
using System.Collections;
using System.Runtime.Serialization;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for Strip.
	/// </summary>
	[Serializable]
	public class Strip : CollectionBase, ISerializable, IComparable, ICloneable
	{
		private string m_strName = null;
		private int m_ifrActive = 0;
        private int m_cfrActive = 1;
		private int m_cHold = 0;

		public Strip(string strName)
		{
			m_strName = strName;
		}

		public object Clone()
		{
			Strip stpNew = new Strip(m_strName);
			for (int i = 0; i < this.Count; i++) {
				stpNew.Add((Frame)this[i].Clone());
			}
			stpNew.m_ifrActive = m_ifrActive;
            stpNew.m_cfrActive = m_cfrActive;
			stpNew.m_cHold = m_cHold;
			return stpNew;
		}

		// IComparable implementation

		public int CompareTo(object ob) {
			return m_strName.CompareTo(((Strip)ob).m_strName);
		}

		// Public properties

		public Frame this[int i] {
			get {
				return (Frame)InnerList[i];
			}
			set {
				// We allow Frames to be added out-of-range in which case
				// we expand the range to include the new Frame.

				while (i >= InnerList.Count)
					InnerList.Add(null);
				InnerList[i] = value;
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

		// Exposed for anyone who wants to keep track of this Strip's ActiveFrame

		public event EventHandler ActiveFrameChanged;

		public int ActiveFrame {
			get {
				return m_ifrActive;
			}
			set {
				m_ifrActive = value;
                ActiveFrameCount = 1;
				if (ActiveFrameChanged != null)
					ActiveFrameChanged(this, EventArgs.Empty);
			}
		}

		public event EventHandler ActiveFrameCountChanged;

        public int ActiveFrameCount {
            get {
                return m_cfrActive;
            }
            set {
                m_cfrActive = value;
                if (m_ifrActive + m_cfrActive > Count) {
                    m_cfrActive = Count - m_ifrActive;
                }
				if (ActiveFrameCountChanged != null)
					ActiveFrameCountChanged(this, EventArgs.Empty);
            }
        }

		public int DefHoldCount {
			get {
				return m_cHold;
			}
			set {
				m_cHold = value;
			}
		}

		public int Add(Frame fr) {
			return ((IList)this).Add(fr);
		}

		public int IndexOf(Frame fr) {
			return ((IList)this).IndexOf(fr);
		}

		public void Insert(int ifr, Frame fr) {
			((IList)this).Insert(ifr, fr);
		}

		// ISerializable interface implementation

		private Strip(SerializationInfo seri, StreamingContext stmc) {
			m_strName = seri.GetString("Name");

			try {
				m_cHold = seri.GetInt32("DefHoldCount");
			} catch {}

			int cfr = seri.GetInt32("FrameCount");
			for (int i = 0; i < cfr; i++) {
				Frame fr = (Frame)seri.GetValue(i.ToString(), typeof(Frame));
				Add(fr);
			}
		}

		void ISerializable.GetObjectData(SerializationInfo seri, StreamingContext stmc) {
			seri.AddValue("Name", m_strName);
			seri.AddValue("DefHoldCount", m_cHold);

			seri.AddValue("FrameCount", InnerList.Count);
			int i = 0;
			foreach (Frame fr in InnerList) {
				seri.AddValue(i.ToString(), fr);
				i++;
			}
		}
	}
}
