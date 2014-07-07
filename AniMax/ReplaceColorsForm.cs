using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for ReplaceColorsForm.
	/// </summary>
	public class ReplaceColorsForm : System.Windows.Forms.Form
	{
		private AnimDoc m_doc;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.TextBox tbR1;
		private System.Windows.Forms.TextBox tbG1;
		private System.Windows.Forms.TextBox tbB1;
		private System.Windows.Forms.TextBox tbR2;
		private System.Windows.Forms.TextBox tbB2;
		private System.Windows.Forms.TextBox tbG2;
		private System.Windows.Forms.Button btnOK;
		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.Button button1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ReplaceColorsForm(AnimDoc doc)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			m_doc = doc;
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
			this.label2 = new System.Windows.Forms.Label();
			this.tbR1 = new System.Windows.Forms.TextBox();
			this.tbG1 = new System.Windows.Forms.TextBox();
			this.tbB1 = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.tbR2 = new System.Windows.Forms.TextBox();
			this.tbB2 = new System.Windows.Forms.TextBox();
			this.tbG2 = new System.Windows.Forms.TextBox();
			this.btnOK = new System.Windows.Forms.Button();
			this.btnCancel = new System.Windows.Forms.Button();
			this.button1 = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 43);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(56, 16);
			this.label1.TabIndex = 0;
			this.label1.Text = "Replace:";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(24, 75);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(32, 16);
			this.label2.TabIndex = 1;
			this.label2.Text = "With:";
			// 
			// tbR1
			// 
			this.tbR1.Location = new System.Drawing.Point(72, 40);
			this.tbR1.Name = "tbR1";
			this.tbR1.Size = new System.Drawing.Size(32, 20);
			this.tbR1.TabIndex = 0;
			this.tbR1.Text = "0";
			// 
			// tbG1
			// 
			this.tbG1.Location = new System.Drawing.Point(120, 40);
			this.tbG1.Name = "tbG1";
			this.tbG1.Size = new System.Drawing.Size(32, 20);
			this.tbG1.TabIndex = 1;
			this.tbG1.Text = "0";
			// 
			// tbB1
			// 
			this.tbB1.Location = new System.Drawing.Point(168, 40);
			this.tbB1.Name = "tbB1";
			this.tbB1.Size = new System.Drawing.Size(32, 20);
			this.tbB1.TabIndex = 2;
			this.tbB1.Text = "0";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(80, 16);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(16, 16);
			this.label3.TabIndex = 3;
			this.label3.Text = "R";
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(128, 16);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(16, 16);
			this.label4.TabIndex = 3;
			this.label4.Text = "G";
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(176, 16);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(16, 16);
			this.label5.TabIndex = 3;
			this.label5.Text = "B";
			// 
			// tbR2
			// 
			this.tbR2.Location = new System.Drawing.Point(72, 72);
			this.tbR2.Name = "tbR2";
			this.tbR2.Size = new System.Drawing.Size(32, 20);
			this.tbR2.TabIndex = 3;
			this.tbR2.Text = "0";
			// 
			// tbB2
			// 
			this.tbB2.Location = new System.Drawing.Point(168, 72);
			this.tbB2.Name = "tbB2";
			this.tbB2.Size = new System.Drawing.Size(32, 20);
			this.tbB2.TabIndex = 5;
			this.tbB2.Text = "0";
			// 
			// tbG2
			// 
			this.tbG2.Location = new System.Drawing.Point(120, 72);
			this.tbG2.Name = "tbG2";
			this.tbG2.Size = new System.Drawing.Size(32, 20);
			this.tbG2.TabIndex = 4;
			this.tbG2.Text = "0";
			// 
			// btnOK
			// 
			this.btnOK.Location = new System.Drawing.Point(24, 112);
			this.btnOK.Name = "btnOK";
			this.btnOK.TabIndex = 6;
			this.btnOK.Text = "OK";
			this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
			// 
			// btnCancel
			// 
			this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnCancel.Location = new System.Drawing.Point(136, 112);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.TabIndex = 7;
			this.btnCancel.Text = "Cancel";
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(16, 10);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(32, 24);
			this.button1.TabIndex = 8;
			this.button1.Text = "Get";
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// ReplaceColorsForm
			// 
			this.AcceptButton = this.btnOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.btnCancel;
			this.ClientSize = new System.Drawing.Size(232, 142);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.button1,
																		  this.btnOK,
																		  this.label3,
																		  this.tbR1,
																		  this.label2,
																		  this.label1,
																		  this.tbG1,
																		  this.tbB1,
																		  this.label4,
																		  this.label5,
																		  this.tbR2,
																		  this.tbB2,
																		  this.tbG2,
																		  this.btnCancel});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "ReplaceColorsForm";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Replace Colors";
			this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.ReplaceColorsForm_MouseDown);
			this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.ReplaceColorsForm_MouseMove);
			this.ResumeLayout(false);

		}
		#endregion

		private void btnOK_Click(object sender, System.EventArgs e) {
			foreach (XBitmap xbm in m_doc.XBitmapSet) {
				int nR1 = int.Parse(tbR1.Text);
				int nG1 = int.Parse(tbG1.Text);
				int nB1 = int.Parse(tbB1.Text);

				int nR2 = int.Parse(tbR2.Text);
				int nG2 = int.Parse(tbG2.Text);
				int nB2 = int.Parse(tbB2.Text);

				bool fDirty = MainForm.ReplaceColor(xbm.Bitmap, Color.FromArgb(nR1, nG1, nB1), 
						Color.FromArgb(nR2, nG2, nB2), true);

				if (fDirty) {
					xbm.Dirty = fDirty;
					m_doc.Dirty = true;
				}
			}

			// UNDONE: this is a hack. Decide on the right way to force selective refreshes

			Globals.StripControl.Invalidate();
			Globals.PreviewControl.Invalidate();

			DialogResult = DialogResult.OK;
		}

		[DllImport("user32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern IntPtr GetDC(IntPtr hWnd);

		[DllImport("user32.dll", ExactSpelling=true)]
		public static extern int ReleaseDC(IntPtr hWnd, IntPtr hDC);

		[DllImport("gdi32.dll", ExactSpelling=true, SetLastError=true)]
		public static extern Int32 GetPixel(IntPtr hdc, Int32 x, Int32 y);

		private void ReplaceColorsForm_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e) {
			if (!Capture)
				return;

			IntPtr hdc = GetDC(IntPtr.Zero);
			Point ptScreen = PointToScreen(new Point(e.X, e.Y));
			Int32 cr = GetPixel(hdc, ptScreen.X, ptScreen.Y);
			tbR1.Text = (cr & 0xff).ToString();
			tbG1.Text = ((cr >> 8) & 0xff).ToString();
			tbB1.Text = ((cr >> 16) & 0xff).ToString();
			ReleaseDC(IntPtr.Zero, hdc);

//			tbR2.Text = ptScreen.X.ToString();
//			tbG2.Text = ptScreen.Y.ToString();
		}

		private void button1_Click(object sender, System.EventArgs e) {
			Capture = true;
		}

		private void ReplaceColorsForm_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
			if (Capture) {
				Capture = false;
			}
		}
	}
}
