using System;
using System.Runtime.Serialization;
using System.Collections;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;
using System.IO;
using SpiffLib;

namespace m
{
	[Serializable]
	public class TemplateDoc : Document, ISerializable, IDeserializationCallback  {
		Template m_tmplBackground = null;
		int m_cookie = 5000;
		ArrayList m_alsTemplates = new ArrayList();
		string m_strNameBackground = null;
		Size m_sizTile;
		Palette m_pal;

		public delegate void BackgroundChangedHandler(TemplateDoc tmpd);
		public event BackgroundChangedHandler BackgroundChanged;
		public delegate void NameChangedHandler(Document doc);
		public event NameChangedHandler NameChanged;

		public TemplateDoc(DocTemplate doct, string strFile, Object[] aobj) : base(doct, strFile) {
			if (aobj != null) {
				m_sizTile = (Size)aobj[0];
			} else {
				m_sizTile = AskTileSize();
			}
			m_doct = (TemplateDocTemplate)doct;
			InitCommon();
		}

		public TemplateDoc(SerializationInfo info, StreamingContext ctx) : base((DocTemplate)(((Hashtable)ctx.Context)["DocTemplate"]), (string)(((Hashtable)ctx.Context)["Filename"])) {
			m_cookie = info.GetInt32("Cookie");

			// Backwards compat

			try {
				m_strNameBackground = info.GetInt32("CookieBackground").ToString();
			} catch {
				m_strNameBackground = "0";
				try {
					m_strNameBackground = info.GetString("NameBackground");
				} catch {
				}
			}

			// Get tile size. If none, default 16,16

			try {
				m_sizTile = (Size)info.GetValue("TileSize", typeof(Size));
			} catch {
				m_sizTile = new Size(16, 16);
			}

			// Get palette

			try {
				m_pal = (Palette)info.GetValue("Palette", typeof(Palette));
			} catch {
				m_pal = null;
			}

			m_alsTemplates = (ArrayList)info.GetValue("TileTemplates", typeof(ArrayList));
		}

		public void OnDeserialization(object obSender) {
			foreach (Template tmpl in m_alsTemplates)
				tmpl.Doc = this;
			SetModified(false);
			InitCommon();
		}

		void InitCommon() {
			TemplateDocTemplate doct = (TemplateDocTemplate)m_doct;
			doct.TemplateChanged += new TemplateDocTemplate.TemplateChangedHandler(TemplateDocTemplate_TemplateChanged);
		}

		public void GetObjectData(SerializationInfo info, StreamingContext context) {
			info.AddValue("Cookie", m_cookie);
			info.AddValue("TileTemplates", m_alsTemplates);
			string strNameBackground = "";
			if (m_tmplBackground != null)
				strNameBackground = m_tmplBackground.Name;
			info.AddValue("NameBackground", strNameBackground);
			info.AddValue("TileSize", m_sizTile);
			if (m_pal != null)
				info.AddValue("Palette", m_pal);
		}

		Size AskTileSize() {
			TileSizeForm form = new TileSizeForm();
			form.ShowDialog(DocManager.GetFrameParent());
			return form.GetTileSize();
		}

		public override void SetPath(string strFile) {
			string strDir = m_strDir;
			string strFileName = m_strFileName;
			base.SetPath(strFile);
			if (strDir != m_strDir || strFileName != m_strFileName) {
				if (NameChanged != null)
					NameChanged(this);
			}
		}

		public override string GetName() {
			if (m_strFileName == null)
				return base.GetName();
			return Path.ChangeExtension(m_strFileName, null);
		}

		public void AddTemplates(string[] astrFileBitmap) {
			ArrayList alsNamesAdded = new ArrayList();
			foreach (string strFileBitmap in astrFileBitmap) {
				Template tmpl = new Template(this, "tmpl" + m_cookie);
				m_cookie++;
				if (tmpl.Import(strFileBitmap)) {
					alsNamesAdded.Add(tmpl.Name);
					m_alsTemplates.Add(tmpl);
				}
			}
			if (alsNamesAdded.Count != 0) {
				TemplateDocTemplate doct = (TemplateDocTemplate)m_doct;
				doct.OnTemplatesAdded(this, (string[])alsNamesAdded.ToArray(typeof(string)));
			}
			SetModified(true);
		}

		public void AddTemplates(Template[] atmpl) {
			if (atmpl.Length == 0)
				return;
			ArrayList alsNamesAdded = new ArrayList();
			foreach (Template tmpl in atmpl) {
				m_alsTemplates.Add(tmpl);
				alsNamesAdded.Add(tmpl.Name);
			}
			TemplateDocTemplate doct = (TemplateDocTemplate)m_doct;
			doct.OnTemplatesAdded(this, (string[])alsNamesAdded.ToArray(typeof(string)));
			SetModified(true);
		}

		public void RemoveTemplates(Template[] atmpl) {
			if (atmpl.Length == 0)
				return;
			ArrayList alsNames = new ArrayList();
			foreach (Template tmpl in atmpl) {
				m_alsTemplates.Remove(tmpl);
				alsNames.Add(tmpl.Name);
				if (tmpl == m_tmplBackground)
					SetBackgroundTemplate(null);
			}
			TemplateDocTemplate doct = (TemplateDocTemplate)m_doct;
			doct.OnTemplatesRemoved(this, (string[])alsNames.ToArray(typeof(string)));
			SetModified(true);
		}

		public Template[] GetTemplates() {
			return (Template[])m_alsTemplates.ToArray(typeof(Template));
		}

		void TemplateDocTemplate_TemplateChanged(TemplateDoc tmpd, string strProperty, string strName, string strParam) {
			if (tmpd == this)
				SetModified(true);
		}

		public Template FindTemplate(string strName) {
			foreach (Template tmpl in m_alsTemplates) {
				if (tmpl.Name != null && tmpl.Name == strName)
					return tmpl;
			}
			return null;
		}

		public void Dispose() {
			RemoveTemplates((Template[])m_alsTemplates.ToArray(typeof(Template)));
		}

		public Template GetBackgroundTemplate() {
			if (m_tmplBackground == null && m_strNameBackground != null)
				m_tmplBackground = FindTemplate(m_strNameBackground);
			return m_tmplBackground;
		}

		public void SetBackgroundTemplate(Template tmpl) {
			if (m_tmplBackground == tmpl)
				return;
			m_tmplBackground = tmpl;
			if (BackgroundChanged != null)
				BackgroundChanged(this);
			SetModified(true);
		}

		public void SetPalette(Palette pal, bool fColorMatch) {
			m_pal = pal;
			SetModified(true);
			if (!fColorMatch)
				return;
			ArrayList alsColors = new ArrayList();
			foreach (Template tmpl in m_alsTemplates) {
				Bitmap bm = tmpl.Bitmap;
				bool[,] afOccupancy = tmpl.OccupancyMap;
				int ctx = afOccupancy.GetLength(1);
				int cty = afOccupancy.GetLength(0);
				for (int ty = 0; ty < cty; ty++) {
					for (int tx = 0; tx < ctx; tx++) {
						if (!afOccupancy[ty, tx])
							continue;
						int xOrigin = tx * m_sizTile.Width;
						int yOrigin = ty * m_sizTile.Height;
						for (int y = yOrigin; y < yOrigin + m_sizTile.Height; y++) {
							for (int x = xOrigin; x < xOrigin + m_sizTile.Width; x++) {
								Color clrOld = bm.GetPixel(x, y);
								Color clrNew = pal[pal.FindClosestEntry(clrOld)];
								bm.SetPixel(x, y, clrNew);
							}
						}
					}
				}
				TemplateDocTemplate doct = (TemplateDocTemplate)m_doct;
				doct.OnTemplateChanged(this, "Bitmap", tmpl.Name, null);
			}
		}

		public Palette GetPalette() {
			return m_pal;
		}

		public bool IsNameUnique(string strName) {
			foreach (Template tmpl in m_alsTemplates) {
				if (tmpl.Name == strName)
					return false;
			}
			return true;
		}

		public Size TileSize {
			get {
				return m_sizTile;
			}
			set {
				// This is only done in special circumstances that don't require notification

				m_sizTile = value;
			}
		}
	}

	public enum TerrainTypes { Start = 0, Open = 0, Blocked, Area, Wall, End };
	public enum TerrainColors { Grass, Cliff, Water, Road };

	[Serializable]
	public class Template : ISerializable {
		public string ImportPath = null;
		private string m_strName = null;
		public bool[,] OccupancyMap = null;
		public TerrainTypes[,] TerrainMap = null;
		public TerrainColors[,] TerrainColors = null;
		public Bitmap Bitmap = null;
		public TemplateDoc Doc = null;
		TemplateDocTemplate m_doct = (TemplateDocTemplate)DocManager.FindDocTemplate(typeof(TemplateDoc));

		public Template(TemplateDoc doc, string strName) {
			Doc = doc;
			m_strName = strName;
		}

		public Template(TemplateDoc doc, Bitmap bm, string strName) {
			m_strName = strName;
			Doc = doc;
			if (!SetBitmap(bm))
				throw new Exception("Invalid tile template");
		}

		public Template(SerializationInfo info, StreamingContext ctx) {
			// Backwards compat
			try {
				m_strName = info.GetInt32("Cookie").ToString();
			} catch {
				m_strName = info.GetString("Name");
			}
			try {
				TerrainColors = (TerrainColors[,])info.GetValue("TerrainColors", typeof(TerrainColors[,]));
			} catch {
				TerrainColors = null;
			}
			OccupancyMap = (bool[,])info.GetValue("OccupancyMap", typeof(bool[,]));
			TerrainMap = (TerrainTypes[,])info.GetValue("TerrainMap", typeof(TerrainTypes[,]));
			Bitmap = (Bitmap)info.GetValue("Bitmap", typeof(Bitmap));
		}

		public void GetObjectData(SerializationInfo info, StreamingContext context) {
			info.AddValue("Name", m_strName);
			info.AddValue("OccupancyMap", OccupancyMap);
			info.AddValue("TerrainMap", TerrainMap);
			info.AddValue("Bitmap", Bitmap);
			info.AddValue("TerrainColors", TerrainColors);
		}

		public bool Import(string strFileBitmap) {
			try {
				Bitmap bm = new Bitmap(strFileBitmap);
				if (SetBitmap(bm)) {
					ImportPath = strFileBitmap;
					return true;
				}
				return false;
			} catch {
				MessageBox.Show("Error opening tile bitmap");
				return false;
			}
		}

		private bool SetBitmap(Bitmap bm) {
			if ((bm.Width % Doc.TileSize.Width) != 0 && (bm.Height % Doc.TileSize.Height) != 0) {
				MessageBox.Show("Tile dimensions not a multiple of tile size");
				return false;
			}
			bm.MakeTransparent(Color.FromArgb(255, 0, 255));
			Bitmap = bm;
			UpdateMaps();
			m_doct.OnTemplateChanged(Doc, "Bitmap", m_strName, null);
			return true;
		}

		// Update the map sizes to reflect the new bitmap

		private void UpdateMaps() {
			int ctx = Bitmap.Width / Doc.TileSize.Width;
			int cty = Bitmap.Height / Doc.TileSize.Height;

			// Try to copy what we can of the old terrain map
			TerrainTypes[,] ater = new TerrainTypes[cty, ctx];
			if (TerrainMap != null) {
				int ctyOld = TerrainMap.GetLength(0);
				int ctxOld = TerrainMap.GetLength(1);
				for (int tx = 0; tx < ctx && tx < ctxOld; tx++) {
					for (int ty = 0; ty < cty && ty < ctyOld; ty++) {
						ater[ty, tx] = TerrainMap[ty, tx];
					}
				}
			}
			TerrainMap = ater;

			// Create a new occupancy map. Old one not relevant.
			bool[,] afOcc = new bool[cty, ctx];
			for (int tx = 0; tx < ctx; tx++) {
				for (int ty = 0; ty < cty; ty++) {
					afOcc[ty, tx] = true;
					Color clr = Bitmap.GetPixel(tx * Doc.TileSize.Width, ty * Doc.TileSize.Height);
					if (clr.A == 0)
						afOcc[ty, tx] = false;
				}
			}
			OccupancyMap = afOcc;
		}

		// Properties

		public string Name {
			get {
				return m_strName;
			}

			set {
				// Only if unique

				if (value == m_strName)
					return;
				if (!Doc.IsNameUnique(value)) {
					MessageBox.Show("The name " + value + " is not unique. Not assigned.");
					return;
				}
				string strNameOld = m_strName;
				m_strName = value;
				m_doct.OnTemplateChanged(Doc, "Name", strNameOld, m_strName);
			}
		}

		public int Ctx {
			get {
				return Bitmap.Width / Doc.TileSize.Width;
			}
		}

		public int Cty {
			get {
				return Bitmap.Height / Doc.TileSize.Height;
			}
		}
	}

	// A doc template for (tile) templates.

	public class TemplateDocTemplate : DocTemplate {
		static string[] astr = { "Template Collection", "Untitled", "Templates", "tc" };

		public delegate void TemplatesAddedHandler(TemplateDoc tmpd, string[] astrName);
		public event TemplatesAddedHandler TemplatesAdded;
		public delegate void TemplatesRemovedHandler(TemplateDoc tmpd, string[] astrName);
		public event TemplatesRemovedHandler TemplatesRemoved;
		public delegate void TemplateChangedHandler(TemplateDoc tmpd, string strProperty, string strName, string strParam);
		public event TemplateChangedHandler TemplateChanged;		
		
		public TemplateDocTemplate() : base(astr, typeof(TemplateDoc), null, null, new TemplateDocBinder()) {
		}

		public override Document OpenDocument(string strFile) {
			// Don't support loading the same templates twice

			string strPathLower = null;
			if (strFile != null)
				strPathLower = Path.GetFullPath(strFile).ToLower();
			foreach (Document doc in m_alsDocuments) {
				string strPath = doc.GetPath();
				if (strPath == null)
					continue;
				if (strPath.ToLower() == strPathLower) {
					SetActiveDocument(doc);
					return doc;
				}
			}

			return base.OpenDocument(strFile);
		}

		public void OnTemplatesAdded(TemplateDoc tmpd, string[] astrName) {
			if (TemplatesAdded != null)
				TemplatesAdded(tmpd, astrName);
		}

		public void OnTemplateChanged(TemplateDoc tmpd, string strProperty, string strName, string strParam) {
			if (TemplateChanged != null)
				TemplateChanged(tmpd, strProperty, strName, strParam);
		}

		public void OnTemplatesRemoved(TemplateDoc tmpd, string[] astrName) {
			if (TemplatesRemoved != null)
				TemplatesRemoved(tmpd, astrName);
		}
	}

	// Compatibility goo

	public class TemplateDocBinder : SerializationBinder {
		public override Type BindToType(string strAssembly, string strType) {
			if (strType == "m.TileTemplateCollection")
				return typeof(TemplateDoc);
			if (strType == "m.TileTemplate")
				return typeof(Template);
			return Type.GetType(strType);
		}
	}
}
