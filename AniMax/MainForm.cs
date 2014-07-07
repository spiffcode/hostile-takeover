using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using Crownwood.Magic.Docking;
using Crownwood.Magic.Common;
using System.Diagnostics;
using System.IO;
using System.Drawing.Imaging;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for MainForm.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		private string gstrAniMax = "AniMax";
		private PreviewPanel m_ctlPreviewPanel;
		private Form m_frmStrips;
		private Content m_tntStrips;
		private Form m_frmBitmaps;
		private Content m_tntBitmaps;
		private StripForm m_frmFrames;
		private Content m_tntFrames;
		private Form m_frmCombiner;
		private Content m_tntCombiner;
		private DockingManager m_dkm;
		private AnimDoc m_doc;
		private WindowContent m_wcStrips;
		private System.Windows.Forms.MenuItem mniFile;
		private System.Windows.Forms.MenuItem menuItem9;
		private System.Windows.Forms.MenuItem menuItem10;
		private System.Windows.Forms.MenuItem menuItem11;
		private System.Windows.Forms.MenuItem menuItem12;
		private System.Windows.Forms.MenuItem menuItem13;
		private System.Windows.Forms.MenuItem menuItem14;
		private System.Windows.Forms.MenuItem mniNew;
		private System.Windows.Forms.MenuItem mniOpen;
		private System.Windows.Forms.MenuItem mniSave;
		private System.Windows.Forms.MenuItem mniExit;
		private System.Windows.Forms.MenuItem mniSaveAs;
		private System.Windows.Forms.MenuItem mniClose;
		private System.Windows.Forms.MenuItem mniImport;
		private System.Windows.Forms.MenuItem mniExport;
		private System.Windows.Forms.MainMenu mnuMain;
		private System.Windows.Forms.SaveFileDialog saveFileDialog;
		private System.Windows.Forms.OpenFileDialog openFileDialog;
		private System.Windows.Forms.MenuItem mniViewBitmaps;
		private System.Windows.Forms.MenuItem mniView;
		private System.Windows.Forms.MenuItem mniViewStrips;
		private System.Windows.Forms.MenuItem mniViewTracks;
		private System.Windows.Forms.MenuItem mniTools;
		private System.Windows.Forms.MenuItem mniOptions;
		private System.Windows.Forms.OpenFileDialog importFileDialog;
		private System.Windows.Forms.MenuItem mniRepairSideColors;
		private System.Windows.Forms.MenuItem mniNormalizeBitmaps;
		private System.Windows.Forms.MenuItem mniUndo;
		private System.Windows.Forms.MenuItem mniEdit;
		private System.Windows.Forms.MenuItem mniHelp;
		private System.Windows.Forms.MenuItem mniViewCombiner;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem mniHelpAbout;
		private System.Windows.Forms.MenuItem mniWallPreview;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem mniReplaceColors;
		private System.ComponentModel.IContainer components = null;

		public MainForm(string strOpenFileName)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_dkm = new DockingManager(this, VisualStyle.IDE);
			Globals.ActiveDocumentChanged += new EventHandler(OnActiveDocumentChanged);
			m_doc = Globals.ActiveDocument;
			Globals.MainForm = this;

			// Create all the "Contents" used to display the various animation components

			m_ctlPreviewPanel = new PreviewPanel(m_doc);
			Globals.PreviewControl = m_ctlPreviewPanel.PreviewControl;
			m_ctlPreviewPanel.Dock = DockStyle.Fill;
			Controls.Add(m_ctlPreviewPanel);
			m_dkm.InnerControl = m_ctlPreviewPanel;

			m_frmStrips = new StripsForm(m_doc);
			Globals.StripsForm = m_frmStrips;
			m_tntStrips = m_dkm.Contents.Add(m_frmStrips, m_frmStrips.Text);
			m_tntStrips.DisplaySize = new Size(ClientSize.Width / 4, ClientSize.Height / 2);
			m_wcStrips = m_dkm.AddContentWithState(m_tntStrips, State.DockLeft);

			m_frmBitmaps = new BitmapsForm(m_doc);
			m_tntBitmaps = m_dkm.Contents.Add(m_frmBitmaps, m_frmBitmaps.Text);
			m_tntBitmaps.DisplaySize = new Size(ClientSize.Width / 4, ClientSize.Height / 2);
			m_dkm.AddContentWithState(m_tntBitmaps, State.DockTop);

			// Add the Bitmaps form to the StripForm's Zone

			m_dkm.AddContentToZone(m_tntBitmaps, m_wcStrips.ParentZone, 1);

			m_frmFrames = new StripForm(m_doc);
			Globals.StripForm = m_frmFrames;
			m_tntFrames = m_dkm.Contents.Add(m_frmFrames, m_frmFrames.Text);
			m_frmFrames.Content = m_tntFrames;
			int cx = ClientSize.Width - (ClientSize.Width / 4);
			int cy = ClientSize.Height / 3;
			m_tntFrames.DisplaySize = new Size(cx, cy);
			m_dkm.AddContentWithState(m_tntFrames, State.DockBottom);

			m_frmCombiner = new CombinerForm();
			m_tntCombiner = m_dkm.Contents.Add(m_frmCombiner, m_frmCombiner.Text);
			m_tntCombiner.DisplaySize = new Size(ClientSize.Width / 2, ClientSize.Height / 2);
//			m_dkm.AddContentWithState(m_tntCombiner, State.Floating);
//			m_dkm.HideContent(m_tntCombiner);

			// Do a little wiring

			((StripControl)Globals.StripControl).FrameOffsetChanged += 
					new FrameOffsetEventHandler(((PreviewControl)Globals.PreviewControl).OnFrameOffsetChanged);
			((PreviewControl)Globals.PreviewControl).FrameOffsetChanged += 
				new FrameOffsetEventHandler(((StripControl)Globals.StripControl).OnFrameOffsetChanged);

			// We always have a document around
			
			if (strOpenFileName == null)
				NewDocument();
			else
				OpenDocument(strOpenFileName);
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MainForm));
			this.mniFile = new System.Windows.Forms.MenuItem();
			this.mniNew = new System.Windows.Forms.MenuItem();
			this.mniOpen = new System.Windows.Forms.MenuItem();
			this.mniClose = new System.Windows.Forms.MenuItem();
			this.menuItem9 = new System.Windows.Forms.MenuItem();
			this.mniSave = new System.Windows.Forms.MenuItem();
			this.mniSaveAs = new System.Windows.Forms.MenuItem();
			this.menuItem10 = new System.Windows.Forms.MenuItem();
			this.mniImport = new System.Windows.Forms.MenuItem();
			this.mniExport = new System.Windows.Forms.MenuItem();
			this.menuItem14 = new System.Windows.Forms.MenuItem();
			this.menuItem13 = new System.Windows.Forms.MenuItem();
			this.menuItem12 = new System.Windows.Forms.MenuItem();
			this.menuItem11 = new System.Windows.Forms.MenuItem();
			this.mniExit = new System.Windows.Forms.MenuItem();
			this.mniOptions = new System.Windows.Forms.MenuItem();
			this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
			this.mniViewTracks = new System.Windows.Forms.MenuItem();
			this.mniViewBitmaps = new System.Windows.Forms.MenuItem();
			this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.mniViewStrips = new System.Windows.Forms.MenuItem();
			this.mnuMain = new System.Windows.Forms.MainMenu();
			this.mniEdit = new System.Windows.Forms.MenuItem();
			this.mniUndo = new System.Windows.Forms.MenuItem();
			this.mniView = new System.Windows.Forms.MenuItem();
			this.mniViewCombiner = new System.Windows.Forms.MenuItem();
			this.mniTools = new System.Windows.Forms.MenuItem();
			this.mniNormalizeBitmaps = new System.Windows.Forms.MenuItem();
			this.mniRepairSideColors = new System.Windows.Forms.MenuItem();
			this.mniWallPreview = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.mniHelp = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.mniHelpAbout = new System.Windows.Forms.MenuItem();
			this.importFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.mniReplaceColors = new System.Windows.Forms.MenuItem();
			// 
			// mniFile
			// 
			this.mniFile.Index = 0;
			this.mniFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mniNew,
																					this.mniOpen,
																					this.mniClose,
																					this.menuItem9,
																					this.mniSave,
																					this.mniSaveAs,
																					this.menuItem10,
																					this.mniImport,
																					this.mniExport,
																					this.menuItem14,
																					this.menuItem13,
																					this.menuItem12,
																					this.menuItem11,
																					this.mniExit});
			this.mniFile.Text = "&File";
			this.mniFile.Popup += new System.EventHandler(this.mniFile_Popup);
			// 
			// mniNew
			// 
			this.mniNew.Index = 0;
			this.mniNew.Shortcut = System.Windows.Forms.Shortcut.CtrlN;
			this.mniNew.Text = "&New";
			this.mniNew.Click += new System.EventHandler(this.mniNew_Click);
			// 
			// mniOpen
			// 
			this.mniOpen.Index = 1;
			this.mniOpen.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
			this.mniOpen.Text = "&Open...";
			this.mniOpen.Click += new System.EventHandler(this.mniOpen_Click);
			// 
			// mniClose
			// 
			this.mniClose.Index = 2;
			this.mniClose.Text = "&Close";
			this.mniClose.Click += new System.EventHandler(this.mniClose_Click);
			// 
			// menuItem9
			// 
			this.menuItem9.Index = 3;
			this.menuItem9.Text = "-";
			// 
			// mniSave
			// 
			this.mniSave.Index = 4;
			this.mniSave.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
			this.mniSave.Text = "&Save";
			this.mniSave.Click += new System.EventHandler(this.mniSave_Click);
			// 
			// mniSaveAs
			// 
			this.mniSaveAs.Index = 5;
			this.mniSaveAs.Text = "Save &As...";
			this.mniSaveAs.Click += new System.EventHandler(this.mniSaveAs_Click);
			// 
			// menuItem10
			// 
			this.menuItem10.Index = 6;
			this.menuItem10.Text = "-";
			// 
			// mniImport
			// 
			this.mniImport.Index = 7;
			this.mniImport.Text = "&Import...";
			this.mniImport.Click += new System.EventHandler(this.mniImport_Click);
			// 
			// mniExport
			// 
			this.mniExport.Enabled = false;
			this.mniExport.Index = 8;
			this.mniExport.Text = "&Export...";
			// 
			// menuItem14
			// 
			this.menuItem14.Index = 9;
			this.menuItem14.Text = "-";
			// 
			// menuItem13
			// 
			this.menuItem13.Enabled = false;
			this.menuItem13.Index = 10;
			this.menuItem13.Text = "Recent Strips";
			// 
			// menuItem12
			// 
			this.menuItem12.Enabled = false;
			this.menuItem12.Index = 11;
			this.menuItem12.Text = "Recent Files";
			// 
			// menuItem11
			// 
			this.menuItem11.Index = 12;
			this.menuItem11.Text = "-";
			// 
			// mniExit
			// 
			this.mniExit.Index = 13;
			this.mniExit.Text = "E&xit";
			// 
			// mniOptions
			// 
			this.mniOptions.Index = 5;
			this.mniOptions.Text = "&Options...";
			this.mniOptions.Click += new System.EventHandler(this.mniOptions_Click);
			// 
			// saveFileDialog
			// 
			this.saveFileDialog.DefaultExt = "amx";
			this.saveFileDialog.FileName = "untitled.amx";
			this.saveFileDialog.Filter = "AniMax files (*.amx)|*.amx|Zipped AniMax files (*.zamx)|*.zamx|All files|*.*";
			this.saveFileDialog.Title = "Save AniMax File";
			// 
			// mniViewTracks
			// 
			this.mniViewTracks.Index = 2;
			this.mniViewTracks.Text = "&Frames Window";
			this.mniViewTracks.Click += new System.EventHandler(this.mniViewTracks_Click);
			// 
			// mniViewBitmaps
			// 
			this.mniViewBitmaps.Index = 0;
			this.mniViewBitmaps.Text = "&Bitmaps Window";
			this.mniViewBitmaps.Click += new System.EventHandler(this.mniViewBitmaps_Click);
			// 
			// openFileDialog
			// 
			this.openFileDialog.DefaultExt = "amx";
			this.openFileDialog.Filter = "AniMax files (*.amx)|*.amx|Zipped AniMax files (*.zamx)|*.zamx|All files|*.*";
			this.openFileDialog.Title = "Open AniMax File";
			// 
			// mniViewStrips
			// 
			this.mniViewStrips.Index = 1;
			this.mniViewStrips.Text = "&Strips Window";
			this.mniViewStrips.Click += new System.EventHandler(this.mniViewStrips_Click);
			// 
			// mnuMain
			// 
			this.mnuMain.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mniFile,
																					this.mniEdit,
																					this.mniView,
																					this.mniTools,
																					this.mniHelp});
			// 
			// mniEdit
			// 
			this.mniEdit.Index = 1;
			this.mniEdit.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mniUndo});
			this.mniEdit.Text = "&Edit";
			this.mniEdit.Popup += new System.EventHandler(this.mniEdit_Popup);
			// 
			// mniUndo
			// 
			this.mniUndo.Index = 0;
			this.mniUndo.Shortcut = System.Windows.Forms.Shortcut.CtrlZ;
			this.mniUndo.Text = "&Undo";
			this.mniUndo.Click += new System.EventHandler(this.mniUndo_Click);
			// 
			// mniView
			// 
			this.mniView.Index = 2;
			this.mniView.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mniViewBitmaps,
																					this.mniViewStrips,
																					this.mniViewTracks,
																					this.mniViewCombiner});
			this.mniView.Text = "&View";
			this.mniView.Popup += new System.EventHandler(this.mniView_Popup);
			// 
			// mniViewCombiner
			// 
			this.mniViewCombiner.Index = 3;
			this.mniViewCombiner.Text = "&Combiner Window";
			this.mniViewCombiner.Click += new System.EventHandler(this.mniViewCombiner_Click);
			// 
			// mniTools
			// 
			this.mniTools.Index = 3;
			this.mniTools.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					 this.mniNormalizeBitmaps,
																					 this.mniRepairSideColors,
																					 this.mniWallPreview,
																					 this.mniReplaceColors,
																					 this.menuItem3,
																					 this.mniOptions});
			this.mniTools.Text = "&Tools";
			// 
			// mniNormalizeBitmaps
			// 
			this.mniNormalizeBitmaps.Enabled = false;
			this.mniNormalizeBitmaps.Index = 0;
			this.mniNormalizeBitmaps.Text = "Normalize Bitmaps";
			// 
			// mniRepairSideColors
			// 
			this.mniRepairSideColors.Index = 1;
			this.mniRepairSideColors.Text = "Repair Side Colors";
			this.mniRepairSideColors.Click += new System.EventHandler(this.mniRepairSideColors_Click);
			// 
			// mniWallPreview
			// 
			this.mniWallPreview.Index = 2;
			this.mniWallPreview.Text = "&Wall Preview";
			this.mniWallPreview.Click += new System.EventHandler(this.mniWallPreview_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 4;
			this.menuItem3.Text = "-";
			// 
			// mniHelp
			// 
			this.mniHelp.Index = 4;
			this.mniHelp.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.menuItem1,
																					this.mniHelpAbout});
			this.mniHelp.Text = "&Help";
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.Shortcut = System.Windows.Forms.Shortcut.F1;
			this.menuItem1.Text = "&Help";
			this.menuItem1.Click += new System.EventHandler(this.mniHelp_Click);
			// 
			// mniHelpAbout
			// 
			this.mniHelpAbout.Index = 1;
			this.mniHelpAbout.Text = "&About SpiffCode AniMax...";
			this.mniHelpAbout.Click += new System.EventHandler(this.mniHelpAbout_Click);
			// 
			// importFileDialog
			// 
			this.importFileDialog.Filter = "Bitmap files (*.bmp,*.png,*.gif,*.jpg,*.exif,*.tif)|*.bmp;*.png;*.gif;*.exif;*.jp" +
				"g|Framedata files|*.txt";
			this.importFileDialog.Multiselect = true;
			this.importFileDialog.Title = "Import";
			// 
			// mniReplaceColors
			// 
			this.mniReplaceColors.Index = 3;
			this.mniReplaceColors.Text = "Replace Colors...";
			this.mniReplaceColors.Click += new System.EventHandler(this.mniReplaceColors_Click);
			// 
			// MainForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(712, 478);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.KeyPreview = true;
			this.Menu = this.mnuMain;
			this.Name = "MainForm";
			this.Text = "AniMax";
			this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.MainForm_KeyDown);
			this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.MainForm_KeyPress);
			this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.MainForm_KeyUp);

		}
		#endregion

		private void OnActiveDocumentChanged(object obSender, EventArgs e) {
			m_doc = Globals.ActiveDocument;
			if (m_doc.FileName == null)
				Text = gstrAniMax;
			else
				Text = gstrAniMax + " - " + m_doc.FileName;
		}

		private void mniNew_Click(object sender, System.EventArgs e) {
			NewDocument();
		}

		private void NewDocument() {
			// First close the open document (if any)

			if (!CloseDocument())
				return;

			// Create a new one and show its views

			Globals.ActiveDocument = new AnimDoc(Globals.TileSize,
                    Globals.FrameRate);
			ShowViews();

			// I'm tired of being asked if I want to save untitled on exit even though I
			// haven't done anything to it.

			m_doc.Dirty = false;
		}

		private void mniSave_Click(object sender, System.EventArgs e) {
			SaveDocument();
		}

		private void mniSaveAs_Click(object sender, System.EventArgs e) {
			SaveAsDocument();
		}

		private void mniOpen_Click(object sender, System.EventArgs e) {

			// If the current doc is dirty give the user a chance to save it
			// before loading the new doc.

			if (!GiveUserChanceToSaveChangesOrCancel())
				return;

			if (openFileDialog.ShowDialog() != DialogResult.OK)
				return;

			OpenDocument(openFileDialog.FileName);
		}

		public void CloseAndOpenDocument(string strFileName) {
			if (!GiveUserChanceToSaveChangesOrCancel())
				return;

			OpenDocument(strFileName);
		}

		private void OpenDocument(string strFileName) {
			//

			Cursor.Current = Cursors.WaitCursor;
			Directory.SetCurrentDirectory(Path.GetDirectoryName(strFileName));
			AnimDoc doc = AnimDoc.Load(strFileName);
			Cursor.Current = Cursors.Arrow;
			if (doc == null) {
				MessageBox.Show(this, String.Format("Unexpected error loading {0} " +
						"or one of the bitmap files it depends on. Sorry.", strFileName), "AniMax");
				return;
			}

			Globals.ActiveDocument = doc;
			Globals.TileSize = doc.TileSize;
			Globals.ActiveStrip = doc.StripSet[0];
			ShowViews();
		}

		private void mniClose_Click(object sender, System.EventArgs e) {
			CloseDocument();
		}

		protected override void OnClosing(System.ComponentModel.CancelEventArgs e) {
			if (!CloseDocument())
				e.Cancel = true;
		}

		private bool SaveDocument() {
			if (m_doc.FileName == null) {
				return SaveAsDocument();
			} else {
				Cursor.Current = Cursors.WaitCursor;
				m_doc.Save(m_doc.FileName);
				Cursor.Current = Cursors.Arrow;
				return true;
			}
		}

		private bool SaveAsDocument() {
			saveFileDialog.FileName = m_doc.FileName;
			if (saveFileDialog.ShowDialog() != DialogResult.OK)
				return false;

			Cursor.Current = Cursors.WaitCursor;
			m_doc.Save(saveFileDialog.FileName);
			Cursor.Current = Cursors.Arrow;
			Text = "AniMax - " + m_doc.FileName;
			return true;
		}

		private bool GiveUserChanceToSaveChangesOrCancel() {
			if (m_doc.Dirty) {
				DialogResult dlgr = MessageBox.Show(this,
					String.Format("Do you want to save the changes to {0}?", m_doc.FileName), 
					"AniMax", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Exclamation);
				if (dlgr == DialogResult.Cancel)
					return false;
				else if (dlgr == DialogResult.Yes)
					return SaveDocument();
			}
			return true;
		}

		private bool CloseDocument() {
			if (!GiveUserChanceToSaveChangesOrCancel())
				return false;

			Globals.ActiveDocument = Globals.NullDocument;
			return true;
		}

		private void mniViewBitmaps_Click(object sender, System.EventArgs e) {
			ShowContent(m_tntBitmaps);
		}

		private void mniViewStrips_Click(object sender, System.EventArgs e) {
			ShowContent(m_tntStrips);
		}

		private void mniViewTracks_Click(object sender, System.EventArgs e) {
			ShowContent(m_tntFrames);
		}

		private void mniViewCombiner_Click(object sender, System.EventArgs e) {
			ShowContent(m_tntCombiner);
		}

		private void ShowViews() {
			ShowContent(m_tntStrips);
			ShowContent(m_tntBitmaps);
			ShowContent(m_tntFrames);
		}

		private void ShowContent(Content tnt) {
			if (!tnt.Visible)
				m_dkm.ShowContent(tnt);
		}

		private void mniView_Popup(object sender, System.EventArgs e) {
			bool fDocExists = m_doc != null;
			mniViewBitmaps.Enabled = fDocExists;
			mniViewStrips.Enabled = fDocExists;
			mniViewTracks.Enabled = fDocExists;
		}

		private void mniFile_Popup(object sender, System.EventArgs e) {
			bool fDocExists = m_doc != null;
			mniClose.Enabled = fDocExists;
			mniSave.Enabled = fDocExists;
			mniSaveAs.Enabled = fDocExists;
			mniExport.Enabled = fDocExists;
		}

		private void mniOptions_Click(object sender, System.EventArgs e) {
			// UNDONE: the options form includes a mishmash of items, some of which are
			// scoped to the application, some to the current document. Document options
			// should be moved to a properties dialog or something.
			// CONSIDER: ye olde PropertyGrid?
			OptionsForm frm = new OptionsForm();

			frm.FrameRate = m_doc != null ? m_doc.FrameRate : Globals.FrameRate;
			frm.GridWidth = Globals.GridWidth;
			frm.GridHeight = Globals.GridHeight;

			if (frm.ShowDialog(this) != DialogResult.OK)
				return;

			if (m_doc != null)
				m_doc.FrameRate = frm.FrameRate;
			else
				Globals.FrameRate = frm.FrameRate;
			Globals.GridWidth = frm.GridWidth;
			Globals.GridHeight = frm.GridHeight;

			frm.Dispose();
		}

		private void mniImport_Click(object sender, System.EventArgs e) {
			if (importFileDialog.ShowDialog() != DialogResult.OK)
				return;

			string[] astrFileNames = importFileDialog.FileNames;

			// If a document doesn't already exist create a new one and show its views

			if (m_doc == Globals.NullDocument) {
				m_doc = new AnimDoc(Globals.TileSize, Globals.FrameRate);
				ShowViews();
			}

			Cursor.Current = Cursors.WaitCursor;
			bool fSuccess = m_doc.Import(astrFileNames);
			Cursor.Current = Cursors.Arrow;
			if (!fSuccess)
				return;

			Globals.ActiveDocument = m_doc;
			Globals.ActiveStrip = m_doc.StripSet[0];
		}

		// We have a universal key handler that any control or child form can register
		// with to take a crack at processing the key press.

		private void MainForm_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e) {
			Globals.OnKeyPress(sender, e);
		}

		private void MainForm_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e) {
			Globals.OnKeyDown(sender, e);
		}

		private void MainForm_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e) {
			Globals.OnKeyUp(sender, e);
		}

		private void mniRepairSideColors_Click(object sender, System.EventArgs e) {
			if (m_doc.XBitmapSet.Count == 0) {
				MessageBox.Show(this, "Hey dork, no Bitmaps have been loaded yet.", "Error");
				return;
			}

			foreach (XBitmap xbm in m_doc.XBitmapSet) {
				bool fDirty = ReplaceColor(xbm.Bitmap, Color.FromArgb(0, 114, 232), Color.FromArgb(0, 116, 232), true);
				fDirty |= ReplaceColor(xbm.Bitmap, Color.FromArgb(0, 112, 232), Color.FromArgb(0, 116, 232), true);
				fDirty |= ReplaceColor(xbm.Bitmap, Color.FromArgb(0,  96, 192), Color.FromArgb(0,  96, 196), true);
				fDirty |= ReplaceColor(xbm.Bitmap, Color.FromArgb(0,  48,  88), Color.FromArgb(0,  48,  92), true);

				if (fDirty) {
					xbm.Dirty = fDirty;
					m_doc.Dirty = true;
				}
			}

			// UNDONE: this is a hack. Decide on the right way to force selective refreshes

			Globals.StripControl.Invalidate();
			Globals.PreviewControl.Invalidate();
		}
		
		#region Handy Helpers

		static public bool ReplaceColor(Bitmap bm, Color clrOld, Color clrNew, bool f6bit) {
			bool fFound = false;

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
		#endregion

		private void mniUndo_Click(object sender, System.EventArgs e) {
			UndoManager.Undo();
		}

		private void mniEdit_Popup(object sender, System.EventArgs e) {
			mniUndo.Enabled = UndoManager.AnyUndos();
		}

		private void mniHelp_Click(object sender, System.EventArgs e) {
			Process.Start("http://www.tinybit.org/AniMax");
		}

		private void mniToolsHack_Click(object sender, System.EventArgs e) {
			foreach (Strip stp in Globals.ActiveDocument.StripSet) {
				if (!stp.Name.ToLower().StartsWith("turret"))
					continue;

				foreach (Frame fr in stp) {
					Point pt = new Point(fr.SpecialPoint.X - 7, fr.SpecialPoint.Y - 3);
					fr.SpecialPoint = pt;
					foreach (BitmapPlacer plc in fr.BitmapPlacers) {
						plc.X += 7;
						plc.Y += 3;
					}
				}
			}

			// Force a redraw of everything

			Globals.ActiveDocument = Globals.ActiveDocument;
		}

		private void mniHelpAbout_Click(object sender, System.EventArgs e) {
			new AboutForm().ShowDialog(this);
		}

		private void mniWallPreview_Click(object sender, System.EventArgs e) {
			new WallPreviewForm().ShowDialog(this);
		}

		private void mniGenerateCompass_Click(object sender, System.EventArgs e) {
			Bitmap bm = new Bitmap(200, 200);
			Graphics g = Graphics.FromImage(bm);
			
			Pen pen = new Pen(Color.White);
			int xC = 100;
			int yC = 100;
			for (int i = 0; i < 16; i++) {
				double nAngle = (Math.PI * 2) * (i / 16.0);
				int xE = (int)(Math.Sin(nAngle) * 100);
				int yE = (int)(Math.Cos(nAngle) * 100);
				g.DrawLine(pen, xC, yC, xC + xE, yC + yE);
			}
			g.Dispose();
			bm.Save(@"c:\compass.png", ImageFormat.Png);
		}

		private void mniReplaceColors_Click(object sender, System.EventArgs e) {
			if (m_doc.XBitmapSet.Count == 0) {
				MessageBox.Show(this, "Hey dork, no Bitmaps have been loaded yet.", "Error");
				return;
			}

			new ReplaceColorsForm(m_doc).ShowDialog(this);
		}
	}
}
