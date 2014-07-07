using System;
using System.Drawing;
using System.Collections;
using System.Xml;
using System.Runtime.Serialization;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for XBitmapSet.
	/// UNDONE: act as a collection of extended bitmaps
	/// NOTE: CollectionBase implements IList
	/// </summary>
	[Serializable]
	public class XBitmapSet : CollectionBase, ISerializable
	{
		public XBitmapSet() {
		}

		public XBitmap this[int i] {
			get {
				return (XBitmap)InnerList[i];
			}
			set {
				InnerList[i] = value;
			}
		}

		public int Add(XBitmap xbm) {
			return ((IList)this).Add(xbm);
		}

		public int IndexOf(XBitmap xbm) {
			return ((IList)this).IndexOf(xbm);
		}

		public int Add(string strFileName) {
			return Add(new XBitmap(strFileName));
		}

		// ISerializable interface implementation

		private XBitmapSet(SerializationInfo seri, StreamingContext stmc) {
			for (int i = 0; i < seri.MemberCount; i++) {
				XBitmap xbm = (XBitmap)seri.GetValue(i.ToString(), typeof(XBitmap));
				Add(xbm);
			}
		}

		void ISerializable.GetObjectData(SerializationInfo seri, StreamingContext stmc) {
			int i = 0;
			foreach (XBitmap xbm in InnerList) {
				seri.AddValue(i.ToString(), xbm);
				i++;
			}
		}
	}
}
