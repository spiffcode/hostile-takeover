using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for StripProperties.
	/// </summary>
	public class StripProperties : System.Windows.Forms.Form
	{
		private System.Windows.Forms.PropertyGrid prgStrip;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.Button btnOK;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public StripProperties(Strip stp)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			prgStrip.SelectedObject = stp;
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
			this.prgStrip = new System.Windows.Forms.PropertyGrid();
			this.panel1 = new System.Windows.Forms.Panel();
			this.btnCancel = new System.Windows.Forms.Button();
			this.btnOK = new System.Windows.Forms.Button();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// prgStrip
			// 
			this.prgStrip.CommandsVisibleIfAvailable = true;
			this.prgStrip.Dock = System.Windows.Forms.DockStyle.Fill;
			this.prgStrip.HelpVisible = false;
			this.prgStrip.LargeButtons = false;
			this.prgStrip.LineColor = System.Drawing.SystemColors.ScrollBar;
			this.prgStrip.Name = "prgStrip";
			this.prgStrip.Size = new System.Drawing.Size(292, 294);
			this.prgStrip.TabIndex = 0;
			this.prgStrip.Text = "Strip Properties";
			this.prgStrip.ViewBackColor = System.Drawing.SystemColors.Window;
			this.prgStrip.ViewForeColor = System.Drawing.SystemColors.WindowText;
			// 
			// panel1
			// 
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.btnCancel,
																				 this.btnOK});
			this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel1.Location = new System.Drawing.Point(0, 294);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(292, 40);
			this.panel1.TabIndex = 1;
			// 
			// btnCancel
			// 
			this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnCancel.Location = new System.Drawing.Point(169, 8);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.TabIndex = 1;
			this.btnCancel.Text = "Cancel";
			// 
			// btnOK
			// 
			this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.btnOK.Location = new System.Drawing.Point(49, 8);
			this.btnOK.Name = "btnOK";
			this.btnOK.TabIndex = 0;
			this.btnOK.Text = "OK";
			// 
			// StripProperties
			// 
			this.AcceptButton = this.btnOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.btnCancel;
			this.ClientSize = new System.Drawing.Size(292, 334);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.prgStrip,
																		  this.panel1});
			this.Name = "StripProperties";
			this.Text = "Strip Properties";
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion
	}
}
