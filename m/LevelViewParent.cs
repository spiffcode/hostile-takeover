using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.IO;

namespace m
{
	/// <summary>
	/// Summary description for LevelViewParent.
	/// </summary>
	public class LevelViewParent : System.Windows.Forms.UserControl, ICommandTarget
	{
		private m.LevelView view;
		private System.Windows.Forms.ToolBar toolBar1;
		private System.Windows.Forms.ComboBox comboDocs;
		private System.Windows.Forms.ImageList imageList1;
		private System.Windows.Forms.ToolBarButton toolBarButtonHideToolbar;
		private System.Windows.Forms.Panel panelToolbar;
		private System.Windows.Forms.Panel panelShowToolbar;
		private System.Windows.Forms.ToolBar toolBarShowToolbar;
		private System.Windows.Forms.ToolBarButton toolBarButtonShowToolbar;
		private System.Windows.Forms.ToolBarButton toolBarButtonToggleTemplates;
		private System.Windows.Forms.ToolBarButton toolBarButtonToggleGobs;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.ComboBox comboZoom;
		private System.Windows.Forms.ToolBarButton toolBarButtonToggleAreas;
		TemplateDoc m_tmpdCurrent = null;

		public LevelViewParent()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// Default
			panelShowToolbar.Hide();
			panelToolbar.Show();

			// Easier than creating a resource file?

			System.Reflection.Assembly ass = typeof(LevelViewParent).Module.Assembly;
			Stream stm = ass.GetManifestResourceStream("m.toolstrip.bmp");
			imageList1.Images.AddStrip(new Bitmap(stm));

			TemplateDocTemplate doct = (TemplateDocTemplate)DocManager.FindDocTemplate(typeof(TemplateDoc));
			doct.DocAdded += new DocTemplate.DocAddedHandler(TemplateDocTemplate_DocAdded);
			doct.DocRemoved += new DocTemplate.DocRemovedHandler(TemplateDocTemplate_DocRemoved);

			// Combo index 0

			FillCombo();
			comboDocs.SelectedIndex = 0;
			UpdateZoomSelection();
			view.ScaleChanged += new EventHandler(View_ScaleChanged);
		}

		public void DispatchCommand(Command cmd) {
			switch (cmd) {
			case Command.Cut:
				view.Cut();
				break;

			case Command.Copy:
				view.Copy();
				break;

			case Command.Paste:
				view.Paste();
				break;

			case Command.Delete:
				view.Delete();
				break;
			}
		}

		void TemplateDocTemplate_DocAdded(Document doc) {
			comboDocs.Items.Add(doc);
			comboDocs.Invalidate();
		}

		void TemplateDocTemplate_DocRemoved(Document doc) {
			// We'll get notified of this after the window has been destroyed. Only
			// handle this event if this isn't the case

			if (!Created)
				return;

			if (m_tmpdCurrent == (TemplateDoc)doc) {
				m_tmpdCurrent = null;
				comboDocs.SelectedIndex = 0;
			}

			comboDocs.Items.Remove(doc);
		}

		void FillCombo() {
			comboDocs.Items.Clear();
			TemplateDocTemplate doct = (TemplateDocTemplate)DocManager.FindDocTemplate(typeof(TemplateDoc));
			Document[] adoc = doct.GetDocuments();
			comboDocs.Items.Add("");
			foreach (Document doc in adoc)
				comboDocs.Items.Add(doc);
		}

		public void SetDocument(Document doc) {
			view.SetDocument(doc);
		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
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
			this.view = new m.LevelView();
			this.panelToolbar = new System.Windows.Forms.Panel();
			this.label2 = new System.Windows.Forms.Label();
			this.comboZoom = new System.Windows.Forms.ComboBox();
			this.label1 = new System.Windows.Forms.Label();
			this.toolBar1 = new System.Windows.Forms.ToolBar();
			this.toolBarButtonToggleTemplates = new System.Windows.Forms.ToolBarButton();
			this.toolBarButtonToggleGobs = new System.Windows.Forms.ToolBarButton();
			this.toolBarButtonToggleAreas = new System.Windows.Forms.ToolBarButton();
			this.toolBarButtonHideToolbar = new System.Windows.Forms.ToolBarButton();
			this.imageList1 = new System.Windows.Forms.ImageList(this.components);
			this.comboDocs = new System.Windows.Forms.ComboBox();
			this.panelShowToolbar = new System.Windows.Forms.Panel();
			this.toolBarShowToolbar = new System.Windows.Forms.ToolBar();
			this.toolBarButtonShowToolbar = new System.Windows.Forms.ToolBarButton();
			this.panelToolbar.SuspendLayout();
			this.panelShowToolbar.SuspendLayout();
			this.SuspendLayout();
			// 
			// view
			// 
			this.view.AllowDrop = true;
			this.view.BackColor = System.Drawing.Color.Black;
			this.view.Dock = System.Windows.Forms.DockStyle.Fill;
			this.view.Name = "view";
			this.view.Size = new System.Drawing.Size(720, 616);
			this.view.TabIndex = 0;
			// 
			// panelToolbar
			// 
			this.panelToolbar.BackColor = System.Drawing.Color.White;
			this.panelToolbar.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panelToolbar.Controls.AddRange(new System.Windows.Forms.Control[] {
																					   this.label2,
																					   this.comboZoom,
																					   this.label1,
																					   this.toolBar1,
																					   this.comboDocs});
			this.panelToolbar.Location = new System.Drawing.Point(-1, -1);
			this.panelToolbar.Name = "panelToolbar";
			this.panelToolbar.Size = new System.Drawing.Size(452, 25);
			this.panelToolbar.TabIndex = 1;
			this.panelToolbar.Visible = false;
			// 
			// label2
			// 
			this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label2.Location = new System.Drawing.Point(200, 4);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(80, 23);
			this.label2.TabIndex = 5;
			this.label2.Text = "Zoom Percent:";
			// 
			// comboZoom
			// 
			this.comboZoom.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.comboZoom.Items.AddRange(new object[] {
														   "50",
														   "75",
														   "100",
														   "125",
														   "150",
														   "200",
														   "250",
														   "300"});
			this.comboZoom.Location = new System.Drawing.Point(280, 1);
			this.comboZoom.Name = "comboZoom";
			this.comboZoom.Size = new System.Drawing.Size(72, 21);
			this.comboZoom.TabIndex = 4;
			this.comboZoom.TabStop = false;
			this.comboZoom.KeyDown += new System.Windows.Forms.KeyEventHandler(this.comboZoom_KeyDown);
			this.comboZoom.SelectedIndexChanged += new System.EventHandler(this.comboZoom_SelectedIndexChanged);
			// 
			// label1
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.Location = new System.Drawing.Point(0, 4);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(56, 23);
			this.label1.TabIndex = 3;
			this.label1.Text = "View with:";
			// 
			// toolBar1
			// 
			this.toolBar1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.toolBar1.Appearance = System.Windows.Forms.ToolBarAppearance.Flat;
			this.toolBar1.Buttons.AddRange(new System.Windows.Forms.ToolBarButton[] {
																						this.toolBarButtonToggleTemplates,
																						this.toolBarButtonToggleGobs,
																						this.toolBarButtonToggleAreas,
																						this.toolBarButtonHideToolbar});
			this.toolBar1.ButtonSize = new System.Drawing.Size(16, 15);
			this.toolBar1.Divider = false;
			this.toolBar1.Dock = System.Windows.Forms.DockStyle.None;
			this.toolBar1.DropDownArrows = true;
			this.toolBar1.ImageList = this.imageList1;
			this.toolBar1.Location = new System.Drawing.Point(357, 1);
			this.toolBar1.Name = "toolBar1";
			this.toolBar1.ShowToolTips = true;
			this.toolBar1.Size = new System.Drawing.Size(99, 22);
			this.toolBar1.TabIndex = 2;
			this.toolBar1.ButtonClick += new System.Windows.Forms.ToolBarButtonClickEventHandler(this.toolBar1_ButtonClick);
			// 
			// toolBarButtonToggleTemplates
			// 
			this.toolBarButtonToggleTemplates.ImageIndex = 8;
			this.toolBarButtonToggleTemplates.Pushed = true;
			this.toolBarButtonToggleTemplates.Style = System.Windows.Forms.ToolBarButtonStyle.ToggleButton;
			this.toolBarButtonToggleTemplates.ToolTipText = "Toggle Templates";
			// 
			// toolBarButtonToggleGobs
			// 
			this.toolBarButtonToggleGobs.ImageIndex = 9;
			this.toolBarButtonToggleGobs.Pushed = true;
			this.toolBarButtonToggleGobs.Style = System.Windows.Forms.ToolBarButtonStyle.ToggleButton;
			this.toolBarButtonToggleGobs.ToolTipText = "Toggle Gobs";
			// 
			// toolBarButtonToggleAreas
			// 
			this.toolBarButtonToggleAreas.ImageIndex = 10;
			this.toolBarButtonToggleAreas.Pushed = true;
			this.toolBarButtonToggleAreas.Style = System.Windows.Forms.ToolBarButtonStyle.ToggleButton;
			this.toolBarButtonToggleAreas.ToolTipText = "Toggle Areas";
			// 
			// toolBarButtonHideToolbar
			// 
			this.toolBarButtonHideToolbar.ImageIndex = 6;
			this.toolBarButtonHideToolbar.ToolTipText = "Hide Toolbar";
			// 
			// imageList1
			// 
			this.imageList1.ColorDepth = System.Windows.Forms.ColorDepth.Depth8Bit;
			this.imageList1.ImageSize = new System.Drawing.Size(16, 15);
			this.imageList1.TransparentColor = System.Drawing.Color.Magenta;
			// 
			// comboDocs
			// 
			this.comboDocs.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.comboDocs.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
			this.comboDocs.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboDocs.Location = new System.Drawing.Point(56, 1);
			this.comboDocs.Name = "comboDocs";
			this.comboDocs.Size = new System.Drawing.Size(136, 21);
			this.comboDocs.TabIndex = 1;
			this.comboDocs.TabStop = false;
			this.comboDocs.SelectedIndexChanged += new System.EventHandler(this.comboDocs_SelectedIndexChanged);
			this.comboDocs.DrawItem += new System.Windows.Forms.DrawItemEventHandler(this.comboDocs_DrawItem);
			// 
			// panelShowToolbar
			// 
			this.panelShowToolbar.BackColor = System.Drawing.Color.White;
			this.panelShowToolbar.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panelShowToolbar.Controls.AddRange(new System.Windows.Forms.Control[] {
																						   this.toolBarShowToolbar});
			this.panelShowToolbar.Location = new System.Drawing.Point(-3, -5);
			this.panelShowToolbar.Name = "panelShowToolbar";
			this.panelShowToolbar.Size = new System.Drawing.Size(22, 23);
			this.panelShowToolbar.TabIndex = 2;
			// 
			// toolBarShowToolbar
			// 
			this.toolBarShowToolbar.Appearance = System.Windows.Forms.ToolBarAppearance.Flat;
			this.toolBarShowToolbar.Buttons.AddRange(new System.Windows.Forms.ToolBarButton[] {
																								  this.toolBarButtonShowToolbar});
			this.toolBarShowToolbar.ButtonSize = new System.Drawing.Size(16, 15);
			this.toolBarShowToolbar.Dock = System.Windows.Forms.DockStyle.None;
			this.toolBarShowToolbar.DropDownArrows = true;
			this.toolBarShowToolbar.ImageList = this.imageList1;
			this.toolBarShowToolbar.Name = "toolBarShowToolbar";
			this.toolBarShowToolbar.ShowToolTips = true;
			this.toolBarShowToolbar.Size = new System.Drawing.Size(25, 24);
			this.toolBarShowToolbar.TabIndex = 0;
			this.toolBarShowToolbar.ButtonClick += new System.Windows.Forms.ToolBarButtonClickEventHandler(this.toolBarShowToolbar_ButtonClick);
			// 
			// toolBarButtonShowToolbar
			// 
			this.toolBarButtonShowToolbar.ImageIndex = 7;
			this.toolBarButtonShowToolbar.ToolTipText = "Show Toolbar";
			// 
			// LevelViewParent
			// 
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panelShowToolbar,
																		  this.panelToolbar,
																		  this.view});
			this.Name = "LevelViewParent";
			this.Size = new System.Drawing.Size(720, 616);
			this.panelToolbar.ResumeLayout(false);
			this.panelShowToolbar.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void toolBar1_ButtonClick(object sender, System.Windows.Forms.ToolBarButtonClickEventArgs e) {
			switch (toolBar1.Buttons.IndexOf(e.Button)) {
			case 0:
				view.SetLayerFlags(view.GetLayerFlags() ^ LayerFlags.Templates);
				break;

			case 1:
				view.SetLayerFlags(view.GetLayerFlags() ^ LayerFlags.Gobs);
				break;

			case 2:
				view.SetLayerFlags(view.GetLayerFlags() ^ LayerFlags.Areas);
				break;

			case 3:
				panelToolbar.Hide();
				panelShowToolbar.Show();
				break;
			}		
		}

		private void toolBarShowToolbar_ButtonClick(object sender, System.Windows.Forms.ToolBarButtonClickEventArgs e) {
			switch (toolBarShowToolbar.Buttons.IndexOf(e.Button)) {
			case 0:
				panelShowToolbar.Hide();
				panelToolbar.Show();
				break;
			}
		}

		private void comboDocs_SelectedIndexChanged(object sender, System.EventArgs e) {
			int nIndex = comboDocs.SelectedIndex;
			if (nIndex == 0) {
				view.SetTemplateDoc(null);
				return;
			}
			view.SetTemplateDoc((TemplateDoc)comboDocs.Items[nIndex]);
		}

		private void comboDocs_DrawItem(object sender, System.Windows.Forms.DrawItemEventArgs e) {
			if (e.Index < 0 || e.Index >= comboDocs.Items.Count)
				return;
			string strName;
			if (e.Index == 0) {
				Document docActive = DocManager.GetActiveDocument(typeof(TemplateDoc));
				if (docActive == null) {
					strName = "None";
				} else {
					strName = "Active (" + docActive.GetName() + ")";
				}
				m_tmpdCurrent = null;
			} else {
				Document doc = (Document)comboDocs.Items[e.Index];
				m_tmpdCurrent = (TemplateDoc)doc;
				strName = doc.GetName();
			}
			e.DrawBackground();
			e.Graphics.DrawString(strName, e.Font, new SolidBrush(e.ForeColor), e.Bounds.X, e.Bounds.Y);
			e.DrawFocusRectangle();
		}

		private void comboZoom_SelectedIndexChanged(object sender, System.EventArgs e) {
			SetScale();
		}

		private void comboZoom_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e) {
			if (e.KeyCode == Keys.Enter) {
				SetScale();
			}
		}

		void SetScale() {
			bool fReset = false;
			try {
				view.SetScale(float.Parse(comboZoom.Text) / 100.0f);
			} catch {
				fReset = true;
			}
			if (fReset)
				UpdateZoomSelection();
		}

		void UpdateZoomSelection() {
			string strT = ((float)(view.GetScale() * 100.0f)).ToString();
			for (int i = 0; i < comboZoom.Items.Count; i++) {
				if (strT == float.Parse((string)comboZoom.Items[i]).ToString()) {
					comboZoom.SelectedIndex = i;
					break;
				}
			}
			comboZoom.Text = strT;
		}

		void View_ScaleChanged(object sender, EventArgs e) {
			UpdateZoomSelection();
		}
	}
}
