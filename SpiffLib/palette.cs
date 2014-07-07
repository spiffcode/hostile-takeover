using System;
using System.IO;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;
using System.Runtime.Serialization;

namespace SpiffLib
{
	/// <summary>
	/// 
	/// </summary>
	[Serializable]
	public class Palette : ISerializable {
		private Color[] m_aclr;
		private Hashtable m_htbl = new Hashtable();
		private bool m_fClearHash = false;
		/// <summary>
		/// 
		/// </summary>
		public string FileName;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFileJasc"></param>
		public Palette(string strFile)
		{
			if (!LoadJasc(strFile)) {
				if (!LoadPhotoshopAct(strFile)) {
					throw new Exception(strFile + " is not a Jasc or Photoshop .ACT format palette!");
				}
			}
			FileName = strFile;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="aclr"></param>
		public Palette(Color[] aclr) {
			m_aclr = (Color[])aclr.Clone();
		}

		/// <summary>
		/// 
		/// </summary>
		public Palette(int cColors) {
			m_aclr = new Color[cColors];
		}

		/// <summary>
		/// 
		/// </summary>
		public Palette() {
			m_aclr = new Color[0];
		}

		public Palette(SerializationInfo info, StreamingContext ctx) {
			m_aclr = (Color[])info.GetValue("Colors", typeof(Color[]));
		}

		public void GetObjectData(SerializationInfo info, StreamingContext context) {
			info.AddValue("Colors", m_aclr);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="cEntriesUpTo"></param>
		/// <param name="clrPad"></param>
		public void Pad(int cEntriesUpTo, Color clrPad) {
			Color[] aclrNew = new Color[cEntriesUpTo];
			for (int iclr = 0; iclr < aclrNew.Length; iclr++) {
				if (iclr < m_aclr.Length) {
					aclrNew[iclr] = m_aclr[iclr];
				} else {
					aclrNew[iclr] = clrPad;
				}
			}
			m_aclr = aclrNew;
			m_fClearHash = true;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFile"></param>
		/// <returns></returns>
		public static Palette OpenDialog(string strFile) {
			OpenFileDialog frmOpen = new OpenFileDialog();
			frmOpen.Filter = "Jasc Palette File (*.pal)|*.pal|Photoshop Act File (*.act)|*.act";
			frmOpen.Title = "Palette File";
			frmOpen.FileName = strFile;
			if (frmOpen.ShowDialog() == DialogResult.Cancel)
				return null;
			try {
				return new Palette(frmOpen.FileName);
			} catch {
				return null;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFile"></param>
		/// <returns></returns>
		public bool SaveDialog() {
			SaveFileDialog frmSave = new SaveFileDialog();
			frmSave.Filter = "Jasc Palette File (*.pal)|*.pal|Photoshop Act File (*.act)|*.act";
			frmSave.Title = "Palette File";
			if (frmSave.ShowDialog() == DialogResult.Cancel)
				return false;

			try {
				SaveJasc(frmSave.FileName);
				return true;
			} catch {
				return false;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFile"></param>
		public void SaveBin(string strFile) {
			// Write binary palette
			Stream stm = new FileStream(strFile, FileMode.Create, FileAccess.Write, FileShare.None);
			BinaryWriter bwtr = new BinaryWriter(stm);
			bwtr.Write(Misc.SwapUShort((ushort)m_aclr.Length));
			foreach (Color clr in m_aclr) {
				bwtr.Write(clr.R);
				bwtr.Write(clr.G);
				bwtr.Write(clr.B);
			}
			bwtr.Close();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFile"></param>
		/// <returns></returns>
		public bool LoadJasc(string strFile) {
			m_htbl.Clear();		// in case of Palette reuse

			StreamReader stmr = new StreamReader(strFile);
			if (stmr.ReadLine() != "JASC-PAL" || stmr.ReadLine() != "0100") {
				stmr.Close();
				return false;
			}
			m_aclr = new Color[int.Parse(stmr.ReadLine())];
			for (int i = 0; i < m_aclr.Length; i++) {
				string[] astr = stmr.ReadLine().Split(' ');
				m_aclr[i] = Color.FromArgb(int.Parse(astr[0]), int.Parse(astr[1]), int.Parse(astr[2]));
			}
			stmr.Close();
			return true;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFile"></param>
		public void SaveJasc(string strFile) {
			// Write Jasc palette
			Stream stm = new FileStream(strFile, FileMode.Create, FileAccess.Write, FileShare.None);
			TextWriter twtr = new StreamWriter(stm);
			twtr.WriteLine("JASC-PAL");
			twtr.WriteLine("0100");
			twtr.WriteLine(m_aclr.Length);
			foreach(Color clr in m_aclr)
				twtr.WriteLine(clr.R.ToString() + " " + clr.G.ToString() + " " + clr.B.ToString());
			twtr.Close();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="strFile"></param>
		/// <returns></returns>
		public bool LoadPhotoshopAct(string strFile) {
			m_htbl.Clear();		// in case of Palette reuse

			FileStream stm = new FileStream(strFile, FileMode.Open, FileAccess.Read);
			if (stm.Length != 768) {
				stm.Close();
				return false;
			}

			m_aclr = new Color[256];
			for (int i = 0; i < 256; i++) {
				byte bRed = (byte)stm.ReadByte();
				byte bGreen = (byte)stm.ReadByte();
				byte bBlue = (byte)stm.ReadByte();
				m_aclr[i] = Color.FromArgb(bRed, bGreen, bBlue);
			}
			stm.Close();
			return true;
		}

		/// <summary>
		/// Save palette in Photoshop Color Table (.ACT) format. This format is extremely
		/// simple. 256 entries, 3 bytes each (r, g, b). No count, no header. The output
		/// file is always exactly 768 bytes long.
		/// </summary>
		/// <param name="strFile"></param>
		public void SavePhotoshopAct(string strFile) {
			// Write binary Photoshop Color Table (.ACT)
			Stream stm = new FileStream(strFile, FileMode.Create, FileAccess.Write, FileShare.None);
			BinaryWriter bwtr = new BinaryWriter(stm);
			foreach (Color clr in m_aclr) {
				bwtr.Write(clr.R);
				bwtr.Write(clr.G);
				bwtr.Write(clr.B);
			}

			// Pad out to 256 entries

			for (int i = m_aclr.Length; i < 256; i++) {
				bwtr.Write(0);
				bwtr.Write(0);
				bwtr.Write(0);
			}

			bwtr.Close();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="clr"></param>
		/// <returns></returns>
		public int FindClosestEntry(Color clr) {
			// If not sure of consistency with palette, clear the hash table
			if (m_fClearHash)
				m_htbl.Clear();

			// Is this mapping available already?
			int key = clr.GetHashCode();
			if (m_htbl.ContainsKey(key))
				return (int)m_htbl[key];

			// Find the entry, the long way
			int nLowest = 256 * 256 * 3;
			int iLowest = 0;
			int nR = clr.R;
			int nG = clr.G;
			int nB = clr.B;
			for (int iclr = 0; iclr < m_aclr.Length; iclr++) {
				Color clrPal = m_aclr[iclr];
				int dR = clrPal.R - nR;
				int dG = clrPal.G - nG;
				int dB = clrPal.B - nB;
				int nD = dR * dR + dG * dG + dB * dB;
				if (nD < nLowest) {
					nLowest = nD;
					iLowest = iclr;
				}
			}

			// Add it to the hash table and return it 
			m_htbl.Add(key, iLowest);
			return iLowest;
		}

		/// <summary>
		/// clr = pal[iclr];
		/// </summary>
		public Color this[int iclr] {
			get {
				return m_aclr[iclr];
			}
			set {
				m_fClearHash = true;
				m_aclr[iclr] = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public Color[] Colors {
			get {
				return (Color[])m_aclr.Clone();
			}
			set {
				m_fClearHash = true;
				m_aclr = (Color[])value.Clone();
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public int Length {
			get {
				return m_aclr.Length;
			}
		}
	}
}
