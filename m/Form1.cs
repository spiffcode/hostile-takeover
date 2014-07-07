using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Resources;
using System.IO;
using System.Diagnostics;	// for Process
using SpiffLib;

namespace m
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItemFile;
		private System.Windows.Forms.MenuItem menuItemExit;
		private System.ComponentModel.IContainer components = null;
		private System.Windows.Forms.MenuItem menuItemOpenLevelDoc;
		private System.Windows.Forms.MenuItem menuItemSaveLevelDoc;
		private System.Windows.Forms.MenuItem menuItemSaveLevelDocAs;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItemNewLevelDoc;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Panel panel4;
		private System.Windows.Forms.Panel panelLeft;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem menuItemLevelDocProperties;
		private System.Windows.Forms.MenuItem menuItemSide1Properties;
		private System.Windows.Forms.MenuItem menuItemSide2Properties;
		private System.Windows.Forms.MenuItem menuItemSide3Properties;
		private System.Windows.Forms.MenuItem menuItemSide4Properties;
		private System.Windows.Forms.MenuItem menuItemSaveTileMapBitmap;
		private System.Windows.Forms.MenuItem menuItemSaveLevelBitmap;
		private System.Windows.Forms.Splitter splitterMapRight;
		private System.Windows.Forms.Panel panelRight;
		private System.Windows.Forms.PropertyGrid propertyGrid1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Panel panel6;
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.Splitter splitterMapLeft;
		private System.Windows.Forms.MenuItem menuItemClose;
		private System.Windows.Forms.MenuItem menuItem6;
		private System.Windows.Forms.MenuItem menuItemWindow;
		private System.Windows.Forms.MenuItem menuItemCascade;
		private System.Windows.Forms.MenuItem menuItemTileHorizontal;
		private System.Windows.Forms.MenuItem menuItemTileVertical;
		private m.GobPanel ctlGobPanel;
		private m.TemplatePanel ctlTemplatePanel;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItemCut;
		private System.Windows.Forms.MenuItem menuItemCopy;
		private System.Windows.Forms.MenuItem menuItemPaste;
		private System.Windows.Forms.MenuItem menuItemDelete;
		private System.Windows.Forms.MenuItem menuItemRenameTemplates;
		private System.Windows.Forms.MenuItem menuItemTest;
		private System.Windows.Forms.MenuItem menuItemTriggers;
		private System.Windows.Forms.MenuItem menuItemUnitGroups;
		private System.Windows.Forms.MenuItem menuItem7;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem menuItemComments;
		private System.Windows.Forms.MenuItem menuItem9;
		private System.Windows.Forms.MenuItem menuItemRun;
		private System.Windows.Forms.MenuItem menuItem8;
		private System.Windows.Forms.MenuItem menuItemSwitches;
		private System.Windows.Forms.MenuItem menuItemCounters;
		private System.Windows.Forms.MenuItem menuItemText;
		private System.Windows.Forms.MenuItem menuItemValidate;
		private System.Windows.Forms.MenuItem menuItemMisc;
		private System.Windows.Forms.MenuItem menuItem10;
		private System.Windows.Forms.StatusBar statusBar1;
		private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.MenuItem menuItemAbout;
		private System.Windows.Forms.Splitter splitterGobTiles;
		private System.Windows.Forms.MenuItem menuItemHelp;
		private System.Windows.Forms.MenuItem menuItem11;
		private System.Windows.Forms.MenuItem menuItem12;
		string m_strFileSettings = null;

		public MainForm()
		{
			new Globals();
			new DocManager();

			DocManager.SetFrameParent(this);
			DocManager.AddTemplate(new LevelDocTemplate(typeof(LevelFrame), typeof(LevelView)));
			DocManager.AddTemplate(new TemplateDocTemplate());

			// Load plug-ins. Plugins are .dll assemblies containing a class named "Plugin" 
			// that implements the IPlugin interface. Plugin dlls must be placed in the same
			// directory as the m.exe

#if STRAFER
			try {
				// UNDONE: hardwired for now. Could enumerate assemblies with a certain extension
				// for maximum extensibility convenience.

				IPlugin plug = (IPlugin)Activator.CreateInstance(@"sidewinder", "m.Plugin").Unwrap();
				Globals.Plugins.Add(plug);
			} catch {}
#endif

			// Set Kit early

			Globals.InitKit();

			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			Globals.PropertyGrid = propertyGrid1;
			Globals.StatusBar = statusBar1;

			// Load settings

			LoadSettings();

			if (Globals.IsKit()) {
				// Remove stuff we don't want in the kit.

				mainMenu1.MenuItems.Remove(menuItemMisc);
			}

			// After the main form and its menus are initialized we give plugins an opportunity
			// to modify them.

			foreach (IPlugin plug in Globals.Plugins) {
				plug.HackMenus(mainMenu1);
			}
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing ) {
			if (disposing) {
				if (components != null)
					components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent() {
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MainForm));
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItemFile = new System.Windows.Forms.MenuItem();
			this.menuItemNewLevelDoc = new System.Windows.Forms.MenuItem();
			this.menuItemOpenLevelDoc = new System.Windows.Forms.MenuItem();
			this.menuItemClose = new System.Windows.Forms.MenuItem();
			this.menuItem6 = new System.Windows.Forms.MenuItem();
			this.menuItemSaveLevelDoc = new System.Windows.Forms.MenuItem();
			this.menuItemSaveLevelDocAs = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.menuItem10 = new System.Windows.Forms.MenuItem();
			this.menuItemExit = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.menuItemCut = new System.Windows.Forms.MenuItem();
			this.menuItemCopy = new System.Windows.Forms.MenuItem();
			this.menuItemPaste = new System.Windows.Forms.MenuItem();
			this.menuItemDelete = new System.Windows.Forms.MenuItem();
			this.menuItem4 = new System.Windows.Forms.MenuItem();
			this.menuItemTriggers = new System.Windows.Forms.MenuItem();
			this.menuItemUnitGroups = new System.Windows.Forms.MenuItem();
			this.menuItemSwitches = new System.Windows.Forms.MenuItem();
			this.menuItemCounters = new System.Windows.Forms.MenuItem();
			this.menuItemText = new System.Windows.Forms.MenuItem();
			this.menuItem7 = new System.Windows.Forms.MenuItem();
			this.menuItemLevelDocProperties = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.menuItemSide1Properties = new System.Windows.Forms.MenuItem();
			this.menuItemSide2Properties = new System.Windows.Forms.MenuItem();
			this.menuItemSide3Properties = new System.Windows.Forms.MenuItem();
			this.menuItemSide4Properties = new System.Windows.Forms.MenuItem();
			this.menuItem9 = new System.Windows.Forms.MenuItem();
			this.menuItemComments = new System.Windows.Forms.MenuItem();
			this.menuItemMisc = new System.Windows.Forms.MenuItem();
			this.menuItemValidate = new System.Windows.Forms.MenuItem();
			this.menuItemRun = new System.Windows.Forms.MenuItem();
			this.menuItem8 = new System.Windows.Forms.MenuItem();
			this.menuItemSaveTileMapBitmap = new System.Windows.Forms.MenuItem();
			this.menuItemSaveLevelBitmap = new System.Windows.Forms.MenuItem();
			this.menuItemRenameTemplates = new System.Windows.Forms.MenuItem();
			this.menuItemWindow = new System.Windows.Forms.MenuItem();
			this.menuItemCascade = new System.Windows.Forms.MenuItem();
			this.menuItemTileHorizontal = new System.Windows.Forms.MenuItem();
			this.menuItemTileVertical = new System.Windows.Forms.MenuItem();
			this.menuItem5 = new System.Windows.Forms.MenuItem();
			this.menuItemHelp = new System.Windows.Forms.MenuItem();
			this.menuItemAbout = new System.Windows.Forms.MenuItem();
			this.menuItemTest = new System.Windows.Forms.MenuItem();
			this.splitterMapLeft = new System.Windows.Forms.Splitter();
			this.panelLeft = new System.Windows.Forms.Panel();
			this.panel1 = new System.Windows.Forms.Panel();
			this.panel4 = new System.Windows.Forms.Panel();
			this.ctlTemplatePanel = new m.TemplatePanel();
			this.splitterGobTiles = new System.Windows.Forms.Splitter();
			this.ctlGobPanel = new m.GobPanel();
			this.splitterMapRight = new System.Windows.Forms.Splitter();
			this.panelRight = new System.Windows.Forms.Panel();
			this.statusBar1 = new System.Windows.Forms.StatusBar();
			this.propertyGrid1 = new System.Windows.Forms.PropertyGrid();
			this.label1 = new System.Windows.Forms.Label();
			this.panel6 = new System.Windows.Forms.Panel();
			this.pictureBox1 = new System.Windows.Forms.PictureBox();
			this.menuItem11 = new System.Windows.Forms.MenuItem();
			this.menuItem12 = new System.Windows.Forms.MenuItem();
			this.panelLeft.SuspendLayout();
			this.panel1.SuspendLayout();
			this.panel4.SuspendLayout();
			this.panelRight.SuspendLayout();
			this.panel6.SuspendLayout();
			this.SuspendLayout();
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItemFile,
																					  this.menuItem1,
																					  this.menuItem4,
																					  this.menuItemMisc,
																					  this.menuItemWindow,
																					  this.menuItem5,
																					  this.menuItemTest});
			// 
			// menuItemFile
			// 
			this.menuItemFile.Index = 0;
			this.menuItemFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						 this.menuItemNewLevelDoc,
																						 this.menuItemOpenLevelDoc,
																						 this.menuItemClose,
																						 this.menuItem6,
																						 this.menuItemSaveLevelDoc,
																						 this.menuItemSaveLevelDocAs,
																						 this.menuItem2,
																						 this.menuItem12,
																						 this.menuItem11,
																						 this.menuItem10,
																						 this.menuItemExit});
			this.menuItemFile.Text = "&File";
			// 
			// menuItemNewLevelDoc
			// 
			this.menuItemNewLevelDoc.Index = 0;
			this.menuItemNewLevelDoc.Shortcut = System.Windows.Forms.Shortcut.CtrlN;
			this.menuItemNewLevelDoc.Text = "&New";
			this.menuItemNewLevelDoc.Click += new System.EventHandler(this.menuItemNewLevelDoc_Click);
			// 
			// menuItemOpenLevelDoc
			// 
			this.menuItemOpenLevelDoc.Index = 1;
			this.menuItemOpenLevelDoc.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
			this.menuItemOpenLevelDoc.Text = "&Open...";
			this.menuItemOpenLevelDoc.Click += new System.EventHandler(this.menuItemOpenLevelDoc_Click);
			// 
			// menuItemClose
			// 
			this.menuItemClose.Index = 2;
			this.menuItemClose.Text = "&Close";
			this.menuItemClose.Click += new System.EventHandler(this.menuItemClose_Click);
			// 
			// menuItem6
			// 
			this.menuItem6.Index = 3;
			this.menuItem6.Text = "-";
			// 
			// menuItemSaveLevelDoc
			// 
			this.menuItemSaveLevelDoc.Index = 4;
			this.menuItemSaveLevelDoc.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
			this.menuItemSaveLevelDoc.Text = "&Save";
			this.menuItemSaveLevelDoc.Click += new System.EventHandler(this.menuItemSaveLevelDoc_Click);
			// 
			// menuItemSaveLevelDocAs
			// 
			this.menuItemSaveLevelDocAs.Index = 5;
			this.menuItemSaveLevelDocAs.Text = "&Save As...";
			this.menuItemSaveLevelDocAs.Click += new System.EventHandler(this.menuItemSaveLevelDocAs_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 6;
			this.menuItem2.Text = "-";
			// 
			// menuItem10
			// 
			this.menuItem10.Index = 9;
			this.menuItem10.Text = "-";
			// 
			// menuItemExit
			// 
			this.menuItemExit.Index = 10;
			this.menuItemExit.Text = "&Exit";
			this.menuItemExit.Click += new System.EventHandler(this.menuItemExit_Click);
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 1;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItemCut,
																					  this.menuItemCopy,
																					  this.menuItemPaste,
																					  this.menuItemDelete});
			this.menuItem1.Text = "&Edit";
			// 
			// menuItemCut
			// 
			this.menuItemCut.Index = 0;
			this.menuItemCut.Shortcut = System.Windows.Forms.Shortcut.CtrlX;
			this.menuItemCut.Text = "Cu&t";
			this.menuItemCut.Click += new System.EventHandler(this.menuItemCut_Click);
			// 
			// menuItemCopy
			// 
			this.menuItemCopy.Index = 1;
			this.menuItemCopy.Shortcut = System.Windows.Forms.Shortcut.CtrlC;
			this.menuItemCopy.Text = "&Copy";
			this.menuItemCopy.Click += new System.EventHandler(this.menuItemCopy_Click);
			// 
			// menuItemPaste
			// 
			this.menuItemPaste.Index = 2;
			this.menuItemPaste.Shortcut = System.Windows.Forms.Shortcut.CtrlP;
			this.menuItemPaste.Text = "&Paste";
			this.menuItemPaste.Click += new System.EventHandler(this.menuItemPaste_Click);
			// 
			// menuItemDelete
			// 
			this.menuItemDelete.Index = 3;
			this.menuItemDelete.Shortcut = System.Windows.Forms.Shortcut.Del;
			this.menuItemDelete.Text = "&Delete";
			this.menuItemDelete.Click += new System.EventHandler(this.menuItemDelete_Click);
			// 
			// menuItem4
			// 
			this.menuItem4.Index = 2;
			this.menuItem4.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItemTriggers,
																					  this.menuItemUnitGroups,
																					  this.menuItemSwitches,
																					  this.menuItemCounters,
																					  this.menuItemText,
																					  this.menuItem7,
																					  this.menuItemLevelDocProperties,
																					  this.menuItem3,
																					  this.menuItemSide1Properties,
																					  this.menuItemSide2Properties,
																					  this.menuItemSide3Properties,
																					  this.menuItemSide4Properties,
																					  this.menuItem9,
																					  this.menuItemComments});
			this.menuItem4.Text = "&Settings";
			// 
			// menuItemTriggers
			// 
			this.menuItemTriggers.Index = 0;
			this.menuItemTriggers.Shortcut = System.Windows.Forms.Shortcut.CtrlT;
			this.menuItemTriggers.Text = "T&riggers...";
			this.menuItemTriggers.Click += new System.EventHandler(this.menuItemTriggers_Click);
			// 
			// menuItemUnitGroups
			// 
			this.menuItemUnitGroups.Index = 1;
			this.menuItemUnitGroups.Shortcut = System.Windows.Forms.Shortcut.CtrlG;
			this.menuItemUnitGroups.Text = "Unit &Groups...";
			this.menuItemUnitGroups.Click += new System.EventHandler(this.menuItemUnitGroups_Click);
			// 
			// menuItemSwitches
			// 
			this.menuItemSwitches.Index = 2;
			this.menuItemSwitches.Text = "&Switches...";
			this.menuItemSwitches.Click += new System.EventHandler(this.menuItemSwitches_Click);
			// 
			// menuItemCounters
			// 
			this.menuItemCounters.Index = 3;
			this.menuItemCounters.Text = "&Counters...";
			this.menuItemCounters.Click += new System.EventHandler(this.menuItemCounters_Click);
			// 
			// menuItemText
			// 
			this.menuItemText.Index = 4;
			this.menuItemText.Shortcut = System.Windows.Forms.Shortcut.CtrlL;
			this.menuItemText.Text = "Level &Text...";
			this.menuItemText.Click += new System.EventHandler(this.menuItemText_Click);
			// 
			// menuItem7
			// 
			this.menuItem7.Index = 5;
			this.menuItem7.Text = "-";
			// 
			// menuItemLevelDocProperties
			// 
			this.menuItemLevelDocProperties.Index = 6;
			this.menuItemLevelDocProperties.Text = "&Level";
			this.menuItemLevelDocProperties.Click += new System.EventHandler(this.menuItemLevelDocProperties_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 7;
			this.menuItem3.Text = "-";
			// 
			// menuItemSide1Properties
			// 
			this.menuItemSide1Properties.Index = 8;
			this.menuItemSide1Properties.Text = "Side &1";
			this.menuItemSide1Properties.Click += new System.EventHandler(this.menuItemSide1Properties_Click);
			// 
			// menuItemSide2Properties
			// 
			this.menuItemSide2Properties.Index = 9;
			this.menuItemSide2Properties.Text = "Side &2";
			this.menuItemSide2Properties.Click += new System.EventHandler(this.menuItemSide2Properties_Click);
			// 
			// menuItemSide3Properties
			// 
			this.menuItemSide3Properties.Index = 10;
			this.menuItemSide3Properties.Text = "Side &3";
			this.menuItemSide3Properties.Click += new System.EventHandler(this.menuItemSide3Properties_Click);
			// 
			// menuItemSide4Properties
			// 
			this.menuItemSide4Properties.Index = 11;
			this.menuItemSide4Properties.Text = "Side &4";
			this.menuItemSide4Properties.Click += new System.EventHandler(this.menuItemSide4Properties_Click);
			// 
			// menuItem9
			// 
			this.menuItem9.Index = 12;
			this.menuItem9.Text = "-";
			// 
			// menuItemComments
			// 
			this.menuItemComments.Index = 13;
			this.menuItemComments.Shortcut = System.Windows.Forms.Shortcut.CtrlShiftC;
			this.menuItemComments.Text = "Comments...";
			this.menuItemComments.Click += new System.EventHandler(this.menuItemComments_Click);
			// 
			// menuItemMisc
			// 
			this.menuItemMisc.Index = 3;
			this.menuItemMisc.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						 this.menuItemValidate,
																						 this.menuItemRun,
																						 this.menuItem8,
																						 this.menuItemSaveTileMapBitmap,
																						 this.menuItemSaveLevelBitmap,
																						 this.menuItemRenameTemplates});
			this.menuItemMisc.Text = "Misc";
			// 
			// menuItemValidate
			// 
			this.menuItemValidate.Index = 0;
			this.menuItemValidate.Shortcut = System.Windows.Forms.Shortcut.F7;
			this.menuItemValidate.Text = "&Validate";
			this.menuItemValidate.Click += new System.EventHandler(this.menuItemValidate_Click);
			// 
			// menuItemRun
			// 
			this.menuItemRun.Index = 1;
			this.menuItemRun.Shortcut = System.Windows.Forms.Shortcut.F5;
			this.menuItemRun.Text = "&Run";
			this.menuItemRun.Click += new System.EventHandler(this.menuItemRun_Click);
			// 
			// menuItem8
			// 
			this.menuItem8.Index = 2;
			this.menuItem8.Text = "-";
			// 
			// menuItemSaveTileMapBitmap
			// 
			this.menuItemSaveTileMapBitmap.Index = 3;
			this.menuItemSaveTileMapBitmap.Text = "Save TileMap Bitmap...";
			this.menuItemSaveTileMapBitmap.Visible = false;
			this.menuItemSaveTileMapBitmap.Click += new System.EventHandler(this.menuItemSaveTileMapBitmap_Click);
			// 
			// menuItemSaveLevelBitmap
			// 
			this.menuItemSaveLevelBitmap.Index = 4;
			this.menuItemSaveLevelBitmap.Text = "Save Level Bitmap...";
			this.menuItemSaveLevelBitmap.Click += new System.EventHandler(this.menuItemSaveLevelBitmap_Click);
			// 
			// menuItemRenameTemplates
			// 
			this.menuItemRenameTemplates.Index = 5;
			this.menuItemRenameTemplates.Text = "Rename Templates...";
			this.menuItemRenameTemplates.Click += new System.EventHandler(this.menuItemRenameTemplates_Click);
			// 
			// menuItemWindow
			// 
			this.menuItemWindow.Index = 4;
			this.menuItemWindow.MdiList = true;
			this.menuItemWindow.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						   this.menuItemCascade,
																						   this.menuItemTileHorizontal,
																						   this.menuItemTileVertical});
			this.menuItemWindow.Text = "&Window";
			// 
			// menuItemCascade
			// 
			this.menuItemCascade.Index = 0;
			this.menuItemCascade.Text = "&Cascade";
			this.menuItemCascade.Click += new System.EventHandler(this.menuItemCascade_Click);
			// 
			// menuItemTileHorizontal
			// 
			this.menuItemTileHorizontal.Index = 1;
			this.menuItemTileHorizontal.Text = "&Tile Horizontal";
			this.menuItemTileHorizontal.Click += new System.EventHandler(this.menuItemTileHorizontal_Click);
			// 
			// menuItemTileVertical
			// 
			this.menuItemTileVertical.Index = 2;
			this.menuItemTileVertical.Text = "&Tile Vertical";
			this.menuItemTileVertical.Click += new System.EventHandler(this.menuItemTileVertical_Click);
			// 
			// menuItem5
			// 
			this.menuItem5.Index = 5;
			this.menuItem5.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItemHelp,
																					  this.menuItemAbout});
			this.menuItem5.Text = "&Help";
			// 
			// menuItemHelp
			// 
			this.menuItemHelp.Index = 0;
			this.menuItemHelp.Text = "Help...";
			this.menuItemHelp.Click += new System.EventHandler(this.menuItemHelp_Click);
			// 
			// menuItemAbout
			// 
			this.menuItemAbout.Index = 1;
			this.menuItemAbout.Text = "About...";
			this.menuItemAbout.Click += new System.EventHandler(this.menuItemAbout_Click);
			// 
			// menuItemTest
			// 
			this.menuItemTest.Index = 6;
			this.menuItemTest.Text = "&Test";
			this.menuItemTest.Visible = false;
			this.menuItemTest.Click += new System.EventHandler(this.menuItemTest_Click);
			// 
			// splitterMapLeft
			// 
			this.splitterMapLeft.BackColor = System.Drawing.SystemColors.Control;
			this.splitterMapLeft.Location = new System.Drawing.Point(168, 0);
			this.splitterMapLeft.Name = "splitterMapLeft";
			this.splitterMapLeft.Size = new System.Drawing.Size(3, 550);
			this.splitterMapLeft.TabIndex = 1;
			this.splitterMapLeft.TabStop = false;
			// 
			// panelLeft
			// 
			this.panelLeft.BackColor = System.Drawing.SystemColors.Control;
			this.panelLeft.Controls.Add(this.panel1);
			this.panelLeft.Dock = System.Windows.Forms.DockStyle.Left;
			this.panelLeft.ForeColor = System.Drawing.SystemColors.Control;
			this.panelLeft.Location = new System.Drawing.Point(0, 0);
			this.panelLeft.Name = "panelLeft";
			this.panelLeft.Size = new System.Drawing.Size(168, 550);
			this.panelLeft.TabIndex = 1;
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.Color.Black;
			this.panel1.Controls.Add(this.panel4);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(168, 550);
			this.panel1.TabIndex = 1;
			// 
			// panel4
			// 
			this.panel4.BackColor = System.Drawing.Color.Black;
			this.panel4.Controls.Add(this.ctlTemplatePanel);
			this.panel4.Controls.Add(this.splitterGobTiles);
			this.panel4.Controls.Add(this.ctlGobPanel);
			this.panel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.panel4.Location = new System.Drawing.Point(0, 0);
			this.panel4.Name = "panel4";
			this.panel4.Size = new System.Drawing.Size(168, 550);
			this.panel4.TabIndex = 2;
			// 
			// ctlTemplatePanel
			// 
			this.ctlTemplatePanel.BackColor = System.Drawing.SystemColors.Control;
			this.ctlTemplatePanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctlTemplatePanel.ForeColor = System.Drawing.SystemColors.Control;
			this.ctlTemplatePanel.Location = new System.Drawing.Point(0, 219);
			this.ctlTemplatePanel.Name = "ctlTemplatePanel";
			this.ctlTemplatePanel.Size = new System.Drawing.Size(168, 331);
			this.ctlTemplatePanel.TabIndex = 3;
			this.ctlTemplatePanel.Load += new System.EventHandler(this.ctlTemplatePanel_Load);
			// 
			// splitterGobTiles
			// 
			this.splitterGobTiles.BackColor = System.Drawing.SystemColors.Control;
			this.splitterGobTiles.Dock = System.Windows.Forms.DockStyle.Top;
			this.splitterGobTiles.Location = new System.Drawing.Point(0, 216);
			this.splitterGobTiles.Name = "splitterGobTiles";
			this.splitterGobTiles.Size = new System.Drawing.Size(168, 3);
			this.splitterGobTiles.TabIndex = 2;
			this.splitterGobTiles.TabStop = false;
			// 
			// ctlGobPanel
			// 
			this.ctlGobPanel.AutoScroll = true;
			this.ctlGobPanel.BackColor = System.Drawing.Color.Black;
			this.ctlGobPanel.Dock = System.Windows.Forms.DockStyle.Top;
			this.ctlGobPanel.Location = new System.Drawing.Point(0, 0);
			this.ctlGobPanel.Name = "ctlGobPanel";
			this.ctlGobPanel.Size = new System.Drawing.Size(168, 216);
			this.ctlGobPanel.TabIndex = 0;
			// 
			// splitterMapRight
			// 
			this.splitterMapRight.BackColor = System.Drawing.SystemColors.Control;
			this.splitterMapRight.Dock = System.Windows.Forms.DockStyle.Right;
			this.splitterMapRight.Location = new System.Drawing.Point(769, 0);
			this.splitterMapRight.Name = "splitterMapRight";
			this.splitterMapRight.Size = new System.Drawing.Size(3, 550);
			this.splitterMapRight.TabIndex = 1;
			this.splitterMapRight.TabStop = false;
			// 
			// panelRight
			// 
			this.panelRight.BackColor = System.Drawing.SystemColors.Control;
			this.panelRight.Controls.Add(this.statusBar1);
			this.panelRight.Controls.Add(this.propertyGrid1);
			this.panelRight.Controls.Add(this.label1);
			this.panelRight.Controls.Add(this.panel6);
			this.panelRight.Dock = System.Windows.Forms.DockStyle.Right;
			this.panelRight.Location = new System.Drawing.Point(772, 0);
			this.panelRight.Name = "panelRight";
			this.panelRight.Size = new System.Drawing.Size(156, 550);
			this.panelRight.TabIndex = 2;
			// 
			// statusBar1
			// 
			this.statusBar1.Location = new System.Drawing.Point(0, 528);
			this.statusBar1.Name = "statusBar1";
			this.statusBar1.Size = new System.Drawing.Size(156, 22);
			this.statusBar1.SizingGrip = false;
			this.statusBar1.TabIndex = 3;
			this.statusBar1.TabStop = true;
			// 
			// propertyGrid1
			// 
			this.propertyGrid1.BackColor = System.Drawing.SystemColors.Control;
			this.propertyGrid1.CommandsVisibleIfAvailable = true;
			this.propertyGrid1.Cursor = System.Windows.Forms.Cursors.HSplit;
			this.propertyGrid1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.propertyGrid1.LargeButtons = false;
			this.propertyGrid1.LineColor = System.Drawing.SystemColors.ScrollBar;
			this.propertyGrid1.Location = new System.Drawing.Point(0, 123);
			this.propertyGrid1.Name = "propertyGrid1";
			this.propertyGrid1.Size = new System.Drawing.Size(156, 427);
			this.propertyGrid1.TabIndex = 2;
			this.propertyGrid1.Text = "propertyGrid1";
			this.propertyGrid1.ViewBackColor = System.Drawing.SystemColors.Window;
			this.propertyGrid1.ViewForeColor = System.Drawing.SystemColors.WindowText;
			this.propertyGrid1.SelectedObjectsChanged += new System.EventHandler(this.propertyGrid1_SelectedObjectsChanged);
			// 
			// label1
			// 
			this.label1.BackColor = System.Drawing.SystemColors.Control;
			this.label1.Dock = System.Windows.Forms.DockStyle.Top;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label1.Location = new System.Drawing.Point(0, 100);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(156, 23);
			this.label1.TabIndex = 1;
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// panel6
			// 
			this.panel6.BackColor = System.Drawing.SystemColors.Control;
			this.panel6.Controls.Add(this.pictureBox1);
			this.panel6.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel6.Location = new System.Drawing.Point(0, 0);
			this.panel6.Name = "panel6";
			this.panel6.Size = new System.Drawing.Size(156, 100);
			this.panel6.TabIndex = 0;
			// 
			// pictureBox1
			// 
			this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.pictureBox1.BackColor = System.Drawing.SystemColors.Control;
			this.pictureBox1.Location = new System.Drawing.Point(2, 4);
			this.pictureBox1.Name = "pictureBox1";
			this.pictureBox1.Size = new System.Drawing.Size(151, 94);
			this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
			this.pictureBox1.TabIndex = 0;
			this.pictureBox1.TabStop = false;
			// 
			// menuItem11
			// 
			this.menuItem11.Index = 8;
			this.menuItem11.Text = "Export Mission Pack...";
			this.menuItem11.Click += new System.EventHandler(this.menuItemExportMissionPack_Click);
			// 
			// menuItem12
			// 
			this.menuItem12.Index = 7;
			this.menuItem12.Text = "Import Mission Pack...";
			this.menuItem12.Click += new System.EventHandler(this.menuItemImportMissionPack_Click);
			// 
			// MainForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.BackColor = System.Drawing.SystemColors.AppWorkspace;
			this.ClientSize = new System.Drawing.Size(928, 550);
			this.Controls.Add(this.splitterMapLeft);
			this.Controls.Add(this.splitterMapRight);
			this.Controls.Add(this.panelRight);
			this.Controls.Add(this.panelLeft);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.IsMdiContainer = true;
			this.Menu = this.mainMenu1;
			this.Name = "MainForm";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show;
			this.Text = "m";
			this.Closing += new System.ComponentModel.CancelEventHandler(this.MainForm_Closing);
			this.panelLeft.ResumeLayout(false);
			this.panel1.ResumeLayout(false);
			this.panel4.ResumeLayout(false);
			this.panelRight.ResumeLayout(false);
			this.panel6.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void menuItemExit_Click(object sender, System.EventArgs e)
		{
			this.Close();
		}

		private void menuItemNewLevelDoc_Click(object sender, System.EventArgs e) {
			DocManager.NewDocument(typeof(LevelDoc), null);
		}

		private void menuItemOpenLevelDoc_Click(object sender, System.EventArgs e) {
			// Allow both .ld and .tc opening

			OpenFileDialog frmOpen = new OpenFileDialog();
			frmOpen.DefaultExt = "ld";
			frmOpen.Filter = "M files (*.ld;*.tc)|*.ld;*.tc";
			frmOpen.Title = "Open...";
			if (frmOpen.ShowDialog() != DialogResult.Cancel)
				DocManager.OpenDocument(frmOpen.FileName);
		}

		private void menuItemSaveLevelDoc_Click(object sender, System.EventArgs e) {
			Document doc = DocManager.GetActiveDocument(typeof(LevelDoc));
			if (doc != null)
				doc.Save();
		}

		private void menuItemSaveLevelDocAs_Click(object sender, System.EventArgs e) {
			Document doc = DocManager.GetActiveDocument(typeof(LevelDoc));
			if (doc != null)
				doc.SaveAs(null);
		}

		private void menuItemSaveTileMapBitmap_Click(object sender, System.EventArgs e) {
#if false
			SaveFileDialog frmSave = new SaveFileDialog();
			frmSave.DefaultExt = "png";
			frmSave.Filter = "Png Files (*.png)|*.png";
			frmSave.Title = "Save TileMap Bitmap As";
			if (frmSave.ShowDialog() == DialogResult.Cancel)
				return;
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument();
			if (lvld != null)
				lvld.GetMapBitmap().Save(frmSave.FileName, ImageFormat.Png);
#endif
		}

		private void menuItemSaveLevelBitmap_Click(object sender, System.EventArgs e) {
			SaveFileDialog frmSave = new SaveFileDialog();
			frmSave.DefaultExt = "png";
			frmSave.Filter = "Png Files (*.png)|*.png";
			frmSave.Title = "Save Level Bitmap As";
			if (frmSave.ShowDialog() == DialogResult.Cancel)
				return;
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null) {
				TemplateDoc tmpd = lvld.GetTemplateDoc();
				lvld.GetMapBitmap(tmpd.TileSize, tmpd, false).Save(frmSave.FileName, ImageFormat.Png);
			}
		}

		private void menuItemLevelDocProperties_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				Globals.PropertyGrid.SelectedObject = lvld;
		}

		private void propertyGrid1_SelectedObjectsChanged(object sender, System.EventArgs e) {
			TemplateDoc tmpd = (TemplateDoc)DocManager.GetActiveDocument(typeof(TemplateDoc));
			Object obj = Globals.PropertyGrid.SelectedObject;
			if (obj == null) {
				label1.Text = null;
				pictureBox1.Image = null;
			} else {
				string strName = obj.GetType().Name;
				if (obj is Unit && !(obj is Activator))
					strName = Helper.GetDisplayName(typeof(UnitType), "kut" + strName);
				else if (obj is Tile)
					strName = ((Tile)obj).Name;
				label1.Text = strName;
				IMapItem mi = obj as IMapItem;
				if (mi == null) {
					pictureBox1.Image = null;
				} else {
					Tile tile = mi as Tile;
					if (tile != null) {
						pictureBox1.Image = Misc.TraceEdges(tile.GetBitmap(new Size(tmpd.TileSize.Width, tmpd.TileSize.Height), tmpd), 1, Color.Black);
					} else {
						pictureBox1.Image = mi.GetBitmap(new Size(tmpd.TileSize.Width, tmpd.TileSize.Height), tmpd);
					}
				}
			}
		}

		private void menuItemSide1Properties_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				Globals.PropertyGrid.SelectedObject = lvld.GetSideInfo(Side.side1);
		}

		private void menuItemSide2Properties_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				Globals.PropertyGrid.SelectedObject = lvld.GetSideInfo(Side.side2);
		}

		private void menuItemSide3Properties_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				Globals.PropertyGrid.SelectedObject = lvld.GetSideInfo(Side.side3);
		}

		private void menuItemSide4Properties_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				Globals.PropertyGrid.SelectedObject = lvld.GetSideInfo(Side.side4);
		}

		private void MainForm_Closing(object sender, System.ComponentModel.CancelEventArgs e) {
			if (!DocManager.CloseAllDocuments()) {
				e.Cancel = true;
				return;
			}

			// Modifiers to toggle kit mode

			if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
				Globals.SetKit(!Globals.IsKit());
			}

			SaveSettings();
		}

		void SaveSettings() {
			// Save settings in ini.
			
			Ini ini = new Ini();			
			Ini.Section secGeneral = new Ini.Section("General");
			secGeneral.Add(new Ini.Property("WindowState", WindowState.ToString()));
			secGeneral.Add(new Ini.Property("X", Bounds.X.ToString()));
			secGeneral.Add(new Ini.Property("Y", Bounds.Y.ToString()));
			secGeneral.Add(new Ini.Property("Width", Bounds.Width.ToString()));
			secGeneral.Add(new Ini.Property("Height", Bounds.Height.ToString()));
			secGeneral.Add(new Ini.Property("AuthorKitPath", AuthorKitPath));
			secGeneral.Add(new Ini.Property("Kit", Globals.IsKit().ToString()));
			secGeneral.Add(new Ini.Property("Eula", "1"));

			ini.Add(secGeneral);
			
			// Place in directory where .exe resides
			
			try {
				if (m_strFileSettings != null)
					ini.Save(m_strFileSettings);
			} catch {
			}
		}

		void LoadSettings() {
			// Settings

			m_strFileSettings = Application.ExecutablePath.Replace(".exe", ".ini");

			Ini ini;
			try {
				ini = new Ini(m_strFileSettings);
			} catch {
				ini = null;
			}

			if (ini == null)
				return;

			Ini.Section sec = ini["General"];
			if (sec == null)
				return;
			Ini.Property prop = sec["WindowState"];
			if (prop != null) {
				switch(prop.Value) {
				case "Maximized":
					WindowState = FormWindowState.Maximized;
					break;

				case "Minimized":
					WindowState = FormWindowState.Minimized;
					break;

				case "Normal":
					WindowState = FormWindowState.Normal;
					break;
				}
				
				if (WindowState == FormWindowState.Normal) {
					Rectangle rc = new Rectangle();
					rc.X = int.Parse(sec["X"].Value);
					rc.Y = int.Parse(sec["Y"].Value);
					rc.Width = int.Parse(sec["Width"].Value);
					rc.Height = int.Parse(sec["Height"].Value);
					Bounds = rc;
				}
			}

			prop = sec["AuthorKitPath"];
			if (prop != null)
				AuthorKitPath = prop.Value;
		}

		private string m_strAuthorKitPath;

		public string AuthorKitPath {
			get {
				return m_strAuthorKitPath;
			}
			set {
				m_strAuthorKitPath = value;
			}
		}

		private void menuItemCascade_Click(object sender, System.EventArgs e) {
			LayoutMdi(MdiLayout.Cascade);
		}

		private void menuItemTileHorizontal_Click(object sender, System.EventArgs e) {
			LayoutMdi(MdiLayout.TileHorizontal);
		}

		private void menuItemTileVertical_Click(object sender, System.EventArgs e) {
			LayoutMdi(MdiLayout.TileVertical);
		}

		private void menuItemCut_Click(object sender, System.EventArgs e) {
			ICommandTarget cmdt = DocManager.GetCommandTarget();
			if (cmdt != null)
				cmdt.DispatchCommand(Command.Cut);
		}

		private void menuItemCopy_Click(object sender, System.EventArgs e) {
			ICommandTarget cmdt = DocManager.GetCommandTarget();
			if (cmdt != null)
				cmdt.DispatchCommand(Command.Copy);
		}

		private void menuItemPaste_Click(object sender, System.EventArgs e) {
			ICommandTarget cmdt = DocManager.GetCommandTarget();
			if (cmdt != null)
				cmdt.DispatchCommand(Command.Paste);
		}

		private void menuItemDelete_Click(object sender, System.EventArgs e) {
			ICommandTarget cmdt = DocManager.GetCommandTarget();
			if (cmdt != null)
				cmdt.DispatchCommand(Command.Delete);
		}

		private void menuItemClose_Click(object sender, System.EventArgs e) {
			if (ActiveMdiChild != null)
				ActiveMdiChild.Close();
		}

		private void menuItemRenameTemplates_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld == null)
				return;

			OpenFileDialog ofd = new OpenFileDialog();

			ofd.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*" ;

			if (ofd.ShowDialog() != DialogResult.OK)
				return;

			StreamReader stmrTemplateNames = new StreamReader(ofd.FileName);

			TemplateDoc tmpd = lvld.GetTemplateDoc();
			Template[] atmpl = tmpd.GetTemplates();
			foreach (Template tmpl in atmpl) {
				string strT = stmrTemplateNames.ReadLine();
				if (strT == null)
					break;
				tmpl.Name = strT;
			}

			stmrTemplateNames.Close();
		}

		private void menuItemTest_Click(object sender, System.EventArgs e) {
			TriggersForm frm = new TriggersForm(new TriggerManager());
			frm.ShowDialog();
		}

		private void menuItemTriggers_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				lvld.EditTriggers();
		}

		private void menuItemUnitGroups_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				lvld.EditUnitGroups();
		}

		private void menuItemComments_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				lvld.EditComments();
		}

		private void menuItemSwitches_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				SwitchesForm.DoModal("Switch", "", CaTypeSwitch.GetSwitchNames());
		}

		private void menuItemCounters_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null)
				CountersForm.DoModal("Counter", "", CaTypeCounter.GetCounterNames());
		}

		private void menuItemRun_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld != null) {
				lvld.Save();

				while (true) {
					Process[] apr = Process.GetProcessesByName("Warfare Incorporated");
					if (apr.Length != 0)
						MessageBox.Show("Please close Warfare Incorporated now.", "Warfare Incorporated already running");
					else
						break;
				}

				ProcessStartInfo psi = new ProcessStartInfo("run.bat");
//				psi.WindowStyle = ProcessWindowStyle.Hidden;
				psi.Arguments = lvld.GetPath() + " " + Path.GetFileNameWithoutExtension(lvld.GetPath()) + ".lvl";
				string strAuthorKitPath = AuthorKitPath;
				if (strAuthorKitPath == null)
					strAuthorKitPath = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
				psi.WorkingDirectory = strAuthorKitPath;
				try {
					Process ps = Process.Start(psi);
				} catch (Win32Exception ex) {
					if (ex.NativeErrorCode == 2)
						MessageBox.Show("Can't find key AuthorKit files. Launch M.exe from inside the AuthorKit directory or specify the AuthorKitPath in m.ini.", "Error Launching Preview");
					else
						throw ex;
				}
			}
		}

		private void menuItemText_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld == null)
				return;
			lvld.EditLevelText();
		}

		private void menuItemValidate_Click(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld == null)
				return;
			OutputForm.HideIt();
			OutputForm.Clear();
			lvld.Validate(new LevelDoc.ValidateErrorDelegate(ValidateCallback));
		}

		//c:\code\ht\m\outputform.cs(21,15): error CS0111:
		private void ValidateCallback(LevelDoc lvld, LevelDoc.ValidateError ve, int tx, int ty, object ob, string str) {
			OutputForm.ShowIt();
#if false
			OutputForm.Error(lvld, ob, "({0},{1},{2}): {3}: {4}\n", tx, ty, ob == null ? "" : ob.GetHashCode().ToString(), ve.ToString(), str);
#else
			if (ob is Unit) {
				OutputForm.Error(lvld, ob, "({0},{1}): {2}: Side {3} {4}\n", tx, ty, ve.ToString(), ((Unit)ob).Side, str);
			} else {
				OutputForm.Error(lvld, ob, "({0},{1}): {2}: {3}\n", tx, ty, ve.ToString(), str);
			}
#endif
		}

		private void ctlTemplatePanel_Load(object sender, System.EventArgs e) {
		
		}

		private void menuItemImportMissionPack_Click(object sender, System.EventArgs e) {
			// Prompt the user to save and close existing missions. This is so if a user
			// imports more than once, missions aren't accumulated by mistake. 

#if false // annoying
			LevelDocTemplate doctLevel = (LevelDocTemplate)DocManager.FindDocTemplate(typeof(LevelDoc));
			Document[] adoc = doctLevel.GetDocuments();
			if (adoc.Length != 0) {
				MessageBox.Show(DocManager.GetFrameParent(), "Close all Level Documents before importing");
				return;
			}
#endif

			// Select the pdb to load, and import it

			OpenFileDialog frmOpen = new OpenFileDialog();
			frmOpen.DefaultExt = "pdb";
			frmOpen.Filter = "Pdb files (*.pdb)|*.pdb";
			frmOpen.Title = "Open Mission Pack...";
			if (frmOpen.ShowDialog() != DialogResult.Cancel)
				OutputTools.ImportExpansionPdb(frmOpen.FileName);
		}

		private void menuItemExportMissionPack_Click(object sender, System.EventArgs e) {
			// Any leveldocs to save?

			LevelDocTemplate doctLevel = (LevelDocTemplate)DocManager.FindDocTemplate(typeof(LevelDoc));
			Document[] adoc = doctLevel.GetDocuments();
			if (adoc.Length == 0) {
				MessageBox.Show(DocManager.GetFrameParent(), "No Level Descriptions loaded!");
				return;
			}

			// First save all level docs

			//annoying
			//DocManager.SaveAllModified(typeof(LevelDoc));

			// Remember this

			LevelDoc lvldActive = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));

			// Validate these first

			OutputForm.HideIt();
			OutputForm.Clear();
			foreach (LevelDoc lvld in adoc) {
				// Unfortunately it seems we have a number of methods during validation and SaveIni-ing that
				// rely on which document is active.
				DocManager.SetActiveDocument(typeof(LevelDoc), lvld);
				int cErrors = lvld.Validate(new LevelDoc.ValidateErrorDelegate(ValidateCallback));
				if (cErrors != 0) {
					MessageBox.Show(DocManager.GetFrameParent(), "Please fix errors in " + lvld.GetName() + " then try again.");
					return;
				}
			}

			// Restore

			if (lvldActive != null)
				DocManager.SetActiveDocument(typeof(LevelDoc), lvldActive);

			// First get the filename

			SaveFileDialog frmSave = new SaveFileDialog();
			frmSave.DefaultExt = "pdb";
			frmSave.Filter = "Mission Pack Files (*.pdb)|*.pdb";
			frmSave.Title = "Export Mission Pack";
			if (frmSave.ShowDialog() == DialogResult.Cancel)
				return;

			// Export expansion .pdb

			OutputTools.SaveExpansionPdb(frmSave.FileName, adoc, "1.1");
		}

		private void menuItemAbout_Click(object sender, System.EventArgs e) {
			AboutForm frm = new AboutForm();
			frm.ShowDialog();
		}

		private void menuItemHelp_Click(object sender, System.EventArgs e) {
			if (m_strFileSettings.EndsWith("mgui.ini")) {
				System.Diagnostics.Process.Start(m_strFileSettings.Replace("mgui.ini", "m.chm"));
			} else if (m_strFileSettings.EndsWith("m.ini")) {
				System.Diagnostics.Process.Start(m_strFileSettings.Replace("m.ini", "m.chm"));
			}
		}
	}

	//
	// Plug-in interface
	//

	public interface IPlugin {
		void HackMenus(MainMenu mnu);
		IMapItem[] GetMapItems();
	}
}
