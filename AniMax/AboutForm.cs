using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Drawing.Imaging;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for AboutForm.
	/// </summary>
	public class AboutForm : Form
	{
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.Button btnOK;
		private System.Windows.Forms.Button btnSystemInfo;
		private System.Windows.Forms.PictureBox pictureBox1;
		private SpiffCode.ScBorder scBorder1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public AboutForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

//			SetBitmap(new Bitmap(@"c:\code\ht\animax\shp.png"));
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(AboutForm));
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.label6 = new System.Windows.Forms.Label();
			this.panel2 = new System.Windows.Forms.Panel();
			this.btnOK = new System.Windows.Forms.Button();
			this.btnSystemInfo = new System.Windows.Forms.Button();
			this.pictureBox1 = new System.Windows.Forms.PictureBox();
			this.scBorder1 = new SpiffCode.ScBorder();
			this.panel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(112, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(200, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "SpiffCode AniMax 2002 (Version 0.31)";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(112, 24);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(336, 16);
			this.label2.TabIndex = 2;
			this.label2.Text = "Copyright © 2002 SpiffCode Incorporated. All Rights Reserved.";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(112, 56);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(168, 16);
			this.label3.TabIndex = 3;
			this.label3.Text = "This program is licensed to:";
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(8, 8);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(100, 16);
			this.label4.TabIndex = 0;
			this.label4.Text = "Mark Soderwall";
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(8, 24);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(128, 16);
			this.label5.TabIndex = 1;
			this.label5.Text = "Extreme Illustrations";
			// 
			// label6
			// 
			this.label6.Location = new System.Drawing.Point(16, 200);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(352, 64);
			this.label6.TabIndex = 5;
			this.label6.Text = @"Warning: This computer program is protected by copyright law and international treaties. Unauthorized reproduction or distribution of this program, or any portion of it, may result in severe civil and criminal penalties, and will be prosecuted to the maximum extent possible under the law.";
			this.label6.UseMnemonic = false;
			// 
			// panel2
			// 
			this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel2.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.label5,
																				 this.label4});
			this.panel2.Location = new System.Drawing.Point(112, 72);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(352, 48);
			this.panel2.TabIndex = 4;
			// 
			// btnOK
			// 
			this.btnOK.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnOK.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btnOK.Location = new System.Drawing.Point(376, 203);
			this.btnOK.Name = "btnOK";
			this.btnOK.Size = new System.Drawing.Size(88, 23);
			this.btnOK.TabIndex = 6;
			this.btnOK.Text = "OK";
			// 
			// btnSystemInfo
			// 
			this.btnSystemInfo.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btnSystemInfo.Location = new System.Drawing.Point(376, 234);
			this.btnSystemInfo.Name = "btnSystemInfo";
			this.btnSystemInfo.Size = new System.Drawing.Size(88, 23);
			this.btnSystemInfo.TabIndex = 7;
			this.btnSystemInfo.Text = "&System Info...";
			// 
			// pictureBox1
			// 
			this.pictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox1.Image")));
			this.pictureBox1.Location = new System.Drawing.Point(-24, 0);
			this.pictureBox1.Name = "pictureBox1";
			this.pictureBox1.Size = new System.Drawing.Size(168, 192);
			this.pictureBox1.TabIndex = 8;
			this.pictureBox1.TabStop = false;
			// 
			// scBorder1
			// 
			this.scBorder1.Location = new System.Drawing.Point(16, 190);
			this.scBorder1.Name = "scBorder1";
			this.scBorder1.Size = new System.Drawing.Size(448, 3);
			this.scBorder1.TabIndex = 9;
			this.scBorder1.Text = "scBorder1";
			// 
			// AboutForm
			// 
			this.AcceptButton = this.btnOK;
			this.AutoScale = false;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(200)), ((System.Byte)(204)), ((System.Byte)(206)));
			this.CancelButton = this.btnOK;
			this.ClientSize = new System.Drawing.Size(480, 272);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.scBorder1,
																		  this.btnSystemInfo,
																		  this.btnOK,
																		  this.label6,
																		  this.panel2,
																		  this.label3,
																		  this.label2,
																		  this.label1,
																		  this.pictureBox1});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "AboutForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "About SpiffCode AniMax";
			this.panel2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion
	}
}
