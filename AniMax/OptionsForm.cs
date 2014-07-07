using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for OptionsForm.
	/// </summary>
	public class OptionsForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.Button btnOK;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox tbFrameRate;
		private System.Windows.Forms.TextBox tbGridWidth;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox tbGridHeight;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public OptionsForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
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
			this.btnOK = new System.Windows.Forms.Button();
			this.tbFrameRate = new System.Windows.Forms.TextBox();
			this.btnCancel = new System.Windows.Forms.Button();
			this.tbGridWidth = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.tbGridHeight = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 16);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(72, 16);
			this.label1.TabIndex = 4;
			this.label1.Text = "Frame Rate:";
			// 
			// label2
			// 
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label2.Location = new System.Drawing.Point(136, 16);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(80, 16);
			this.label2.TabIndex = 5;
			this.label2.Text = "(in milliseconds)";
			// 
			// btnOK
			// 
			this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.btnOK.Location = new System.Drawing.Point(79, 112);
			this.btnOK.Name = "btnOK";
			this.btnOK.TabIndex = 1;
			this.btnOK.Text = "OK";
			// 
			// tbFrameRate
			// 
			this.tbFrameRate.Location = new System.Drawing.Point(80, 13);
			this.tbFrameRate.Name = "tbFrameRate";
			this.tbFrameRate.Size = new System.Drawing.Size(48, 20);
			this.tbFrameRate.TabIndex = 6;
			this.tbFrameRate.Text = "";
			// 
			// btnCancel
			// 
			this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnCancel.Location = new System.Drawing.Point(207, 112);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.TabIndex = 3;
			this.btnCancel.Text = "Cancel";
			// 
			// tbGridWidth
			// 
			this.tbGridWidth.Location = new System.Drawing.Point(80, 40);
			this.tbGridWidth.Name = "tbGridWidth";
			this.tbGridWidth.Size = new System.Drawing.Size(48, 20);
			this.tbGridWidth.TabIndex = 7;
			this.tbGridWidth.Text = "";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(8, 44);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(64, 16);
			this.label3.TabIndex = 8;
			this.label3.Text = "Grid Width:";
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(8, 68);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(72, 16);
			this.label4.TabIndex = 9;
			this.label4.Text = "Grid Height:";
			// 
			// tbGridHeight
			// 
			this.tbGridHeight.Location = new System.Drawing.Point(80, 64);
			this.tbGridHeight.Name = "tbGridHeight";
			this.tbGridHeight.Size = new System.Drawing.Size(48, 20);
			this.tbGridHeight.TabIndex = 10;
			this.tbGridHeight.Text = "";
			// 
			// OptionsForm
			// 
			this.AcceptButton = this.btnOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.btnCancel;
			this.ClientSize = new System.Drawing.Size(362, 143);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.tbGridHeight,
																		  this.label4,
																		  this.label3,
																		  this.tbGridWidth,
																		  this.tbFrameRate,
																		  this.label2,
																		  this.label1,
																		  this.btnCancel,
																		  this.btnOK});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "OptionsForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Options";
			this.ResumeLayout(false);

		}
		#endregion

		public int FrameRate {
			get {
				return Convert.ToInt32(tbFrameRate.Text);
			}
			set {
				tbFrameRate.Text = value.ToString();
			}
		}

		public int GridWidth {
			get {
				return Convert.ToInt32(tbGridWidth.Text);
			}
			set {
				tbGridWidth.Text = value.ToString();
			}
		}

		public int GridHeight {
			get {
				return Convert.ToInt32(tbGridHeight.Text);
			}
			set {
				tbGridHeight.Text = value.ToString();
			}
		}
	}
}
