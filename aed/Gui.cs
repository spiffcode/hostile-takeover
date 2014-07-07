using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;
using SpiffLib;
using System.Xml.Serialization;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace AED {
	/// <summary>
	/// Summary description for Gui.
	/// </summary>
	public class Gui : System.Windows.Forms.Form {
		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem7;
		private System.Windows.Forms.MenuItem menuItem9;
		private System.Windows.Forms.OpenFileDialog importFileDialog;
		private System.Windows.Forms.Splitter splt;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.TreeView trvAnimSet;
		private System.Windows.Forms.ImageList imageList1;
		private System.Windows.Forms.HScrollBar sbh;
		private System.Windows.Forms.NumericUpDown spnFrameNumber;
		private System.Windows.Forms.Button btnStop;
		private System.Windows.Forms.Button btnPlay;
		private System.Windows.Forms.Button btnLoopForward;
		private System.Windows.Forms.Panel pnlSbPbcGroup;
		private System.Windows.Forms.Panel pnlPlaybackControls;
		private System.Windows.Forms.Timer tmrAnim;
		private System.Windows.Forms.MenuItem mniShowOrigin;
		private System.Windows.Forms.MenuItem mniImport;
		private System.Windows.Forms.MenuItem mniExit;
		private System.Windows.Forms.MenuItem mniSetBackgroundColor;
		private System.Windows.Forms.TrackBar trackBar1;
		private System.Windows.Forms.SaveFileDialog saveFileDialog;
		private System.Windows.Forms.MenuItem mniSaveAs;
		private System.Windows.Forms.MenuItem mniOpen;
		private System.Windows.Forms.MenuItem mniSave;
		private System.Windows.Forms.MenuItem mniExport;
		private System.Windows.Forms.SaveFileDialog exportFileDialog;
		private System.Windows.Forms.OpenFileDialog openPaletteFileDialog;
		private System.Windows.Forms.MenuItem mniFile;
		private System.Windows.Forms.Button btnShiftUp;
		private System.Windows.Forms.Button btnShiftDown;
		private System.Windows.Forms.Button btnShiftLeft;
		private System.Windows.Forms.Button btnShiftRight;
		private System.Windows.Forms.ToolTip ttip;
		private System.Windows.Forms.Label lblFrameInfo;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem mniEditScript;
		private System.Windows.Forms.MenuItem mniRunScript;
		private System.Windows.Forms.ContextMenu mnuTreeView;
		private System.Windows.Forms.MenuItem mniTreeViewProperties;
		private System.Windows.Forms.MenuItem mniOptions;
		private System.Windows.Forms.MenuItem mniShowGrid;
		private System.Windows.Forms.RadioButton rbtnA;
		private System.Windows.Forms.RadioButton rbtnB;
		private System.Windows.Forms.MenuItem mniShowBOverA;
		private System.Windows.Forms.Label lblTest;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.MenuItem mniMapSideColors;
		private System.Windows.Forms.Label lblFPS;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.NumericUpDown nudFrameRate;
		private System.Windows.Forms.MenuItem mniSetBackgroundBitmap;
		private System.Windows.Forms.OpenFileDialog openBitmapDialog;
		private System.ComponentModel.IContainer components;

		/// <summary>
		/// 
		/// </summary>
		public Gui(AnimSet anis) {
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			// Non-Designer initialization

			m_strAedDir = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
			try {
				// UNDONE: problems with this approach to settings:
				// 1. proper location of .config file? Documents and Settings?
				// 2. default values
				// 3. maintenance of Settings class
				// 4. what happens if member isn't present?
				// 5. order-dependent?
				// 6. XML file less readable than .ini
				// 7. more code/maintenance than ini.write(m_nScale), etc w/ overloads
				// 8. doesn't handle Color and other types
				TextReader trdr = new StreamReader(m_strAedDir + @"\AED.config");
				XmlSerializer xser = new XmlSerializer(typeof(Settings));
				Settings settings = (Settings)xser.Deserialize(trdr);

				m_nScale = settings.nPreviewSize;
				m_fShowOrigin = settings.fShowOrigin;
				m_fShowGrid = settings.fShowGrid;
				m_fShowBOverA = settings.fShowBOverA;
				m_clrBackground = Color.FromArgb(settings.nArgbBackground);
				m_fMapSideColors = settings.fMapSideColors;
				// UNDONE: default frame rate
				trdr.Close();
			} catch {
			}

			// Initialize frame rate dependent components

			SetFrameRate(80);

			// UNDONE: doesn't work because panel1 can't accept Focus
			panel1.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.panel1_MouseWheel);
			trackBar1.Value = m_nScale;
			SetPreviewPanelBackColor(m_clrBackground);

			if (anis != null) {
				m_anis = anis;
				ResetTreeView();
				ShowFirstFrame();
			}

			// Script stuff

			m_se = new ScriptEngine();
			m_se.ScriptDone += new EventHandler(OnScriptDone);
			m_se.AddGlobal("AED", this);
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing ) {
			if( disposing ) {
				if(components != null) {
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
		private void InitializeComponent() {
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Gui));
			this.mnuTreeView = new System.Windows.Forms.ContextMenu();
			this.mniTreeViewProperties = new System.Windows.Forms.MenuItem();
			this.menuItem9 = new System.Windows.Forms.MenuItem();
			this.mniSetBackgroundColor = new System.Windows.Forms.MenuItem();
			this.spnFrameNumber = new System.Windows.Forms.NumericUpDown();
			this.menuItem7 = new System.Windows.Forms.MenuItem();
			this.btnStop = new System.Windows.Forms.Button();
			this.imageList1 = new System.Windows.Forms.ImageList(this.components);
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.mniEditScript = new System.Windows.Forms.MenuItem();
			this.mniRunScript = new System.Windows.Forms.MenuItem();
			this.exportFileDialog = new System.Windows.Forms.SaveFileDialog();
			this.sbh = new System.Windows.Forms.HScrollBar();
			this.mniOpen = new System.Windows.Forms.MenuItem();
			this.mniSave = new System.Windows.Forms.MenuItem();
			this.importFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.mniFile = new System.Windows.Forms.MenuItem();
			this.mniSaveAs = new System.Windows.Forms.MenuItem();
			this.mniImport = new System.Windows.Forms.MenuItem();
			this.mniExport = new System.Windows.Forms.MenuItem();
			this.mniExit = new System.Windows.Forms.MenuItem();
			this.trackBar1 = new System.Windows.Forms.TrackBar();
			this.ttip = new System.Windows.Forms.ToolTip(this.components);
			this.btnShiftUp = new System.Windows.Forms.Button();
			this.btnShiftDown = new System.Windows.Forms.Button();
			this.btnPlay = new System.Windows.Forms.Button();
			this.btnLoopForward = new System.Windows.Forms.Button();
			this.btnShiftRight = new System.Windows.Forms.Button();
			this.btnShiftLeft = new System.Windows.Forms.Button();
			this.trvAnimSet = new System.Windows.Forms.TreeView();
			this.rbtnB = new System.Windows.Forms.RadioButton();
			this.rbtnA = new System.Windows.Forms.RadioButton();
			this.nudFrameRate = new System.Windows.Forms.NumericUpDown();
			this.mniOptions = new System.Windows.Forms.MenuItem();
			this.mniShowOrigin = new System.Windows.Forms.MenuItem();
			this.mniShowGrid = new System.Windows.Forms.MenuItem();
			this.mniMapSideColors = new System.Windows.Forms.MenuItem();
			this.mniShowBOverA = new System.Windows.Forms.MenuItem();
			this.mniSetBackgroundBitmap = new System.Windows.Forms.MenuItem();
			this.pnlSbPbcGroup = new System.Windows.Forms.Panel();
			this.pnlPlaybackControls = new System.Windows.Forms.Panel();
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.openPaletteFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.splt = new System.Windows.Forms.Splitter();
			this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
			this.tmrAnim = new System.Windows.Forms.Timer(this.components);
			this.panel1 = new System.Windows.Forms.Panel();
			this.lblTest = new System.Windows.Forms.Label();
			this.lblFrameInfo = new System.Windows.Forms.Label();
			this.panel2 = new System.Windows.Forms.Panel();
			this.label1 = new System.Windows.Forms.Label();
			this.lblFPS = new System.Windows.Forms.Label();
			this.openBitmapDialog = new System.Windows.Forms.OpenFileDialog();
			((System.ComponentModel.ISupportInitialize)(this.spnFrameNumber)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.trackBar1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nudFrameRate)).BeginInit();
			this.pnlSbPbcGroup.SuspendLayout();
			this.pnlPlaybackControls.SuspendLayout();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// mnuTreeView
			// 
			this.mnuTreeView.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						this.mniTreeViewProperties});
			// 
			// mniTreeViewProperties
			// 
			this.mniTreeViewProperties.Index = 0;
			this.mniTreeViewProperties.Text = "Properties...";
			this.mniTreeViewProperties.Click += new System.EventHandler(this.mniTreeViewProperties_Click);
			// 
			// menuItem9
			// 
			this.menuItem9.Index = 6;
			this.menuItem9.Text = "-";
			// 
			// mniSetBackgroundColor
			// 
			this.mniSetBackgroundColor.Index = 4;
			this.mniSetBackgroundColor.Text = "Set &Background Color...";
			this.mniSetBackgroundColor.Click += new System.EventHandler(this.mniSetBackgroundColor_Click);
			// 
			// spnFrameNumber
			// 
			this.spnFrameNumber.Location = new System.Drawing.Point(74, 3);
			this.spnFrameNumber.Name = "spnFrameNumber";
			this.spnFrameNumber.Size = new System.Drawing.Size(50, 20);
			this.spnFrameNumber.TabIndex = 2;
			this.spnFrameNumber.TabStop = false;
			this.ttip.SetToolTip(this.spnFrameNumber, "Frame number");
			this.spnFrameNumber.ValueChanged += new System.EventHandler(this.spnFrameNumber_ValueChanged);
			// 
			// menuItem7
			// 
			this.menuItem7.Index = 3;
			this.menuItem7.Text = "-";
			// 
			// btnStop
			// 
			this.btnStop.Image = ((System.Drawing.Bitmap)(resources.GetObject("btnStop.Image")));
			this.btnStop.ImageIndex = 0;
			this.btnStop.ImageList = this.imageList1;
			this.btnStop.Name = "btnStop";
			this.btnStop.Size = new System.Drawing.Size(24, 24);
			this.btnStop.TabIndex = 1;
			this.btnStop.TabStop = false;
			this.ttip.SetToolTip(this.btnStop, "Stop playing");
			this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
			// 
			// imageList1
			// 
			this.imageList1.ColorDepth = System.Windows.Forms.ColorDepth.Depth8Bit;
			this.imageList1.ImageSize = new System.Drawing.Size(16, 16);
			this.imageList1.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList1.ImageStream")));
			this.imageList1.TransparentColor = System.Drawing.Color.White;
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 2;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mniEditScript,
																					  this.mniRunScript});
			this.menuItem1.Text = "Script";
			// 
			// mniEditScript
			// 
			this.mniEditScript.Index = 0;
			this.mniEditScript.Text = "&Edit Script...";
			this.mniEditScript.Click += new System.EventHandler(this.mniEditScript_Click);
			// 
			// mniRunScript
			// 
			this.mniRunScript.Index = 1;
			this.mniRunScript.Text = "&Run Script";
			// 
			// exportFileDialog
			// 
			this.exportFileDialog.AddExtension = false;
			this.exportFileDialog.FileName = "no file name needed, click Save";
			this.exportFileDialog.Filter = "Directories|*.nomatchesplease";
			this.exportFileDialog.Title = "Export";
			this.exportFileDialog.ValidateNames = false;
			// 
			// sbh
			// 
			this.sbh.Dock = System.Windows.Forms.DockStyle.Fill;
			this.sbh.LargeChange = 1;
			this.sbh.Name = "sbh";
			this.sbh.Size = new System.Drawing.Size(301, 24);
			this.sbh.TabIndex = 2;
			this.ttip.SetToolTip(this.sbh, "Frame selector");
			this.sbh.Scroll += new System.Windows.Forms.ScrollEventHandler(this.sbh_Scroll);
			// 
			// mniOpen
			// 
			this.mniOpen.Enabled = false;
			this.mniOpen.Index = 0;
			this.mniOpen.Text = "&Open...";
			// 
			// mniSave
			// 
			this.mniSave.Index = 1;
			this.mniSave.Text = "&Save";
			this.mniSave.Click += new System.EventHandler(this.mniSave_Click);
			// 
			// importFileDialog
			// 
			this.importFileDialog.Filter = "Bitmap files (*.bmp,*.png,*.gif,*.jpg,*.exif,*.tif)|*.bmp;*.png;*.gif;*.exif;*.jp" +
				"g;*.tif|All files (*.*)|*.*";
			this.importFileDialog.Multiselect = true;
			this.importFileDialog.Title = "Import";
			// 
			// mniFile
			// 
			this.mniFile.Index = 0;
			this.mniFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mniOpen,
																					this.mniSave,
																					this.mniSaveAs,
																					this.menuItem7,
																					this.mniImport,
																					this.mniExport,
																					this.menuItem9,
																					this.mniExit});
			this.mniFile.Text = "&File";
			this.mniFile.Popup += new System.EventHandler(this.mniFile_Popup);
			// 
			// mniSaveAs
			// 
			this.mniSaveAs.Index = 2;
			this.mniSaveAs.Text = "Save &As...";
			this.mniSaveAs.Click += new System.EventHandler(this.mniSaveAs_Click);
			// 
			// mniImport
			// 
			this.mniImport.Index = 4;
			this.mniImport.Text = "&Import...";
			this.mniImport.Click += new System.EventHandler(this.mniImport_Click);
			// 
			// mniExport
			// 
			this.mniExport.Index = 5;
			this.mniExport.Text = "&Export...";
			this.mniExport.Click += new System.EventHandler(this.mniExport_Click);
			// 
			// mniExit
			// 
			this.mniExit.Index = 7;
			this.mniExit.Text = "E&xit";
			this.mniExit.Click += new System.EventHandler(this.mniExit_Click);
			// 
			// trackBar1
			// 
			this.trackBar1.Location = new System.Drawing.Point(0, 8);
			this.trackBar1.Maximum = 20;
			this.trackBar1.Minimum = 1;
			this.trackBar1.Name = "trackBar1";
			this.trackBar1.Size = new System.Drawing.Size(96, 45);
			this.trackBar1.TabIndex = 5;
			this.trackBar1.TickFrequency = 2;
			this.trackBar1.TickStyle = System.Windows.Forms.TickStyle.TopLeft;
			this.ttip.SetToolTip(this.trackBar1, "Zoom level");
			this.trackBar1.Value = 2;
			this.trackBar1.Scroll += new System.EventHandler(this.trackBar1_Scroll);
			// 
			// btnShiftUp
			// 
			this.btnShiftUp.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btnShiftUp.Location = new System.Drawing.Point(35, 56);
			this.btnShiftUp.Name = "btnShiftUp";
			this.btnShiftUp.Size = new System.Drawing.Size(24, 23);
			this.btnShiftUp.TabIndex = 6;
			this.btnShiftUp.TabStop = false;
			this.btnShiftUp.Text = "▲";
			this.ttip.SetToolTip(this.btnShiftUp, "Shift frame up relative to origin point");
			this.btnShiftUp.Click += new System.EventHandler(this.btnShiftUp_Click);
			// 
			// btnShiftDown
			// 
			this.btnShiftDown.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btnShiftDown.Location = new System.Drawing.Point(36, 104);
			this.btnShiftDown.Name = "btnShiftDown";
			this.btnShiftDown.Size = new System.Drawing.Size(24, 23);
			this.btnShiftDown.TabIndex = 6;
			this.btnShiftDown.TabStop = false;
			this.btnShiftDown.Text = "▼";
			this.ttip.SetToolTip(this.btnShiftDown, "Shift frame down relative to origin point");
			this.btnShiftDown.Click += new System.EventHandler(this.btnShiftDown_Click);
			// 
			// btnPlay
			// 
			this.btnPlay.BackColor = System.Drawing.SystemColors.Control;
			this.btnPlay.Image = ((System.Drawing.Bitmap)(resources.GetObject("btnPlay.Image")));
			this.btnPlay.ImageIndex = 1;
			this.btnPlay.ImageList = this.imageList1;
			this.btnPlay.Location = new System.Drawing.Point(24, 0);
			this.btnPlay.Name = "btnPlay";
			this.btnPlay.Size = new System.Drawing.Size(24, 24);
			this.btnPlay.TabIndex = 1;
			this.btnPlay.TabStop = false;
			this.ttip.SetToolTip(this.btnPlay, "Play once");
			this.btnPlay.Click += new System.EventHandler(this.btnPlay_Click);
			// 
			// btnLoopForward
			// 
			this.btnLoopForward.Image = ((System.Drawing.Bitmap)(resources.GetObject("btnLoopForward.Image")));
			this.btnLoopForward.ImageIndex = 2;
			this.btnLoopForward.ImageList = this.imageList1;
			this.btnLoopForward.Location = new System.Drawing.Point(48, 0);
			this.btnLoopForward.Name = "btnLoopForward";
			this.btnLoopForward.Size = new System.Drawing.Size(24, 24);
			this.btnLoopForward.TabIndex = 1;
			this.btnLoopForward.TabStop = false;
			this.ttip.SetToolTip(this.btnLoopForward, "Play looped");
			this.btnLoopForward.Click += new System.EventHandler(this.btnLoopForward_Click);
			// 
			// btnShiftRight
			// 
			this.btnShiftRight.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btnShiftRight.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.btnShiftRight.Location = new System.Drawing.Point(59, 80);
			this.btnShiftRight.Name = "btnShiftRight";
			this.btnShiftRight.Size = new System.Drawing.Size(24, 23);
			this.btnShiftRight.TabIndex = 6;
			this.btnShiftRight.TabStop = false;
			this.btnShiftRight.Text = "►";
			this.ttip.SetToolTip(this.btnShiftRight, "Shift frame right relative to origin point");
			this.btnShiftRight.Click += new System.EventHandler(this.btnShiftRight_Click);
			// 
			// btnShiftLeft
			// 
			this.btnShiftLeft.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btnShiftLeft.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.btnShiftLeft.Location = new System.Drawing.Point(11, 80);
			this.btnShiftLeft.Name = "btnShiftLeft";
			this.btnShiftLeft.Size = new System.Drawing.Size(24, 23);
			this.btnShiftLeft.TabIndex = 6;
			this.btnShiftLeft.TabStop = false;
			this.btnShiftLeft.Text = "◄";
			this.ttip.SetToolTip(this.btnShiftLeft, "Shift frame left relative to origin point");
			this.btnShiftLeft.Click += new System.EventHandler(this.btnShiftLeft_Click);
			// 
			// trvAnimSet
			// 
			this.trvAnimSet.Dock = System.Windows.Forms.DockStyle.Left;
			this.trvAnimSet.FullRowSelect = true;
			this.trvAnimSet.HideSelection = false;
			this.trvAnimSet.ImageIndex = -1;
			this.trvAnimSet.Indent = 15;
			this.trvAnimSet.Name = "trvAnimSet";
			this.trvAnimSet.SelectedImageIndex = -1;
			this.trvAnimSet.ShowLines = false;
			this.trvAnimSet.Size = new System.Drawing.Size(80, 461);
			this.trvAnimSet.TabIndex = 1;
			this.ttip.SetToolTip(this.trvAnimSet, "FrameSet selector");
			this.trvAnimSet.MouseDown += new System.Windows.Forms.MouseEventHandler(this.trvAnimSet_MouseDown);
			this.trvAnimSet.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.trvAnimSet_AfterSelect);
			// 
			// rbtnB
			// 
			this.rbtnB.Appearance = System.Windows.Forms.Appearance.Button;
			this.rbtnB.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.rbtnB.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.rbtnB.Location = new System.Drawing.Point(56, 136);
			this.rbtnB.Name = "rbtnB";
			this.rbtnB.Size = new System.Drawing.Size(24, 24);
			this.rbtnB.TabIndex = 10;
			this.rbtnB.Text = "B";
			this.rbtnB.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.ttip.SetToolTip(this.rbtnB, "Switch to AnimSet B");
			this.rbtnB.CheckedChanged += new System.EventHandler(this.rbtnB_CheckedChanged);
			// 
			// rbtnA
			// 
			this.rbtnA.Appearance = System.Windows.Forms.Appearance.Button;
			this.rbtnA.Checked = true;
			this.rbtnA.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.rbtnA.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.rbtnA.Location = new System.Drawing.Point(16, 136);
			this.rbtnA.Name = "rbtnA";
			this.rbtnA.Size = new System.Drawing.Size(24, 24);
			this.rbtnA.TabIndex = 9;
			this.rbtnA.TabStop = true;
			this.rbtnA.Text = "A";
			this.rbtnA.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.ttip.SetToolTip(this.rbtnA, "Switch to AnimSet A");
			this.rbtnA.CheckedChanged += new System.EventHandler(this.rbtnA_CheckedChanged);
			// 
			// nudFrameRate
			// 
			this.nudFrameRate.CausesValidation = false;
			this.nudFrameRate.Increment = new System.Decimal(new int[] {
																		   10,
																		   0,
																		   0,
																		   0});
			this.nudFrameRate.Location = new System.Drawing.Point(0, 352);
			this.nudFrameRate.Maximum = new System.Decimal(new int[] {
																		 1000,
																		 0,
																		 0,
																		 0});
			this.nudFrameRate.Minimum = new System.Decimal(new int[] {
																		 16,
																		 0,
																		 0,
																		 0});
			this.nudFrameRate.Name = "nudFrameRate";
			this.nudFrameRate.Size = new System.Drawing.Size(48, 20);
			this.nudFrameRate.TabIndex = 15;
			this.ttip.SetToolTip(this.nudFrameRate, "Playback Rate (in milliseconds per frame)");
			this.nudFrameRate.Value = new System.Decimal(new int[] {
																	   80,
																	   0,
																	   0,
																	   0});
			this.nudFrameRate.ValueChanged += new System.EventHandler(this.nudFrameRate_ValueChanged);
			// 
			// mniOptions
			// 
			this.mniOptions.Index = 1;
			this.mniOptions.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					   this.mniShowOrigin,
																					   this.mniShowGrid,
																					   this.mniMapSideColors,
																					   this.mniShowBOverA,
																					   this.mniSetBackgroundColor,
																					   this.mniSetBackgroundBitmap});
			this.mniOptions.Text = "&Options";
			this.mniOptions.Popup += new System.EventHandler(this.mniOptions_Popup);
			// 
			// mniShowOrigin
			// 
			this.mniShowOrigin.Index = 0;
			this.mniShowOrigin.Text = "Show &Origin Point";
			this.mniShowOrigin.Click += new System.EventHandler(this.mniShowOrigin_Click);
			// 
			// mniShowGrid
			// 
			this.mniShowGrid.Index = 1;
			this.mniShowGrid.Text = "Show 16x16 &Grid";
			this.mniShowGrid.Click += new System.EventHandler(this.mniShowGrid_Click);
			// 
			// mniMapSideColors
			// 
			this.mniMapSideColors.Index = 2;
			this.mniMapSideColors.Text = "Map &Side Colors";
			this.mniMapSideColors.Click += new System.EventHandler(this.mniMapSideColors_Click);
			// 
			// mniShowBOverA
			// 
			this.mniShowBOverA.Index = 3;
			this.mniShowBOverA.Text = "Show B over A";
			this.mniShowBOverA.Click += new System.EventHandler(this.mniShowBOverA_Click);
			// 
			// mniSetBackgroundBitmap
			// 
			this.mniSetBackgroundBitmap.Index = 5;
			this.mniSetBackgroundBitmap.Text = "Set Background Bitmap...";
			this.mniSetBackgroundBitmap.Click += new System.EventHandler(this.mniSetBackgroundBitmap_Click);
			// 
			// pnlSbPbcGroup
			// 
			this.pnlSbPbcGroup.Controls.AddRange(new System.Windows.Forms.Control[] {
																						this.sbh,
																						this.pnlPlaybackControls});
			this.pnlSbPbcGroup.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.pnlSbPbcGroup.Location = new System.Drawing.Point(0, 437);
			this.pnlSbPbcGroup.Name = "pnlSbPbcGroup";
			this.pnlSbPbcGroup.Size = new System.Drawing.Size(429, 24);
			this.pnlSbPbcGroup.TabIndex = 4;
			// 
			// pnlPlaybackControls
			// 
			this.pnlPlaybackControls.Controls.AddRange(new System.Windows.Forms.Control[] {
																							  this.spnFrameNumber,
																							  this.btnStop,
																							  this.btnPlay,
																							  this.btnLoopForward});
			this.pnlPlaybackControls.Dock = System.Windows.Forms.DockStyle.Right;
			this.pnlPlaybackControls.Location = new System.Drawing.Point(301, 0);
			this.pnlPlaybackControls.Name = "pnlPlaybackControls";
			this.pnlPlaybackControls.Size = new System.Drawing.Size(128, 24);
			this.pnlPlaybackControls.TabIndex = 3;
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mniFile,
																					  this.mniOptions,
																					  this.menuItem1});
			// 
			// openPaletteFileDialog
			// 
			this.openPaletteFileDialog.DefaultExt = "pal";
			this.openPaletteFileDialog.Filter = "Jasc Palette (*.pal)|*.pal";
			this.openPaletteFileDialog.RestoreDirectory = true;
			this.openPaletteFileDialog.Title = "Specify a Palette to map to";
			// 
			// splt
			// 
			this.splt.BackColor = System.Drawing.SystemColors.ActiveBorder;
			this.splt.Location = new System.Drawing.Point(80, 0);
			this.splt.Name = "splt";
			this.splt.Size = new System.Drawing.Size(3, 461);
			this.splt.TabIndex = 3;
			this.splt.TabStop = false;
			// 
			// saveFileDialog
			// 
			this.saveFileDialog.DefaultExt = "ani";
			this.saveFileDialog.FileName = "untitled.ani";
			this.saveFileDialog.Filter = "AED files (*.ani)|*.ani|All files (*.*)|*.*";
			this.saveFileDialog.Title = "Save File As";
			// 
			// tmrAnim
			// 
			this.tmrAnim.Interval = 80;
			this.tmrAnim.Tick += new System.EventHandler(this.tmrAnim_Tick);
			// 
			// panel1
			// 
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.pnlSbPbcGroup});
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(83, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(429, 461);
			this.panel1.TabIndex = 4;
			this.panel1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.panel1_MouseUp);
			this.panel1.Paint += new System.Windows.Forms.PaintEventHandler(this.panel1_Paint);
			this.panel1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.panel1_MouseMove);
			this.panel1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.panel1_MouseDown);
			// 
			// lblTest
			// 
			this.lblTest.Location = new System.Drawing.Point(0, 264);
			this.lblTest.Name = "lblTest";
			this.lblTest.Size = new System.Drawing.Size(104, 40);
			this.lblTest.TabIndex = 11;
			this.lblTest.UseMnemonic = false;
			// 
			// lblFrameInfo
			// 
			this.lblFrameInfo.BackColor = System.Drawing.SystemColors.Control;
			this.lblFrameInfo.Font = new System.Drawing.Font("Comic Sans MS", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblFrameInfo.Location = new System.Drawing.Point(0, 176);
			this.lblFrameInfo.Name = "lblFrameInfo";
			this.lblFrameInfo.Size = new System.Drawing.Size(96, 80);
			this.lblFrameInfo.TabIndex = 7;
			this.lblFrameInfo.UseMnemonic = false;
			// 
			// panel2
			// 
			this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel2.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.nudFrameRate,
																				 this.label1,
																				 this.lblFPS,
																				 this.btnShiftRight,
																				 this.btnShiftLeft,
																				 this.btnShiftUp,
																				 this.btnShiftDown,
																				 this.trackBar1,
																				 this.rbtnB,
																				 this.rbtnA,
																				 this.lblFrameInfo,
																				 this.lblTest});
			this.panel2.Dock = System.Windows.Forms.DockStyle.Right;
			this.panel2.Location = new System.Drawing.Point(512, 0);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(96, 461);
			this.panel2.TabIndex = 5;
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(0, 328);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(100, 24);
			this.label1.TabIndex = 14;
			this.label1.Text = "Playback Rate (msec/frame):";
			this.label1.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// lblFPS
			// 
			this.lblFPS.Font = new System.Drawing.Font("Microsoft Sans Serif", 6.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblFPS.Location = new System.Drawing.Point(46, 355);
			this.lblFPS.Name = "lblFPS";
			this.lblFPS.Size = new System.Drawing.Size(96, 16);
			this.lblFPS.TabIndex = 13;
			this.lblFPS.Text = "(12.5 FPS)";
			// 
			// openBitmapDialog
			// 
			this.openBitmapDialog.Filter = "Bitmap files (*.bmp,*.png,*.gif,*.jpg,*.exif,*.tif)|*.bmp;*.png;*.gif;*.exif;*.jp" +
				"g;*.tif|All files (*.*)|*.*";
			this.openBitmapDialog.Multiselect = true;
			this.openBitmapDialog.Title = "Import";
			// 
			// Gui
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(608, 461);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panel1,
																		  this.panel2,
																		  this.splt,
																		  this.trvAnimSet});
			this.KeyPreview = true;
			this.Menu = this.mainMenu1;
			this.Name = "Gui";
			this.Text = "AED";
			this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.Gui_KeyPress);
			this.Closed += new System.EventHandler(this.Gui_Closed);
			((System.ComponentModel.ISupportInitialize)(this.spnFrameNumber)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.trackBar1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nudFrameRate)).EndInit();
			this.pnlSbPbcGroup.ResumeLayout(false);
			this.pnlPlaybackControls.ResumeLayout(false);
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		//
		// Non-Windows.Forms state
		//

		ToggleableState m_ts = new ToggleableState();
		private AnimSet m_anis;
		private int m_nScale = 2;
		private bool m_fLoop = false;
		private bool m_fShowOrigin = false;
		private bool m_fShowGrid = false;
		private bool m_fShowBOverA = false;
		private bool m_fMapSideColors = true;
		private Color m_clrBackground = Color.FromKnownColor(KnownColor.Beige);
		private int[] m_aiCustomColors = null;
#if false
		private string m_strPaletteFileName = null;
#endif
		private Palette m_pal = null;
		private ScriptEngine m_se;
		private string m_strScript = null;
		private ArrayList m_alstFrames = new ArrayList();
		private TreeNode m_trnMenued = null;
		private string m_strAedDir = null;
		private string m_strSaveFileName;
		private int m_cmsFrameRate;
		private Bitmap m_bmBackground = null;
		private Point m_ptBackgroundOffset;
		private Point m_ptDragStart, m_ptPreDragBackgroundOffset;
		private bool m_fDraggingBackground = false;
		
		//
		// Public Properties
		//

		/// <summary>
		/// 
		/// </summary>
		public ArrayList Frames {
			get {
				return m_alstFrames;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public Frame ActiveFrame {
			get {
				return m_ts.ActiveFrameSet[m_ts.ActiveFrameIndex];
			}
		}

		//
		// Public Methods
		//

		/// <summary>
		/// 
		/// </summary>
		/// <param name="ob"></param>
		public void ShowProperties(object ob) {
			PropertyInspector prpi = new PropertyInspector(ob);
			prpi.PropertyValueChanged += new PropertyValueChangedEventHandler(OnPropertyValueChanged);
			prpi.Show();
		}

		//
		// UI event handlers
		//

		private void trvAnimSet_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e) {
			TreeNode trn = e.Node;
			if (trn.Parent == null) {
				trn = trn.FirstNode;
				trvAnimSet.SelectedNode = trn;
			}
			SetActiveFrameSet(trn.Parent.Text, trn.Text);
		}

		private void btnPlay_Click(object sender, System.EventArgs e) {
			m_ts.ActiveFrameIndex = 0;
			m_fLoop = false;
			tmrAnim.Start();
		}

		private void btnStop_Click(object sender, System.EventArgs e) {
			tmrAnim.Stop();
			m_fLoop = false;
		}

		private void btnLoopForward_Click(object sender, System.EventArgs e) {
			m_fLoop = !m_fLoop;
			if (m_fLoop)
				tmrAnim.Start();
			else
				tmrAnim.Stop();
		}

		private void tmrAnim_Tick(object sender, System.EventArgs e) {
			if (m_ts.ActiveFrameSet == null)
				return;

			int ifrm = m_ts.ActiveFrameIndex + 1;
			if (ifrm >= m_ts.ActiveFrameSet.Count) {
				ifrm = 0;
				if (!m_fLoop)
					tmrAnim.Stop();
			}

			SetActiveFrame(ifrm);
		}

		private void Gui_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e) {
			e.Handled = true;

			switch (e.KeyChar) {
			case '[':
				if (m_ts.ActiveFrameSet != null) {
					int ifrm = m_ts.ActiveFrameIndex - 1;
					if (ifrm < 0)
						ifrm = m_ts.ActiveFrameSet.Count - 1;
					SetActiveFrame(ifrm);
				}
				break;

			case ']':
				if (m_ts.ActiveFrameSet != null) {
					int ifrm = m_ts.ActiveFrameIndex + 1;
					if (ifrm >= m_ts.ActiveFrameSet.Count)
						ifrm = 0;
					SetActiveFrame(ifrm);
				}
				break;

			default:
				e.Handled = false;
				break;
			}
		}

		//
		// Menu event handlers
		//

		private void mniShowGrid_Click(object sender, System.EventArgs e) {
			m_fShowGrid = !m_fShowGrid;
			PaintFrame();
		}

		private void mniFile_Popup(object sender, System.EventArgs e) {
			bool fHaveAnimSet = m_anis != null;
			mniSaveAs.Enabled = fHaveAnimSet;
			mniSave.Enabled = fHaveAnimSet;
			mniExport.Enabled = fHaveAnimSet;
		}

		private void mniImport_Click(object sender, System.EventArgs e) {
			if (importFileDialog.ShowDialog() != DialogResult.OK)
				return;

			string[] astrFileNames = importFileDialog.FileNames;
			if (astrFileNames == null)
				return;

			m_ts = new ToggleableState();

			m_anis = new AnimSet();
			if (!AED.Import(m_anis, astrFileNames))
				return;

			rbtnA.Checked = true;
			//			rbtnB.Checked = false;

			ResetTreeView();
			UpdateFrameList();
			ShowFirstFrame();
		}

		void ResetTreeView() {
			trvAnimSet.Nodes.Clear();

			foreach (DictionaryEntry deAnimSet in m_anis.Items) {
				Anim ani = (Anim)deAnimSet.Value;
				TreeNode trn = new TreeNode(ani.Name);
				trvAnimSet.Nodes.Add(trn);

				// Sort the framesets using a numeric sort

				FrameSet[] afrms = new FrameSet[ani.Items.Count];

				int i = 0;
				foreach (DictionaryEntry deAnim in ani.Items)
					afrms[i++] = (FrameSet)deAnim.Value;

				Array.Sort(afrms, new FrameSetComparer());

				// Add the framesets to the tree

				foreach (FrameSet frms in afrms) {
					trn.Nodes.Add(new TreeNode(frms.Name));
				}
				trn.Expand();
			}
		}

		void ShowFirstFrame() {
			if (m_anis.Items.Count == 0)
				return;

			Anim aniT = m_anis[0];
			SetActiveFrameSet(aniT.Name, aniT[0].Name);
			trvAnimSet.SelectedNode = trvAnimSet.Nodes[0].FirstNode;
		}

		private void mniExit_Click(object sender, System.EventArgs e) {
			Close();
		}

		private void mniShowOrigin_Click(object sender, System.EventArgs e) {
			m_fShowOrigin = !m_fShowOrigin;
			PaintFrame();
		}

		private void mniSetBackgroundColor_Click(object sender, System.EventArgs e) {
			ColorDialog dlgColor = new ColorDialog();
			dlgColor.CustomColors = m_aiCustomColors;
			dlgColor.Color = m_clrBackground;
			dlgColor.ShowDialog();
			SetPreviewPanelBackColor(dlgColor.Color);
			m_aiCustomColors = dlgColor.CustomColors;
			PaintFrame();
		}

		private void mniSaveAs_Click(object sender, System.EventArgs e) {
			if (saveFileDialog.ShowDialog() != DialogResult.OK)
				return;
			if (!AED.SaveAs(m_anis, saveFileDialog.FileName)) {
				MessageBox.Show("AED.SaveAs failed");
				return;
			}

			m_strSaveFileName = saveFileDialog.FileName;
		}

		private void mniSave_Click(object sender, System.EventArgs e) {
			if (m_strSaveFileName == null) {
				mniSaveAs_Click(sender, e);
				return;
			}

			if (!AED.SaveAs(m_anis, m_strSaveFileName))
				MessageBox.Show("AED.SaveAs failed");
		}

		private void mniExport_Click(object sender, System.EventArgs e) {
			if (exportFileDialog.ShowDialog() != DialogResult.OK)
				return;
			string strExportDir = Path.GetDirectoryName(exportFileDialog.FileName);
#if false
			if (m_strPaletteFileName == null) {
				if (openPaletteFileDialog.ShowDialog() != DialogResult.OK)
					return;
				m_strPaletteFileName = openPaletteFileDialog.FileName;
				m_pal = new Palette(m_strPaletteFileName);
			}
#endif

			AED.Export(m_anis, strExportDir, m_pal, false, false);
		}
		
		private void sbh_Scroll(object sender, System.Windows.Forms.ScrollEventArgs e) {
			SetActiveFrame(e.NewValue, false);
		}

		private void panel1_Paint(object sender, System.Windows.Forms.PaintEventArgs e) {
			PaintFrame();
		}

		private void panel1_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e) {
			int nSize = m_nScale + e.Delta;
			if (nSize < 1)
				nSize = 1;
			if (nSize != m_nScale) {
				m_nScale = nSize;
				PaintFrame();
			}
		}

		private void spnFrameNumber_ValueChanged(object sender, System.EventArgs e) {
			SetActiveFrame(System.Convert.ToInt32(spnFrameNumber.Value));
		}

		private void trackBar1_Scroll(object sender, System.EventArgs e) {
			m_nScale = trackBar1.Value;
			PaintFrame();
		}

		// Origin shifting buttons

		private void btnShiftUp_Click(object sender, System.EventArgs e) {
			if (m_ts.ActiveFrameSet == null)
				return;

			if ((ModifierKeys & Keys.Shift) != 0) {
				foreach (Frame frm in m_ts.ActiveFrameSet)
					frm.OriginY++;
			} else {
				m_ts.ActiveFrameSet[m_ts.ActiveFrameIndex].OriginY++;
			}
			PaintFrame();
		}

		private void btnShiftDown_Click(object sender, System.EventArgs e) {
			if (m_ts.ActiveFrameSet == null)
				return;

			if ((ModifierKeys & Keys.Shift) != 0) {
				foreach (Frame frm in m_ts.ActiveFrameSet)
					frm.OriginY--;
			} else {
				m_ts.ActiveFrameSet[m_ts.ActiveFrameIndex].OriginY--;
			}
			PaintFrame();
		}

		private void btnShiftLeft_Click(object sender, System.EventArgs e) {
			if (m_ts.ActiveFrameSet == null)
				return;

			if ((ModifierKeys & Keys.Shift) != 0) {
				foreach (Frame frm in m_ts.ActiveFrameSet)
					frm.OriginX++;
			} else {
				m_ts.ActiveFrameSet[m_ts.ActiveFrameIndex].OriginX++;
			}
			PaintFrame();
		}

		private void btnShiftRight_Click(object sender, System.EventArgs e) {
			if (m_ts.ActiveFrameSet == null)
				return;

			if ((ModifierKeys & Keys.Shift) != 0) {
				foreach (Frame frm in m_ts.ActiveFrameSet)
					frm.OriginX--;
			} else {
				m_ts.ActiveFrameSet[m_ts.ActiveFrameIndex].OriginX--;
			}
			PaintFrame();
		}

		//
		// Script event handlers
		//

		private void OnScriptDone(object obSender, EventArgs e) {
			PaintFrame();
		}

		private void OnScriptEditorClosing(object obSender, EventArgs e) {
			m_strScript = ((ScriptEditor)obSender).Script;
		}

		//
		// Everything else
		//

		private Point WxyFromFxy(Point ptFrame) {
			Rectangle rcClient = GetAnimViewRect();
			int xCenter = rcClient.Width / 2;
			int yCenter = rcClient.Height / 2;

			Bitmap bmSrc = m_ts.ActiveFrameSet[m_ts.ActiveFrameIndex].Bitmap;

			return new Point(xCenter - ((bmSrc.Width / 2) * m_nScale) + (ptFrame.X * m_nScale),
					yCenter - ((bmSrc.Height / 2) * m_nScale) + (ptFrame.Y * m_nScale));
		}

		private Point FxyFromWxy(Point ptWindow) {
			Rectangle rcClient = GetAnimViewRect();
			int xCenter = rcClient.Width / 2;
			int yCenter = rcClient.Height / 2;

			int dx = ptWindow.X - xCenter;
			if (dx < 0)
				dx -= m_nScale;
			int dy = ptWindow.Y - yCenter;
			if (dy < 0)
				dy -= m_nScale;
			return new Point(dx / m_nScale, dy / m_nScale);
		}

		private void PaintFrame() {
			Rectangle rcClient = GetAnimViewRect();
			int xCenter = rcClient.Width / 2;
			int yCenter = rcClient.Height / 2;

			int cxT = ((rcClient.Width + m_nScale - 1) / m_nScale) + 2;
			int cyT = ((rcClient.Height + m_nScale - 1) / m_nScale) + 2;
			int xCenterT = cxT / 2;
			int yCenterT = cyT / 2;

			// NOTE: these 'using' statements (a 'shortcut' for calling .Dispose()) are 
			// absolutely necessary or we chew up all virtual memory while animating

			using (Graphics g = panel1.CreateGraphics()) {

				// Create a temporary bitmap for compositing the grid, frames, origin indicator, etc into

				using (Bitmap bmT = new Bitmap(cxT, cyT)) {

					using (Graphics gT = Graphics.FromImage(bmT)) {
						gT.Clear(m_clrBackground);
						
						// Draw background bitmap, if any

						if (m_bmBackground != null)
							gT.DrawImage(m_bmBackground, xCenterT - (m_bmBackground.Width / 2) + m_ptBackgroundOffset.X,
									yCenterT - (m_bmBackground.Height / 2) + m_ptBackgroundOffset.Y, 
									m_bmBackground.Width, m_bmBackground.Height);

						// Draw grid (if enabled)
						// UNDONE: use alpha to draw grid (e.g., brighten or darken)

						if (m_fShowGrid) {
							Brush br = new SolidBrush(Color.FromKnownColor(KnownColor.LightGray));
							for (int x = xCenterT % 16; x < cxT; x += 16)
								gT.FillRectangle(br, x, 0, 1, cyT);

							for (int y = yCenterT % 16; y < cyT; y += 16)
								gT.FillRectangle(br, 0, y, cxT, 1);
						}

						BitmapData bmdDst = bmT.LockBits(new Rectangle(0, 0, bmT.Width, bmT.Height), 
								ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);

						if (m_ts.FrameSetArray[0] != null && (m_ts.Active == 0 || m_fShowBOverA)) {
							Frame frm = m_ts.FrameSetArray[0][m_ts.FrameIndexArray[0]];
							Bitmap bmSrc = frm.Bitmap;

							BitmapData bmdSrc = bmSrc.LockBits(new Rectangle(0, 0, bmSrc.Width, bmSrc.Height), 
								ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);

							SuperBlt(bmdSrc, 0, 0, bmdDst, xCenterT - frm.OriginX, yCenterT - frm.OriginY, bmSrc.Width, bmSrc.Height, m_fMapSideColors);

							bmSrc.UnlockBits(bmdSrc);
						}

						if (m_ts.FrameSetArray[1] != null && (m_ts.Active == 1 || m_fShowBOverA)) {
							Frame frm = m_ts.FrameSetArray[1][m_ts.FrameIndexArray[1]];
							Bitmap bmSrc = frm.Bitmap;

							BitmapData bmdSrc = bmSrc.LockBits(new Rectangle(0, 0, bmSrc.Width, bmSrc.Height), 
								ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);

							SuperBlt(bmdSrc, 0, 0, bmdDst, xCenterT - frm.OriginX, yCenterT - frm.OriginY, bmSrc.Width, bmSrc.Height, m_fMapSideColors);

							bmSrc.UnlockBits(bmdSrc);
						}

						bmT.UnlockBits(bmdDst);

						// Draw origin point (if enabled)

						if (m_fShowOrigin)
							bmT.SetPixel(xCenterT, yCenterT, Color.FromKnownColor(KnownColor.Orange));

						// Force a nice simple, fast old-school stretchblt

						g.InterpolationMode = InterpolationMode.NearestNeighbor;

						// NOTE: _without_ this the first row and column are only scaled by half!

						g.PixelOffsetMode = PixelOffsetMode.Half;

						// StretchBlt the temporary composite to the screen
						
						g.DrawImage(bmT, rcClient.Left - ((xCenterT * m_nScale) - xCenter), 
							rcClient.Top - ((yCenterT * m_nScale) - yCenter), 
							cxT * m_nScale, cyT * m_nScale);
					}
				}
			}
		}

		struct RgbData {
			public RgbData(byte bRed, byte bGreen, byte bBlue) {
				this.bRed = bRed;
				this.bGreen = bGreen;
				this.bBlue = bBlue;
			}

			public byte bBlue;
			public byte bGreen;
			public byte bRed;
		}

		static RgbData[] argbSide = { 
			new RgbData(232, 32, 0), 
			new RgbData(196, 28, 0),
			new RgbData(128, 8, 0),
			new RgbData(92, 8, 0),
			new RgbData(64, 8, 0)
		};

		// Skips dst where src has transparent color.
		// Darkens dst where src has shadow color.
		// Translates side colors.
		// NOTE: Performs dst but not src clipping!!!
		// NOTE: Assumes src and dst BitmapData are PixelFormat.Format24bppRgb

		private unsafe void SuperBlt(BitmapData bmdSrc, int xSrc, int ySrc, BitmapData bmdDst, int xDst, int yDst, int cx, int cy, bool fMapSideColors) {

			// If completely off dst bounds, just return.

			if ((xDst >= bmdDst.Width || xDst + cx < 0) || (yDst >= bmdDst.Height) || (yDst + cy < 0))
				return;

			// Dst clip

			if (xDst + cx > bmdDst.Width)
				cx = bmdDst.Width - xDst;
			if (yDst + cy > bmdDst.Height)
				cy = bmdDst.Height - yDst;

			if (xDst < 0) {
				cx += xDst;
				xSrc -= xDst; 
				xDst = 0;
			}

			if (yDst < 0) {
				cy += yDst;
				ySrc -= yDst;
				yDst = 0;
			}

			RgbData* prgbSrc = (RgbData*)((byte*)bmdSrc.Scan0 + (ySrc * bmdSrc.Stride) + (xSrc * sizeof(RgbData)));
			RgbData* prgbDst = (RgbData*)((byte*)bmdDst.Scan0 + (yDst * bmdDst.Stride) + (xDst * sizeof(RgbData)));

			while (cy-- > 0) {
				RgbData* prgbDstT = prgbDst;
				RgbData* prgbSrcT = prgbSrc;

				for (int x = 0; x < cx; x++) {
					RgbData rgbSrc = *prgbSrcT++;

					// Handle shadow color

					if (rgbSrc.bRed == 156 && rgbSrc.bGreen == 212 & rgbSrc.bBlue == 248) {
						prgbDstT->bRed = (byte)((prgbDstT->bRed * 60) / 100);
						prgbDstT->bGreen = (byte)((prgbDstT->bGreen * 60) / 100);
						prgbDstT->bBlue = (byte)((prgbDstT->bBlue * 60) / 100);
						prgbDstT++;

					// Handle transparent color

					} else if (rgbSrc.bRed == 255 && rgbSrc.bGreen == 0 && rgbSrc.bBlue == 255) {
						prgbDstT++;

					// Handle side colors

					} else if (fMapSideColors) {
						if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 116 && rgbSrc.bBlue == 232) {
							*prgbDstT++ = argbSide[0];
						} else if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 96 && rgbSrc.bBlue == 196) {
							*prgbDstT++ = argbSide[1];
						} else if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 64 && rgbSrc.bBlue == 120) {
							*prgbDstT++ = argbSide[2];
						} else if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 48 && rgbSrc.bBlue == 92) {
							*prgbDstT++ = argbSide[3];
						} else if (rgbSrc.bRed == 0 && rgbSrc.bGreen == 32 && rgbSrc.bBlue == 64) {
							*prgbDstT++ = argbSide[4];
						} else {
							*prgbDstT++ = rgbSrc;
						}

					// Just copy everything else unaltered

					} else {
						*prgbDstT++ = rgbSrc;
					}
				}

				// Advance to next scan line

				prgbDst = (RgbData*)(((byte*)prgbDst) + bmdDst.Stride);
				prgbSrc = (RgbData*)(((byte*)prgbSrc) + bmdSrc.Stride);
			}
		}

		private void SetFrameRate(int cms) {
			m_cmsFrameRate = cms;
			tmrAnim.Interval = m_cmsFrameRate;
			nudFrameRate.Value = m_cmsFrameRate;
			lblFPS.Text = string.Format("({0:###.#} FPS)", 1000.0f / m_cmsFrameRate);
		}

		private void SetActiveFrameSet(string strAniName, string strFrameSetName) {
			m_ts.ActiveAnim = m_anis[strAniName];
			m_ts.ActiveFrameSet = m_ts.ActiveAnim[strFrameSetName];
			OnActiveFrameSetChanged();
		}

		private void SetActiveFrame(int ifrm) {
			SetActiveFrame(ifrm, true);
		}

		private void SetActiveFrame(int ifrm, bool fSetScrollbar) {
			m_ts.ActiveFrameIndex = ifrm;
			OnActiveFrameChanged(fSetScrollbar);
		}

		private void OnActiveFrameSetChanged() {
			int nMax;
			if (m_ts.ActiveFrameSet == null)
				nMax = 0;
			else
				nMax = m_ts.ActiveFrameSet.Count - 1;
			sbh.Minimum = 0;
			sbh.Maximum = nMax;
			spnFrameNumber.Maximum = nMax;
			SetActiveFrame(m_ts.ActiveFrameIndex);
		}

		private void OnActiveFrameChanged(bool fSetScrollbar) {
			if (fSetScrollbar)
				sbh.Value = m_ts.ActiveFrameIndex;
			spnFrameNumber.Value = m_ts.ActiveFrameIndex;
			PaintFrame();
			if (m_ts.ActiveFrameSet != null) {
				Bitmap bm = m_ts.ActiveFrameSet[m_ts.ActiveFrameIndex].Bitmap;
				lblFrameInfo.Text = string.Format("Anim: {0}\nSet: {1}\nFrame: {2} of {3}\n({4}x{5})",
					m_ts.ActiveAnim.Name, m_ts.ActiveFrameSet.Name, m_ts.ActiveFrameIndex + 1, m_ts.ActiveFrameSet.Count, bm.Width, bm.Height);
			} else {
				lblFrameInfo.Text = "";
			}
		}

		private Rectangle GetAnimViewRect() {
			return new Rectangle(0, 0, panel1.Width, panel1.Height - sbh.Height);
		}

		private void mniEditScript_Click(object sender, System.EventArgs e) {
			ScriptEditor se = new ScriptEditor(m_se, m_strScript);
			se.ScriptEditorClosing += new EventHandler(this.OnScriptEditorClosing);
			se.Show();
		}

		private void UpdateFrameList() {
			m_alstFrames.Clear();

			foreach (DictionaryEntry deAnimSet in m_anis.Items) {
				Anim ani = (Anim)deAnimSet.Value;
				foreach (DictionaryEntry deAnim in ani.Items) {
					FrameSet frms = (FrameSet)deAnim.Value;
					foreach (Frame frm in frms) {
						m_alstFrames.Add(frm);
					}
				}
			}
		}

		private void trvAnimSet_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button != MouseButtons.Right)
				return;
			m_trnMenued = trvAnimSet.GetNodeAt(e.X, e.Y);
			trvAnimSet.SelectedNode = m_trnMenued;
			mnuTreeView.Show(trvAnimSet, new Point(e.X, e.Y));
		}

		// PropertyInspector stuff

		private void mniTreeViewProperties_Click(object sender, System.EventArgs e) {
			ShowProperties(ActiveFrame);
		}

		private void OnPropertyValueChanged(object obSender, PropertyValueChangedEventArgs ea) {
			PaintFrame();
		}

		//

		private void mniOptions_Popup(object sender, System.EventArgs e) {
			mniShowOrigin.Checked = m_fShowOrigin;
			mniShowGrid.Checked = m_fShowGrid;
			mniShowBOverA.Checked = m_fShowBOverA;
			mniMapSideColors.Checked = m_fMapSideColors;
		}

		private void Gui_Closed(object sender, System.EventArgs e) {
			TextWriter twtr = new StreamWriter(m_strAedDir + @"\AED.config");
			XmlSerializer xser = new XmlSerializer(typeof(Settings));
			Settings settings = new Settings();
			settings.nPreviewSize = m_nScale;
			settings.fShowOrigin = m_fShowOrigin;
			settings.fShowGrid = m_fShowGrid;
			settings.nArgbBackground = m_clrBackground.ToArgb();
			settings.fShowBOverA = m_fShowBOverA;
			settings.fMapSideColors = m_fMapSideColors;
			xser.Serialize(twtr, settings);
			twtr.Close();
		}

		private void SetPreviewPanelBackColor(Color clr) {
			m_clrBackground = clr;
		}

		private void mniShowBOverA_Click(object sender, System.EventArgs e) {
			m_fShowBOverA = !m_fShowBOverA;
			PaintFrame();
		}

		private void rbtnA_CheckedChanged(object sender, System.EventArgs e) {
			if (rbtnA.Checked) {
				m_ts.Activate(0);
				OnActiveFrameSetChanged();
			}
		}

		private void rbtnB_CheckedChanged(object sender, System.EventArgs e) {
			if (rbtnB.Checked) {
				m_ts.Activate(1);
				OnActiveFrameSetChanged();
			}
		}

		private void panel1_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
			if ((ModifierKeys & Keys.Shift) != 0) {
				m_fDraggingBackground = true;
				m_ptPreDragBackgroundOffset = m_ptBackgroundOffset;
				m_ptDragStart = FxyFromWxy(new Point(e.X, e.Y));
			}
		}

		private void panel1_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e) {
			if (m_fDraggingBackground)
				m_fDraggingBackground = false;
		}

		private void panel1_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e) {
			Point ptFrame = FxyFromWxy(new Point(e.X, e.Y));
			if (m_fDraggingBackground) {
				m_ptBackgroundOffset.X = m_ptPreDragBackgroundOffset.X + ptFrame.X - m_ptDragStart.X;
				m_ptBackgroundOffset.Y = m_ptPreDragBackgroundOffset.Y + ptFrame.Y - m_ptDragStart.Y;
				PaintFrame();
			}

			if (m_ts.ActiveFrameSet == null)
				return;

			FrameSet frms = m_ts.ActiveFrameSet;
			Frame frm = frms[m_ts.ActiveFrameIndex];
			Point ptBitmap = new Point(ptFrame.X + frm.OriginX, ptFrame.Y + frm.OriginY);
			Bitmap bm = frm.Bitmap;
			Color clr;
			if (ptBitmap.X < 0 || ptBitmap.X >= bm.Width || ptBitmap.Y < 0 || ptBitmap.Y >= bm.Height)
				clr = Color.Black;
			else
				clr = bm.GetPixel(ptBitmap.X, ptBitmap.Y);
			lblTest.Text = string.Format("x: {0}, y: {1}\nrgb: {2}, {3}, {4}", ptFrame.X, ptFrame.Y, clr.R, clr.G, clr.B);

		}

		private void mniMapSideColors_Click(object sender, System.EventArgs e) {
			m_fMapSideColors = !m_fMapSideColors;
			PaintFrame();
		}

		private void nudFrameRate_ValueChanged(object sender, System.EventArgs e) {
			SetFrameRate((int)nudFrameRate.Value);
		}

		private void nudFrameRate_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e) {
			SetFrameRate((int)nudFrameRate.Value);
		}

		private void mniSetBackgroundBitmap_Click(object sender, System.EventArgs e) {
			if (openBitmapDialog.ShowDialog() != DialogResult.OK)
				return;

			if (openBitmapDialog.FileName == null)
				return;

			m_ts = new ToggleableState();

			m_bmBackground = new Bitmap(openBitmapDialog.FileName);
			PaintFrame();
		}

		class ToggleableState { // ts
			public void Activate(int i) {
				m_iActive = i;

				if (m_aani[i] == null)
					m_aani[i] = m_aani[i ^ 1];
				if (m_afrms[i] == null) {
					m_afrms[i] = m_afrms[i ^ 1];
					m_aifrm[i] = m_aifrm[i ^ 1];
				}
			}

			public int Active {
				get {
					return m_iActive;
				}
			}

			public Anim ActiveAnim {
				get {
					return m_aani[m_iActive];
				}
				set {
					m_aani[m_iActive] = value;
				}
			}

			public FrameSet ActiveFrameSet {
				get {
					return m_afrms[m_iActive];
				}
				set {
					m_afrms[m_iActive] = value;
				}
			}

			public int ActiveFrameIndex {
				get {
					return m_aifrm[m_iActive];
				}
				set {
					m_aifrm[m_iActive] = value;
				}
			}

			public Anim[] AnimArray {
				get {
					return m_aani;
				}
			}

			public FrameSet[] FrameSetArray {
				get {
					return m_afrms;
				}
			}

			public int[] FrameIndexArray {
				get {
					return m_aifrm;
				}
			}

			private Anim[] m_aani = new Anim[2];
			private FrameSet[] m_afrms = new FrameSet[2];
			private int[] m_aifrm = new int[2];
			private int m_iActive = 0;
		}
	}

	/// <summary>
	/// 
	/// </summary>
	public class FrameSetComparer : IComparer {

		/// <summary>
		/// 
		/// </summary>
		/// <param name="obA"></param>
		/// <param name="obB"></param>
		/// <returns></returns>
		public int Compare(object obA, object obB) {
			FrameSet frmsA = (FrameSet)obA;
			FrameSet frmsB = (FrameSet)obB;

			try {
				int nA = Int32.Parse(frmsA.Name);
				int nB = Int32.Parse(frmsB.Name);
				return nA - nB;
			} catch (FormatException) {
				return frmsA.Name.CompareTo(frmsB.Name);
			}
		}
	}

	/// <summary>
	/// 
	/// </summary>
	public class Settings {
		/// <summary>
		/// 
		/// </summary>
		public int nVersion;

		/// <summary>
		/// 
		/// </summary>
		public int nPreviewSize;

		/// <summary>
		/// 
		/// </summary>
		public bool fShowOrigin;

		/// <summary>
		/// 
		/// </summary>
		public int nArgbBackground;

		/// <summary>
		/// 
		/// </summary>
		public bool fShowGrid;

		/// <summary>
		/// 
		/// </summary>
		public bool fShowBOverA;

		/// <summary>
		/// 
		/// </summary>
		public bool fMapSideColors;
	}
}
