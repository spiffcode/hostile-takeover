using System;
using System.Collections;
using System.Runtime.Serialization;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for StripSet.
	/// </summary>
	[Serializable]
	public class StripSet : CollectionBase, ISerializable
	{
		public StripSet() {
		}

		public Strip this[int i] {
			get {
				return (Strip)InnerList[i];
			}
			set {
				InnerList[i] = value;
			}
		}

		public Strip this[string strStripName] {
			get {
				foreach (Strip stp in InnerList) {
					if (stp.Name == strStripName)
						return stp;
				}
				return null;
			}
		}

		// For whatever reason the designers of CollectionBase decided to implement
		// IList.Add as 'explicit' which means it is effectively hidden from  users 
		// of derived classes. So we must add our own Add method which does have the 
		// benefit of being type-safe (possibly why they hid IList.Add in the first place)

		public int Add(Strip stp) {
			return ((IList)this).Add(stp);
		}

		public int IndexOf(Strip stp) {
			return ((IList)this).IndexOf(stp);
		}

		public void Sort() {
			InnerList.Sort();
		}

		// ISerializable interface implementation

		private StripSet(SerializationInfo seri, StreamingContext stmc) {
			for (int i = 0; i < seri.MemberCount; i++) {
				Strip stp = (Strip)seri.GetValue(i.ToString(), typeof(Strip));
				Add(stp);
			}
		}

		void ISerializable.GetObjectData(SerializationInfo seri, StreamingContext stmc) {
			int i = 0;
			foreach (Strip stp in InnerList) {
				seri.AddValue(i.ToString(), stp);
				i++;
			}
		}
	}
}
