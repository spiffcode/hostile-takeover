using System;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using System.Text;

namespace SpiffLib
{
	public class Ini {
		private ArrayList m_alsSections = new ArrayList();

		public Ini() {
		}

		public Ini(string strFile) {
			Load(strFile);
		}

		public Ini(Stream stm) {
			Load(stm);
		}

		public Ini(ArrayList alsSections) {
			m_alsSections = alsSections;
		}

		public void Load(string strFile) {
			FileStream stm = new FileStream(strFile, FileMode.Open);
			Load(stm);
			stm.Close();
		}

		public void Load(Stream stm) {
			TextReader tr = new StreamReader(stm);
			Regex rexSec = new Regex(@"^\s*\[\s*(?<section>.+)\s*\]\s*$");	
			Regex rexProperty = new Regex(@"^\s*(?<name>.+)\s*=\s*(?<value>.*)$");
			ArrayList alsComments = new ArrayList();
			Section secCurrent = null;

			m_alsSections.Clear();
			while (true) {
				string strLine = tr.ReadLine();
				if (strLine == null)
					break;

				// Search for a section header
				Match matSec = rexSec.Match(strLine);
				if (matSec.Groups["section"].Value.Length != 0) {
					secCurrent = new Section(matSec.Groups["section"].Value, (string[])alsComments.ToArray(typeof(string)));
					Add(secCurrent);
					alsComments.Clear();
					continue;
				}

				// See if it is a property.
				if (secCurrent != null) {
					Match matProperty = rexProperty.Match(strLine);
					if (matProperty.Groups["name"].Value.Length != 0) {
						secCurrent.Add(new Property(matProperty.Groups["name"].Value, matProperty.Groups["value"].Value, (string[])alsComments.ToArray(typeof(string))));
						alsComments.Clear();
						continue;
					}
				}

				// No clue what it is, retain it as a comment
				alsComments.Add(strLine);
			}
			tr.Close();
		}

#if false
		// Ini binary format

		struct SecChunk { // sck
			word offSecNext; // 0 if no next
			short cprop;
			// char szSecName[];
		};

		struct PropChunk { // pck
			// char szProp[]; // zero terminated
			// char szPropValue[]; // zero terminated
		};
#endif

		static string ReadString(BinaryReader brdr) {
			ArrayList alsBytes = new ArrayList();
			while (true) {
				byte b = brdr.ReadByte();
				if (b == 0) {
					break;
				}
				alsBytes.Add(b);
			}
			byte[] ab = (byte[])alsBytes.ToArray(typeof(byte));
			return Encoding.ASCII.GetString(ab);
		}

		public static Ini LoadBinary(Stream stm) {
			BinaryReader brdr = new BinaryReader(stm);
			ArrayList alsSections = new ArrayList();
			while (true) {
				int offNextSection = Misc.SwapUShort(brdr.ReadUInt16());
				int cProps = Misc.SwapUShort(brdr.ReadUInt16());
				string strNameSection = ReadString(brdr);
				Section sec = new Section(strNameSection);
				for (int iProp = 0; iProp < cProps; iProp++) {
					string strKey = ReadString(brdr);
					string strValue = ReadString(brdr);
					sec.Add(new Property(strKey, strValue));
				}
				alsSections.Add(sec);
				if ((brdr.BaseStream.Position & 1) != 0) {
					brdr.ReadByte();
				}
				if (offNextSection == 0) {
					break;
				}
			}

			return new Ini(alsSections);
		}

		public void SaveBinary(Stream stm) {
			BinaryWriter bwtr = new BinaryWriter(stm);
			for (int n = 0; n < Count; n++) {
				Section sec = (Section)m_alsSections[n];
				long posSection = bwtr.BaseStream.Position;
				bwtr.Write((ushort)0);
				bwtr.Write(Misc.SwapUShort((ushort)sec.Count));
				bwtr.Write(Misc.GetByteArrayFromString(sec.Name));
				foreach (Property prop in sec) {
					bwtr.Write(Misc.GetByteArrayFromString(prop.Name));
					bwtr.Write(Misc.GetByteArrayFromString(prop.Value));
				}
				if ((bwtr.BaseStream.Position & 1) != 0)
					bwtr.Write((byte)0);
				long posCurrent = bwtr.BaseStream.Position;
				bwtr.Seek((int)posSection, SeekOrigin.Begin);
				if (n == Count - 1) {
					bwtr.Write((ushort)0);
				} else {
					bwtr.Write(Misc.SwapUShort((ushort)(posCurrent - posSection)));
				}
				bwtr.Seek((int)posCurrent, SeekOrigin.Begin);
			}
			bwtr.Close();
		}

		public void Save(Stream stm) {
			TextWriter tw = new StreamWriter(stm);
			foreach (Section sec in this) {
				if (sec.Comments != null) {
					foreach (string str in sec.Comments)
						tw.WriteLine(str);
				}
				tw.WriteLine("[" + sec.Name + "]");
				foreach (Property prop in sec) {
					if (prop.Comments != null) {
						foreach (string str in prop.Comments)
							tw.WriteLine(str);
					}
					tw.WriteLine(prop.Name + "=" + prop.Value);
				}
			}
			tw.Flush();
		}

		public void Save(string strFile) {
			FileStream stm = new FileStream(strFile, FileMode.Create);
			Save(stm);
			stm.Close();
		}

		public string GetProperty(string strSection, string strProp) {
			Section sec = this[strSection];
			if (sec == null)
				return null;
			Property prop = sec[strProp];
			if (prop == null)
				return null;
			return prop.Value;
		}

		public void SetProperty(string strSection, string strProp, string strValue) {
			Section sec = this[strSection];
			if (sec == null) {
				sec = new Section(strSection);
				Add(sec);
			}
			Property prop = sec[strProp];
			if (prop == null) {
				prop = new Property(strProp, strValue);
				sec.Add(prop);
			} else {
				prop.Value = strValue;
			}
		}

		public Section Add(Section sec) {
			if (!m_alsSections.Contains(sec))
				m_alsSections.Add(sec);
			return sec;
		}

		public void Remove(Section sec) {
			m_alsSections.Remove(sec);
		}

		public void Insert(int n, Section sec) {
			m_alsSections.Insert(n, sec);
		}

		public Section this[int index] {
			get {
				return (Section)m_alsSections[index];
			}
		}

		public Section this[string strSection] {
			get {
				foreach (Section sec in m_alsSections) {
					if (strSection.ToUpper() == sec.Name.ToUpper())
						return sec;
				}
				return null;
			}
		}

		public Section[] Sections {
			get {
				return (Section[])m_alsSections.ToArray(typeof(Section));
			}
		}

		public int Count {
			get {
				return m_alsSections.Count;
			}
		}

		public SectionEnumerator GetEnumerator() {
			return new SectionEnumerator(this);
		}

		public class SectionEnumerator {
			private Ini m_ini;
			private int m_pos = -1;

			public SectionEnumerator(Ini ini) {
				m_ini = ini;
			}

			public bool MoveNext() {
				if (m_pos < m_ini.m_alsSections.Count - 1) {
					m_pos++;
					return true;
				}
				return false;
			}

			public void Reset() {
				m_pos = -1;
			}

			public Section Current {
				get {
					return (Section)m_ini.m_alsSections[m_pos];
				}
			}
		}

		public class Section {
			private ArrayList m_alsProperties = new ArrayList();
			public string Name;
			public string[] Comments;

			public Section(string strName, string[] astrComments) {
				Name = strName;
				Comments = astrComments;
			}

			public Section(string strName) {
				Name = strName;
				Comments = new string[] { "" };
			}

			public Property Add(Property prop) {
				if (!m_alsProperties.Contains(prop))
					m_alsProperties.Add(prop);
				return prop;
			}

			public void Remove(Property prop) {
				m_alsProperties.Remove(prop);
			}

			public void Insert(int n, Property prop) {
				m_alsProperties.Insert(n, prop);
			}

			public Property this[int index] {
				get {
					return (Property)m_alsProperties[index];
				}
			}

			public Property this[string strProperty] {
				get {
					foreach (Property prop in m_alsProperties) {
						if (prop.Name.ToUpper() == strProperty.ToUpper())
							return prop;
					}
					return null;
				}
			}

			public Property[] Properties {
				get {
					return (Property[])m_alsProperties.ToArray(typeof(Property));
				}
			}

			public int Count {
				get {
					return m_alsProperties.Count;
				}
			}

			public PropertyEnumerator GetEnumerator() {
				return new PropertyEnumerator(this);
			}

			public class PropertyEnumerator {
				private Section m_sec;
				private int m_pos = -1;

				public PropertyEnumerator(Section sec) {
					m_sec = sec;
				}

				public bool MoveNext() {
					if (m_pos < m_sec.m_alsProperties.Count - 1) {
						m_pos++;
						return true;
					}
					return false;
				}

				public void Reset() {
					m_pos = -1;
				}

				public Property Current {
					get {
						return (Property)m_sec.m_alsProperties[m_pos];
					}
				}
			}
		}

		public class Property {
			public string Name;
			public string Value;
			public string[] Comments;

			public Property(string strName, string strValue, string[] astrComments) {
				Name = strName;
				Value = strValue;
				Comments = astrComments;
			}

			public Property(string strName, string strValue) {
				Name = strName;
				Value = strValue;
				Comments = null;
			}
		}
	}
}
