using System;
using System.IO;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.Data;
using System.Windows.Forms;
using SpiffLib;

namespace m
{
	/// <summary>
	/// Summary description for TemplatePanel.
	/// </summary>
	public class TemplatePanel : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.ComboBox comboDocs;
		private m.FlowPanel flowPanel;
		private System.Windows.Forms.ToolBar toolBar;
		protected internal System.Windows.Forms.ImageList imageList1;
		private System.Windows.Forms.ToolBarButton toolBarButtonNew;
		private System.Windows.Forms.ToolBarButton toolBarButtonOpen;
		private System.Windows.Forms.ToolBarButton toolBarButtonSave;
		private System.Windows.Forms.ContextMenu contextMenuTiles;
		private System.Windows.Forms.MenuItem menuItemDeleteTile;
		private System.Windows.Forms.MenuItem menuItemTileBackground;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.ToolBarButton toolBarButtonClose;
		private System.Windows.Forms.ContextMenu contextMenuToolbar;
		private System.Windows.Forms.MenuItem menuItemAddTemplates;
		private System.Windows.Forms.MenuItem menuItemEditTerrain;
		private System.Windows.Forms.ToolBarButton toolBarButtonMisc;
		private System.Windows.Forms.ToolBarButton toolBarButtonSeparator;
		private System.Windows.Forms.ContextMenu contextMenuSave;
		private System.Windows.Forms.MenuItem menuItemSaveAs;
		private System.Windows.Forms.MenuItem menuItemSaveAll;
		private System.Windows.Forms.MenuItem menuItemSave;
		private System.Windows.Forms.MenuItem menuItemProperties;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItemTemplProperties;
		private System.Windows.Forms.MenuItem menuItemImportBitmap;
		private System.Windows.Forms.MenuItem menuItemExportBitmap;
		private System.Windows.Forms.MenuItem menuItemScaleDown;
		private System.Windows.Forms.MenuItem menuItemSavePalette;
		private System.Windows.Forms.MenuItem menuItemQuantizeOnly;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.ToolTip toolTip;
		TemplateDoc m_tmpdActive = null;

		public TemplatePanel()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			if (Globals.IsKit()) {
				Controls.Remove(toolBar);
				toolBar.Visible = false;
			}

			// Easier than creating a resource file?

			System.Reflection.Assembly ass = typeof(TemplatePanel).Module.Assembly;
			Stream stm = ass.GetManifestResourceStream("m.toolstrip.bmp");
			imageList1.Images.AddStrip(new Bitmap(stm));

			// We want to know about changes to template docs

			TemplateDocTemplate doctTemplate = (TemplateDocTemplate)DocManager.FindDocTemplate(typeof(TemplateDoc));
			if (doctTemplate != null) {
				doctTemplate.DocActive += new TemplateDocTemplate.DocActiveHandler(TemplateDocTemplate_DocActive);
				doctTemplate.DocAdded += new TemplateDocTemplate.DocAddedHandler(TemplateDocTemplate_DocAdded);
				doctTemplate.DocRemoved += new TemplateDocTemplate.DocRemovedHandler(TemplateDocTemplate_DocRemoved);
				doctTemplate.TemplatesAdded += new TemplateDocTemplate.TemplatesAddedHandler(TemplateDocTemplate_TemplatesAdded);
				doctTemplate.TemplatesRemoved += new TemplateDocTemplate.TemplatesRemovedHandler(TemplateDocTemplate_TemplatesRemoved);
				doctTemplate.TemplateChanged += new TemplateDocTemplate.TemplateChangedHandler(TemplateDocTemplate_TemplateChanged);
			}

			// We want to know when the active level doc changes

			LevelDocTemplate doctLevel = (LevelDocTemplate)DocManager.FindDocTemplate(typeof(LevelDoc));
			if (doctLevel != null)
				doctLevel.DocActive += new TemplateDocTemplate.DocActiveHandler(LevelDocTemplate_DocActive);
		}

		void LevelDocTemplate_DocActive(Document doc) {
			LevelDoc lvld = (LevelDoc)doc;
			if (lvld != null)
				SetActiveDoc(lvld.GetTemplateDoc());
		}

		void TemplateDocTemplate_DocActive(Document doc) {
			SetActiveDoc((TemplateDoc)doc);
		}

		void SetActiveDoc(TemplateDoc tmpd) {
			if (tmpd == m_tmpdActive)
				return;
			if (tmpd == null) {
				flowPanel.Controls.Clear();
			} else {
				ComboItem ciFound = null;
				for (int nIndex = 0; nIndex < comboDocs.Items.Count; nIndex++) {
					ComboItem ci = (ComboItem)comboDocs.Items[nIndex];
					if (ci.m_tmpd == tmpd) {
						ciFound = ci;
						comboDocs.SelectedIndex = nIndex;
						break;
					}
				}
				if (ciFound != null)
					FillPanel(ciFound.m_alsPictureBoxes);
			}
			m_tmpdActive = tmpd;
		}

		void FillPanel(ArrayList alsPictureBoxes) {
			flowPanel.SuspendLayout();
			flowPanel.Controls.Clear();
			flowPanel.Controls.AddRange((Control[])alsPictureBoxes.ToArray(typeof(Control)));
			flowPanel.ResumeLayout();
			flowPanel.RefreshScrollbar();
		}

		PictureBox CreatePictureBox(TemplateDoc tmpd, Size sizTile, IMapItem mi) {
			PictureBox picb = new PictureBox();
			picb.Image = Misc.TraceEdges(mi.GetBitmap(sizTile, tmpd), 1, Color.Azure);
			picb.SizeMode = PictureBoxSizeMode.AutoSize;
			picb.Tag = (Object)mi;
			picb.MouseDown += new MouseEventHandler(PictureBox_MouseDown);
			return picb;
		}

		public class ComboItem {
			public TemplateDoc m_tmpd;
			public ArrayList m_alsPictureBoxes;
			public ComboItem(TemplateDoc tmpd) {
				m_tmpd = tmpd;
				m_alsPictureBoxes = new ArrayList();
			}
			public override string ToString() {
				return m_tmpd.GetName();
			}
		}

		void TemplateDocTemplate_DocAdded(Document doc) {
			TemplateDoc tmpd = (TemplateDoc)doc;
			tmpd.ModifiedChanged += new TemplateDoc.ModifiedChangedHandler(TemplateDoc_ModifiedChanged);			
			tmpd.NameChanged += new TemplateDoc.NameChangedHandler(TemplateDoc_NameChanged);			
			ComboItem ci = new ComboItem(tmpd);
			comboDocs.SelectedIndex = comboDocs.Items.Add(ci);
			Template[] atmpl = tmpd.GetTemplates();
			ArrayList alsNames = new ArrayList();
			foreach (Template tmpl in atmpl)
				alsNames.Add(tmpl.Name);
			toolTip.RemoveAll();
			TemplateDocTemplate_TemplatesAdded(tmpd, (string[])alsNames.ToArray(typeof(string)));
		}

		void TemplateDocTemplate_DocRemoved(Document doc) {
			TemplateDoc tmpd = (TemplateDoc)doc;
			tmpd.ModifiedChanged -= new TemplateDoc.ModifiedChangedHandler(TemplateDoc_ModifiedChanged);
			tmpd.NameChanged -= new TemplateDoc.NameChangedHandler(TemplateDoc_NameChanged);						
			ComboItem ci = FindComboItem((TemplateDoc)doc);
			if (ci != null)
				comboDocs.Items.Remove(ci);
			toolTip.RemoveAll();
		}

		int FindComboIndex(TemplateDoc tmpd) {
			for (int nIndex = 0; nIndex < comboDocs.Items.Count; nIndex++) {
				ComboItem ci = (ComboItem)comboDocs.Items[nIndex];
				if (ci.m_tmpd == tmpd)
					return nIndex;
			}
			return -1;
		}

		ComboItem FindComboItem(TemplateDoc tmpd) {
			int nIndex = FindComboIndex(tmpd);
			if (nIndex == -1)
				return null;
			return (ComboItem)comboDocs.Items[nIndex];
		}

		void TemplateDocTemplate_TemplatesAdded(TemplateDoc tmpd, string[] astrNames) {
			ComboItem ci = FindComboItem(tmpd);
			foreach (string strName in astrNames) {
				PictureBox picb = CreatePictureBox(tmpd, tmpd.TileSize, new Tile(tmpd, strName, 0, 0));
				ci.m_alsPictureBoxes.Add(picb);
				toolTip.SetToolTip(picb, strName);
			}
			if (tmpd == m_tmpdActive)
				FillPanel(ci.m_alsPictureBoxes);
		}

		void TemplateDocTemplate_TemplatesRemoved(TemplateDoc tmpd, string[] astrNames) {
			ComboItem ci = FindComboItem(tmpd);
			foreach (string strName in astrNames) {
				foreach (PictureBox picb in ci.m_alsPictureBoxes) {
					Tile tile = (Tile)picb.Tag;
					if (strName == tile.Name) {
						ci.m_alsPictureBoxes.Remove(picb);
						break;
					}
				}
			}
			if (tmpd == m_tmpdActive)
				FillPanel(ci.m_alsPictureBoxes);
		}

		void TemplateDocTemplate_TemplateChanged(TemplateDoc tmpd, string strProperty, string strName, string strParam) {
			ComboItem ci = FindComboItem(tmpd);
			foreach (PictureBox picb in ci.m_alsPictureBoxes) {
				Tile tile = (Tile)picb.Tag;
				if (tile.Name == strName) {
					Template tmpl = tmpd.FindTemplate(strProperty == "Name" ? strParam : strName);
					picb.Image = Misc.TraceEdges(tmpl.Bitmap, 1, Color.Azure);
					break;
				}
			}
			if (tmpd == m_tmpdActive)
				FillPanel(ci.m_alsPictureBoxes);
		}

		void TemplateDoc_ModifiedChanged(Document doc, bool fModified) {
			comboDocs.Invalidate();
		}

		void TemplateDoc_NameChanged(Document doc) {
			comboDocs.Invalidate();
		}

		private void comboDocs_DrawItem(object sender, System.Windows.Forms.DrawItemEventArgs e) {
			if (e.Index < 0 || e.Index >= comboDocs.Items.Count)
				return;
			ComboItem ci = (ComboItem)comboDocs.Items[e.Index];
			string strName = ci.ToString() + (ci.m_tmpd.IsModified() ? "*" : "");
			e.DrawBackground();
			e.Graphics.DrawString(strName, e.Font, new SolidBrush(e.ForeColor), e.Bounds.X, e.Bounds.Y);
			e.DrawFocusRectangle();
		}

		private void comboDocs_SelectedIndexChanged(object sender, System.EventArgs e) {
			ComboItem ci = (ComboItem)comboDocs.Items[comboDocs.SelectedIndex];
			DocManager.SetActiveDocument(typeof(TemplateDoc), ci.m_tmpd);
		}

		private void PictureBox_MouseDown(Object sender, MouseEventArgs e) {
			Control ctlSelected = (Control)sender;

			if (!Globals.IsKit()) {
				if (e.Button == MouseButtons.Right) {
					contextMenuTiles.Show(ctlSelected, new Point(e.X, e.Y));
					return;
				}
			}

			Tile tile = (Tile)((PictureBox)sender).Tag;
			Globals.PropertyGrid.SelectedObject = tile.GetTemplate(m_tmpdActive);

			// Start drag drop

			LevelData ldat = new LevelData();
			IMapItem mi = (IMapItem)ctlSelected.Tag;
			ldat.ami = new IMapItem[] { mi };
			Size sizTile = m_tmpdActive.TileSize;
			ldat.txMouse = e.X / (double)sizTile.Width;
			ldat.tyMouse = e.Y / (double)sizTile.Height;
			ldat.Grid.Width = mi.Grid.Width;
			ldat.Grid.Height = mi.Grid.Height;
			DoDragDrop(ldat, DragDropEffects.Copy);
		}

		private void AddTemplates() {
			// Get tile filename(s)

			OpenFileDialog frmOpen = new OpenFileDialog();
			frmOpen.Multiselect = true;
			frmOpen.Filter = "Image Files (*.*)|*.*";
			frmOpen.Title = "Add Templates";
			if (frmOpen.ShowDialog() == DialogResult.Cancel)
				return;

			// Load them up. If there is no template doc yet, make a new one

			if (m_tmpdActive == null)
				DocManager.NewDocument(typeof(TemplateDoc), null);
			m_tmpdActive.AddTemplates(frmOpen.FileNames);
		}

		private void toolBar_ButtonClick(object sender, System.Windows.Forms.ToolBarButtonClickEventArgs e) {
			switch (toolBar.Buttons.IndexOf(e.Button)) {
			case 0:
				DocManager.NewDocument(typeof(TemplateDoc), null);
				break;

			case 1:
				DocManager.OpenDocument(typeof(TemplateDoc));
				break;

			case 2:
				if (m_tmpdActive != null)
					m_tmpdActive.Save();
				break;

			case 3:
				AddTemplates();
				//Rectangle rc = toolBar.Buttons[toolBar.Buttons.IndexOf(e.Button)].Rectangle;
				//contextMenuToolbar.Show(toolBar, new Point(rc.X, rc.Y + rc.Height));
				break;

			case 4:
				// Separator
				break;

			case 5:
				if (m_tmpdActive == null)
					return;
				m_tmpdActive.Close();
				break;
			}
		}

		private void menuItemSave_Click(object sender, System.EventArgs e) {
			if (m_tmpdActive != null)
				m_tmpdActive.Save();		
		}

		private void menuItemSaveAs_Click(object sender, System.EventArgs e) {
			if (m_tmpdActive != null)
				m_tmpdActive.SaveAs(null);
		}

		private void menuItemSaveAll_Click(object sender, System.EventArgs e) {
			DocManager.SaveAllModified(typeof(TemplateDoc));
		}

		private void menuItemAddTemplates_Click(object sender, System.EventArgs e) {
			AddTemplates();
		}

		private void menuItemEditTerrain_Click(object sender, System.EventArgs e) {
			if (m_tmpdActive == null)
				return;
			Form frmEditTerrain = new EditTerrainForm(m_tmpdActive);
			frmEditTerrain.ShowDialog();
		}

		private IMapItem GetMapItem(Object sender) {
			ContextMenu contextMenu = (ContextMenu)((MenuItem)sender).Parent;
			return (IMapItem)((PictureBox)contextMenu.SourceControl).Tag;
		}
		
		private void menuItemImportBitmap_Click(object sender, System.EventArgs e) {
			Tile tile = GetMapItem(sender) as Tile;
			if (tile == null)
				return;
			Template tmpl = tile.GetTemplate(m_tmpdActive);
			OpenFileDialog frmOpen = new OpenFileDialog();
			frmOpen.FileName = tmpl.ImportPath;
			if (frmOpen.ShowDialog() == DialogResult.Cancel)
				return;
			tmpl.Import(frmOpen.FileName);
		}

		private void menuItemExportBitmap_Click(object sender, System.EventArgs e) {
			Tile tile = GetMapItem(sender) as Tile;
			if (tile == null)
				return;
			Template tmpl = tile.GetTemplate(m_tmpdActive);
			SaveFileDialog frmSave = new SaveFileDialog();
			frmSave.DefaultExt = "png";
			frmSave.Filter = "Png Files (*.png)|*.png";
			frmSave.Title = "Save Template Bitmap As";
			if (tmpl.ImportPath != null) {
				frmSave.FileName = tmpl.ImportPath;
			} else {
				frmSave.FileName = tmpl.Name;
			}
			if (frmSave.ShowDialog() == DialogResult.Cancel)
				return;
			tmpl.Bitmap.Save(frmSave.FileName, ImageFormat.Png);
		}

		private void menuItemProperties_Click(object sender, System.EventArgs e) {
			Globals.PropertyGrid.SelectedObject = m_tmpdActive;
		}

		private void menuItemDeleteTile_Click(object sender, System.EventArgs e) {
			Tile tile = GetMapItem(sender) as Tile;
			if (tile == null)
				return;
			if (MessageBox.Show("Are you sure?", "Delete Tile", MessageBoxButtons.YesNo) == DialogResult.Yes)
				m_tmpdActive.RemoveTemplates(new Template[] { tile.GetTemplate(m_tmpdActive) });
		}

		private void menuItemTileBackground_Click(object sender, System.EventArgs e) {
			Tile tile = GetMapItem(sender) as Tile;
			if (tile == null)
				return;
			m_tmpdActive.SetBackgroundTemplate(tile.GetTemplate(m_tmpdActive));
		}

		private void menuItemTemplProperties_Click(object sender, System.EventArgs e) {
			Tile tile = GetMapItem(sender) as Tile;
			if (tile == null)
				return;
			Globals.PropertyGrid.SelectedObject = tile.GetTemplate(m_tmpdActive);
		}

		private void menuItemScaleDown_Click(object sender, System.EventArgs e) {
			// If no template doc active, bail

			if (m_tmpdActive == null)
				return;

			// Make sure 24 x 24 (could actually allow any sort of conversion...)

			if (m_tmpdActive.TileSize.Width != 24 && m_tmpdActive.TileSize.Height != 24) {
				MessageBox.Show(DocManager.GetFrameParent(), "The current template collection must be 24 x 24 tile size");
				return;
			}

			// Get busy

			TemplateDoc tmpdDst = TemplateTools.CloneTemplateDoc(m_tmpdActive);
			TemplateTools.ScaleTemplates(tmpdDst, new Size(16, 16));
			TemplateTools.QuantizeTemplates(tmpdDst, null, 0, 0, 0);
			DocManager.SetActiveDocument(typeof(TemplateDoc), tmpdDst);
		}

		private void menuItemQuantizeOnly_Click(object sender, System.EventArgs e) {
			if (m_tmpdActive == null)
				return;
			TemplateDoc tmpdDst = TemplateTools.CloneTemplateDoc(m_tmpdActive);
			TemplateTools.QuantizeTemplates(tmpdDst, null, 0, 0, 0);
			DocManager.SetActiveDocument(typeof(TemplateDoc), tmpdDst);
		}

		private void menuItemSavePalette_Click(object sender, System.EventArgs e) {
			if (m_tmpdActive == null)
				return;
			Palette pal = m_tmpdActive.GetPalette();
			if (pal == null) {
				MessageBox.Show(DocManager.GetFrameParent(), "No palette on this template collection. You need to create one!");
				return;
			}
			pal.SaveDialog();
		}

		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.comboDocs = new System.Windows.Forms.ComboBox();
			this.flowPanel = new m.FlowPanel();
			this.toolBar = new System.Windows.Forms.ToolBar();
			this.toolBarButtonNew = new System.Windows.Forms.ToolBarButton();
			this.toolBarButtonOpen = new System.Windows.Forms.ToolBarButton();
			this.toolBarButtonSave = new System.Windows.Forms.ToolBarButton();
			this.contextMenuSave = new System.Windows.Forms.ContextMenu();
			this.menuItemSave = new System.Windows.Forms.MenuItem();
			this.menuItemSaveAs = new System.Windows.Forms.MenuItem();
			this.menuItemSaveAll = new System.Windows.Forms.MenuItem();
			this.toolBarButtonMisc = new System.Windows.Forms.ToolBarButton();
			this.contextMenuToolbar = new System.Windows.Forms.ContextMenu();
			this.menuItemAddTemplates = new System.Windows.Forms.MenuItem();
			this.menuItemEditTerrain = new System.Windows.Forms.MenuItem();
			this.menuItemScaleDown = new System.Windows.Forms.MenuItem();
			this.menuItemQuantizeOnly = new System.Windows.Forms.MenuItem();
			this.menuItemSavePalette = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.menuItemProperties = new System.Windows.Forms.MenuItem();
			this.toolBarButtonSeparator = new System.Windows.Forms.ToolBarButton();
			this.toolBarButtonClose = new System.Windows.Forms.ToolBarButton();
			this.imageList1 = new System.Windows.Forms.ImageList(this.components);
			this.contextMenuTiles = new System.Windows.Forms.ContextMenu();
			this.menuItemImportBitmap = new System.Windows.Forms.MenuItem();
			this.menuItemExportBitmap = new System.Windows.Forms.MenuItem();
			this.menuItemDeleteTile = new System.Windows.Forms.MenuItem();
			this.menuItemTileBackground = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.menuItemTemplProperties = new System.Windows.Forms.MenuItem();
			this.toolTip = new System.Windows.Forms.ToolTip(this.components);
			this.SuspendLayout();
			// 
			// comboDocs
			// 
			this.comboDocs.Dock = System.Windows.Forms.DockStyle.Top;
			this.comboDocs.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
			this.comboDocs.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboDocs.Location = new System.Drawing.Point(0, 24);
			this.comboDocs.Name = "comboDocs";
			this.comboDocs.Size = new System.Drawing.Size(168, 21);
			this.comboDocs.TabIndex = 0;
			this.comboDocs.SelectedIndexChanged += new System.EventHandler(this.comboDocs_SelectedIndexChanged);
			this.comboDocs.DrawItem += new System.Windows.Forms.DrawItemEventHandler(this.comboDocs_DrawItem);
			// 
			// flowPanel
			// 
			this.flowPanel.AutoScroll = true;
			this.flowPanel.BackColor = System.Drawing.Color.Black;
			this.flowPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowPanel.Location = new System.Drawing.Point(0, 45);
			this.flowPanel.Name = "flowPanel";
			this.flowPanel.Size = new System.Drawing.Size(168, 403);
			this.flowPanel.TabIndex = 1;
			// 
			// toolBar
			// 
			this.toolBar.Appearance = System.Windows.Forms.ToolBarAppearance.Flat;
			this.toolBar.Buttons.AddRange(new System.Windows.Forms.ToolBarButton[] {
																					   this.toolBarButtonNew,
																					   this.toolBarButtonOpen,
																					   this.toolBarButtonSave,
																					   this.toolBarButtonMisc,
																					   this.toolBarButtonSeparator,
																					   this.toolBarButtonClose});
			this.toolBar.ButtonSize = new System.Drawing.Size(16, 16);
			this.toolBar.DropDownArrows = true;
			this.toolBar.ImageList = this.imageList1;
			this.toolBar.Name = "toolBar";
			this.toolBar.ShowToolTips = true;
			this.toolBar.Size = new System.Drawing.Size(168, 24);
			this.toolBar.TabIndex = 2;
			this.toolBar.ButtonClick += new System.Windows.Forms.ToolBarButtonClickEventHandler(this.toolBar_ButtonClick);
			// 
			// toolBarButtonNew
			// 
			this.toolBarButtonNew.ImageIndex = 0;
			this.toolBarButtonNew.ToolTipText = "New Template Collection";
			// 
			// toolBarButtonOpen
			// 
			this.toolBarButtonOpen.ImageIndex = 1;
			this.toolBarButtonOpen.ToolTipText = "Open Template Collection";
			// 
			// toolBarButtonSave
			// 
			this.toolBarButtonSave.DropDownMenu = this.contextMenuSave;
			this.toolBarButtonSave.ImageIndex = 2;
			this.toolBarButtonSave.Style = System.Windows.Forms.ToolBarButtonStyle.DropDownButton;
			this.toolBarButtonSave.ToolTipText = "Save Template Collection";
			// 
			// contextMenuSave
			// 
			this.contextMenuSave.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							this.menuItemSave,
																							this.menuItemSaveAs,
																							this.menuItemSaveAll});
			// 
			// menuItemSave
			// 
			this.menuItemSave.Index = 0;
			this.menuItemSave.Text = "Save";
			this.menuItemSave.Click += new System.EventHandler(this.menuItemSave_Click);
			// 
			// menuItemSaveAs
			// 
			this.menuItemSaveAs.Index = 1;
			this.menuItemSaveAs.Text = "Save As...";
			this.menuItemSaveAs.Click += new System.EventHandler(this.menuItemSaveAs_Click);
			// 
			// menuItemSaveAll
			// 
			this.menuItemSaveAll.Index = 2;
			this.menuItemSaveAll.Text = "Save All";
			this.menuItemSaveAll.Click += new System.EventHandler(this.menuItemSaveAll_Click);
			// 
			// toolBarButtonMisc
			// 
			this.toolBarButtonMisc.DropDownMenu = this.contextMenuToolbar;
			this.toolBarButtonMisc.ImageIndex = 4;
			this.toolBarButtonMisc.Style = System.Windows.Forms.ToolBarButtonStyle.DropDownButton;
			this.toolBarButtonMisc.ToolTipText = "Add Templates and other tools";
			// 
			// contextMenuToolbar
			// 
			this.contextMenuToolbar.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							   this.menuItemAddTemplates,
																							   this.menuItemEditTerrain,
																							   this.menuItemScaleDown,
																							   this.menuItemQuantizeOnly,
																							   this.menuItemSavePalette,
																							   this.menuItem1,
																							   this.menuItemProperties});
			// 
			// menuItemAddTemplates
			// 
			this.menuItemAddTemplates.Index = 0;
			this.menuItemAddTemplates.Text = "Add Templates...";
			this.menuItemAddTemplates.Click += new System.EventHandler(this.menuItemAddTemplates_Click);
			// 
			// menuItemEditTerrain
			// 
			this.menuItemEditTerrain.Index = 1;
			this.menuItemEditTerrain.Text = "Edit Terrain...";
			this.menuItemEditTerrain.Click += new System.EventHandler(this.menuItemEditTerrain_Click);
			// 
			// menuItemScaleDown
			// 
			this.menuItemScaleDown.Index = 2;
			this.menuItemScaleDown.Text = "Scale && Quantize...";
			this.menuItemScaleDown.Click += new System.EventHandler(this.menuItemScaleDown_Click);
			// 
			// menuItemQuantizeOnly
			// 
			this.menuItemQuantizeOnly.Index = 3;
			this.menuItemQuantizeOnly.Text = "Quantize Only...";
			this.menuItemQuantizeOnly.Click += new System.EventHandler(this.menuItemQuantizeOnly_Click);
			// 
			// menuItemSavePalette
			// 
			this.menuItemSavePalette.Index = 4;
			this.menuItemSavePalette.Text = "Save Palette...";
			this.menuItemSavePalette.Click += new System.EventHandler(this.menuItemSavePalette_Click);
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 5;
			this.menuItem1.Text = "-";
			// 
			// menuItemProperties
			// 
			this.menuItemProperties.Index = 6;
			this.menuItemProperties.Text = "Properties";
			this.menuItemProperties.Click += new System.EventHandler(this.menuItemProperties_Click);
			// 
			// toolBarButtonSeparator
			// 
			this.toolBarButtonSeparator.Style = System.Windows.Forms.ToolBarButtonStyle.Separator;
			// 
			// toolBarButtonClose
			// 
			this.toolBarButtonClose.ImageIndex = 5;
			this.toolBarButtonClose.ToolTipText = "Close Template Collection";
			// 
			// imageList1
			// 
			this.imageList1.ColorDepth = System.Windows.Forms.ColorDepth.Depth8Bit;
			this.imageList1.ImageSize = new System.Drawing.Size(16, 15);
			this.imageList1.TransparentColor = System.Drawing.Color.Magenta;
			// 
			// contextMenuTiles
			// 
			this.contextMenuTiles.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							 this.menuItemImportBitmap,
																							 this.menuItemExportBitmap,
																							 this.menuItemDeleteTile,
																							 this.menuItemTileBackground,
																							 this.menuItem2,
																							 this.menuItemTemplProperties});
			// 
			// menuItemImportBitmap
			// 
			this.menuItemImportBitmap.Index = 0;
			this.menuItemImportBitmap.Text = "Import Bitmap...";
			this.menuItemImportBitmap.Click += new System.EventHandler(this.menuItemImportBitmap_Click);
			// 
			// menuItemExportBitmap
			// 
			this.menuItemExportBitmap.Index = 1;
			this.menuItemExportBitmap.Text = "Export Bitmap...";
			this.menuItemExportBitmap.Click += new System.EventHandler(this.menuItemExportBitmap_Click);
			// 
			// menuItemDeleteTile
			// 
			this.menuItemDeleteTile.Index = 2;
			this.menuItemDeleteTile.Text = "Remove";
			this.menuItemDeleteTile.Click += new System.EventHandler(this.menuItemDeleteTile_Click);
			// 
			// menuItemTileBackground
			// 
			this.menuItemTileBackground.Index = 3;
			this.menuItemTileBackground.Text = "Background";
			this.menuItemTileBackground.Click += new System.EventHandler(this.menuItemTileBackground_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 4;
			this.menuItem2.Text = "-";
			// 
			// menuItemTemplProperties
			// 
			this.menuItemTemplProperties.Index = 5;
			this.menuItemTemplProperties.Text = "Properties";
			this.menuItemTemplProperties.Click += new System.EventHandler(this.menuItemTemplProperties_Click);
			// 
			// TemplatePanel
			// 
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.flowPanel,
																		  this.comboDocs,
																		  this.toolBar});
			this.ForeColor = System.Drawing.SystemColors.Control;
			this.Name = "TemplatePanel";
			this.Size = new System.Drawing.Size(168, 448);
			this.ResumeLayout(false);

		}
		#endregion
	}
}
