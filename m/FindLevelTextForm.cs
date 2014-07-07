using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace m
{
	/// <summary>
	/// Summary description for FindLevelTextForm.
	/// </summary>
	public class FindLevelTextForm : System.Windows.Forms.Form
	{
		private static string s_strLastFind = "";
		private RichTextBox m_rtb;
		private int m_ichLastFind;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button btnFind;
		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.TextBox tbcFind;
		private System.Windows.Forms.CheckBox chkbWholeWord;
		private System.Windows.Forms.CheckBox chkbCase;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public FindLevelTextForm(RichTextBox rtb)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_rtb = rtb;
			tbcFind.Text = s_strLastFind;
			btnFind.Enabled = tbcFind.Text != "";
			m_ichLastFind = m_rtb.SelectionStart + m_rtb.SelectionLength;
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
			this.tbcFind = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.btnFind = new System.Windows.Forms.Button();
			this.chkbWholeWord = new System.Windows.Forms.CheckBox();
			this.chkbCase = new System.Windows.Forms.CheckBox();
			this.btnCancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// tbcFind
			// 
			this.tbcFind.Location = new System.Drawing.Point(64, 10);
			this.tbcFind.Name = "tbcFind";
			this.tbcFind.Size = new System.Drawing.Size(176, 20);
			this.tbcFind.TabIndex = 0;
			this.tbcFind.Text = "";
			this.tbcFind.TextChanged += new System.EventHandler(this.tbcFind_TextChanged);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(56, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "Fi&nd what:";
			// 
			// btnFind
			// 
			this.btnFind.Location = new System.Drawing.Point(264, 9);
			this.btnFind.Name = "btnFind";
			this.btnFind.TabIndex = 2;
			this.btnFind.Text = "&Find Next";
			this.btnFind.Click += new System.EventHandler(this.btnFind_Click);
			// 
			// chkbWholeWord
			// 
			this.chkbWholeWord.Enabled = false;
			this.chkbWholeWord.Location = new System.Drawing.Point(8, 40);
			this.chkbWholeWord.Name = "chkbWholeWord";
			this.chkbWholeWord.Size = new System.Drawing.Size(160, 16);
			this.chkbWholeWord.TabIndex = 3;
			this.chkbWholeWord.Text = "Match &whole word only";
			// 
			// chkbCase
			// 
			this.chkbCase.Location = new System.Drawing.Point(8, 64);
			this.chkbCase.Name = "chkbCase";
			this.chkbCase.Size = new System.Drawing.Size(160, 16);
			this.chkbCase.TabIndex = 4;
			this.chkbCase.Text = "Match &case";
			// 
			// btnCancel
			// 
			this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnCancel.Location = new System.Drawing.Point(264, 40);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.TabIndex = 5;
			this.btnCancel.Text = "Cancel";
			this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
			// 
			// FindLevelTextForm
			// 
			this.AcceptButton = this.btnFind;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.btnCancel;
			this.ClientSize = new System.Drawing.Size(352, 94);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.btnCancel,
																		  this.chkbCase,
																		  this.chkbWholeWord,
																		  this.btnFind,
																		  this.label1,
																		  this.tbcFind});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "FindLevelTextForm";
			this.ShowInTaskbar = false;
			this.Text = "Find";
			this.ResumeLayout(false);

		}
		#endregion

		private void tbcFind_TextChanged(object sender, System.EventArgs e) {
			btnFind.Enabled = tbcFind.Text != "";
		}

		private void btnFind_Click(object sender, System.EventArgs e) {
			string strSrc = m_rtb.Text;
			string strFind = tbcFind.Text;

			if (!chkbCase.Checked) {
				strSrc = strSrc.ToUpper();
				strFind = strFind.ToUpper();
			}

			int ich = strSrc.IndexOf(strFind, m_rtb.SelectionStart + m_rtb.SelectionLength);

			if (ich == -1) {
				MessageBox.Show(this, "Finished searching the document", "M", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			} else {
				m_rtb.SelectionStart = ich;
				m_rtb.SelectionLength = tbcFind.Text.Length;
				m_rtb.ScrollToCaret();
			}
		}

		private void btnCancel_Click(object sender, System.EventArgs e) {
			s_strLastFind = tbcFind.Text;
			Close();
			Dispose();
		}
	}
}
