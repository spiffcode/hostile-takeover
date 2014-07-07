using System;
using System.Drawing;
using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows.Forms;

namespace m
{
	/// <summary>
	/// Summary description for EditSwitchForm.
	/// </summary>
	public class SwitchesForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button buttonOk;
		private System.Windows.Forms.Button buttonCancel;
		private System.Windows.Forms.ListBox listBox1;
		private System.Windows.Forms.Button buttonNew;
		private System.Windows.Forms.Button buttonModify;
		private System.Windows.Forms.Button buttonDelete;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public SwitchesForm(string strTitle, string strCurrent, StringCollection strc)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//

			foreach (string str in strc)
				listBox1.Items.Add(str);
			listBox1.SelectedIndex = strCurrent == "" ? -1 : listBox1.Items.IndexOf(strCurrent);
			Text = strTitle;
			label1.Text = label1.Text.Replace("$1", strTitle);
		}

		public Switch GetSelectedSwitch() {
			if (listBox1.SelectedIndex == -1)
				return null;
			SwitchManager swm = ((LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc))).SwitchManager;
			return swm[(string)listBox1.SelectedItem];
		}

		static public Switch DoModal(string strTitle, string strCurrent, StringCollection strc) {
			SwitchesForm frm = new SwitchesForm(strTitle, strCurrent, strc);
			DialogResult res = frm.ShowDialog();
			if (res == DialogResult.Cancel)
				return null;
			return frm.GetSelectedSwitch();
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
			this.label1 = new System.Windows.Forms.Label();
			this.buttonOk = new System.Windows.Forms.Button();
			this.buttonCancel = new System.Windows.Forms.Button();
			this.listBox1 = new System.Windows.Forms.ListBox();
			this.buttonNew = new System.Windows.Forms.Button();
			this.buttonModify = new System.Windows.Forms.Button();
			this.buttonDelete = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(160, 16);
			this.label1.TabIndex = 0;
			this.label1.Text = "Choose one $1:";
			// 
			// buttonOk
			// 
			this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.buttonOk.Location = new System.Drawing.Point(184, 8);
			this.buttonOk.Name = "buttonOk";
			this.buttonOk.TabIndex = 2;
			this.buttonOk.Text = "OK";
			this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
			// 
			// buttonCancel
			// 
			this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonCancel.Location = new System.Drawing.Point(184, 40);
			this.buttonCancel.Name = "buttonCancel";
			this.buttonCancel.TabIndex = 2;
			this.buttonCancel.Text = "Cancel";
			this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
			// 
			// listBox1
			// 
			this.listBox1.Location = new System.Drawing.Point(8, 24);
			this.listBox1.Name = "listBox1";
			this.listBox1.Size = new System.Drawing.Size(168, 173);
			this.listBox1.Sorted = true;
			this.listBox1.TabIndex = 3;
			this.listBox1.DoubleClick += new System.EventHandler(this.listBox1_DoubleClick);
			// 
			// buttonNew
			// 
			this.buttonNew.Location = new System.Drawing.Point(184, 96);
			this.buttonNew.Name = "buttonNew";
			this.buttonNew.TabIndex = 4;
			this.buttonNew.Text = "New...";
			this.buttonNew.Click += new System.EventHandler(this.buttonNew_Click);
			// 
			// buttonModify
			// 
			this.buttonModify.Location = new System.Drawing.Point(184, 128);
			this.buttonModify.Name = "buttonModify";
			this.buttonModify.TabIndex = 5;
			this.buttonModify.Text = "Modify...";
			this.buttonModify.Click += new System.EventHandler(this.buttonModify_Click);
			// 
			// buttonDelete
			// 
			this.buttonDelete.Location = new System.Drawing.Point(184, 160);
			this.buttonDelete.Name = "buttonDelete";
			this.buttonDelete.TabIndex = 6;
			this.buttonDelete.Text = "Delete";
			this.buttonDelete.Click += new System.EventHandler(this.buttonDelete_Click);
			// 
			// SwitchesForm
			// 
			this.AcceptButton = this.buttonOk;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.buttonCancel;
			this.ClientSize = new System.Drawing.Size(266, 208);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.buttonDelete,
																		  this.buttonModify,
																		  this.buttonNew,
																		  this.listBox1,
																		  this.buttonOk,
																		  this.label1,
																		  this.buttonCancel});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "SwitchesForm";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "EditSwitchForm";
			this.ResumeLayout(false);

		}
		#endregion

		private void buttonOk_Click(object sender, System.EventArgs e) {
			DialogResult = DialogResult.OK;
		}

		private void buttonCancel_Click(object sender, System.EventArgs e) {
			DialogResult = DialogResult.Cancel;
		}

		private void listBox1_DoubleClick(object sender, System.EventArgs e) {
			DialogResult = DialogResult.OK;
		}

		private void buttonNew_Click(object sender, System.EventArgs e) {
			string str = EditStringForm.DoModal("New Switch", "New switch name:", null);
			if (str != null) {
				SwitchManager swm = ((LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc))).SwitchManager;
				if (swm[str] == null) {
					swm.AddSwitch(new Switch(str));
					int i = listBox1.Items.Add(str);
					listBox1.SelectedIndex = i;
					// UNDONE: doc is modified
				}
			}
		}

		private void buttonModify_Click(object sender, System.EventArgs e) {
			string str = (string)listBox1.SelectedItem;
			if (str == null)
				return;
			SwitchManager swm = ((LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc))).SwitchManager;
			Switch sw = swm[str];
			string strNew = EditStringForm.DoModal("Modify Switch", "New switch name:", str);
			if (strNew == null)
				return;
			if (strNew != str) {
				sw.Name = strNew;
				listBox1.Items.Remove(str);
				int i = listBox1.Items.Add(strNew);
				listBox1.SelectedIndex = i;
				// UNDONE: doc is modified
			}
		}

		private void buttonDelete_Click(object sender, System.EventArgs e) {
			string str = (string)listBox1.SelectedItem;
			if (str == null)
				return;
			SwitchManager swm = ((LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc))).SwitchManager;
			Switch sw = swm[str];
			swm.RemoveSwitch(sw);
			listBox1.Items.Remove(str);
			// UNDONE: doc is modified
		}
	}
}
