using System;
using System.Collections;
using System.Collections.Specialized;
using System.IO;
using System.Windows.Forms;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.Serialization.Formatters.Soap;
using System.ComponentModel;
using SpiffLib;
using System.Drawing;
using System.Drawing.Imaging;
using System.Text;
using System.Text.RegularExpressions;

namespace m {
	public enum LayerType { Start = 0, TileMap = 0, Galaxite, SurfaceDecal, Scenery, DepthSorted, SmokeFire, Area, Selection, End };

	[Flags]
	public enum LayerFlags { Templates = 1, Gobs = 2, Areas = 4, Default = (Templates | Gobs | Areas) };

	[Serializable]
	public class LevelDoc : Document, ISerializable, IDeserializationCallback {
		string m_strOutputFilename = null;
		bool m_fSwitchTemplatesEnabled = true;
		IMapItem[] m_amiDeserializationTemp;
		SideInfo[] m_asidiDeserializationTemp;
		TemplateDoc m_tmpd = null;
		string m_strTitle = null;
		Rectangle m_trcBounds = new Rectangle(1, 1, 62, 62);
		int m_nPlayersMin = 1;
		int m_nPlayersMax = 1;
		ArrayList m_alsmi = new ArrayList();
		ArrayList m_alsidi = new ArrayList();
		bool m_fUpdateDirty = false;
		static bool s_fLoading = false;
		int m_ctx = 64, m_cty = 64;
		TriggerManager m_tgrm = new TriggerManager();
		UnitGroupManager m_ugm = new UnitGroupManager();
		SwitchManager m_swm = new SwitchManager();
		CounterManager m_ctrm = new CounterManager();
		string m_strComment;
		int[] m_mpDirToDx = new int[] { 0, 1, 1, 1, 0, -1, -1, -1 };
		int[] m_mpDirToDy = new int[] { -1, -1, 0, 1, 1, 1, 0, -1 };
		ArrayList m_alsmiSelected = new ArrayList();

		public delegate void ImageChangedHandler();
		public event ImageChangedHandler ImageChanged;
		public delegate void ItemsRemovedHandler(IMapItem[] ami);
		public event ItemsRemovedHandler ItemsRemoved;
		public delegate void NameChangedHandler(LevelDoc lvld);
		public event NameChangedHandler NameChanged;

		public LevelDoc(DocTemplate doct, string strFile, Object aobj) : base(doct, strFile) {
			m_strTitle = m_doct.GetString(DocTemplate.Strings.NewFileName);
			SetTemplateDoc((TemplateDoc)DocManager.GetActiveDocument(typeof(TemplateDoc)));
			InitCommon();
		}

		// ISerializable implemented to prevent events from being serialized
		public LevelDoc(SerializationInfo info, StreamingContext ctx) : base((DocTemplate)(((Hashtable)ctx.Context)["DocTemplate"]), (string)(((Hashtable)ctx.Context)["Filename"])) {
			s_fLoading = true;
			CaTypeSwitch.InitFixupList();

			Title = info.GetString("Title");
			m_trcBounds.X = info.GetInt32("BoundsX");
			m_trcBounds.Y = info.GetInt32("BoundsY");
			m_trcBounds.Width = info.GetInt32("BoundsWidth");
			m_trcBounds.Height = info.GetInt32("BoundsHeight");

			try {
				// These were introduced at the same time so either they're both present or they both aren't

				m_ctx = info.GetInt32("Width");
				m_cty = info.GetInt32("Height");
			} catch (SerializationException) {
			}

			// Default values in case they aren't present

			m_nPlayersMin = 1;
			m_nPlayersMax = 1;
			try {
				// These were introduced at the same time so either they're both present or they both aren't

				m_nPlayersMin = info.GetInt32("MinPlayers");
				m_nPlayersMax = info.GetInt32("MaxPlayers");
			} catch (SerializationException) {
			}

			// Side info

			try {
				m_asidiDeserializationTemp = (SideInfo[])info.GetValue("SideInfos", typeof(SideInfo[]));
			} catch (SerializationException) {
			}

			// TriggerManager

			try {
				m_tgrm = (TriggerManager)info.GetValue("TriggerManager", typeof(TriggerManager));
			} catch (SerializationException) {
				m_tgrm = new TriggerManager();
			}

			// UnitGroupManager

			try {
				m_ugm = (UnitGroupManager)info.GetValue("UnitGroupManager", typeof(UnitGroupManager));
			} catch (SerializationException) {
				m_ugm = new UnitGroupManager();
			}

			// SwitchManager

			try {
				m_swm = (SwitchManager)info.GetValue("SwitchManager", typeof(SwitchManager));
			} catch (SerializationException) {
				m_swm = new SwitchManager();
			}

			// CounterManager

			try {
				m_ctrm = (CounterManager)info.GetValue("CounterManager", typeof(CounterManager));
			} catch (SerializationException) {
				m_ctrm = new CounterManager();
			}

			// Comment string

			try {
				m_strComment = info.GetString("Comment");
			} catch (SerializationException) {
			}

			// Support old .ld's that have separate .map files

			string strTemplatesFileName = null;
			try {
				string strFile = info.GetString("MapFileName");
				if (strFile != null) {
					Stream stm = null;
					try {
						IFormatter fmtr = new BinaryFormatter();
						stm = new FileStream(m_strDir + Path.DirectorySeparatorChar + strFile, FileMode.Open, FileAccess.Read, FileShare.Read);
						strTemplatesFileName = (string)fmtr.Deserialize(stm);
						SetTemplateDoc((TemplateDoc)DocManager.OpenDocument(m_strDir + Path.DirectorySeparatorChar + strTemplatesFileName));
						m_alsmi.AddRange((IMapItem[])fmtr.Deserialize(stm));
						stm.Close();
					} catch {
						if (stm != null)
							stm.Close();
					}
				}
			} catch(SerializationException) {
			}

			// Load the desired template set

			if (m_tmpd == null) {
				try {
					strTemplatesFileName = info.GetString("TemplatesFileName");
					SetTemplateDoc((TemplateDoc)DocManager.OpenDocument(m_strDir + Path.DirectorySeparatorChar + strTemplatesFileName));
				} catch (SerializationException) {
					// If couldn't load it, use the active one

					SetTemplateDoc(null);
					string strT = "Couldn't load " + strTemplatesFileName;
					if (m_tmpd != null)
						strT += " - Using " + m_tmpd.GetName();
					MessageBox.Show(strT);
				}
			}

			// Load the map items

			m_amiDeserializationTemp = (IMapItem[])info.GetValue("MapItems", typeof(IMapItem[]));
		}

		public void OnDeserialization(object obSender) {

			// NEW FOR .NET 1.1
			// Couldn't do this during the deserialization constructor any more because the
			// result is null references in the ArrayList.

			m_alsidi.AddRange(m_asidiDeserializationTemp);
			m_asidiDeserializationTemp = null;

			// NEW FOR .NET 1.1
			// Couldn't do this during the deserialization constructor any more because the
			// result is null references in the ArrayList.

			m_alsmi.AddRange(m_amiDeserializationTemp);

			// Set events

			AddEventHandlers(m_amiDeserializationTemp);
			m_amiDeserializationTemp = null;
    
			// Incorporate old-style switches into the SwitchManager 
    
			CaTypeSwitch.Fixup(this); 
    
			// Common initialization 
    
			InitCommon(); 
		}                

		public void GetObjectData(SerializationInfo info, StreamingContext context) {
			info.AddValue("Title", Title);
			info.AddValue("MapItems", (IMapItem[])m_alsmi.ToArray(typeof(IMapItem)));
			info.AddValue("BoundsX", Bounds.X);
			info.AddValue("BoundsY", Bounds.Y);
			info.AddValue("BoundsWidth", Bounds.Width);
			info.AddValue("BoundsHeight", Bounds.Height);
			info.AddValue("MinPlayers", MinPlayers);
			info.AddValue("MaxPlayers", MaxPlayers);
			info.AddValue("SideInfos", GetSideInfos());
			info.AddValue("Width", Width);
			info.AddValue("Height", Height);
			info.AddValue("TriggerManager", m_tgrm);
			info.AddValue("UnitGroupManager", m_ugm);
			info.AddValue("SwitchManager", m_swm);
			info.AddValue("CounterManager", m_ctrm);
			info.AddValue("Comment", m_strComment);
			if (m_tmpd == null) {
				info.AddValue("TemplatesFileName", "");
			} else {
				info.AddValue("TemplatesFileName", Path.GetFileName(m_tmpd.GetPath()));
			}
		}

		void InitCommon() {
			// We want to know when the application goes idle for batching MapItem notifications
			// into single doc notifications

			Application.Idle += new EventHandler(Application_IdleHandler);

			// We want to know when the active template doc changes

			TemplateDocTemplate doct = (TemplateDocTemplate)DocManager.FindDocTemplate(typeof(TemplateDoc));
			doct.DocActive += new DocTemplate.DocActiveHandler(TemplateDocTemplate_DocActiveHandler);
			doct.TemplatesRemoved += new TemplateDocTemplate.TemplatesRemovedHandler(TemplateDocTemplate_TemplatesRemovedHandler);
			doct.TemplatesAdded += new TemplateDocTemplate.TemplatesAddedHandler(TemplateDocTemplate_TemplatesAddedHandler);
			SetModified(false);
		}

		public void Dispose() {
			m_alsmi.Clear();
		}

		void Application_IdleHandler(object sender, EventArgs e) {
			s_fLoading = false;
			if (!m_fUpdateDirty)
				return;
			m_fUpdateDirty = false;
			if (ImageChanged != null)
				ImageChanged();
		}

		public void Refresh() {
			m_fUpdateDirty = true;
		}

		void TemplateDocTemplate_DocActiveHandler(Document doc) {
			// TemplateDocs get loaded and activated during LevelDoc
			// deserialization. We don't want to change the template doc
			// of the currently active level doc in this case

			if (!s_fLoading && m_fSwitchTemplatesEnabled) {
				if (DocManager.GetActiveDocument(typeof(LevelDoc)) == this)
					SetTemplateDoc((TemplateDoc)doc);
			}
		}

		public bool SwitchTemplatesEnabled {
			get {
				return m_fSwitchTemplatesEnabled;
			}
			set {
				m_fSwitchTemplatesEnabled = value;
			}
		}

		public string OutputFilename {
			get {
				return m_strOutputFilename;
			}
			set {
				m_strOutputFilename = value;
			}
		}

		void SetTemplateDoc(TemplateDoc tmpd) {
			if (m_tmpd == tmpd)
				return;

			// Active template doc change. Remove event handlers from old one, add event handlers to new

			if (m_tmpd != null)
				m_tmpd.BackgroundChanged -= new TemplateDoc.BackgroundChangedHandler(TemplateDoc_BackgroundChangedHandler);

			// Assign new. If null, use current active

			m_tmpd = tmpd;
			if (m_tmpd == null)
				m_tmpd = (TemplateDoc)DocManager.GetActiveDocument(typeof(TemplateDoc));

			// Put in hooks

			if (m_tmpd != null)
				m_tmpd.BackgroundChanged += new TemplateDoc.BackgroundChangedHandler(TemplateDoc_BackgroundChangedHandler);

			// Raise changed event

			m_fUpdateDirty = true;

			// Doc modified

			SetModified(true);
		}

		public override bool Save() {
			// Save the template doc when we save the level doc. For convenience

			if (base.Save()) {
				if (m_tmpd != null && m_tmpd.IsModified())
					m_tmpd.Save();
				return true;
			}
			return false;
		}

		public override bool SaveAs(string strFile) {
			// Save the template doc when we save the level doc. For convenience

			if (base.SaveAs(strFile)) {
				if (m_tmpd != null)
					m_tmpd.Save();
				return true;
			}
			return false;
		}

		public TemplateDoc GetTemplateDoc() {
			return m_tmpd;
		}

		public override string GetName() {
			return m_strTitle;
		}

		private SideInfo[] GetSideInfos() {
			return (SideInfo[])m_alsidi.ToArray(typeof(SideInfo));
		}

		public SideInfo GetSideInfo(Side side) {
			foreach (SideInfo sidi in m_alsidi) {
				if (sidi.Side == side)
					return sidi;
			}

			SideInfo sidiNew = new SideInfo(side);
			m_alsidi.Add(sidiNew);
			SetModified(true);
			return sidiNew;
		}

		public void Draw(Bitmap bm, IMapItem miExclude, Size sizTile, TemplateDoc tmpd, LayerFlags lyrf) {
			ArrayList alsmiSelected = m_alsmiSelected;

			// Draw tile map
			DrawTileMap(bm, alsmiSelected, sizTile, tmpd, lyrf);

			// Draw other layers			
			using (Graphics g = Graphics.FromImage(bm)) {
				for (LayerType layer = LayerType.Galaxite; layer < LayerType.End; layer++) {
					if (layer == LayerType.Area) {
						if ((lyrf & LayerFlags.Areas) == 0)
							continue;
					} else {
						if ((lyrf & LayerFlags.Gobs) == 0)
							continue;
					}
					foreach (IMapItem mi in m_alsmi) {
						if (mi != miExclude) {
							int x = (int)(mi.tx * sizTile.Width);
							int y = (int)(mi.ty * sizTile.Height);
							mi.Draw(g, x, y, sizTile, tmpd, layer, alsmiSelected != null ? alsmiSelected.Contains(mi) : false);
						}
					}
				}

				// Draw bounds
				Pen pen = new Pen(new SolidBrush(Color.FromArgb(0, 255, 0)));
				pen.Width = 2;
				g.DrawRectangle(pen, Bounds.X * sizTile.Width - pen.Width + 1, Bounds.Y * sizTile.Height - pen.Width + 1, Bounds.Width * sizTile.Width + pen.Width, Bounds.Height * sizTile.Height + pen.Width);
			}
		}

		public void DrawTileMap(Bitmap bm, ArrayList alsmiSelected, Size sizTile, TemplateDoc tmpd, LayerFlags lyrf) {
			// Draw background

			using (Graphics g = Graphics.FromImage(bm)) {
				Template tmplBackground = null;
				if (tmpd != null)
					tmplBackground = tmpd.GetBackgroundTemplate();
				if (tmplBackground == null) {
					g.FillRectangle(new SolidBrush(Color.Black), 0, 0, bm.Width, bm.Height);
					for (int x = sizTile.Width; x < bm.Width; x += sizTile.Width) {
						for (int y = sizTile.Height; y < bm.Height; y += sizTile.Height) {
							bm.SetPixel(x, y, Color.BlanchedAlmond);
						}
					}
				} else {
					for (int x = 0; x < bm.Width; x += tmplBackground.Bitmap.Width) {
						for (int y = 0; y < bm.Height; y += tmplBackground.Bitmap.Height)
							g.DrawImage(tmplBackground.Bitmap, x, y);
					}
				}

				// Draw templates

				if ((lyrf & LayerFlags.Templates) != 0) {
					foreach (IMapItem mi in m_alsmi) {
						int x = (int)mi.tx * sizTile.Width;
						int y = (int)mi.ty * sizTile.Height;
						mi.Draw(g, x, y, sizTile, tmpd, LayerType.TileMap, alsmiSelected != null ? alsmiSelected.Contains(mi) : false);
					}
				}
			}
		}

		public void AddMapItems(IMapItem[] ami) {
			m_alsmi.AddRange(ami);
			AddEventHandlers(ami);
			m_fUpdateDirty = true;
			SetModified(true);
		}

		public void RemoveMapItems(IMapItem[] ami) {
			for (int n = 0; n < ami.Length; n++)
				m_alsmi.Remove(ami[n]);
			RemoveEventHandlers(ami);
			if (ItemsRemoved != null)
				ItemsRemoved(ami);
			m_fUpdateDirty = true;
			SetModified(true);
		}

		void AddEventHandlers(IMapItem[] ami) {
			foreach (IMapItem mi in ami)
				mi.PropertyChanged += new PropertyChangedHandler(IMapItem_PropertyChanged);
		}

		void RemoveEventHandlers(IMapItem[] ami) {
			foreach (IMapItem mi in ami)
				mi.PropertyChanged -= new PropertyChangedHandler(IMapItem_PropertyChanged);
		}

		void IMapItem_PropertyChanged(IMapItem mi, string strProperty) {
			SetModified(true);
			m_fUpdateDirty = true;
		}

		void TemplateDoc_BackgroundChangedHandler(TemplateDoc tmpd) {
			m_fUpdateDirty = true;
		}

		private void TemplateDocTemplate_TemplatesAddedHandler(TemplateDoc tmpd, string[] astrNames) {
			TemplatesAddedRemoved(astrNames);
		}


		private void TemplateDocTemplate_TemplatesRemovedHandler(TemplateDoc tmpd, string[] astrNames) {
			TemplatesAddedRemoved(astrNames);
		}

		void TemplatesAddedRemoved(string[] astrNames) {
			foreach (string strName in astrNames) {
				for (int n = 0; n < m_alsmi.Count; n++) {
					Tile tile = m_alsmi[n] as Tile;
					if (tile != null && tile.Name == strName) {
						m_fUpdateDirty = true;
						SetModified(true);
						return;
					}
				}
			}
		}

		public IMapItem HitTest(int x, int y, Size sizTile, TemplateDoc tmpd, LayerFlags lyrf) {
			// MapItems on top are first
			for (int i = m_alsmi.Count - 1; i >= 0; i--) {
				IMapItem mi = (IMapItem)m_alsmi[i];
				if (mi.HitTest(x, y, sizTile, tmpd)) {
					if (mi is Area) {
						if ((lyrf & LayerFlags.Areas) != 0)
							return mi;
					} else if (mi is Tile) {
						if ((lyrf & LayerFlags.Templates) != 0)
							return mi;
					} else {
						if ((lyrf & LayerFlags.Gobs) != 0)
							return mi;
					}
				}
			}
			return null;
		}

		public ArrayList HitTest(Rectangle rc, Size sizTile, TemplateDoc tmpd, LayerFlags lyrf) {
			ArrayList alsmi = new ArrayList();
			foreach (IMapItem mi in m_alsmi) {
				Point ptCenter = mi.GetCenterPoint(sizTile);
				if (rc.Contains(ptCenter)) {
					if (mi is Area) {
						if ((lyrf & LayerFlags.Areas) != 0)
							alsmi.Add(mi);
					} else if (mi is Tile) {
						if ((lyrf & LayerFlags.Templates) != 0)
							alsmi.Add(mi);
					} else {
						if ((lyrf & LayerFlags.Gobs) != 0)
							alsmi.Add(mi);
					}
				}
			}
			return alsmi;
		}

		public Bitmap GetMapBitmap(Size sizTile, TemplateDoc tmpd, bool fTilesOnly) {
			Bitmap bm = new Bitmap(Width * sizTile.Width, Height * sizTile.Height, PixelFormat.Format24bppRgb);

			DrawTileMap(bm, null, sizTile, tmpd, LayerFlags.Default);

			if (!fTilesOnly) {
				using (Graphics g = Graphics.FromImage(bm)) {
					for (LayerType layer = LayerType.Galaxite; layer < LayerType.End; layer++) {
						foreach (IMapItem mi in m_alsmi) {
							int x = (int)(mi.tx * sizTile.Width);
							int y = (int)(mi.ty * sizTile.Height);
							mi.Draw(g, x, y, sizTile, tmpd, layer, false);
						}
					}
				}
			}

			Bitmap bmT = new Bitmap(Bounds.Width * sizTile.Width, Bounds.Height * sizTile.Height, PixelFormat.Format24bppRgb);
			using (Graphics g = Graphics.FromImage(bmT)) {
				Rectangle rcSrc = new Rectangle(Bounds.X * sizTile.Width, Bounds.Y * sizTile.Height, bmT.Width, bmT.Height);
				g.DrawImage(bm, 0, 0, rcSrc, GraphicsUnit.Pixel);
			}
			bm.Dispose();
			return bmT;
		}

		public TerrainTypes[,] GetTerrainMap(Size sizTile, TemplateDoc tmpd, bool fStructures) {
			// Get raw terrain map

			TerrainTypes[,] aterMap = GetRawTerrainMap(sizTile, tmpd);

			// Flood fill to mark areas. The "biggest" area is accessible.
			// All the smaller areas are not; mark them as such.

			int[,] anFill = new int[aterMap.GetLength(0), aterMap.GetLength(1)];

			// Mark all the areas in the fill map that are known to be not accessible.

			for (int ty = 0; ty < aterMap.GetLength(0); ty++) {
				for (int tx = 0; tx < aterMap.GetLength(1); tx++) {
					if (aterMap[ty, tx] == TerrainTypes.Blocked) {
						anFill[ty, tx] = -1;
					}
				}
			}

			// Mark structures if asked, before flood filling
			// This is used for terrain diffing to see if structures make terrain inaccessible

			if (fStructures) {
				foreach (IMapItem mi in m_alsmi) {
					if (mi is Structure) {
						for (int ty = (int)mi.ty; ty < mi.ty + mi.cty; ty++) {
							for (int tx = (int)mi.tx; tx < mi.tx + mi.ctx; tx++) {
								if (Bounds.Contains(tx, ty)) {
									int txT = tx - Bounds.Left;
									int tyT = ty - Bounds.Top;
									anFill[tyT, txT] = -1;
								}
							}
						}
					}
				}
			}

			// Start flood filling areas

			int nFillValue = 1;
			ArrayList alsFillCounts = new ArrayList();
			alsFillCounts.Add(0);
			for (int ty = 0; ty < anFill.GetLength(0); ty++) {
				for (int tx = 0; tx < anFill.GetLength(1); tx++) {
					if (anFill[ty, tx] == 0) {
						int cFill = FloodFill(anFill, tx, ty, nFillValue);
						nFillValue++;
						alsFillCounts.Add(cFill);
					}
				}
			}

			// Find the largest count; that is the accessible area

			int nFillValueLargest = -1;
			int cLargest = 0;
			for (int n = 0; n < alsFillCounts.Count; n++) {
				if ((int)alsFillCounts[n] > cLargest) {
					cLargest = (int)alsFillCounts[n];
					nFillValueLargest = n;
				}
			}

			// Now mark the areas that aren't this fill value as inaccessible

			for (int ty = 0; ty < anFill.GetLength(0); ty++) {
				for (int tx = 0; tx < anFill.GetLength(1); tx++) {
					if (anFill[ty, tx] != nFillValueLargest) {
						aterMap[ty, tx] = TerrainTypes.Blocked;
					}
				}
			}

			// Mark where areas are in the terrain

			foreach (IMapItem mi in m_alsmi) {
				if (mi is Area) {
					for (int ty = (int)mi.ty; ty < mi.ty + mi.cty; ty++) {
						for (int tx = (int)mi.tx; tx < mi.tx + mi.ctx; tx++) {
							if (Bounds.Contains(tx, ty)) {
								int txT = tx - Bounds.Left;
								int tyT = ty - Bounds.Top;
								if (aterMap[tyT, txT] == TerrainTypes.Open)
									aterMap[tyT, txT] = TerrainTypes.Area;
							}
						}
					}
				}
			}

			// Mark where walls are in the terrain

			foreach (IMapItem mi in m_alsmi) {
				if (mi is Wall) {
					int tx = (int)mi.tx;
					int ty = (int)mi.ty;
					if (Bounds.Contains(tx, ty)) {
						int txT = tx - Bounds.Left;
						int tyT = ty - Bounds.Top;
						aterMap[tyT, txT] = TerrainTypes.Wall;
					}
				}
			}

			// Done

			return aterMap;
		}

		int FloodFill(int[,] anFill, int tx, int ty, int nFillValue) {
			int nFillMatch = anFill[ty, tx];
			if (nFillMatch == nFillValue)
				return 0;

			ArrayList alsPoints = new ArrayList();
			anFill[ty, tx] = nFillValue;
			alsPoints.Add(new Point(tx, ty));
			int ipt = 0;
			int cFill = 1;

			while (true) {
				bool fFound = false;
				Point ptT = (Point)alsPoints[ipt];
				for (int dir = 0; dir < 8; dir++) {
					int txT = ptT.X + m_mpDirToDx[dir];
					int tyT = ptT.Y + m_mpDirToDy[dir];
					if (txT < 0 || txT >= anFill.GetLength(1))
						continue;
					if (tyT < 0 || tyT >= anFill.GetLength(0))
						continue;
					if (anFill[tyT, txT] == nFillMatch) {
						cFill++;
						anFill[tyT, txT] = nFillValue;
						alsPoints.Add(new Point(txT, tyT));
						ipt++;
						fFound = true;
						break;
					}
				}
				if (!fFound) {
					alsPoints.RemoveAt(ipt);
					ipt--;
					if (ipt < 0)
						break;
				}
			}

			return cFill;
		}

		TerrainTypes[,] GetRawTerrainMap(Size sizTile, TemplateDoc tmpd) {
			TerrainTypes[,] aterMap = new TerrainTypes[Bounds.Height, Bounds.Width];
			foreach (IMapItem mi in m_alsmi) {
				if (mi is Tile) {
					Tile tile = mi as Tile;
					int x = (int)mi.tx * sizTile.Width;
					int y = (int)mi.ty * sizTile.Height;
					Rectangle rc = tile.GetBoundingRectAt(x, y, sizTile, m_tmpd);
					rc.Width /= sizTile.Width;
					rc.Height /= sizTile.Height;
					rc.X /= sizTile.Width;
					rc.Y /= sizTile.Height;
					rc.Intersect(Bounds);

					for (int ty = rc.Y - (int)tile.ty; ty < rc.Bottom - (int)tile.ty; ty++) {
						for (int tx = rc.X - (int)tile.tx; tx < rc.Right - (int)tile.tx; tx++) {
							if (tile.Visibility == null || tile.IsVisible(tx, ty)) {
								Template tmpl = tile.GetTemplate(m_tmpd);

								// Maybe this template doesn't exist in document's current tile collection

								if (tmpl == null)
									continue;

								// Make sure this part of the template is occupied first

								if (tmpl.OccupancyMap[ty, tx])
									aterMap[ty + (int)tile.ty - Bounds.Y, tx + (int)tile.tx - Bounds.X] = tmpl.TerrainMap[ty, tx];
							}
						}
					}
				}
				if (mi is Wall) {
					int tx = (int)mi.tx - Bounds.X;
					int ty = (int)mi.ty - Bounds.Y;
					if (tx < 0 || tx >= Bounds.Width)
						continue;
					if (ty < 0 || ty >= Bounds.Height)
						continue;
					aterMap[ty, tx] = TerrainTypes.Blocked;
				}
			}
			return aterMap;
		}

		public TerrainColors[,] GetTerrainColorsMap(Size sizTile, TemplateDoc tmpd) {
			TerrainColors[,] atclrMap = new TerrainColors[Bounds.Height * 2, Bounds.Width * 2];
			foreach (IMapItem mi in m_alsmi) {
				Tile tile = mi as Tile;
				if (tile == null)
					continue;
				int x = (int)mi.tx * sizTile.Width;
				int y = (int)mi.ty * sizTile.Height;
				Rectangle rc = tile.GetBoundingRectAt(x, y, sizTile, m_tmpd);
				rc.Width /= sizTile.Width;
				rc.Height /= sizTile.Height;
				rc.X /= sizTile.Width;
				rc.Y /= sizTile.Height;
				rc.Intersect(Bounds);

				for (int ty = rc.Y - (int)tile.ty; ty < rc.Bottom - (int)tile.ty; ty++) {
					for (int tx = rc.X - (int)tile.tx; tx < rc.Right - (int)tile.tx; tx++) {
						if (tile.Visibility == null || tile.Visibility[ty, tx]) {
							Template tmpl = tile.GetTemplate(m_tmpd);

							// Maybe this template doesn't exist in document's current tile collection

							if (tmpl == null)
								continue;

							// TerrainColors are a 2x2 subgrid inside each tile

							for (int tyT = 0; tyT < 2; tyT++) {
								for (int txT = 0; txT < 2; txT++) {
									TerrainColors tclr = TerrainColors.Grass;
									if (tmpl.TerrainColors != null)
										tclr = tmpl.TerrainColors[ty * 2 + tyT, tx * 2 + txT];
									atclrMap[(ty + (int)tile.ty - Bounds.Y) * 2 + tyT, (tx + (int)tile.tx - Bounds.X) * 2 + txT] = tclr;
								}
							}
						}
					}
				}
			}
			return atclrMap;
		}

		//
		// Level validation stuff
		// 

		public enum ValidateError {
			Info,
			Warning,
			Error
		}

		public delegate void ValidateErrorDelegate(LevelDoc lvld, ValidateError ve, int tx, int ty, object ob, string str);

#if false
		public int Validate(ValidateErrorDelegate dgt) {
			return 0;
		}
#else
		enum ItemMask { None = 0, Galaxite = 1, Wall = 2, Unreachable = 4, Scenery = 8, Structure = 16, MobileUnit = 32 };
		
		public int Validate(ValidateErrorDelegate dgt) {
			int cError = 0;

			// Validate switch count

			int cSwitchesMax = 16;
			if (m_swm.Items.Count > cSwitchesMax) {
				dgt(this, ValidateError.Error, 0, 0, null, String.Format("Max switches is {0}, this mission has {1}", cSwitchesMax, m_swm.Items.Count));
				cError++;
			}

			// Validate area count

			int cAreas = 0;
			foreach (IMapItem mi in m_alsmi) {
				if (mi is Area)
					cAreas++;
			}
			int cAreasMax = 32;
			if (cAreas > cAreasMax) {
				dgt(this, ValidateError.Error, 0, 0, null, String.Format("Max areas is {0}, this mission has {1}", cAreasMax, cAreas));
				cError++;
			}

			// Validate SideInfo

			int cHuman = 0;
			foreach (SideInfo sidi in m_alsidi) {
				if (sidi.Intelligence == Intelligence.Human)
					cHuman++;

#if false
				if (sidi.InitialCredits == 0)
					dgt(this, ValidateError.Warning, 0, 0, sidi, String.Format("Side {0} starts with no credits", sidi.Side));
#endif
			}


#if false
			// Don't allow multi-player yet

			if (MaxPlayers != 1) {
				dgt(this, ValidateError.Error, 0, 0, null, "Multi-player not supported yet: MaxPlayers should be 1");
				cError++;
			}

			// Must have 1 human side

			if (cHuman != 1) {
				dgt(this, ValidateError.Error, 0, 0, null, "Must have 1 human side!");
				cError++;
			}
#endif

			if (cHuman > MaxPlayers) {
				dgt(this, ValidateError.Error, 0, 0, null, String.Format("MaxPlayers is {0}, # of human sides is {1}", MaxPlayers, cHuman));
				cError++;
			}

			// Validate gob inside/outside of boundaries

			foreach (IMapItem mi in m_alsmi) {
				if (mi is Galaxite)
					continue;
				if (mi is Wall)
					continue;
				if (mi is Tile)
					continue;

				// Gobs can be inside or outside of boundaries. If gobs are intersected by the boundaries
				// that is not ok and will send the game into fits

				Rectangle rcT = new Rectangle(Bounds.Location, Bounds.Size);
				if (!Bounds.Contains(new Rectangle((int)mi.tx, (int)mi.ty, mi.ctx, mi.cty))) {
					if (mi is Area) {
						Area area = (Area)mi;
						dgt(this, ValidateError.Error, (int)mi.tx, (int)mi.ty, area, String.Format("Area '{0}' out of bounds", area.Name));
						cError++;
					} else {
						// If we have a gob that is "partially" outside, and gets compiled into the game,
						// errors will occur at runtime, so make this an error

						dgt(this, ValidateError.Error, (int)mi.tx, (int)mi.ty, mi, mi.ToString() + " out of bounds");
						cError++;
					}
				}
			}

			// Collect placement information. 

			// enum ItemMask { None = 0, Galaxite = 1, Wall = 2, Unreachable = 4, Scenery = 8, Structure = 16, MobileUnit = 32 };

			// Initialize with all "unreachable areas" appropriately

			TerrainTypes[,] aterMap = GetTerrainMap(m_tmpd.TileSize, m_tmpd, false);
			ItemMask[,] aimMap = new ItemMask[Bounds.Height, Bounds.Width];
			for (int ty = 0; ty < Bounds.Height; ty++) {
				for (int tx = 0; tx < Bounds.Width; tx++) {
					switch (aterMap[ty, tx]) {
					case TerrainTypes.Blocked:
					case TerrainTypes.Wall:
						aimMap[ty, tx] |= ItemMask.Unreachable;
						break;
					}
				}
			}

			// Validate placement

			foreach (IMapItem mi in m_alsmi) {
				ItemMask im = ItemMask.None;
				ItemMask imInvalid = ItemMask.None;
				if (mi is Galaxite) {
					im = ItemMask.Galaxite;
					imInvalid = ItemMask.Wall | ItemMask.Unreachable | ItemMask.Structure;
				} else if (mi is Wall) {
					im = ItemMask.Wall;
					imInvalid = ItemMask.Galaxite | ItemMask.Wall | ItemMask.Structure | ItemMask.MobileUnit;
				} else if (mi is Scenery) {
					im = ItemMask.Scenery;
					imInvalid = ItemMask.None;
				} else if (mi is Structure) {
					im = ItemMask.Structure;
					imInvalid = ItemMask.Galaxite | ItemMask.Wall | ItemMask.Unreachable | ItemMask.Structure | ItemMask.MobileUnit;
				} else if (mi is MobileUnit) {
					im = ItemMask.MobileUnit;
					imInvalid = ItemMask.Wall | ItemMask.Unreachable | ItemMask.Structure | ItemMask.MobileUnit;
				}
				if (im == ItemMask.None)
					continue;

				// Check each tile occupied by this mi

				ItemMask imError = ItemMask.None;
				for (int ty = (int)mi.ty; ty < (int)mi.ty + mi.cty; ty++) {
					for (int tx = (int)mi.tx; tx < (int)mi.tx + mi.ctx; tx++) {
						int txT = tx - Bounds.Left;
						int tyT = ty - Bounds.Top;
						if (txT < 0 || tyT < 0)
							continue;
						if (txT >= Bounds.Width || tyT >= Bounds.Height)
							continue;
						ItemMask imInvalidOverlap = aimMap[tyT, txT] & imInvalid;
						aimMap[tyT, txT] |= im;
						ItemMask imErrorNew = (ItemMask)(imInvalidOverlap & ~imError);
						if (imErrorNew != ItemMask.None) {
							imError |= imErrorNew;

							// Build up the error string

							ItemMask[] aimValues = (ItemMask[])Enum.GetValues(typeof(ItemMask));
							string strT = "";
							for (int n = 0; n < aimValues.Length; n++) {
								if ((aimValues[n] & imErrorNew) != 0)
									strT += aimValues[n].ToString() + ",";
							}
							if (strT != "")
								strT = strT.Substring(0, strT.Length - 1);

							dgt(this, ValidateError.Error, (int)mi.tx, (int)mi.ty, mi, mi.ToString() + " is on top of: " + strT);
							cError++;
						}
					}
				}
			}

			// Validate that structures don't make terrain inaccessible
			// Everywhere there is a blocked section in aterMapStructs that isn't
			// blocked in aterMap and isn't a structure is now inaccessible because
			// of a structure block

			TerrainTypes[,] aterMapStructs = GetTerrainMap(m_tmpd.TileSize, m_tmpd, true);
			for (int ty = 0; ty < aterMap.GetLength(0); ty++) {
				for (int tx = 0; tx < aterMap.GetLength(1); tx++) {
					// Check

					bool fSrcOpen = (aimMap[ty, tx] & (ItemMask.Unreachable | ItemMask.Structure)) == 0;
					if (fSrcOpen && aterMapStructs[ty, tx] == TerrainTypes.Blocked) {
						dgt(this, ValidateError.Error, tx + Bounds.Left, ty + Bounds.Top, null, String.Format("Terrain at {0},{1} is inaccessible due to structure blockage", tx + Bounds.Left, ty + Bounds.Top));
						cError++;
					}
				}
			}

			// Validate areas in triggers exist

			StringCollection strc = CaTypeArea.GetAreaNames();
			foreach (Side side in Enum.GetValues(typeof(Side))) {
				Trigger[] atgr = m_tgrm.GetTriggerList(side);
				foreach (Trigger tgr in atgr) {
					foreach (CaBase cab in tgr.Conditions) {
						foreach (CaType cat in cab.GetTypes()) {
							if (cat is CaTypeArea) {
								CaTypeArea catArea = (CaTypeArea)cat;
								if (strc.IndexOf(catArea.Area) < 0) {
									dgt(this, ValidateError.Error, 0, 0, null, "Area " + catArea.Area + " in trigger " + tgr.ToString() + " doesn't exist!");
									cError++;
								}
							}
						}
					}
					foreach (CaBase cab in tgr.Actions) {
						foreach (CaType cat in cab.GetTypes()) {
							if (cat is CaTypeArea) {
								CaTypeArea catArea = (CaTypeArea)cat;
								if (strc.IndexOf(catArea.Area) < 0) {
									dgt(this, ValidateError.Error, 0, 0, null, "Area " + catArea.Area + " in trigger " + tgr.ToString() + " doesn't exist!");
									cError++;
								}
							}
						}
					}
				}
			}

			// Validate unit groups in triggers

			foreach (Side side in Enum.GetValues(typeof(Side))) {
				Trigger[] atgr = m_tgrm.GetTriggerList(side);
				foreach (Trigger tgr in atgr) {
					foreach (CaBase cab in tgr.Conditions) {
						foreach (CaType cat in cab.GetTypes()) {
							if (cat is CaTypeUnitGroup) {
								CaTypeUnitGroup catUg = (CaTypeUnitGroup)cat;
								if (Array.IndexOf(atgr, tgr) < 0) {
									dgt(this, ValidateError.Error, 0, 0, null, "Orphaned UnitGroup in trigger " + tgr.ToString());
									cError++;
								}
							}
						}
					}
					foreach (CaBase cab in tgr.Actions) {
						foreach (CaType cat in cab.GetTypes()) {
							if (cat is CaTypeArea) {
								CaTypeArea catArea = (CaTypeArea)cat;
								if (strc.IndexOf(catArea.Area) < 0) {
									dgt(this, ValidateError.Error, 0, 0, null, "Orphaned UnitGroup in trigger " + tgr.ToString());
									cError++;
								}
							}
						}
					}
				}
			}

			// Validate triggers per side limit

			int cTriggersPerSideMax = 128;
			foreach (Side side in Enum.GetValues(typeof(Side))) {
				Trigger[] atgr = m_tgrm.GetTriggerList(side);
				if (atgr == null)
					continue;
				if (atgr.Length > cTriggersPerSideMax) {
					string strT = String.Format("Triggers per side {0}. Side {1} has {2} triggers", cTriggersPerSideMax, side.ToString(), atgr.Length);
					dgt(this, ValidateError.Error, 0, 0, null, strT);
					cError++;
				}
			}


			// Validate gob limits

			int[] acStructures = new int[Enum.GetNames(typeof(Side)).Length];
			int[] acMunts = new int[Enum.GetNames(typeof(Side)).Length];
			int cScenery = 0;
			foreach (IMapItem mi in m_alsmi) {
				if (mi is Unit) {
					Unit unit = (Unit)mi;
					if (unit is Structure) {
						acStructures[(int)unit.Side]++;
					} else {
						acMunts[(int)unit.Side]++;
					}
					continue;
				}
				if (mi is Scenery) {
					cScenery++;
					continue;
				}
			}

			// Scenery Limit

			int cSceneryMax = 100;
			if (cScenery > cSceneryMax) {
				string strT = String.Format("{0} scenery; {1} allowed", cScenery, cSceneryMax);
				dgt(this, ValidateError.Error, 0, 0, null, strT);
				cError++;
			}

			// Unit counts

			if (MaxPlayers == 1) {
				// Single player - asymmetric: one human count, shared computer counts
				// #define kcStructGobsHumanMin 39
				// #define kcStructGobsComputerMin 52
				// #define kcMuntGobsHumanMin 60
				// #define kcMuntGobsComputerMin 80

				int cStructsHumanMax = 39;
				int cMuntsHumanMax = 60;
				int cStructsComputerMax = 52;
				int cMuntsComputerMax = 80;

				int cStructsHuman = 0;
				int cMuntsHuman = 0;
				int cStructsComputer = 0;
				int cMuntsComputer = 0;

				foreach (SideInfo sidi in m_alsidi) {
					if (sidi.Intelligence == Intelligence.Human) {
						cStructsHuman += acStructures[(int)sidi.Side];
						cMuntsHuman += acMunts[(int)sidi.Side];
					} else {
						cStructsComputer += acStructures[(int)sidi.Side];
						cMuntsComputer += acMunts[(int)sidi.Side];
					}
				}

				// HACK ALERT: Sometimes levels have more human structures than the human limit. In this
				// case, take some from the computer side if possible. Hack: Reserve 5 structs for computer building

				int cStructuresAvailable = cStructsComputerMax - cStructsComputer - 5;
				if (cStructuresAvailable < 0)
					cStructuresAvailable = 0;
				cStructsHumanMax += cStructuresAvailable;
				cStructsComputerMax -= cStructuresAvailable;

				// Check

				if (cStructsHuman > cStructsHumanMax) {
					string strT = String.Format("Human Side has {0} structures; {1} allowed", cStructsHuman, cStructsHumanMax);
					dgt(this, ValidateError.Error, 0, 0, null, strT);
					cError++;
				}
				if (cMuntsHuman > cMuntsHumanMax) {
					string strT = String.Format("Human Side has {0} mobile units; {1} allowed", cMuntsHuman, cMuntsHumanMax);
					dgt(this, ValidateError.Error, 0, 0, null, strT);
					cError++;
				}
				if (cStructsComputer > cStructsComputerMax) {
					string strT = String.Format("Computer Side has {0} structures; {1} allowed", cStructsComputer, cStructsComputerMax);
					dgt(this, ValidateError.Error, 0, 0, null, strT);
					cError++;
				}
				if (cMuntsComputer > cMuntsComputerMax) {
					string strT = String.Format("Computer Side has {0} mobile units; {1} allowed", cMuntsComputer, cMuntsComputerMax);
					dgt(this, ValidateError.Error, 0, 0, null, strT);
					cError++;
				}
			} else {
				// Multi-player - symmetric: same counts for each side
				// #define kcStructGobsMax 55
				// #define kcMuntGobsMax 88

				int cStructsMax = 55;
				int cMuntsMax = 88;

				foreach (Side side in Enum.GetValues(typeof(Side))) {
					if (acStructures[(int)side] > cStructsMax) {
						string strT = String.Format("Side {0} has {1} structures; {2} allowed", side.ToString(), acStructures[(int)side], cStructsMax);
						dgt(this, ValidateError.Error, 0, 0, null, strT);
						cError++;
					}
					if (acMunts[(int)side] > cMuntsMax) {
						string strT = String.Format("Side {0} has {1} mobile units; {2} allowed", side.ToString(), acMunts[(int)side], cMuntsMax);
						dgt(this, ValidateError.Error, 0, 0, null, strT);
						cError++;
					}
				}
			}

			// UNDONE: Validate legal multiplayer triggers

			// UNDONE: Validate computer sides have enough power

			// UNDONE: Validate computer sides have a surveillance center if they have towers

			// UNDONE: info -- total credits value of on-map Galaxite

			// UNDONE: info -- power supply/demand for each side

			return cError;
		}
#endif

		public void SaveIni(string strFile, int nVersion, string strFileTmap, string strFileTrmap) {
			FileStream stm = new FileStream(strFile, FileMode.Create);
			SaveIni(stm, nVersion, strFileTmap, strFileTrmap, false);
			stm.Close();
		}

		public void SaveIni(Stream stm, int nVersion, string strFileTmap, string strFileTrmap, bool fDemoCheckTrigger) {
			Ini ini = new Ini();
			Ini.Section sec;

			// [Intro]
			sec = new Ini.Section("Intro");
			sec.Add(new Ini.Property("String", "This is a test level!"));
			ini.Add(sec);

			// [Side1-n]
			int txOrigin = Bounds.X;
			int tyOrigin = Bounds.Y;

			// Hack - there should be a real "neutral" side
			ArrayList alsidiT = (ArrayList)m_alsidi.Clone();
			SideInfo sidiNeutral = new SideInfo(Side.sideNeutral);
			sidiNeutral.Intelligence = Intelligence.ComputerNeutral;
			sidiNeutral.InitialCredits = 0;
			sidiNeutral.InitialView = new Point(0, 0);
			alsidiT.Add(sidiNeutral);

			foreach (SideInfo sidi in alsidiT) {
				sec = new Ini.Section(sidi.Side.ToString());
				sec.Add(new Ini.Property("InitialView", String.Format("{0},{1}", 
					sidi.InitialView.X - txOrigin, sidi.InitialView.Y - tyOrigin)));
				sec.Add(new Ini.Property("InitialCredits", sidi.InitialCredits.ToString()));
				sec.Add(new Ini.Property("Intelligence", "knIntelligence" + sidi.Intelligence.ToString()));

				// How many units for this side?

				int cStructures = 0;
				int cMobileUnits = 0;
				foreach (IMapItem mi in m_alsmi) {
					if (mi is Unit) {
						Unit unt = (Unit)mi;
						if (unt.Side == sidi.Side) {
							if (mi is MobileUnit) {
								cMobileUnits++;
							}
							if (mi is Structure) {
								cStructures++;
							}
						}
					}
				}
				sec.Add(new Ini.Property("InitialMobileUnitCount", cMobileUnits.ToString()));
				sec.Add(new Ini.Property("InitialStructureCount", cStructures.ToString()));
				ini.Add(sec);
			}

			// [GameObjects]
			sec = new Ini.Section("GameObjects");
			foreach (IMapItem mi in m_alsmi) {
				if (mi is Galaxite)
					continue;
				if (mi is Area)
					continue;
				if (mi is Wall)
					continue;
				if (mi is Tile)
					continue;

				Ini.Property prop = mi.GetIniProperty(txOrigin, tyOrigin);
				if (prop == null)
					continue;

				// Skip Gobs that are out of bounds
				// UNDONE: can't do the right thing to make sure Gob's right/bottom
				// edges aren't out of bounds because M doesn't know the true
				// width and height of Gobs.

				if (!Bounds.Contains(new Rectangle((int)mi.tx, (int)mi.ty, mi.ctx, mi.cty))) {
					Console.WriteLine("{0} out of bounds", mi);
					continue;
				}
				sec.Add(prop);
			}
			ini.Add(sec);

			// [Galaxite]
			sec = new Ini.Section("Galaxite");
			foreach (IMapItem mi in m_alsmi) {
				if (!(mi is Galaxite))
					continue;

				Ini.Property prop = mi.GetIniProperty(txOrigin, tyOrigin);
				if (prop == null)
					continue;

				// Skip Galaxite that is out of bounds

				if (!Bounds.Contains((int)mi.tx, (int)mi.ty)) {
					Console.WriteLine("{0} out of bounds", mi);
					continue;
				}

				sec.Add(prop);
			}
			ini.Add(sec);

#if false
// In terrain now
			// [Walls]
			sec = new Ini.Section("Walls");
			foreach (IMapItem mi in m_alsmi) {
				if (!(mi is Wall))
					continue;

				Ini.Property prop = mi.GetIniProperty(txOrigin, tyOrigin);
				if (prop == null)
					continue;

				// Skip Walls that are out of bounds

				if (!Bounds.Contains((int)mi.tx, (int)mi.ty)) {
					Console.WriteLine("{0} out of bounds", mi);
					continue;
				}

				sec.Add(prop);
			}
			ini.Add(sec);
#endif

			// [Areas]
			ArrayList alT = new ArrayList();
			foreach (IMapItem mi in m_alsmi) {
				if (!(mi is Area))
					continue;
				alT.Add(mi);
			}
			alT.Sort();

			sec = new Ini.Section("Areas");
			foreach (IMapItem mi in alT) {
				Ini.Property prop = mi.GetIniProperty(txOrigin, tyOrigin);
				if (prop == null)
					continue;

				Area area = (Area)mi;
				if (!Bounds.Contains(new Rectangle((int)mi.tx, (int)mi.ty, mi.ctx, mi.cty))) {
					MessageBox.Show(String.Format("The area \"{0}\" lies outside of the map's bounds", area.Name), "Error Compiling Level");
				}

				sec.Add(prop);
			}
			ini.Add(sec);

			// [Triggers]
			// NOTE: Triggers must be written before UnitGroups because some trigger actions
			// e.g., CreateUnitAtArea will dynamically create UnitGroups and add them to the UnitGroup list
			ini.Add(m_tgrm.GetIniSection(fDemoCheckTrigger));

			// [UnitGroup 0-n]
			m_ugm.SaveIni(ini);

			// [Switches]
			sec = new Ini.Section("Switches");
			foreach (Switch sw in SwitchManager.Items)
				sec.Add(new Ini.Property(sw.Name, ""));
			ini.Add(sec);

			// [General]
			// This section is written last in case any of the values are modified by
			// the process of writing out the prior sections (e.g., CreateUnitAtArea actions add UnitGroups)
			sec = new Ini.Section("General", null);
			sec.Add(new Ini.Property("Title", Title));
			sec.Add(new Ini.Property("TileMap", strFileTmap));
			sec.Add(new Ini.Property("TerrainMap", strFileTrmap));
			// sec.Add(new Ini.Property("Palette", strFilePalette));
			sec.Add(new Ini.Property("MinPlayers", m_nPlayersMin.ToString()));
			sec.Add(new Ini.Property("MaxPlayers", m_nPlayersMax.ToString()));
			sec.Add(new Ini.Property("UnitGroupCount", m_ugm.Items.Count.ToString()));

			// < 0 means use the current version, otherwise use the passed version
			// This is the "level file format" version

			if (nVersion < 0)
				nVersion = 1;
			sec.Add(new Ini.Property("Version", nVersion.ToString()));

			// Add a random number for the revision #. This # is used to determine if saved games are
			// based on older versions of a mission.

			if (nVersion > 0) {
				Random rand = new Random();
				uint dwRevision = (uint)rand.Next();
				sec.Add(new Ini.Property("Revision", dwRevision.ToString()));
			}

			ini.Add(sec);

			// Done

			ini.Save(stm);

			// Mostly Done.
			// Clean out the "__cuaa" unit groups created by CreateUnitAtAreaTriggerAction

			ArrayList alsRemove = new ArrayList();
			UnitGroup[] aug = (UnitGroup[])m_ugm.Items.ToArray(typeof(UnitGroup));
			for (int i = 0; i < m_ugm.Items.Count; i++) {
				if (((UnitGroup)m_ugm.Items[i]).Name.StartsWith("__cuaa")) {
					alsRemove.Add(m_ugm.Items[i]);
				}
			}
			foreach (UnitGroup ug in alsRemove) {
				m_ugm.RemoveUnitGroup(ug);
			}
		}

		IMapItem CreateGameObject(string strSecName, string strName, string strValue) {
			if (strSecName == "Galaxite") {
				return new Galaxite(strName, strValue, Bounds.Left, Bounds.Top);
			} else if (strSecName == "Areas") {
				return new Area(strName, strValue, Bounds.Left, Bounds.Top);
			} else if (strSecName == "GameObjects") {
				int gt = int.Parse(strValue.Split(',')[0]);
				switch (gt) {
				case 1: // kgtShortRangeInfantry
					return new ShortRangeInfantry(strName, strValue, Bounds.Left, Bounds.Top);

				case 2: // kgtLongRangeInfantry
					return new LongRangeInfantry(strName, strValue, Bounds.Left, Bounds.Top);

				case 3: // kgtHumanResourceCenter
					return new HumanResourceCenter(strName, strValue, Bounds.Left, Bounds.Top);

				case 5: // kgtScenery
					return new Scenery(strName, strValue, Bounds.Left, Bounds.Top);

				case 7: // kgtReactor
					return new Reactor(strName, strValue, Bounds.Left, Bounds.Top);

				case 8: // kgtProcessor
					return new Processor(strName, strValue, Bounds.Left, Bounds.Top);

				case 11: // kgtGalaxMiner
					return new GalaxMiner(strName, strValue, Bounds.Left, Bounds.Top);

				case 12: // kgtHeadquarters
					return new Headquarters(strName, strValue, Bounds.Left, Bounds.Top);

				case 13: // kgtResearchCenter
					return new ResearchCenter(strName, strValue, Bounds.Left, Bounds.Top);

				case 14: // kgtVehicleTransportStation
					return new VehicleTransportStation(strName, strValue, Bounds.Left, Bounds.Top);

				case 15: // kgtRadar
					return new Radar(strName, strValue, Bounds.Left, Bounds.Top);

				case 16: // kgtLightTank
					return new LightTank(strName, strValue, Bounds.Left, Bounds.Top);

				case 17: // kgtMediumTank
					return new MediumTank(strName, strValue, Bounds.Left, Bounds.Top);

				case 18: // kgtMachineGunVehicle
					return new MachineGunVehicle(strName, strValue, Bounds.Left, Bounds.Top);

				case 19: // kgtRocketVehicle
					return new RocketVehicle(strName, strValue, Bounds.Left, Bounds.Top);

				case 20: // kgtTakeoverSpecialist
					return new TakeoverSpecialist(strName, strValue, Bounds.Left, Bounds.Top);

				case 21: // kgtWarehouse
					return new Warehouse(strName, strValue, Bounds.Left, Bounds.Top);

				case 22: // kgtMobileHeadquarters
					return new MobileHeadquarters(strName, strValue, Bounds.Left, Bounds.Top);

				case 26: // kgtMachineGunTower
					return new MachineGunTower(strName, strValue, Bounds.Left, Bounds.Top);

				case 27: // kgtRocketTower
					return new RocketTower(strName, strValue, Bounds.Left, Bounds.Top);

				case 32: // kgtArtillery
					return new Artillery(strName, strValue, Bounds.Left, Bounds.Top);

				case 34: // kgtAndy
					return new Andy(strName, strValue, Bounds.Left, Bounds.Top);

				case 35: // kgtReplicator
					return new Replicator(strName, strValue, Bounds.Left, Bounds.Top);

				case 36: // kgtActivator
					return new Activator(strName, strValue, Bounds.Left, Bounds.Top);

				case 37: // kgtFox
					return new Fox(strName, strValue, Bounds.Left, Bounds.Top);

				//case 4: // kgtSurfaceDecal
				//case 6: // kgtAnimation:
				//case 9: // kgtStructure
				//case 10: // kgtUnit
				//case 23: // kgtOvermind
				//case 24: // kgtTankShot
				//case 25: // kgtRocket
				//case 28: // kgtScorch
				//case 29: // kgtSmoke
				//case 30: // kgtPuff
				//case 31: // kgtBullet
				//case 33: // kgtArtilleryShot
				//case 38: // kgtAndyShot
				default:
					return null;
				}
			}
			return null;
		}

		public void LoadIni(Ini ini) {
			// General

			Ini.Section secGen = ini["General"];
			m_strTitle = secGen["Title"].Value;
			m_nPlayersMin = int.Parse(secGen["MinPlayers"].Value);
			m_nPlayersMax = int.Parse(secGen["MaxPlayers"].Value);

			// SideInfo

			for (Side side = Side.side1; side <= Side.side4; side++) {
				Ini.Section sec = ini[side.ToString()];
				if (sec == null) {
					continue;
				}
				SideInfo sidi = new SideInfo(side);
				string s = sec["InitialView"].Value;
				Regex re = new Regex(@"^(?<tx>(-)?\d+),(?<ty>(-)?\d+)$");
				Match m = re.Match(s);
				int tx = int.Parse(m.Groups["tx"].Value);
				int ty = int.Parse(m.Groups["ty"].Value);
				Point ptInitialView = new Point(tx + Bounds.Left, ty + Bounds.Top);
				if (ptInitialView.X < Bounds.Left) {
					ptInitialView.X = Bounds.Left;
				}
				if (ptInitialView.Y < Bounds.Top) {
					ptInitialView.Y = Bounds.Top;
				}
				sidi.InitialView = ptInitialView;
				sidi.InitialCredits = int.Parse(sec["InitialCredits"].Value);
				sidi.Intelligence = (Intelligence)int.Parse(sec["Intelligence"].Value);
				m_alsidi.Add(sidi);
			}

			// Misc MapItems. Areas must come before GameObjects because GameObjects
			// can refer to Areas by index in UnitActions.

			string[] secNames = { "Galaxite", "Areas", "GameObjects" };
			foreach (string secName in secNames) {
				Ini.Section sec = ini[secName];
				ArrayList alsMapItems = new ArrayList();
				foreach (Ini.Property prop in sec) {
					IMapItem mi = CreateGameObject(secName, prop.Name, prop.Value);
					if (mi != null) {
						alsMapItems.Add(mi);
					}
				}
				// Sometimes Areas are given the same name. Then on export the sort order
				// doesn't match the original, which is a problem since in the game, area
				// creation order is important.

				if (secName == "Areas") {
					for (int i = 0; i < alsMapItems.Count; i++) {
						Area ar = (Area)alsMapItems[i];
						ar.BonusSortKey = i;
					}
				}
				AddMapItems((IMapItem[])alsMapItems.ToArray(typeof(IMapItem)));
			}

			// Switches (must be before triggers).

			foreach (Ini.Property prop in ini["Switches"]) {
				m_swm.AddSwitch(new Switch(prop.Name));
			}

			// UnitGroups (must be before triggers).

			m_ugm.LoadIni(ini);

			// Triggers

			m_tgrm.LoadIni(ini);

			// Clear out __cuaa UnitGroups now that triggers have been loaded
			// (CreateUnitAtAreaTrigger loads state from __cuaa triggers).

			ArrayList alsRemove = new ArrayList();
			for (int i = 0; i < m_ugm.Items.Count; i++) {
				UnitGroup ug = (UnitGroup)m_ugm.Items[i];
				if (ug.Name.StartsWith("__cuaa")) {
					alsRemove.Add(ug);
				}
			}
			foreach (UnitGroup ug in alsRemove) {
				m_ugm.RemoveUnitGroup(ug);
			}

			// TODO: remove the demo check trigger
		}

		public void EditTriggers() {
			TriggersForm frm = new TriggersForm(m_tgrm);
			m_tgrm.ClearModified();
			frm.ShowDialog();
			if (m_tgrm.IsModified())
				SetModified(true);
		}

		public void EditUnitGroups() {
			UnitGroupsForm frm = new UnitGroupsForm(this, m_ugm);
#if false
			frm.Show();
#else
			frm.ShowDialog();
#endif
		}

		public void EditComments() {
			EditCommentsForm frm = new EditCommentsForm(m_strComment);
			if (frm.ShowDialog() == DialogResult.OK) {
				m_strComment = frm.textBox1.Text;
				SetModified(true);
			}
		}

		public void EditLevelText() {
			new EditLevelTextForm(this).ShowDialog();
		}

		public string GetLevelText() {
			StringBuilder strb = new StringBuilder();
			ArrayList alsTriggers = m_tgrm.Triggers;
			foreach (Trigger tgr in alsTriggers) {
				ArrayList alsActions = tgr.Actions;
				foreach (CaBase cab in alsActions) {
					CaType[] acat = cab.GetTypes();
					foreach (CaType cat in acat) {
						if (cat is CaTypeText || cat is CaTypeRichText) {
							int cch = "TriggerAction".Length;
							string strAction = cab.GetType().Name;
							strAction = strAction.Remove(strAction.Length - cch, cch);
//							string strCaType = cat.GetType().Name;
//							strCaType = strCaType.Substring("CaType".Length);

							if (strAction == "Ecom") {
								string str = " ($1 w/ $2) from $3 to $4";
								for (int j = 0; j < acat.Length - 1; j++)
									str = str.Replace("$" + (j + 1), acat[j].ToString());
								strAction += str;
							}

							if (strb.Length != 0)
								strb.Append('\n');
							strb.AppendFormat("[{0}]\n", strAction);
							strb.Append(cat.ToString());
						}
					}
				}
			}

			return strb.ToString();
		}

		public bool SetLevelText(string strLevelText, out int ichErrorPos) {
			ichErrorPos = 0;

			// Validation pass

			StringReader strr;

			strr = new StringReader(strLevelText);

			ArrayList alsTriggers = m_tgrm.Triggers;
			foreach (Trigger tgr in alsTriggers) {
				ArrayList alsActions = tgr.Actions;
				foreach (CaBase cab in alsActions) {
					CaType[] acat = cab.GetTypes();
					foreach (CaType cat in acat) {
						if (cat is CaTypeText || cat is CaTypeRichText) {
							int cch = "TriggerAction".Length;
							string strAction = cab.GetType().Name;
							strAction = strAction.Remove(strAction.Length - cch, cch);
//							string strCaType = cat.GetType().Name;
//							strCaType = strCaType.Substring("CaType".Length);

							if (strAction == "Ecom") {
								string str = " ($1 w/ $2) from $3 to $4";
								for (int j = 0; j < acat.Length - 1; j++)
									str = str.Replace("$" + (j + 1), acat[j].ToString());
								strAction += str;
							}

							string strType = String.Format("[{0}]", strAction);
							string strT = strr.ReadLine();

							if (strType != strT) {
								MessageBox.Show(String.Format("Expected \"{0}\" but found \"{1}\"", strType, strT), "Error");
								return false;
							}

							ichErrorPos += strT.Length + 1; // for \n

							while (strr.Peek() != '[' && strr.Peek() != -1) {
								strT = strr.ReadLine();
								ichErrorPos += strT.Length + 1; // for \n
							}
						}
					}
				}
			}

			strr = new StringReader(strLevelText);

			alsTriggers = m_tgrm.Triggers;
			foreach (Trigger tgr in alsTriggers) {
				ArrayList alsActions = tgr.Actions;
				foreach (CaBase cab in alsActions) {
					CaType[] acat = cab.GetTypes();
					foreach (CaType cat in acat) {
						if (cat is CaTypeText || cat is CaTypeRichText) {
							int cch = "TriggerAction".Length;
							string strAction = cab.GetType().Name;
							strAction = strAction.Remove(strAction.Length - cch, cch);
//							string strCaType = cat.GetType().Name;
//							strCaType = strCaType.Substring("CaType".Length);

							string strType = String.Format("[{0}]", strAction);
							string strT = strr.ReadLine();

							StringBuilder strb = new StringBuilder();
							while (strr.Peek() != '[' && strr.Peek() != -1) {
								strT = strr.ReadLine();
								strb.Append(strT);
								if (strr.Peek() != '[' && strr.Peek() != -1) 
									strb.Append('\n');
							}

							string strOld = cat.ToString();
							string strNew = strb.ToString();

							if (strOld != strNew) {
								if (cat is CaTypeText) {
									((CaTypeText)cat).Text = strNew;
								} else if (cat is CaTypeRichText) {
									((CaTypeRichText)cat).Text = strNew;
								}
							}
						}
					}
				}
			}

			SetModified(true);
			return true;
		}

		[BrowsableAttribute(false)]
		public UnitGroupManager UnitGroupManager {
			get {
				return m_ugm;
			}
		}

		[BrowsableAttribute(false)]
		public SwitchManager SwitchManager {
			get {
				return m_swm;
			}
		}

		[BrowsableAttribute(false)]
		public CounterManager CounterManager {
			get {
				return m_ctrm;
			}
		}

		[BrowsableAttribute(false)]
		public IMapItem[] MapItems {
			get {
				return (IMapItem[])m_alsmi.ToArray(typeof(IMapItem));
			}
		}

		public string Title {
			get {
				return m_strTitle;
			}
			set {
				if (m_strTitle == value)
					return;
				m_strTitle = value;
				SetModified(true);
				if (NameChanged != null)
					NameChanged(this);
				m_strOutputFilename = null;
			}
		}

		[BrowsableAttribute(false)]
		public int Width {
			get {
				return m_ctx;
			}
			set {
				m_ctx = value;
				if (ImageChanged != null)
					ImageChanged();
			}
		}

		[BrowsableAttribute(false)]
		public int Height {
			get {
				return m_cty;
			}
			set {
				m_cty = value;
				if (ImageChanged != null)
					ImageChanged();
			}
		}

		public Rectangle Bounds {
			get {
				return m_trcBounds;
			}
			set {
				if (value.Left < 0 || value.Left >= m_ctx || value.Right < 0 || value.Right >= m_ctx ||
						value.Top < 0 || value.Top >= m_cty || value.Bottom < 0 || value.Bottom >= m_cty) {
					return;
				}
				m_trcBounds = value;
				m_fUpdateDirty = true;
				SetModified(true);
			}
		}

		public int MinPlayers {
			get {
				return m_nPlayersMin;
			}
			set {
				if (m_nPlayersMin == value)
					return;
				m_nPlayersMin = value;
				SetModified(true);
			}
		}

		public int MaxPlayers {
			get {
				return m_nPlayersMax;
			}
			set {
				if (m_nPlayersMax == value)
					return;
				m_nPlayersMax = value;
				SetModified(true);
			}
		}

		public ArrayList Selection {
			get {
				return (ArrayList)m_alsmiSelected.Clone();
			}
			set {
				m_alsmiSelected = value;
				m_fUpdateDirty = true;
			}
		}
	}

	public enum Intelligence {
		Human,
		Computer,
		ComputerOvermind,
		ComputerNeutral,
	}

	[Serializable]
	public class SideInfo : ISerializable {
		private Side m_side;
		private int m_nInitialCredits;
		private Point m_ptInitialView;
		private Intelligence m_nIntelligence = Intelligence.Computer;

		protected SideInfo(SerializationInfo info, StreamingContext ctx) {
			m_side = (Side)info.GetValue("m_side", typeof(Side)); 
			m_nInitialCredits = info.GetInt32("m_nInitialCredits");
			m_ptInitialView = (Point)info.GetValue("m_ptInitialView", typeof(Point));

			try {
				m_nIntelligence = (Intelligence)info.GetValue("m_nIntelligence", typeof(Intelligence));
			} catch (SerializationException) {
				m_nIntelligence = m_side == Side.side1 ? Intelligence.Human : Intelligence.ComputerOvermind;
			}
		}

		public void GetObjectData(SerializationInfo info, StreamingContext context) {
			info.AddValue("m_side", m_side);
			info.AddValue("m_nInitialCredits", m_nInitialCredits);
			info.AddValue("m_ptInitialView", m_ptInitialView);
			info.AddValue("m_nIntelligence", m_nIntelligence);
		}

		public SideInfo(Side side) {
			m_side = side;
		}

		public Side Side {
			get {
				return m_side;
			}
		}

		public int InitialCredits {
			get {
				return m_nInitialCredits;
			}
			set {
				m_nInitialCredits = value;
			}
		}

		public Point InitialView {
			get {
				return m_ptInitialView;
			}
			set {
				m_ptInitialView = value;
			}
		}

		public Intelligence Intelligence {
			get {
				return m_nIntelligence;
			}
			set {
				m_nIntelligence = value;
			}
		}
	}

	public class LevelDocTemplate : DocTemplate {
		static string[] astr = { "Level Document", "UntitledLevel", "Level Docs", "ld" };
		public LevelDocTemplate(Type typeFrame, Type typeView) : base(astr, typeof(LevelDoc), typeFrame, typeView, new LevelDocBinder()) {
		}
	}

	// Compatibility goo

	public class LevelDocBinder : SerializationBinder {
		public override Type BindToType(string strAssembly, string strType) {
			if (strType == "m.LevelDescription")
				return typeof(LevelDoc);
#if false
			// This is how we migrate a serialized class from one Assembly to another.

			if (strType.StartsWith("m.SideWinder")) {
				System.Reflection.Assembly ass = Globals.Plugins[0].GetType().Assembly;
				Type typeT = ass.GetType(strType);
				return typeT;
			}
#endif
			// Backwards compatibility with old levels

			switch (strType) {
			case "m.CenterViewAction":
				strType = "m.CenterViewTriggerAction";
				break;

			case "m.SetNextMissionAction":
				strType = "m.SetNextMissionTriggerAction";
				break;

			case "m.EndMissionAction":
				strType = "m.EndMissionTriggerAction";
				break;

			case "m.SetAllowedUnitsAction":
				strType = "m.SetAllowedUnitsTriggerAction";
				break;

			case "m.EcomAction":
				strType = "m.EcomTriggerAction";
				break;

			case "m.SetObjectiveAction":
				strType = "m.SetObjectiveTriggerAction";
				break;

			case "m.WaitAction":
				strType = "m.WaitTriggerAction";
				break;

			case "m.SetSwitchAction":
				strType = "m.SetSwitchTriggerAction";
				break;

			case "m.PerserveTriggerAction":
				strType = "m.PreserveTriggerTriggerAction";
				break;

			case "m.DefogAreaAction":
				strType = "m.DefogAreaTriggerAction";
				break;

			case "m.TakeoverSpecialistInfantry":
				strType = "m.TakeoverSpecialist";
				break;

			case "m.BuildUnitGroupTriggerAction":
				strType = "m.CreateUnitGroupTriggerAction";
				break;

#if false
			case "m.MoveUnitAction":
				strType = "m.MoveUnitGroupAction";
				break;

			case "m.SetSwitchUnitAction":
				strType = "m.SetSwitchUnitGroupAction";
				break;

			case "m.WaitUnitAction":
				strType = "m.WaitUnitGroupAction";
				break;
#endif
			}

			return Type.GetType(strType + ", " + strAssembly);
		}
	}
}
