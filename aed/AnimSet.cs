using System;
using System.Drawing;
using System.Xml;
using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;

namespace AED
{
	/// <summary>
	/// Summary description for AnimSet.
	/// </summary>

	public class AnimSet { // anis
		/// <summary>
		/// 
		/// </summary>
		public string Name;
		/// <summary>
		/// 
		/// </summary>
		public ListDictionary Items = new ListDictionary();

		/// <summary>
		/// 
		/// </summary>
		public AnimSet() {}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strName"></param>
		public AnimSet(string strName) {
			Name = strName;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="ani"></param>
		/// <returns></returns>
		public bool AddAnim(Anim ani) {
			Items.Add(ani.Name, ani);
			return true;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strAnim"></param>
		/// <param name="strFrameSet"></param>
		/// <param name="strFrame"></param>
		/// <param name="frm"></param>
		/// <returns></returns>
		public bool AddFrame(string strAnim, string strFrameSet, string strFrame, Frame frm) {
			Anim ani;
			if (Items.Contains(strAnim)) {
				ani = (Anim)Items[strAnim];
			} else {
				ani = new Anim(strAnim);
				Items.Add(strAnim, ani);
			}
			ani.AddFrame(strFrameSet, strFrame, frm);
			return true;
		}

		/// <summary>
		/// 
		/// </summary>
		public Anim this[string strAnimName] {
			get {
				return (Anim)Items[strAnimName];
			}
		}

		// OPT: not terribly efficient...
		/// <summary>
		/// 
		/// </summary>
		public Anim this[int i] {
			get {
				Anim[] aani = new Anim[Items.Count];
				Items.Values.CopyTo(aani, 0);
				return aani[i];
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="xwtr"></param>
		public void Serialize(XmlWriter xwtr) {
			xwtr.WriteStartElement("AnimSet");

			xwtr.WriteAttributeString("Name", Name);
			foreach (DictionaryEntry de in Items)
				((Anim)de.Value).Serialize(xwtr);

			xwtr.WriteEndElement();
		}
	}

	/// <summary>
	/// 
	/// </summary>
	public class Anim { // ani
		/// <summary>
		/// 
		/// </summary>
		public string Name;

		/// <summary>
		/// 
		/// </summary>
		public ListDictionary Items = new ListDictionary();

		/// <summary>
		/// 
		/// </summary>
		public Anim() {}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strName"></param>
		public Anim(string strName) {
			Name = strName;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFrameSet"></param>
		/// <param name="strFrame"></param>
		/// <param name="frm"></param>
		/// <returns></returns>
		public bool AddFrame(string strFrameSet, string strFrame, Frame frm) {
			FrameSet frms;
			if (Items.Contains(strFrameSet)) {
				frms = (FrameSet)Items[strFrameSet];
			} else {
				frms = new FrameSet(strFrameSet);
				Items.Add(strFrameSet, frms);
			}
			frms.Add(frm);
			return true;
		}

		/// <summary>
		/// 
		/// </summary>
		public FrameSet this[string strFrameSetName] {
			get {
				return (FrameSet)Items[strFrameSetName];
			}
		}

		// OPT: not terribly efficient...
		/// <summary>
		/// 
		/// </summary>
		public FrameSet this[int i] {
			get {
				FrameSet[] afrms = new FrameSet[Items.Count];
				Items.Values.CopyTo(afrms, 0);
				return afrms[i];
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="xwtr"></param>
		public void Serialize(XmlWriter xwtr) {
			xwtr.WriteStartElement("Anim");
			xwtr.WriteAttributeString("Name", Name);

			foreach (DictionaryEntry de in Items)
				((FrameSet)de.Value).Serialize(xwtr);

			xwtr.WriteEndElement();
		}
	}

	/// <summary>
	/// 
	/// </summary>
	public class FrameSet : CollectionBase { // frms
		/// <summary>
		/// 
		/// </summary>
		public string Name;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strName"></param>
		public FrameSet(string strName) {
			Name = strName;
		}

		/// <summary>
		/// 
		/// </summary>
		public Frame this[int i] { 
			get {
				return (Frame)InnerList[i];
			}
            set {
				InnerList[i] = value;
			} 
		}

		// UNDONE: BUGBUG: This shouldn't be necessary. FrameSet inherits 
		// from CollectionBase which implements IList so IList's methods
		// (e.g., Add, IndexOf) should automatically be exposed by FrameSet.

		/// <summary>
		/// 
		/// </summary>
		/// <param name="frm"></param>
		/// <returns></returns>
		public int Add(Frame frm) {
			return ((IList)this).Add(frm);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="frm"></param>
		/// <returns></returns>
		public int IndexOf(Frame frm) {
			return ((IList)this).IndexOf(frm);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="xwtr"></param>
		public void Serialize(XmlWriter xwtr) {
			xwtr.WriteStartElement("FrameSet");
			xwtr.WriteAttributeString("Name", Name);

			foreach (object ob in InnerList)
				((Frame)ob).Serialize(xwtr);

			xwtr.WriteEndElement();
		}

		// CollectionBase overrides

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i"></param>
		/// <param name="ob"></param>
		protected override void OnInsertComplete(int i, object ob) {
			((Frame)ob).Parent = this;
		}

	}

	/// <summary>
	/// 
	/// </summary>
	public class Frame { // frm
		private Bitmap m_bm;
		private FrameSet m_frmsParent;
		private int m_xOrigin, m_yOrigin;

		/// <summary>
		/// 
		/// </summary>
		[Category("FYI")]
		public Bitmap Bitmap {
			get {
				return m_bm;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public FrameSet Parent {
			get {
				return m_frmsParent;
			}
			set {
				m_frmsParent = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		[Category("Layout")]
		public int OriginX {
			get {
				return m_xOrigin;
			}
			set {
				m_xOrigin = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		[Category("Layout")]
		public int OriginY {
			get {
				return m_yOrigin;
			}
			set {
				m_yOrigin = value;
			}
		}
		
		/// <summary>
		/// 
		/// </summary>
		/// <param name="bm"></param>
		/// <param name="xOrigin"></param>
		/// <param name="yOrigin"></param>
		public Frame(Bitmap bm, int xOrigin, int yOrigin) {
			m_bm = bm;
//			m_bm.MakeTransparent(Color.FromArgb(255, 0, 255));
#if false
			// Map shadow color into black with light alpha
			Color clrShadow = Color.FromArgb(156, 212, 248);
			Color clrAlphaShadow = Color.FromArgb(101, 0, 0, 0);
			for (int y = 0; y < m_bm.Height; y++) {
				for (int x = 0; x < m_bm.Width; x++) {
					Color clr = m_bm.GetPixel(x, y);
					if (clr == clrShadow)
						m_bm.SetPixel(x, y, clrAlphaShadow);
				}
			}
#endif
			m_xOrigin = xOrigin;
			m_yOrigin = yOrigin;
		}

		/// <summary>
		/// 
		/// </summary>
		[Category("FYI")]
		public int Index {
			get { 
				return ((IList)Parent).IndexOf(this);
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="xwtr"></param>
		public void Serialize(XmlWriter xwtr) {
			xwtr.WriteStartElement("Frame");
			xwtr.WriteAttributeString("Index", Index.ToString());
			xwtr.WriteAttributeString("xOrigin", m_xOrigin.ToString());
			xwtr.WriteAttributeString("yOrigin", m_yOrigin.ToString());
			xwtr.WriteEndElement();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="clrOld"></param>
		/// <param name="clrNew"></param>
		/// <param name="f6bit"></param>
		/// <returns></returns>
		public bool ReplaceColor(Color clrOld, Color clrNew, bool f6bit) {
			bool fFound = false;
			Bitmap bm = Bitmap;

			if (f6bit) {
				clrOld = Color.FromArgb(clrOld.R & 0xfc, clrOld.G & 0xfc, clrOld.B & 0xfc);
				clrNew = Color.FromArgb(clrNew.R & 0xfc, clrNew.G & 0xfc, clrNew.B & 0xfc);
			}

			for (int y = 0; y < bm.Height; y++) {
				for (int x = 0; x < bm.Width; x++) {
					Color clr = bm.GetPixel(x, y);
					if (f6bit)
						clr = Color.FromArgb(clr.R & 0xfc, clr.G & 0xfc, clr.B & 0xfc);

					if (clr == clrOld) {
						fFound = true;
						bm.SetPixel(x, y, clrNew);
					}
				}
			}

			return fFound;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="aiOld"></param>
		/// <param name="aiNew"></param>
		/// <param name="f6bit"></param>
		/// <returns></returns>
		public bool ReplaceColor(int[] aiOld, int[] aiNew, bool f6bit) {
			Color clrOld = Color.FromArgb(aiOld[0], aiOld[1], aiOld[2]);
			Color clrNew = Color.FromArgb(aiNew[0], aiNew[1], aiNew[2]);
			return ReplaceColor(clrOld, clrNew, f6bit);
		}
	}
}
