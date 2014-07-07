using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;
using System.Text;

namespace m
{
	/// <summary>
	/// Summary description for EditTextForm.
	/// </summary>
	public class EditLevelTextForm : System.Windows.Forms.Form
	{
		private string m_strLevelText;
		private LevelDoc m_lvld;
		public System.Windows.Forms.RichTextBox richTextBox1;
		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem mniExport;
		private System.Windows.Forms.MenuItem mniImport;
		private System.Windows.Forms.OpenFileDialog ofd;
		private System.Windows.Forms.SaveFileDialog sfd;
		private System.Windows.Forms.MenuItem mniExit;
		private System.Windows.Forms.MenuItem mniCut;
		private System.Windows.Forms.MenuItem mniCopy;
		private System.Windows.Forms.MenuItem mniPaste;
		private System.Windows.Forms.MenuItem mniFind;
		private System.Windows.Forms.MenuItem menuItem9;
		private System.Windows.Forms.MenuItem mniSave;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public EditLevelTextForm(LevelDoc lvld)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_lvld = lvld;
			m_strLevelText = m_lvld.GetLevelText();
			richTextBox1.Text = m_strLevelText;
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

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.richTextBox1 = new System.Windows.Forms.RichTextBox();
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.mniSave = new System.Windows.Forms.MenuItem();
			this.mniImport = new System.Windows.Forms.MenuItem();
			this.mniExport = new System.Windows.Forms.MenuItem();
			this.mniExit = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.mniCut = new System.Windows.Forms.MenuItem();
			this.mniCopy = new System.Windows.Forms.MenuItem();
			this.mniPaste = new System.Windows.Forms.MenuItem();
			this.menuItem9 = new System.Windows.Forms.MenuItem();
			this.mniFind = new System.Windows.Forms.MenuItem();
			this.ofd = new System.Windows.Forms.OpenFileDialog();
			this.sfd = new System.Windows.Forms.SaveFileDialog();
			this.SuspendLayout();
			// 
			// richTextBox1
			// 
			this.richTextBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.richTextBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.richTextBox1.HideSelection = false;
			this.richTextBox1.Name = "richTextBox1";
			this.richTextBox1.Size = new System.Drawing.Size(720, 710);
			this.richTextBox1.TabIndex = 0;
			this.richTextBox1.Text = "richTextBox1";
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem1,
																					  this.menuItem2});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mniSave,
																					  this.mniImport,
																					  this.mniExport,
																					  this.mniExit});
			this.menuItem1.Text = "&File";
			// 
			// mniSave
			// 
			this.mniSave.Index = 0;
			this.mniSave.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
			this.mniSave.Text = "&Save";
			this.mniSave.Click += new System.EventHandler(this.mniSave_Click);
			// 
			// mniImport
			// 
			this.mniImport.Index = 1;
			this.mniImport.Text = "&Import...";
			this.mniImport.Click += new System.EventHandler(this.mniImport_Click);
			// 
			// mniExport
			// 
			this.mniExport.Index = 2;
			this.mniExport.Text = "&Export...";
			this.mniExport.Click += new System.EventHandler(this.mniExport_Click);
			// 
			// mniExit
			// 
			this.mniExit.Index = 3;
			this.mniExit.Text = "E&xit";
			this.mniExit.Click += new System.EventHandler(this.mniExit_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 1;
			this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mniCut,
																					  this.mniCopy,
																					  this.mniPaste,
																					  this.menuItem9,
																					  this.mniFind});
			this.menuItem2.Text = "&Edit";
			// 
			// mniCut
			// 
			this.mniCut.Index = 0;
			this.mniCut.Shortcut = System.Windows.Forms.Shortcut.CtrlX;
			this.mniCut.Text = "C&ut";
			this.mniCut.Click += new System.EventHandler(this.mniCut_Click);
			// 
			// mniCopy
			// 
			this.mniCopy.Index = 1;
			this.mniCopy.Shortcut = System.Windows.Forms.Shortcut.CtrlC;
			this.mniCopy.Text = "&Copy";
			this.mniCopy.Click += new System.EventHandler(this.mniCopy_Click);
			// 
			// mniPaste
			// 
			this.mniPaste.Index = 2;
			this.mniPaste.Shortcut = System.Windows.Forms.Shortcut.CtrlV;
			this.mniPaste.Text = "&Paste";
			this.mniPaste.Click += new System.EventHandler(this.mniPaste_Click);
			// 
			// menuItem9
			// 
			this.menuItem9.Index = 3;
			this.menuItem9.Text = "-";
			// 
			// mniFind
			// 
			this.mniFind.Index = 4;
			this.mniFind.Shortcut = System.Windows.Forms.Shortcut.CtrlF;
			this.mniFind.Text = "&Find...";
			this.mniFind.Click += new System.EventHandler(this.mniFind_Click);
			// 
			// ofd
			// 
			this.ofd.DefaultExt = "txt";
			this.ofd.Filter = "Text files|*.txt|All files|*.*";
			this.ofd.Title = "Import";
			// 
			// sfd
			// 
			this.sfd.DefaultExt = "txt";
			this.sfd.FileName = "doc1";
			this.sfd.Filter = "Text files|*.txt|All files|*.*";
			this.sfd.Title = "Export As";
			// 
			// EditLevelTextForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(720, 710);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.richTextBox1});
			this.Menu = this.mainMenu1;
			this.Name = "EditLevelTextForm";
			this.Text = "Level Text";
			this.ResumeLayout(false);

		}
		#endregion

		protected override void OnClosing(System.ComponentModel.CancelEventArgs e) {
			if (richTextBox1.Text == m_strLevelText)
				return;

			DialogResult dlgr = MessageBox.Show(this, "Save Changes?", "M", MessageBoxButtons.YesNoCancel, 
					MessageBoxIcon.Question);
			switch (dlgr) {
			case DialogResult.Yes:
				int ichErrorPos;
				if (m_lvld.SetLevelText(richTextBox1.Text, out ichErrorPos)) {
					DialogResult = DialogResult.OK;
				} else {
					richTextBox1.Select(ichErrorPos, 0);
					e.Cancel = true;
				}
				break;

			case DialogResult.No:
				DialogResult = DialogResult.Cancel;
				break;

			case DialogResult.Cancel:
				e.Cancel = true;
				break;
			}
		}

		private void mniImport_Click(object sender, System.EventArgs e) {
			if (ofd.FileName == "")
				ofd.FileName = Path.GetFileNameWithoutExtension(m_lvld.GetPath()) + ".txt";

			DialogResult dlgr;
			if (richTextBox1.Text != m_strLevelText) {
				dlgr = MessageBox.Show(this, "Save changes first?", "M", MessageBoxButtons.YesNoCancel, 
					MessageBoxIcon.Question);
				switch (dlgr) {
				case DialogResult.Yes:
					int ichErrorPos;
					if (m_lvld.SetLevelText(richTextBox1.Text, out ichErrorPos)) {
						DialogResult = DialogResult.OK;
					} else {
						richTextBox1.Select(ichErrorPos, 0);
						return;
					}
					break;

				case DialogResult.No:
					break;

				case DialogResult.Cancel:
					return;
				}
			}			

			dlgr = ofd.ShowDialog(this);
			if (dlgr != DialogResult.OK)
				return;

			StreamReader stmr = new StreamReader(ofd.FileName);
			string str = stmr.ReadToEnd();
			stmr.Close();

			richTextBox1.Text = str;
			Scrub();
		}

		private void mniExport_Click(object sender, System.EventArgs e) {
			sfd.FileName = Path.GetFileNameWithoutExtension(m_lvld.GetPath()) + ".txt";
			DialogResult dlgr = sfd.ShowDialog(this);
			if (dlgr != DialogResult.OK)
				return;

			StringReader strr = new StringReader(richTextBox1.Text);
			StreamWriter stmw = new StreamWriter(sfd.FileName);

			while (true) {
				string strT = strr.ReadLine();
				if (strT == null)
					break;
				stmw.WriteLine(strT);
			}

			stmw.Close();
			strr.Close();
		}

		private void mniExit_Click(object sender, System.EventArgs e) {
			Close();
		}

		private void mniCut_Click(object sender, System.EventArgs e) {
			richTextBox1.Cut();
		}

		private void mniCopy_Click(object sender, System.EventArgs e) {
			richTextBox1.Copy();
		}

		private void mniPaste_Click(object sender, System.EventArgs e) {
			richTextBox1.Paste();
			Scrub();
		}

		private void Scrub() {
			bool fModified = false;
			string strT = richTextBox1.Text;
			StringBuilder strb = new StringBuilder();
			for (int ich = 0; ich < strT.Length; ich++) {
				char ch = strT[ich];
				if (ch == 0x2018 || ch == 0x2019) { // curly-single-quotes
					ch = '\'';
					fModified = true;
				} else if (ch == 0x201c || ch == 0x201d) { // curly-double-quotes
					ch = '"';
					fModified = true;
				} else if (ch == 0x2013) { // --
					ch = '-';
					strb.Append(ch);
					fModified = true;
				} else if (ch == 0x2026) { // ...
					ch = '.';
					strb.Append(ch);
					strb.Append(ch);
					fModified = true;
				}

				if (ch > 127)
					MessageBox.Show(this, String.Format("Unhandled illegal character {0} (0x{1:X})", ch, (Int32)ch), "M");

				strb.Append(ch);
			}

			if (fModified) {
				MessageBox.Show(this, "Invalid characters were found and converted to valid ones.", "M");
				richTextBox1.Text = strb.ToString();
			}
		}

		private void mniDelete_Click(object sender, System.EventArgs e) {
			MessageBox.Show("Coming soon...");
		}

		private void mniFind_Click(object sender, System.EventArgs e) {
			FindLevelTextForm frm = new FindLevelTextForm(richTextBox1);
			frm.Owner = this;
			frm.Show();
		}

		private void mniSave_Click(object sender, System.EventArgs e) {
			Scrub();

			int ichErrorPos;
			if (!m_lvld.SetLevelText(richTextBox1.Text, out ichErrorPos))
				richTextBox1.Select(ichErrorPos, 0);
			else
				m_strLevelText = richTextBox1.Text;
		}
	}
}
