using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using Microsoft.Vsa;
using System.IO;

namespace AED
{
	/// <summary>
	/// Summary description for ScriptEditor.
	/// </summary>
	public class ScriptEditor : System.Windows.Forms.Form
	{
		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.RichTextBox rtbEdit;
		private System.Windows.Forms.RichTextBox rtbOutput;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem mniCompile;
		private System.Windows.Forms.MenuItem mniRun;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.MenuItem mniOpen;
		private System.Windows.Forms.OpenFileDialog openFileDialog;
		private System.Windows.Forms.StatusBar stb;
		private System.Windows.Forms.StatusBarPanel stbpText;
		private System.Windows.Forms.StatusBarPanel stbpLine;
		private System.Windows.Forms.StatusBarPanel stbpColumn;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem mniLibraryColorMapper;
		private System.Windows.Forms.MenuItem mniLibraryOriginOffsetter;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="se"></param>
		/// <param name="strScript"></param>
		public ScriptEditor(ScriptEngine se, string strScript)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			// Non-Designer initialization

			m_se = se;
			m_se.CompileError += new CompileErrorEventHandler(OnCompileError);
			Script = strScript;

			m_tbwtr = new TextBoxWriter(rtbOutput);
//			m_tbwtr.WriteLine("s: {0}, l: {1}", rtbEdit.SelectionStart, rtbEdit.SelectionLength);
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
			this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.mniOpen = new System.Windows.Forms.MenuItem();
			this.stbpColumn = new System.Windows.Forms.StatusBarPanel();
			this.splitter1 = new System.Windows.Forms.Splitter();
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.menuItem4 = new System.Windows.Forms.MenuItem();
			this.menuItem5 = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.mniCompile = new System.Windows.Forms.MenuItem();
			this.mniRun = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.mniLibraryColorMapper = new System.Windows.Forms.MenuItem();
			this.mniLibraryOriginOffsetter = new System.Windows.Forms.MenuItem();
			this.rtbEdit = new System.Windows.Forms.RichTextBox();
			this.stbpLine = new System.Windows.Forms.StatusBarPanel();
			this.stb = new System.Windows.Forms.StatusBar();
			this.stbpText = new System.Windows.Forms.StatusBarPanel();
			this.rtbOutput = new System.Windows.Forms.RichTextBox();
			((System.ComponentModel.ISupportInitialize)(this.stbpColumn)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.stbpLine)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.stbpText)).BeginInit();
			this.SuspendLayout();
			// 
			// openFileDialog
			// 
			this.openFileDialog.DefaultExt = "js";
			this.openFileDialog.Filter = "EcmaScript/JavaScript (*.js)|*.js|All files (*.*)|*.*";
			this.openFileDialog.Title = "Open Script";
			// 
			// mniOpen
			// 
			this.mniOpen.Index = 0;
			this.mniOpen.Text = "&Open...";
			this.mniOpen.Click += new System.EventHandler(this.mniOpen_Click);
			// 
			// stbpColumn
			// 
			this.stbpColumn.MinWidth = 40;
			this.stbpColumn.Width = 50;
			// 
			// splitter1
			// 
			this.splitter1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.splitter1.Location = new System.Drawing.Point(0, 284);
			this.splitter1.Name = "splitter1";
			this.splitter1.Size = new System.Drawing.Size(520, 3);
			this.splitter1.TabIndex = 2;
			this.splitter1.TabStop = false;
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem2,
																					  this.menuItem1,
																					  this.menuItem3});
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 0;
			this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mniOpen,
																					  this.menuItem4,
																					  this.menuItem5});
			this.menuItem2.Text = "&File";
			// 
			// menuItem4
			// 
			this.menuItem4.Enabled = false;
			this.menuItem4.Index = 1;
			this.menuItem4.Text = "&Save";
			// 
			// menuItem5
			// 
			this.menuItem5.Enabled = false;
			this.menuItem5.Index = 2;
			this.menuItem5.Text = "Save &As...";
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 1;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mniCompile,
																					  this.mniRun});
			this.menuItem1.Text = "Script";
			// 
			// mniCompile
			// 
			this.mniCompile.Index = 0;
			this.mniCompile.Shortcut = System.Windows.Forms.Shortcut.F7;
			this.mniCompile.Text = "&Compile";
			this.mniCompile.Click += new System.EventHandler(this.mniCompile_Click);
			// 
			// mniRun
			// 
			this.mniRun.Index = 1;
			this.mniRun.Shortcut = System.Windows.Forms.Shortcut.F5;
			this.mniRun.Text = "&Run";
			this.mniRun.Click += new System.EventHandler(this.mniRun_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 2;
			this.menuItem3.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mniLibraryColorMapper,
																					  this.mniLibraryOriginOffsetter});
			this.menuItem3.Text = "&Library";
			// 
			// mniLibraryColorMapper
			// 
			this.mniLibraryColorMapper.Index = 0;
			this.mniLibraryColorMapper.Text = "&Color Mapper";
			this.mniLibraryColorMapper.Click += new System.EventHandler(this.mniLibraryColorMapper_Click);
			// 
			// mniLibraryOriginOffsetter
			// 
			this.mniLibraryOriginOffsetter.Index = 1;
			this.mniLibraryOriginOffsetter.Text = "&Origin Offsetter";
			this.mniLibraryOriginOffsetter.Click += new System.EventHandler(this.mniLibraryOriginOffsetter_Click);
			// 
			// rtbEdit
			// 
			this.rtbEdit.Dock = System.Windows.Forms.DockStyle.Fill;
			this.rtbEdit.Name = "rtbEdit";
			this.rtbEdit.Size = new System.Drawing.Size(520, 383);
			this.rtbEdit.TabIndex = 0;
			this.rtbEdit.Text = "";
			this.rtbEdit.WordWrap = false;
			this.rtbEdit.SelectionChanged += new System.EventHandler(this.rtbEdit_SelectionChanged);
			// 
			// stbpLine
			// 
			this.stbpLine.MinWidth = 40;
			this.stbpLine.Width = 50;
			// 
			// stb
			// 
			this.stb.Location = new System.Drawing.Point(0, 383);
			this.stb.Name = "stb";
			this.stb.Panels.AddRange(new System.Windows.Forms.StatusBarPanel[] {
																				   this.stbpText,
																				   this.stbpLine,
																				   this.stbpColumn});
			this.stb.ShowPanels = true;
			this.stb.Size = new System.Drawing.Size(520, 20);
			this.stb.TabIndex = 3;
			// 
			// stbpText
			// 
			this.stbpText.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Spring;
			this.stbpText.Width = 404;
			// 
			// rtbOutput
			// 
			this.rtbOutput.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.rtbOutput.Location = new System.Drawing.Point(0, 287);
			this.rtbOutput.Name = "rtbOutput";
			this.rtbOutput.Size = new System.Drawing.Size(520, 96);
			this.rtbOutput.TabIndex = 1;
			this.rtbOutput.Text = "";
			this.rtbOutput.WordWrap = false;
			// 
			// ScriptEditor
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(520, 403);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.splitter1,
																		  this.rtbOutput,
																		  this.rtbEdit,
																		  this.stb});
			this.Menu = this.mainMenu1;
			this.Name = "ScriptEditor";
			this.Text = "Script Editor";
			this.Closing += new System.ComponentModel.CancelEventHandler(this.ScriptEditor_Closing);
			((System.ComponentModel.ISupportInitialize)(this.stbpColumn)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.stbpLine)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.stbpText)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		//
		// Non-Windows.Forms state
		//

		private ScriptEngine m_se;
		private TextBoxWriter m_tbwtr;

		//
		// Public Properties
		//

		public string Script {
			get {
				return rtbEdit.Text;
			}

			set {
				rtbEdit.Text = value;
				UpdateLineColumnIndicator();

				// For some reason this can't be set until after it has been read (inside
				// of UpdateLineColumnIndicator in this case). Otherwise a "SelectionStart
				// argument (-1) invalid" exception is raised.

				rtbEdit.SelectionLength = 0;
			}
		}

		public event EventHandler ScriptEditorClosing;

		//
		// UI event handlers
		//

		private void mniCompile_Click(object sender, System.EventArgs e) {
			ClearErrors();
			ClearOutput();
			// UNDONE: clear colored (error) text
			bool fSuccess = false;
//			try {
				fSuccess = m_se.Compile(rtbEdit.Text);
//			} catch (Exception ex) {
//				WriteLine("Errors found during compile:\n{0}", ex.Message);
//				return;
//			}

			if (fSuccess)
				WriteLine("No errors");
		}

		private void mniRun_Click(object sender, System.EventArgs e) {
			ClearErrors();
			ClearOutput();
			TextWriter twtrConsoleOut = Console.Out;
			Console.SetOut(m_tbwtr);

			// UNDONE: clear colored (error) text
			//			try {
				m_se.Compile(rtbEdit.Text);
//			} catch {
//				return;
//			}
			m_se.Run();

			Console.SetOut(twtrConsoleOut);
		}

		private void mniOpen_Click(object sender, System.EventArgs e) {
			if (openFileDialog.ShowDialog() != DialogResult.OK)
				return;
			rtbEdit.LoadFile(openFileDialog.FileName);
		}

		// UNDONE: somehow determine where the insertion point really is and use it, not the
		// selection start.
		private void rtbEdit_SelectionChanged(object sender, System.EventArgs e) {
			UpdateLineColumnIndicator();
		}

		//
		// Everything else
		//

		protected virtual void OnScriptEditorClosing(EventArgs evta) {
			if (ScriptEditorClosing != null)
				ScriptEditorClosing(this, evta);
			m_se.CompileError -= new CompileErrorEventHandler(OnCompileError);
		}

		private void OnCompileError(object obSender, IVsaError vsaerr) {
			WriteLine("({0},{1}): sev {4}, {5} {2:x}: {3}", vsaerr.Line, vsaerr.StartColumn,
					vsaerr.Number, vsaerr.Description, vsaerr.Severity, vsaerr.Severity > 0 ? "warning" : "error");
			int ichSelStart = rtbEdit.SelectionStart;
			int ichSelLength = rtbEdit.SelectionLength;
			int ichT = GetCharIndexFromLine(rtbEdit.Text, vsaerr.Line - 1);
			rtbEdit.SelectionStart = ichT + vsaerr.StartColumn - 1; // columns are numbered starting at 1
			rtbEdit.SelectionLength = vsaerr.EndColumn - vsaerr.StartColumn;
			rtbEdit.SelectionColor = Color.FromArgb(255, 0, 128);
			rtbEdit.SelectionStart = ichSelStart;
			rtbEdit.SelectionLength = ichSelLength;
		}

		private int GetCharIndexFromLine(string strText, int iLine) {
			if (iLine == 0)
				return 0;
			int cLines = 0;
			for (int i = 0; i < strText.Length; i++) {
				if (strText[i] == '\n') {
					cLines++;
					if (cLines == iLine)
						return i + 1;
				}
			}
			return 0;
		}

		private void ClearOutput() {
			rtbOutput.Clear();
		}

		private void ClearErrors() {
			string strT = rtbEdit.Text;
			int ichSelStart = rtbEdit.SelectionStart;
			int ichSelLength = rtbEdit.SelectionLength;
			rtbEdit.Text = null;
			rtbEdit.Text = strT;
			rtbEdit.SelectionStart = ichSelStart;
			rtbEdit.SelectionLength = ichSelLength;
		}

		private void WriteLine(string strFormat, params object[] aobParams) {
			Write(strFormat, aobParams);
			Write("\n");
		}

		private void Write(string strFormat, params object[] aobParams) {
			string strT = string.Format(strFormat, aobParams);
			rtbOutput.Text += strT;
		}

		private void ScriptEditor_Closing(object sender, System.ComponentModel.CancelEventArgs e) {
			OnScriptEditorClosing(EventArgs.Empty);
		}


		private void UpdateLineColumnIndicator() {
			int ichInsertionPoint = rtbEdit.SelectionStart;
			int iLine = rtbEdit.GetLineFromCharIndex(ichInsertionPoint) + 1;
			stbpLine.Text = "Ln " + iLine;
			int ich = ichInsertionPoint;
			while (--ich >= 0 && rtbEdit.Text[ich] != '\n');
			stbpColumn.Text = "Col " + (ichInsertionPoint - ich);
		}

		private void mniLibraryColorMapper_Click(object sender, System.EventArgs e) {
			rtbEdit.AppendText(@"// Change a bunch of colors in all frames

for (frm in AED.Frames) {
	var clrOld = new Array(0,114,232)
	var clrNew = new Array(0,116,232)
	frm.ReplaceColor(clrOld, clrNew, true)

	clrOld = new Array(0,112,232)
	clrNew = new Array(0,116,232)
	frm.ReplaceColor(clrOld, clrNew, true)

	clrOld = new Array(0,96,192)
	clrNew = new Array(0,96,196)
	frm.ReplaceColor(clrOld, clrNew, true)

	clrOld = new Array(0,48,88)
	clrNew = new Array(0,48,92)
	frm.ReplaceColor(clrOld, clrNew, true)
}
");
		}

		private void mniLibraryOriginOffsetter_Click(object sender, System.EventArgs e) {
			rtbEdit.AppendText(@"// Offset all frames

for (frm in AED.Frames) {
   frm.OriginX -= 16;
   frm.OriginY -= 15;
}
");
		}
	}

	//
	// Wrap a TextBox control in a TextWriter stream so StdOut, etc output can be
	// routed to the TextBox.
	//

	class TextBoxWriter : TextWriter {
		private TextBoxBase m_tb = null;

		public TextBoxWriter(TextBoxBase tb) {
			m_tb = tb;
		}

		override public void WriteLine(string str) {
			m_tb.Text += str + "\n";
		}

#if false
		override public void WriteLine(string strFormat, params object[] aobParams) {
			Write(strFormat, aobParams);
			Write("\n");
		}

		override public void Write(string strFormat, params object[] aobParams) {
			string strT = string.Format(strFormat, aobParams);
			m_tb.Text += strT;
		}
#endif

		// Standard property we have to implement (...grumble...)
		override public System.Text.Encoding Encoding {
			get {
				return System.Text.Encoding.ASCII;
			}
		}
	}
}
