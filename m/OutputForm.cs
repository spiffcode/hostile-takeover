using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace m
{
	/// <summary>
	/// Summary description for OutputForm.
	/// </summary>
	public class OutputForm : System.Windows.Forms.Form
	{
		static private OutputForm s_frm = null;
		private ArrayList m_alLevelErrors = new ArrayList();

		static public void ShowIt() {
			if (s_frm == null) {
				s_frm = new OutputForm();
				s_frm.Owner = DocManager.GetFrameParent();
			}
			s_frm.Show();
			s_frm.BringToFront();
		}

		static public void HideIt() {
			if (s_frm != null)
				s_frm.Hide();
		}

		static public void Clear() {
			if (s_frm != null) {
				s_frm.tbcOutput.Clear();
				s_frm.m_alLevelErrors.Clear();
			}
		}

		static public void AppendText(string strFormat, params object[] aob) {
			string str = String.Format(strFormat, aob);
			s_frm.tbcOutput.AppendText(str);
		}

		static public void Error(LevelDoc lvld, object ob, string strFormat, params object[] aob) {
			string str = String.Format(strFormat, aob);
			s_frm.AddError(lvld, ob, str);
		}

		public void AddError(LevelDoc lvld, object ob, string str) {
			int i = m_alLevelErrors.Add(new LevelError(lvld, ob));
			tbcOutput.AppendText(i.ToString() + "> " + str);
		}

		private System.Windows.Forms.TextBox tbcOutput;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public OutputForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			Debug.Assert(s_frm == null);
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
			s_frm = null;
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.tbcOutput = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// tbcOutput
			// 
			this.tbcOutput.AcceptsReturn = true;
			this.tbcOutput.AcceptsTab = true;
			this.tbcOutput.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tbcOutput.Multiline = true;
			this.tbcOutput.Name = "tbcOutput";
			this.tbcOutput.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.tbcOutput.Size = new System.Drawing.Size(832, 246);
			this.tbcOutput.TabIndex = 0;
			this.tbcOutput.Text = "";
			this.tbcOutput.DoubleClick += new System.EventHandler(this.tbcOutput_DoubleClick);
			// 
			// OutputForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(832, 246);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.tbcOutput});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
			this.Name = "OutputForm";
			this.Text = "Output";
			this.ResumeLayout(false);

		}
		#endregion

		private void tbcOutput_DoubleClick(object sender, System.EventArgs e) {
			LevelDoc lvld = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
			if (lvld == null)
				return;
			
			// scan backwards from the insertion point to the beginning of the line

			string strT = tbcOutput.Text;
			int i = tbcOutput.SelectionStart;
			while (i > 0 && strT[i] != '\n')
				i--;
			if (i != 0)
				i++;
			strT = strT.Substring(i, strT.IndexOf('>', i) - i);

			// Select the item with the error

			i = int.Parse(strT);
			LevelError lvle = (LevelError)m_alLevelErrors[i];
			
			if (lvle.Object is IMapItem) {
				ArrayList al = new ArrayList();
				al.Add(lvle.Object);
				lvle.LevelDoc.Selection = al;
			} else if (lvle.Object is SideInfo) {
				Globals.PropertyGrid.SelectedObject = lvle.Object;
			}
		}
	}

	public class LevelError {
		private LevelDoc m_lvld;
		private object m_ob;

		public LevelError(LevelDoc lvld, object ob) {
			m_lvld = lvld;
			m_ob = ob;
		}

		public LevelDoc LevelDoc {
			get {
				return m_lvld;
			}
		}

		public object Object {
			get {
				return m_ob;
			}
		}
}
}
