using System;
using System.IO;
using System.Drawing;
using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using SpiffLib;
using LoMaN.IO;

namespace Schemer
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form, IComparer
	{
		bool m_fFillingListBox;
		bool m_fPlaySound;
		ListViewItem m_itemDisplayed;
		ListViewItem m_itemForCopy;
		CaseInsensitiveComparer m_comparer = new CaseInsensitiveComparer();
		int m_iColumnSort;
		bool m_fAscending = true;
		string m_strSfxH = null;
		string m_strPdbFile = null;
		StringCollection m_strcPriorities = null;
		ArrayList m_alsNames = new ArrayList();
		ArrayList m_alsSfxEnabled = new ArrayList();
		string m_strSfxFile;
		int m_nComPort = 2;

		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.Panel paneSfxSoundsParent;
		private System.Windows.Forms.Panel panelSfx;
		private System.Windows.Forms.Splitter splitter2;
		private System.Windows.Forms.Panel panelSfxProperties;
		private System.Windows.Forms.ListView listViewSfx;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.Panel panelSounds;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.ColumnHeader columnHeaderSound;
		private System.Windows.Forms.ColumnHeader columnHeaderChannel;
		private System.Windows.Forms.ColumnHeader columnHeaderPriority;
		private System.Windows.Forms.ColumnHeader columnHeaderComment;
		private System.Windows.Forms.Panel panelMain;
		private System.Windows.Forms.StatusBar statusBar1;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.TextBox textBoxSoundsDir;
		private System.Windows.Forms.ListBox listBoxSounds;
		private System.Windows.Forms.ColumnHeader columnHeaderSfx;
		private System.Windows.Forms.Label labelPropertiesSfx;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox textBoxPropertiesSound;
		private System.Windows.Forms.ComboBox comboBoxPropertiesPriority;
		private System.Windows.Forms.TextBox textBoxPropertiesComment;
		private System.Windows.Forms.MenuItem menuItemSaveAs;
		private System.Windows.Forms.MenuItem menuItem6;
		private System.Windows.Forms.MenuItem menuItem7;
		private System.Windows.Forms.MenuItem menuItemMakePdb;
		private System.Windows.Forms.MenuItem menuItem8;
		private System.Windows.Forms.MenuItem menuItemNew;
		private System.Windows.Forms.MenuItem menuItemOpen;
		private System.Windows.Forms.MenuItem menuItemSave;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItemCheckAll;
		private System.Windows.Forms.MenuItem menuItemUncheckAll;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem menuItemCopy;
		private System.Windows.Forms.MenuItem menuItemPaste;
		private System.Windows.Forms.MenuItem menuItemClear;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem menuItemCheckPdbSize;
		private System.Windows.Forms.MenuItem menuItemTestCom2;
		private System.Windows.Forms.ContextMenu contextMenuListBox;
		private System.Windows.Forms.MenuItem menuItemPlayOnPalm;
		private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.MenuItem menuItemCom1;
		private System.Windows.Forms.MenuItem menuItemCom2;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public Form1()
		{
			Application.Idle += new System.EventHandler(AppIdleEventHandler);

			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			m_strcPriorities = new StringCollection();
			m_strcPriorities.Add("Voice Status");
			m_strcPriorities.Add("UI Response");
			m_strcPriorities.Add("Voice Reponse");
			m_strcPriorities.Add("High Priority");
			m_strcPriorities.Add("Explosion");
			m_strcPriorities.Add("Missle Impact");
			m_strcPriorities.Add("Tank Impact");
			m_strcPriorities.Add("Missle Shot");
			m_strcPriorities.Add("Tank Shot");
			m_strcPriorities.Add("Machine Gun");
			m_strcPriorities.Add("Background");
			m_strcPriorities.Add("Unknown");
					
			comboBoxPropertiesPriority.DataSource = m_strcPriorities;

			if (!LoadSettings())
				Close();
			NewScheme();
		}
	
		void AppIdleEventHandler(object sender, System.EventArgs e) {
			m_fPlaySound = true;
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
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.menuItemNew = new System.Windows.Forms.MenuItem();
			this.menuItemOpen = new System.Windows.Forms.MenuItem();
			this.menuItemSave = new System.Windows.Forms.MenuItem();
			this.menuItemSaveAs = new System.Windows.Forms.MenuItem();
			this.menuItem6 = new System.Windows.Forms.MenuItem();
			this.menuItemMakePdb = new System.Windows.Forms.MenuItem();
			this.menuItem8 = new System.Windows.Forms.MenuItem();
			this.menuItem7 = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.menuItemCopy = new System.Windows.Forms.MenuItem();
			this.menuItemPaste = new System.Windows.Forms.MenuItem();
			this.menuItemClear = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.menuItemCheckAll = new System.Windows.Forms.MenuItem();
			this.menuItemUncheckAll = new System.Windows.Forms.MenuItem();
			this.menuItem4 = new System.Windows.Forms.MenuItem();
			this.menuItemCheckPdbSize = new System.Windows.Forms.MenuItem();
			this.menuItemTestCom2 = new System.Windows.Forms.MenuItem();
			this.panelMain = new System.Windows.Forms.Panel();
			this.paneSfxSoundsParent = new System.Windows.Forms.Panel();
			this.panelSfx = new System.Windows.Forms.Panel();
			this.listViewSfx = new System.Windows.Forms.ListView();
			this.columnHeaderSfx = new System.Windows.Forms.ColumnHeader();
			this.columnHeaderSound = new System.Windows.Forms.ColumnHeader();
			this.columnHeaderChannel = new System.Windows.Forms.ColumnHeader();
			this.columnHeaderPriority = new System.Windows.Forms.ColumnHeader();
			this.columnHeaderComment = new System.Windows.Forms.ColumnHeader();
			this.splitter2 = new System.Windows.Forms.Splitter();
			this.panelSfxProperties = new System.Windows.Forms.Panel();
			this.textBoxPropertiesComment = new System.Windows.Forms.TextBox();
			this.label8 = new System.Windows.Forms.Label();
			this.comboBoxPropertiesPriority = new System.Windows.Forms.ComboBox();
			this.label6 = new System.Windows.Forms.Label();
			this.textBoxPropertiesSound = new System.Windows.Forms.TextBox();
			this.label7 = new System.Windows.Forms.Label();
			this.labelPropertiesSfx = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.splitter1 = new System.Windows.Forms.Splitter();
			this.panelSounds = new System.Windows.Forms.Panel();
			this.listBoxSounds = new System.Windows.Forms.ListBox();
			this.textBoxSoundsDir = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.statusBar1 = new System.Windows.Forms.StatusBar();
			this.contextMenuListBox = new System.Windows.Forms.ContextMenu();
			this.menuItemPlayOnPalm = new System.Windows.Forms.MenuItem();
			this.menuItem5 = new System.Windows.Forms.MenuItem();
			this.menuItemCom1 = new System.Windows.Forms.MenuItem();
			this.menuItemCom2 = new System.Windows.Forms.MenuItem();
			this.panelMain.SuspendLayout();
			this.paneSfxSoundsParent.SuspendLayout();
			this.panelSfx.SuspendLayout();
			this.panelSfxProperties.SuspendLayout();
			this.panelSounds.SuspendLayout();
			this.SuspendLayout();
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem1,
																					  this.menuItem2,
																					  this.menuItem4});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItemNew,
																					  this.menuItemOpen,
																					  this.menuItemSave,
																					  this.menuItemSaveAs,
																					  this.menuItem6,
																					  this.menuItemMakePdb,
																					  this.menuItem8,
																					  this.menuItem7});
			this.menuItem1.Text = "File";
			// 
			// menuItemNew
			// 
			this.menuItemNew.Index = 0;
			this.menuItemNew.Text = "New";
			this.menuItemNew.Click += new System.EventHandler(this.menuItemNew_Click);
			// 
			// menuItemOpen
			// 
			this.menuItemOpen.Index = 1;
			this.menuItemOpen.Text = "Open...";
			this.menuItemOpen.Click += new System.EventHandler(this.menuItemOpen_Click);
			// 
			// menuItemSave
			// 
			this.menuItemSave.Index = 2;
			this.menuItemSave.Text = "Save";
			this.menuItemSave.Click += new System.EventHandler(this.menuItemSave_Click);
			// 
			// menuItemSaveAs
			// 
			this.menuItemSaveAs.Index = 3;
			this.menuItemSaveAs.Text = "Save As...";
			this.menuItemSaveAs.Click += new System.EventHandler(this.menuItemSaveAs_Click);
			// 
			// menuItem6
			// 
			this.menuItem6.Index = 4;
			this.menuItem6.Text = "-";
			// 
			// menuItemMakePdb
			// 
			this.menuItemMakePdb.Index = 5;
			this.menuItemMakePdb.Text = "Make Pdb...";
			this.menuItemMakePdb.Click += new System.EventHandler(this.menuItemMakePdb_Click);
			// 
			// menuItem8
			// 
			this.menuItem8.Index = 6;
			this.menuItem8.Text = "-";
			// 
			// menuItem7
			// 
			this.menuItem7.Index = 7;
			this.menuItem7.Text = "Exit";
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 1;
			this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItemCopy,
																					  this.menuItemPaste,
																					  this.menuItemClear,
																					  this.menuItem3,
																					  this.menuItemCheckAll,
																					  this.menuItemUncheckAll});
			this.menuItem2.Text = "Edit";
			// 
			// menuItemCopy
			// 
			this.menuItemCopy.Index = 0;
			this.menuItemCopy.Shortcut = System.Windows.Forms.Shortcut.CtrlC;
			this.menuItemCopy.Text = "Copy";
			this.menuItemCopy.Click += new System.EventHandler(this.menuItemCopy_Click);
			// 
			// menuItemPaste
			// 
			this.menuItemPaste.Index = 1;
			this.menuItemPaste.Shortcut = System.Windows.Forms.Shortcut.CtrlV;
			this.menuItemPaste.Text = "Paste";
			this.menuItemPaste.Click += new System.EventHandler(this.menuItemPaste_Click);
			// 
			// menuItemClear
			// 
			this.menuItemClear.Index = 2;
			this.menuItemClear.Shortcut = System.Windows.Forms.Shortcut.Del;
			this.menuItemClear.Text = "Clear";
			this.menuItemClear.Click += new System.EventHandler(this.menuItemClear_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 3;
			this.menuItem3.Text = "-";
			// 
			// menuItemCheckAll
			// 
			this.menuItemCheckAll.Index = 4;
			this.menuItemCheckAll.Text = "Check All";
			this.menuItemCheckAll.Click += new System.EventHandler(this.menuItemCheckAll_Click);
			// 
			// menuItemUncheckAll
			// 
			this.menuItemUncheckAll.Index = 5;
			this.menuItemUncheckAll.Text = "Uncheck All";
			this.menuItemUncheckAll.Click += new System.EventHandler(this.menuItemUncheckAll_Click);
			// 
			// menuItem4
			// 
			this.menuItem4.Index = 2;
			this.menuItem4.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItemCheckPdbSize,
																					  this.menuItemTestCom2});
			this.menuItem4.Text = "Misc";
			// 
			// menuItemCheckPdbSize
			// 
			this.menuItemCheckPdbSize.Index = 0;
			this.menuItemCheckPdbSize.Text = "Check Pdb Size";
			this.menuItemCheckPdbSize.Click += new System.EventHandler(this.menuItemCheckPdbSize_Click);
			// 
			// menuItemTestCom2
			// 
			this.menuItemTestCom2.Index = 1;
			this.menuItemTestCom2.Text = "Test Com2";
			this.menuItemTestCom2.Visible = false;
			this.menuItemTestCom2.Click += new System.EventHandler(this.menuItemTestCom2_Click);
			// 
			// panelMain
			// 
			this.panelMain.Controls.AddRange(new System.Windows.Forms.Control[] {
																					this.paneSfxSoundsParent});
			this.panelMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panelMain.Name = "panelMain";
			this.panelMain.Size = new System.Drawing.Size(768, 480);
			this.panelMain.TabIndex = 1;
			// 
			// paneSfxSoundsParent
			// 
			this.paneSfxSoundsParent.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.paneSfxSoundsParent.Controls.AddRange(new System.Windows.Forms.Control[] {
																							  this.panelSfx,
																							  this.splitter1,
																							  this.panelSounds});
			this.paneSfxSoundsParent.Dock = System.Windows.Forms.DockStyle.Fill;
			this.paneSfxSoundsParent.Name = "paneSfxSoundsParent";
			this.paneSfxSoundsParent.Size = new System.Drawing.Size(768, 480);
			this.paneSfxSoundsParent.TabIndex = 1;
			// 
			// panelSfx
			// 
			this.panelSfx.Controls.AddRange(new System.Windows.Forms.Control[] {
																				   this.listViewSfx,
																				   this.splitter2,
																				   this.panelSfxProperties,
																				   this.label1});
			this.panelSfx.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panelSfx.Name = "panelSfx";
			this.panelSfx.Size = new System.Drawing.Size(610, 476);
			this.panelSfx.TabIndex = 0;
			// 
			// listViewSfx
			// 
			this.listViewSfx.Activation = System.Windows.Forms.ItemActivation.OneClick;
			this.listViewSfx.AllowDrop = true;
			this.listViewSfx.AutoArrange = false;
			this.listViewSfx.BackColor = System.Drawing.Color.White;
			this.listViewSfx.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.listViewSfx.CheckBoxes = true;
			this.listViewSfx.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																						  this.columnHeaderSfx,
																						  this.columnHeaderSound,
																						  this.columnHeaderChannel,
																						  this.columnHeaderPriority,
																						  this.columnHeaderComment});
			this.listViewSfx.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listViewSfx.FullRowSelect = true;
			this.listViewSfx.GridLines = true;
			this.listViewSfx.LabelWrap = false;
			this.listViewSfx.Location = new System.Drawing.Point(0, 23);
			this.listViewSfx.MultiSelect = false;
			this.listViewSfx.Name = "listViewSfx";
			this.listViewSfx.Size = new System.Drawing.Size(610, 350);
			this.listViewSfx.TabIndex = 3;
			this.listViewSfx.View = System.Windows.Forms.View.Details;
			this.listViewSfx.MouseDown += new System.Windows.Forms.MouseEventHandler(this.listViewSfx_MouseDown);
			this.listViewSfx.ItemActivate += new System.EventHandler(this.listViewSfx_ItemActivate);
			this.listViewSfx.MouseUp += new System.Windows.Forms.MouseEventHandler(this.listViewSfx_MouseUp);
			this.listViewSfx.DragOver += new System.Windows.Forms.DragEventHandler(this.listViewSfx_DragOver);
			this.listViewSfx.DragDrop += new System.Windows.Forms.DragEventHandler(this.listViewSfx_DragDrop);
			this.listViewSfx.Leave += new System.EventHandler(this.listViewSfx_Leave);
			this.listViewSfx.DragEnter += new System.Windows.Forms.DragEventHandler(this.listViewSfx_DragEnter);
			this.listViewSfx.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listViewSfx_ColumnClick);
			this.listViewSfx.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.listViewSfx_ItemDrag);
			this.listViewSfx.SelectedIndexChanged += new System.EventHandler(this.listViewSfx_SelectedIndexChanged);
			// 
			// columnHeaderSfx
			// 
			this.columnHeaderSfx.Text = "Sound Effect";
			this.columnHeaderSfx.Width = 210;
			// 
			// columnHeaderSound
			// 
			this.columnHeaderSound.Text = "File";
			this.columnHeaderSound.Width = 100;
			// 
			// columnHeaderChannel
			// 
			this.columnHeaderChannel.Text = "Channel";
			this.columnHeaderChannel.Width = 0;
			// 
			// columnHeaderPriority
			// 
			this.columnHeaderPriority.Text = "Priority";
			this.columnHeaderPriority.Width = 90;
			// 
			// columnHeaderComment
			// 
			this.columnHeaderComment.Text = "Comment";
			this.columnHeaderComment.Width = 210;
			// 
			// splitter2
			// 
			this.splitter2.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(128)), ((System.Byte)(64)), ((System.Byte)(64)));
			this.splitter2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.splitter2.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.splitter2.Location = new System.Drawing.Point(0, 373);
			this.splitter2.Name = "splitter2";
			this.splitter2.Size = new System.Drawing.Size(610, 3);
			this.splitter2.TabIndex = 6;
			this.splitter2.TabStop = false;
			// 
			// panelSfxProperties
			// 
			this.panelSfxProperties.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(247)), ((System.Byte)(223)), ((System.Byte)(225)));
			this.panelSfxProperties.Controls.AddRange(new System.Windows.Forms.Control[] {
																							 this.textBoxPropertiesComment,
																							 this.label8,
																							 this.comboBoxPropertiesPriority,
																							 this.label6,
																							 this.textBoxPropertiesSound,
																							 this.label7,
																							 this.labelPropertiesSfx,
																							 this.label3});
			this.panelSfxProperties.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panelSfxProperties.Location = new System.Drawing.Point(0, 376);
			this.panelSfxProperties.Name = "panelSfxProperties";
			this.panelSfxProperties.Size = new System.Drawing.Size(610, 100);
			this.panelSfxProperties.TabIndex = 5;
			// 
			// textBoxPropertiesComment
			// 
			this.textBoxPropertiesComment.Location = new System.Drawing.Point(280, 23);
			this.textBoxPropertiesComment.Multiline = true;
			this.textBoxPropertiesComment.Name = "textBoxPropertiesComment";
			this.textBoxPropertiesComment.Size = new System.Drawing.Size(288, 68);
			this.textBoxPropertiesComment.TabIndex = 11;
			this.textBoxPropertiesComment.Text = "";
			this.textBoxPropertiesComment.TextChanged += new System.EventHandler(this.textBoxPropertiesComment_TextChanged);
			// 
			// label8
			// 
			this.label8.Location = new System.Drawing.Point(277, 5);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(128, 13);
			this.label8.TabIndex = 10;
			this.label8.Text = "Comment:";
			// 
			// comboBoxPropertiesPriority
			// 
			this.comboBoxPropertiesPriority.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxPropertiesPriority.Location = new System.Drawing.Point(56, 53);
			this.comboBoxPropertiesPriority.Name = "comboBoxPropertiesPriority";
			this.comboBoxPropertiesPriority.Size = new System.Drawing.Size(112, 21);
			this.comboBoxPropertiesPriority.TabIndex = 9;
			this.comboBoxPropertiesPriority.SelectedIndexChanged += new System.EventHandler(this.comboBoxPropertiesPriority_SelectedIndexChanged);
			// 
			// label6
			// 
			this.label6.Location = new System.Drawing.Point(16, 56);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(48, 16);
			this.label6.TabIndex = 8;
			this.label6.Text = "Priority:";
			// 
			// textBoxPropertiesSound
			// 
			this.textBoxPropertiesSound.Location = new System.Drawing.Point(56, 25);
			this.textBoxPropertiesSound.Name = "textBoxPropertiesSound";
			this.textBoxPropertiesSound.Size = new System.Drawing.Size(198, 20);
			this.textBoxPropertiesSound.TabIndex = 5;
			this.textBoxPropertiesSound.Text = "";
			this.textBoxPropertiesSound.TextChanged += new System.EventHandler(this.textBoxPropertiesSound_TextChanged);
			// 
			// label7
			// 
			this.label7.Location = new System.Drawing.Point(31, 27);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(32, 16);
			this.label7.TabIndex = 4;
			this.label7.Text = "File:";
			// 
			// labelPropertiesSfx
			// 
			this.labelPropertiesSfx.Location = new System.Drawing.Point(57, 5);
			this.labelPropertiesSfx.Name = "labelPropertiesSfx";
			this.labelPropertiesSfx.Size = new System.Drawing.Size(195, 15);
			this.labelPropertiesSfx.TabIndex = 1;
			this.labelPropertiesSfx.Text = "LightTankVehicleFire";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(32, 5);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(26, 16);
			this.label3.TabIndex = 0;
			this.label3.Text = "Sfx:";
			// 
			// label1
			// 
			this.label1.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(185)), ((System.Byte)(190)), ((System.Byte)(240)));
			this.label1.Dock = System.Windows.Forms.DockStyle.Top;
			this.label1.Font = new System.Drawing.Font("Times New Roman", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label1.ForeColor = System.Drawing.Color.Black;
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(610, 23);
			this.label1.TabIndex = 2;
			this.label1.Text = "Sound Effects";
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// splitter1
			// 
			this.splitter1.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(128)), ((System.Byte)(64)), ((System.Byte)(64)));
			this.splitter1.Dock = System.Windows.Forms.DockStyle.Right;
			this.splitter1.Location = new System.Drawing.Point(610, 0);
			this.splitter1.Name = "splitter1";
			this.splitter1.Size = new System.Drawing.Size(4, 476);
			this.splitter1.TabIndex = 1;
			this.splitter1.TabStop = false;
			// 
			// panelSounds
			// 
			this.panelSounds.Controls.AddRange(new System.Windows.Forms.Control[] {
																					  this.listBoxSounds,
																					  this.textBoxSoundsDir,
																					  this.label2});
			this.panelSounds.Dock = System.Windows.Forms.DockStyle.Right;
			this.panelSounds.Location = new System.Drawing.Point(614, 0);
			this.panelSounds.Name = "panelSounds";
			this.panelSounds.Size = new System.Drawing.Size(150, 476);
			this.panelSounds.TabIndex = 2;
			// 
			// listBoxSounds
			// 
			this.listBoxSounds.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.listBoxSounds.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listBoxSounds.IntegralHeight = false;
			this.listBoxSounds.Location = new System.Drawing.Point(0, 43);
			this.listBoxSounds.Name = "listBoxSounds";
			this.listBoxSounds.Size = new System.Drawing.Size(150, 433);
			this.listBoxSounds.TabIndex = 3;
			this.listBoxSounds.MouseDown += new System.Windows.Forms.MouseEventHandler(this.listBoxSounds_MouseDown);
			this.listBoxSounds.SelectedIndexChanged += new System.EventHandler(this.listBoxSounds_SelectedIndexChanged);
			// 
			// textBoxSoundsDir
			// 
			this.textBoxSoundsDir.Dock = System.Windows.Forms.DockStyle.Top;
			this.textBoxSoundsDir.Location = new System.Drawing.Point(0, 23);
			this.textBoxSoundsDir.Name = "textBoxSoundsDir";
			this.textBoxSoundsDir.Size = new System.Drawing.Size(150, 20);
			this.textBoxSoundsDir.TabIndex = 2;
			this.textBoxSoundsDir.Text = "";
			this.textBoxSoundsDir.KeyDown += new System.Windows.Forms.KeyEventHandler(this.textBoxSoundsDir_KeyDown);
			// 
			// label2
			// 
			this.label2.BackColor = System.Drawing.Color.Navy;
			this.label2.Dock = System.Windows.Forms.DockStyle.Top;
			this.label2.Font = new System.Drawing.Font("Times New Roman", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label2.ForeColor = System.Drawing.Color.White;
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(150, 23);
			this.label2.TabIndex = 1;
			this.label2.Text = "Library";
			this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// statusBar1
			// 
			this.statusBar1.Location = new System.Drawing.Point(0, 480);
			this.statusBar1.Name = "statusBar1";
			this.statusBar1.Size = new System.Drawing.Size(768, 22);
			this.statusBar1.TabIndex = 2;
			this.statusBar1.Text = "statusBar1";
			// 
			// contextMenuListBox
			// 
			this.contextMenuListBox.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							   this.menuItemPlayOnPalm,
																							   this.menuItem5,
																							   this.menuItemCom1,
																							   this.menuItemCom2});
			// 
			// menuItemPlayOnPalm
			// 
			this.menuItemPlayOnPalm.Index = 0;
			this.menuItemPlayOnPalm.Text = "Play on Palm";
			this.menuItemPlayOnPalm.Click += new System.EventHandler(this.menuItemPlayOnPalm_Click);
			// 
			// menuItem5
			// 
			this.menuItem5.Index = 1;
			this.menuItem5.Text = "-";
			// 
			// menuItemCom1
			// 
			this.menuItemCom1.Index = 2;
			this.menuItemCom1.Text = "Com1";
			this.menuItemCom1.Click += new System.EventHandler(this.menuItemCom1_Click);
			// 
			// menuItemCom2
			// 
			this.menuItemCom2.Checked = true;
			this.menuItemCom2.Index = 3;
			this.menuItemCom2.Text = "Com2";
			this.menuItemCom2.Click += new System.EventHandler(this.menuItemCom2_Click);
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(768, 502);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panelMain,
																		  this.statusBar1});
			this.Menu = this.mainMenu1;
			this.Name = "Form1";
			this.Text = "Sound Scheme Editor";
			this.Closing += new System.ComponentModel.CancelEventHandler(this.Form1_Closing);
			this.panelMain.ResumeLayout(false);
			this.paneSfxSoundsParent.ResumeLayout(false);
			this.panelSfx.ResumeLayout(false);
			this.panelSfxProperties.ResumeLayout(false);
			this.panelSounds.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}

		void NewScheme() {
			ListViewItem[] aitem = new ListViewItem[m_alsNames.Count];
			for (int iItem = 0; iItem < m_alsNames.Count; iItem++) {
				string strT = ((StringCollection)m_alsNames[iItem])[0];
				ListViewItem item = new ListViewItem(new System.Windows.Forms.ListViewItem.ListViewSubItem[] {
						new ListViewItem.ListViewSubItem(null, strT, System.Drawing.SystemColors.WindowText, System.Drawing.SystemColors.Window, new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)))),
						new ListViewItem.ListViewSubItem(null, ""),
						new ListViewItem.ListViewSubItem(null, "0"),
						new ListViewItem.ListViewSubItem(null, "Unknown"),
						new ListViewItem.ListViewSubItem(null, "")}, -1);
				item.Checked = true;
				if (!(bool)m_alsSfxEnabled[iItem])
					item.ForeColor = Color.Salmon;
				//item.BackColor = Color.AliceBlue;
				aitem[iItem] = item;
			}
			listViewSfx.Items.Clear();
			listViewSfx.Items.AddRange(aitem);
		}

		private void textBoxSoundsDir_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e) {
			if (e.KeyCode == Keys.Enter) {
				SetSoundsDir(textBoxSoundsDir.Text);
			}		
		}

		void SetSoundsDir(string strDir) {
			m_fFillingListBox = true;
			try {
				textBoxSoundsDir.Text = strDir;
				ArrayList alsFiles = new ArrayList();
				alsFiles.AddRange(Directory.GetFiles(strDir, "*.wav"));
				alsFiles.AddRange(Directory.GetFiles(strDir, "*.snd"));
				ArrayList alsItems = new ArrayList();
				string strPath = Path.GetFullPath(textBoxSoundsDir.Text);
				foreach (string strFileFull in alsFiles) {
					string strFile = Path.GetFileName(strFileFull);
					Pcm pcm = new Pcm(strPath + "\\" + strFile);
					pcm.ConvertTo8Bit();
					alsItems.Add(strFile + ", " + pcm.Data8Bit.Length / 2 + " bytes");
				}
				listBoxSounds.DataSource = (string[])alsItems.ToArray(typeof(string));
			} catch {
				MessageBox.Show("Invalid directory!");
			}
			m_fFillingListBox = false;
		}

		private void listBoxSounds_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (Control.MouseButtons == MouseButtons.None)
				PlaySound(GetListboxItemFileName(listBoxSounds.SelectedIndex));
		}

		string GetListboxItemFileName(int iItem) {
			string strItem = (string)listBoxSounds.Items[iItem];
			int ichComma = strItem.IndexOf(',');
			return strItem.Remove(ichComma, strItem.Length - ichComma);
		}

		void PlaySound(string strFile) {
			try {
				if (!m_fFillingListBox && m_fPlaySound) {
					string strPath = Path.GetFullPath(textBoxSoundsDir.Text);
					Pcm pcm = new Pcm(strPath + "\\" + strFile);
					pcm.Play();
				}		
			} catch {
			}
		}

		private void Form1_Closing(object sender, System.ComponentModel.CancelEventArgs e) {
			SaveSettings();
		}

		void SaveSettings() {
			// Save settings in ini.
			
			Ini ini = new Ini();			
			Ini.Section secGeneral = new Ini.Section("General");
			secGeneral.Add(new Ini.Property("SoundsDir", Path.GetFullPath(textBoxSoundsDir.Text)));
			secGeneral.Add(new Ini.Property("Sfx.h", m_strSfxH));
			ini.Add(secGeneral);
			
			// Place in directory where .exe resides
			
			ini.Save(Application.ExecutablePath.Replace(".exe", ".ini"));
		}

		bool LoadSettings() {
			Ini ini;
			try {
				ini = new Ini(Application.ExecutablePath.Replace(".exe", ".ini"));
			} catch {
				ini = null;
			}
			
			// Sounds directory

			string strDirSounds = (ini != null) ? ini["General"]["SoundsDir"].Value : Directory.GetCurrentDirectory();
			SetSoundsDir(strDirSounds);
			
			// Sfx names

			string strSfxH = (ini != null && ini["General"]["Sfx.H"] != null) ? ini["General"]["Sfx.h"].Value : null;
			while (true) {
				if (strSfxH == null) {
					// Get filename
					OpenFileDialog frmOpen = new OpenFileDialog();
					frmOpen.Filter = "Include Files (*.h)|*.h";
					frmOpen.Title = "Open SoundEffects.h";
					if (frmOpen.ShowDialog() == DialogResult.Cancel)
						return false;
					strSfxH = frmOpen.FileName;
				}
				if (ParseNames(strSfxH))
					break;
				strSfxH = null;
			}
			m_strSfxH = strSfxH;

			return true;
		}

		private void menuItemPlayOnPalm_Click(object sender, System.EventArgs e) {
			int iItem = listBoxSounds.SelectedIndex;
			if (iItem == -1)
				return;
			string strFile = GetListboxItemFileName(iItem);
			try {
				string strPath = Path.GetFullPath(textBoxSoundsDir.Text);
				Pcm pcm = new Pcm(strPath + "\\" + strFile);
				PlayOnPalm(strFile, pcm.GetSndEncoding());
			} catch {
				MessageBox.Show("Couldn't load " + strFile);
			}
		}

		void PlayOnPalm(string strFile, byte[] abPcm) {
			try {
				SerialStream stm = new SerialStream("com" + m_nComPort + ":", FileAccess.Write);
				stm.SetPortSettings(19200, 8, SerialStream.StopBits.One, SerialStream.Parity.None, SerialStream.FlowControl.Hardware);
				stm.WriteByte((byte)'s');
				stm.WriteByte((byte)'c');
				stm.WriteByte((byte)'o');
				stm.WriteByte((byte)'t');
				stm.WriteByte((byte)'t');
				stm.WriteByte((byte)'l');
				stm.WriteByte((byte)'u');
				stm.WriteByte(0);
				foreach (char ch in strFile) {
					stm.WriteByte((byte)ch);
				}
				stm.WriteByte(0);
				stm.WriteByte((byte)((abPcm.Length >> 8) & 0xff));
				stm.WriteByte((byte)(abPcm.Length & 0xff));
				foreach (byte b in abPcm)
					stm.WriteByte(b);
				stm.Close();
			} catch {
				MessageBox.Show("Error sending data to Palm. Hotsync closed?");
			}
		}

		private void listBoxSounds_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
			for (int iItem = 0; iItem < listBoxSounds.Items.Count; iItem++) {
				Rectangle rcItem = listBoxSounds.GetItemRectangle(iItem);
				if (rcItem.Contains(e.X, e.Y)) {
					string strFile = GetListboxItemFileName(iItem);
					switch (e.Button) {
					case MouseButtons.Left:
						DragDropEffects effects = listBoxSounds.DoDragDrop(strFile, DragDropEffects.Link);
						if (effects == DragDropEffects.None) {
							// If the mouse came up at e.X, e.Y, play the sound since this is
							// just a mouse click

							Point ptNew = listBoxSounds.PointToClient(Control.MousePosition);
							if (ptNew.X == e.X && ptNew.Y == e.Y)
								PlaySound(strFile);
						}
						return;

					case MouseButtons.Right:
						listBoxSounds.SelectedIndex = iItem;
						contextMenuListBox.Show(listBoxSounds, new Point(e.X, e.Y));
						return;
					}
				}
			}
		}

		private void listViewSfx_DragEnter(object sender, System.Windows.Forms.DragEventArgs e) {
			if (e.Data.GetDataPresent(DataFormats.Text)) {
				e.Effect = DragDropEffects.Link;
			} else {
				e.Effect = DragDropEffects.None;
			}
		}

		private void listViewSfx_DragDrop(object sender, System.Windows.Forms.DragEventArgs e) {
			// Which item are we over?

			Point ptClient = listViewSfx.PointToClient(new Point(e.X, e.Y));
			ListViewItem itemOver = listViewSfx.GetItemAt(ptClient.X, ptClient.Y);
			if (itemOver == null)
				return;

			// Dragging another item or a string?

			if (e.Data.GetDataPresent(typeof(ListViewItem))) {
				ListViewItem item = (ListViewItem)e.Data.GetData(typeof(ListViewItem));
				for (int iSubItem = 1; iSubItem < 5; iSubItem++)
					itemOver.SubItems[iSubItem] = item.SubItems[iSubItem];
			} else {
				string strFile = e.Data.GetData(DataFormats.Text).ToString();
				itemOver.SubItems[1].Text = strFile;
			}

			// Select row, play sound

			SelectListViewSfxItem(itemOver);
			UpdateProperties(itemOver);
			PlaySound(itemOver.SubItems[1].Text);
			m_fPlaySound = false;
		}

		private void listViewSfx_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
#if false
			// Filter out our WM_LBUTTONDOWN selection hack

			Point ptClient = listViewSfx.PointToClient(Control.MousePosition);
			if (ptClient.X != e.X || ptClient.Y != e.Y)
				return;
#endif

#if false
			// For dragging sfx

			ListViewItem item = listViewSfx.GetItemAt(e.X, e.Y);
			DragDropEffects effects = DragDropEffects.None;
			if (item != null) {
				UpdateProperties(item);
				effects = listViewSfx.DoDragDrop(item, DragDropEffects.Copy);
			}
			Point ptT = listViewSfx.PointToClient(Control.MousePosition);
			item = listViewSfx.GetItemAt(ptT.X, ptT.Y);
			if (item != null) {
				SelectListViewSfxItem(item);
				UpdateProperties(item);
			}
			if (effects == DragDropEffects.None)
				m_fPlaySound = false;
#endif
		}

		private void listViewSfx_ItemDrag(object sender, System.Windows.Forms.ItemDragEventArgs e) {
			// For dragging sfx

			ListViewItem item = (ListViewItem)e.Item;
			UpdateProperties(item);
			listViewSfx.DoDragDrop(item, DragDropEffects.Copy);
			Point ptT = listViewSfx.PointToClient(Control.MousePosition);
			item = listViewSfx.GetItemAt(ptT.X, ptT.Y);
			if (item != null) {
				SelectListViewSfxItem(item);
				UpdateProperties(item);
			}
		}

		[DllImportAttribute("user32.dll")]
		private static extern void PostMessageW(IntPtr hwnd, uint wm, uint wp, uint lp);

		void SelectListViewSfxItem(ListViewItem item) {
#if false
			ListView.SelectedListViewItemCollection coll = new ListView.SelectedListViewItemCollection(listViewSfx);
			coll.Clear();
			item.Selected = true;
#else
			//Rectangle rc = listViewSfx.GetItemRect(item.Index);
			//Point pt = new Point(rc.Left + (rc.Right - rc.Left) / 2, rc.Top + (rc.Bottom - rc.Top) / 2);
			//uint wp = 0;
			//uint lp = (uint)((ushort)pt.X + (uint)(pt.Y << 16));
			//PostMessageW(listViewSfx.Handle, 0x201, wp, lp);
#endif
		}

		private void listViewSfx_DragOver(object sender, System.Windows.Forms.DragEventArgs e) {
			Point ptClient = listViewSfx.PointToClient(new Point(e.X, e.Y));
			ListViewItem item = listViewSfx.GetItemAt(ptClient.X, ptClient.Y);
			if (item == null || !(bool)m_alsSfxEnabled[item.Index]) {
				e.Effect = DragDropEffects.None;
				return;
			}
			e.Effect = DragDropEffects.Link | DragDropEffects.Copy;
		}

		void UpdateProperties(ListViewItem item) {
			// Save the displayed item

#if false
			if (m_itemDisplayed != null) {
				m_itemDisplayed.SubItems[1].Text = textBoxPropertiesSound.Text;
				m_itemDisplayed.SubItems[2].Text = comboBoxPropertiesChannel.SelectedIndex.ToString();
				m_itemDisplayed.SubItems[3].Text = comboBoxPropertiesPriority.SelectedIndex.ToString();
				m_itemDisplayed.SubItems[4].Text = textBoxPropertiesComment.Text;
			}
#endif

			// Display the new item

			m_itemDisplayed = item;
			if (m_itemDisplayed != null) {
				labelPropertiesSfx.Text = m_itemDisplayed.SubItems[0].Text;
				textBoxPropertiesSound.Text = m_itemDisplayed.SubItems[1].Text;
				//comboBoxPropertiesChannel.SelectedIndex = int.Parse(m_itemDisplayed.SubItems[2].Text);
				comboBoxPropertiesPriority.Text = m_itemDisplayed.SubItems[3].Text;
				textBoxPropertiesComment.Text = m_itemDisplayed.SubItems[4].Text;
			} else {
				labelPropertiesSfx.Text = "";
				textBoxPropertiesSound.Text = "";
				//comboBoxPropertiesChannel.SelectedIndex = -1;
				comboBoxPropertiesPriority.SelectedIndex = -1;
				textBoxPropertiesComment.Text = "";
			}
		}

		private void listViewSfx_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (listViewSfx.SelectedItems.Count == 0) {
				UpdateProperties(null);
			} else {
				UpdateProperties(listViewSfx.SelectedItems[0]);
			}
		}

		private void textBoxPropertiesComment_TextChanged(object sender, System.EventArgs e) {
			if (m_itemDisplayed != null)
				m_itemDisplayed.SubItems[4].Text = textBoxPropertiesComment.Text;
		}

		private void textBoxPropertiesSound_TextChanged(object sender, System.EventArgs e) {
			if (m_itemDisplayed != null)
				m_itemDisplayed.SubItems[1].Text = textBoxPropertiesSound.Text;
		}

#if false
		private void comboBoxPropertiesChannel_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (m_itemDisplayed != null)
				m_itemDisplayed.SubItems[2].Text = comboBoxPropertiesChannel.SelectedIndex.ToString();
		}
#endif

		private void comboBoxPropertiesPriority_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (m_itemDisplayed != null)
				m_itemDisplayed.SubItems[3].Text = comboBoxPropertiesPriority.Text;
		}

		private void listViewSfx_ItemActivate(object sender, System.EventArgs e) {
#if false
			string strSound = listViewSfx.SelectedItems[0].SubItems[1].Text;
			PlaySound(strSound);
#endif
		}

		private void listViewSfx_Leave(object sender, System.EventArgs e) {
			//UpdateProperties(null);		
		}

		public int Compare(object obj1, object obj2) {
			ListViewItem item1 = (ListViewItem)obj1;
			ListViewItem item2 = (ListViewItem)obj2;
			int n = m_comparer.Compare(item1.SubItems[m_iColumnSort].Text, item2.SubItems[m_iColumnSort].Text);
			return m_fAscending ? n : -n;
		}

		private void listViewSfx_ColumnClick(object sender, System.Windows.Forms.ColumnClickEventArgs e) {
			if (m_iColumnSort == e.Column) {
				m_fAscending = !m_fAscending;
			} else {
				m_fAscending = true;
				m_iColumnSort = e.Column;
			}

			if (listViewSfx.ListViewItemSorter == null) {
				// Setting the property causes Sort() to be called. Special.

				listViewSfx.ListViewItemSorter = this;
			} else {
				listViewSfx.Sort();
			}
		}

		private void listViewSfx_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e) {
			ListViewItem itemOver = listViewSfx.GetItemAt(e.X, e.Y);
			if (itemOver != null) {
				UpdateProperties(itemOver);
				PlaySound(itemOver.SubItems[1].Text);
			}
		}

		private void menuItemSaveAs_Click(object sender, System.EventArgs e) {
			SaveSfx(null);
		}

		private void menuItemSave_Click(object sender, System.EventArgs e) {
			SaveSfx(m_strSfxFile);
		}

		private void menuItemOpen_Click(object sender, System.EventArgs e) {
			LoadSfx();		
		}

		private void menuItemNew_Click(object sender, System.EventArgs e) {
			NewScheme();
		}

		void SaveSfx(string strFile) {
			if (strFile == null) {
				SaveFileDialog frmSave = new SaveFileDialog();
				frmSave.DefaultExt = "sfx";
				frmSave.Filter = "Sound Effect Files (*.sfx)|*.sfx";
				frmSave.Title = "Save As";
				frmSave.FileName = m_strSfxFile;
				if (frmSave.ShowDialog() == DialogResult.Cancel)
					return;
				strFile = frmSave.FileName;
			}

			TextWriter tw = new StreamWriter(strFile);
			for (int isfx = 0; isfx < m_alsNames.Count; isfx++) {
				ListViewItem item = listViewSfx.Items[isfx];
				if (item.Checked) {
					tw.WriteLine("checked");
				} else {
					tw.WriteLine("unchecked");
				}
				for (int iSubItem = 0; iSubItem < 5; iSubItem++) {
					string strT = item.SubItems[iSubItem].Text;
					if (strT == null)
						strT = "";
					tw.WriteLine(strT);
				}
			}
			tw.Close();
			m_strSfxFile = strFile;
		}

		void LoadSfx() {
			OpenFileDialog frmOpen = new OpenFileDialog();
			frmOpen.Filter = "Sound Effect Files (*.sfx)|*.sfx";
			frmOpen.Title = "Open";
			if (frmOpen.ShowDialog() == DialogResult.Cancel)
				return;
			string strFile = frmOpen.FileName;

			try {
				NewScheme();

				// Add soundeffects.h entries into list

				for (int iName = 0; iName < m_alsNames.Count; iName++) {
					StringCollection strc = (StringCollection)m_alsNames[iName];
					listViewSfx.Items[iName].Checked = true;
					listViewSfx.Items[iName].SubItems[0].Text = strc[0];
					listViewSfx.Items[iName].SubItems[1].Text = "";
					listViewSfx.Items[iName].SubItems[2].Text = "0";
					listViewSfx.Items[iName].SubItems[3].Text = "Unknown";
					listViewSfx.Items[iName].SubItems[4].Text = "";
				}

				// Now load sfx into this list

				TextReader tr = new StreamReader(strFile);
				while (true) {
					// checked or unchecked?

					string strChecked = tr.ReadLine();
					bool fChecked;
					if (strChecked == "checked") {
						fChecked = true;
					} else {
						fChecked = false;
					}

					// Look up sound effect name. Match against deprecated names too, and if found
					// map to new names

					bool fFound = false;
					string strName = tr.ReadLine();
					if (strName == null) {
						tr.Close();
						break;
					}

					int iName;
					for (iName = 0; iName < m_alsNames.Count; iName++) {
						StringCollection strc = (StringCollection)m_alsNames[iName];
						if (strc.IndexOf(strName) >= 0) {
							strName = strc[0];
							fFound = true;
							break;
						}
					}
					if (!fFound) {
						MessageBox.Show("Unknown sound effect: " + strName);
						tr.ReadLine();
						tr.ReadLine();
						tr.ReadLine();
						tr.ReadLine();
						continue;
					}

					// Add effect into this spot

					listViewSfx.Items[iName].Checked = fChecked;
					listViewSfx.Items[iName].SubItems[0].Text = strName;
					listViewSfx.Items[iName].SubItems[1].Text = tr.ReadLine();
					listViewSfx.Items[iName].SubItems[2].Text = tr.ReadLine();
					string strPriority = tr.ReadLine();
					if (m_strcPriorities.IndexOf(strPriority) == -1)
						strPriority = "Unknown";
					listViewSfx.Items[iName].SubItems[3].Text = strPriority;
					listViewSfx.Items[iName].SubItems[4].Text = tr.ReadLine();
				}
			} catch {
				MessageBox.Show("Error reading from " + strFile);
			}
			m_strSfxFile = strFile;
		}

		void SavePdb(string strPdbFile) {
			// Make a list of unique sound files

			PdbPacker pdbp = new PdbPacker();
			int cSfx = m_alsNames.Count;
			StringCollection strcUniqueSounds = new StringCollection();
			ArrayList alsPcm = new ArrayList();
			for (int iSfx = 0; iSfx < cSfx; iSfx++) {
				if (!listViewSfx.Items[iSfx].Checked)
					continue;
				if (!(bool)m_alsSfxEnabled[iSfx])
					continue;
				string strFile = listViewSfx.Items[iSfx].SubItems[1].Text;
				if (strFile == null)
					continue;
				strFile.Trim();
				if (strFile.Length == 0)
					continue;
				int istr = strcUniqueSounds.IndexOf(strFile);
				if (istr == -1)
					istr = strcUniqueSounds.Add(strFile);
			}

			// Serialize names out

			ArrayList alsStringOffsets = new ArrayList();
			BinaryWriter bwtr = new BinaryWriter(new MemoryStream());
			bwtr.Write(Misc.SwapUShort((ushort)strcUniqueSounds.Count));
			for (int iSound = 0; iSound < strcUniqueSounds.Count; iSound++) {
				alsStringOffsets.Add(bwtr.BaseStream.Position);
				string strFile = Path.ChangeExtension(strcUniqueSounds[iSound], ".snd");
				char[] sz = strFile.ToCharArray();
				bwtr.Write(sz);
				bwtr.Write((byte)0);
			}
			byte[] abSoundFiles = new byte[bwtr.BaseStream.Length];
			bwtr.BaseStream.Seek(0, SeekOrigin.Begin);
			bwtr.BaseStream.Read(abSoundFiles, 0, (int)bwtr.BaseStream.Length);
			bwtr.Close();

			// soundfiles file

			PdbPacker.File fileSounds = new PdbPacker.File("soundfiles", abSoundFiles);
			pdbp.Add(fileSounds);

			// Now serialize the sfx entries in the order of the names

			bwtr = new BinaryWriter(new MemoryStream());
			for (int iName = 0; iName < m_alsNames.Count; iName++) {
				// Need to find the entry in listViewSfx for this name since the persist
				// order needs to match soundeffects.h.

				string strName = ((StringCollection)m_alsNames[iName])[0];
				int iSfx;
				bool fFound = false;
				for (iSfx = 0; iSfx < cSfx; iSfx++) {
					if (strName == listViewSfx.Items[iSfx].SubItems[0].Text) {
						fFound = true;
						break;
					}
				}
				if (!fFound)
					throw new Exception("Internal error");

				string strFile = listViewSfx.Items[iSfx].SubItems[1].Text;
				if (!listViewSfx.Items[iSfx].Checked)
					strFile = null;
				if (!(bool)m_alsSfxEnabled[iSfx])
					strFile = null;
				if (strFile == null) {
					bwtr.Write((byte)0xff);
				} else {
					strFile.Trim();
					if (strFile.Length == 0) {
						bwtr.Write((byte)0xff);
					} else {
						bwtr.Write((byte)strcUniqueSounds.IndexOf(strFile));
					}
				}
				bwtr.Write((byte)0); // bwtr.Write(byte.Parse(listViewSfx.Items[iSfx].SubItems[2].Text));
				int nPriority = m_strcPriorities.IndexOf(listViewSfx.Items[iSfx].SubItems[3].Text);
				if (nPriority < 0) {
					MessageBox.Show("Warning: " + listViewSfx.Items[iSfx].SubItems[0].Text + " has an unfamiliar priority.");
				}
				bwtr.Write((byte)nPriority);
			}
			byte[] abSfxEntries = new byte[bwtr.BaseStream.Length];
			bwtr.BaseStream.Seek(0, SeekOrigin.Begin);
			bwtr.BaseStream.Read(abSfxEntries, 0, (int)bwtr.BaseStream.Length);
			bwtr.Close();

			PdbPacker.File fileSfxEntries = new PdbPacker.File("SfxEntries", abSfxEntries);
			pdbp.Add(fileSfxEntries);

			// Now add in all the sounds

			for (int istrFile = 0; istrFile < strcUniqueSounds.Count; istrFile++) {
				string strFile = Path.GetFullPath(textBoxSoundsDir.Text) + "\\" + strcUniqueSounds[istrFile];
				Pcm pcm = new Pcm(strFile);
				PdbPacker.File fileT = new PdbPacker.File();
				fileT.str = Path.ChangeExtension(strcUniqueSounds[istrFile], ".snd");
				fileT.ab = pcm.GetSndEncoding();
				fileT.fCompress = false;
				pdbp.Add(fileT);
			}

			// Ready to save pdb

			pdbp.Save(strPdbFile, "WARI");
		}

		unsafe static byte[] SerializeStructure(byte[] ab, void *pv, int cb) {
			byte *pb = (byte *)pv;
			byte[] abNew = null;
			if (ab == null) {
				abNew = new byte[cb];
				for (int i = 0; i < cb; i++)
					abNew[i] = pb[i];
			} else {
				abNew = new byte[ab.Length + cb];
				for (int i = 0; i < ab.Length; i++)
					abNew[i] = ab[i];
				for (int i = 0; i < cb; i++)
					abNew[i + ab.Length] = pb[i];
			}

			return abNew;
		}

		private void menuItemMakePdb_Click(object sender, System.EventArgs e) {
			SaveFileDialog frmSave = new SaveFileDialog();
			frmSave.DefaultExt = "pdb";
			frmSave.Filter = "Palm Database Files (*.pdb)|*.pdb";
			frmSave.Title = "Save As";
			frmSave.FileName = m_strPdbFile;
			if (frmSave.ShowDialog() == DialogResult.Cancel)
				return;
			SavePdb(frmSave.FileName);		
		}

		public bool ParseNames(string strFile) {
			// Read it

			TextReader tr;
			try {
				tr = new StreamReader(strFile);
			} catch {
				return false;
			}

			// Find the line that starts with "enum"

			while (true) {
				string strT = tr.ReadLine();
				if (strT == null) {
					tr.Close();
					return false;
				}
				if (strT.StartsWith("enum"))
					break;
			}

			// Start reading names.

			m_alsNames.Clear();
			m_alsSfxEnabled.Clear();
			while (true) {
				string strT = tr.ReadLine();
				if (strT == null)
					break;
				if (strT == "// stop")
					break;

				Regex rexNames0 = new Regex(@"^\s*(?<name0>[a-zA-Z_0-9]+)\,\s\/\/\s*(?<enabled>[a-zA-Z]+)\;.*$");
				Match matNames0 = rexNames0.Match(strT);
				if (matNames0.Groups["name0"].Value.Length != 0) {
					StringCollection strc = new StringCollection();
					strc.Add(matNames0.Groups["name0"].Value);
					bool fEnabled;
					if (matNames0.Groups["enabled"].Value == "disabled") {
						fEnabled = false;
					} else {
						fEnabled = true;
					}

					Regex rexNames1 = new Regex(@"^\s*(?<name0>[a-zA-Z_0-9]+)\,\s\/\/\s*(?<enabled>[a-zA-Z]+)\;\s*(?<name1>[a-zA-Z_0-9]+).*$");
					Match matNames1 = rexNames1.Match(strT);
					string strMatch = matNames1.Groups["name1"].Value;
					if (strMatch.Length != 0) {
						strc.Add(matNames1.Groups["name1"].Value);

						Regex rexNames2 = new Regex(@"^\s*(?<name0>[a-zA-Z_0-9]+)\,\s\/\/\s*(?<enabled>[a-zA-Z]+)\;\s*(?<name1>[a-zA-Z_0-9]+)\,\s*(?<name2>[a-zA-Z_0-9]+).*$");
						Match matNames2 = rexNames2.Match(strT);
						strMatch = matNames2.Groups["name2"].Value;
						if (strMatch.Length != 0) {
							strc.Add(matNames2.Groups["name2"].Value);

							Regex rexNames3 = new Regex(@"^\s*(?<name0>[a-zA-Z_0-9]+)\,\s\/\/\s*(?<enabled>[a-zA-Z]+)\;\s*(?<name1>[a-zA-Z_0-9]+)\,\s*(?<name2>[a-zA-Z_0-9]+)\,\s*(?<name3>[a-zA-Z_0-9]+).*$");
							Match matNames3 = rexNames3.Match(strT);
							strMatch = matNames3.Groups["name3"].Value;
							if (strMatch.Length != 0) {
								strc.Add(matNames3.Groups["name3"].Value);
							}
						}
					}
					m_alsSfxEnabled.Add(fEnabled);
					m_alsNames.Add(strc);
				}
			}
			tr.Close();
			return (m_alsNames.Count != 0);
		}

		private void menuItemCheckAll_Click(object sender, System.EventArgs e) {
			foreach (ListViewItem item in listViewSfx.Items)
				item.Checked = true;
		}

		private void menuItemUncheckAll_Click(object sender, System.EventArgs e) {
			foreach (ListViewItem item in listViewSfx.Items)
				item.Checked = false;
		}

		private void menuItemCopy_Click(object sender, System.EventArgs e) {
			if (listViewSfx.SelectedItems == null)
				return;
			ListViewItem item = listViewSfx.SelectedItems[0];
			if (item == null)
				return;
			m_itemForCopy = new ListViewItem(new System.Windows.Forms.ListViewItem.ListViewSubItem[] {
						new ListViewItem.ListViewSubItem(null, "n/a", System.Drawing.SystemColors.WindowText, System.Drawing.SystemColors.Window, new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)))),
						new ListViewItem.ListViewSubItem(null, item.SubItems[1].Text),
						new ListViewItem.ListViewSubItem(null, item.SubItems[2].Text),
						new ListViewItem.ListViewSubItem(null, item.SubItems[3].Text),
						new ListViewItem.ListViewSubItem(null, item.SubItems[4].Text)
					} , -1);
			m_itemForCopy.Checked = item.Checked;
		}

		private void menuItemPaste_Click(object sender, System.EventArgs e) {
			if (m_itemForCopy == null)
				return;
			ListViewItem itemNew = listViewSfx.SelectedItems[0];
			if (itemNew == null)
				return;
			if (!(bool)m_alsSfxEnabled[itemNew.Index])
				return;
			if (itemNew.Index == m_itemForCopy.Index)
				return;
			itemNew.Checked = m_itemForCopy.Checked;
			for (int i = 1; i < 5; i++) {
				itemNew.SubItems[i].Text = m_itemForCopy.SubItems[i].Text;
			}
			PlaySound(itemNew.SubItems[1].Text);
		}

		private void menuItemClear_Click(object sender, System.EventArgs e) {
			ListViewItem item = listViewSfx.SelectedItems[0];
			if (item == null)
				return;
			item.SubItems[1].Text = "";
			item.SubItems[2].Text = "0";
			item.SubItems[3].Text = "Highest";
			item.SubItems[4].Text = "";
		}

		private void menuItemCheckPdbSize_Click(object sender, System.EventArgs e) {
			SavePdb("checksize.pdb");
			Stream stm = File.OpenRead("checksize.pdb");
			MessageBox.Show(stm.Length + " bytes.");
			stm.Close();
		}

		private void menuItemTestCom2_Click(object sender, System.EventArgs e) {
#if false
			SerialStream stm = new SerialStream("com2:", FileAccess.Write);
			stm.SetPortSettings(19200, 8, SerialStream.StopBits.One, SerialStream.Parity.None, SerialStream.FlowControl.None);
			byte[] ab = new Byte[] { (byte)'h', (byte)'e', (byte)'l', (byte)'l', (byte)'o', (byte)' ', (byte)'w', (byte)'o', (byte)'r', (byte)'l', (byte)'d', 0 };
			stm.Write(ab, 0, ab.Length);
			stm.Close();
#endif
		}

		private void menuItemCom2_Click(object sender, System.EventArgs e) {
			m_nComPort = 2;
			menuItemCom1.Checked = false;
			menuItemCom2.Checked = true;
		}

		private void menuItemCom1_Click(object sender, System.EventArgs e) {
			m_nComPort = 1;
			menuItemCom1.Checked = true;
			menuItemCom2.Checked = false;
		}
	}
}
