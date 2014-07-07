using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace m
{
	/// <summary>
	/// Summary description for TileSizeForm.
	/// </summary>
	public class TileSizeForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.RadioButton radioButton24x24;
		private System.Windows.Forms.RadioButton radioButton16x16;
		private System.Windows.Forms.RadioButton radioButtonCustom;
		private System.Windows.Forms.TextBox textBoxWidth;
		private System.Windows.Forms.TextBox textBoxHeight;
		private System.Windows.Forms.Button buttonOK;
		private System.Windows.Forms.Label labelWidth;
		private System.Windows.Forms.Label labelHeight;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		Size m_sizTile;

		public TileSizeForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			SelectRadioButton(radioButton24x24);
		}

		void SelectRadioButton(RadioButton rbtn) {
			bool fEnable = (rbtn == radioButtonCustom);
			labelWidth.Enabled = fEnable;
			textBoxWidth.Enabled = fEnable;
			labelHeight.Enabled = fEnable;
			textBoxHeight.Enabled = fEnable;

			if (rbtn == radioButton24x24)
				m_sizTile = new Size(24, 24);

			if (rbtn == radioButton16x16)
				m_sizTile = new Size(16, 16);
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
			this.radioButton24x24 = new System.Windows.Forms.RadioButton();
			this.radioButton16x16 = new System.Windows.Forms.RadioButton();
			this.radioButtonCustom = new System.Windows.Forms.RadioButton();
			this.labelWidth = new System.Windows.Forms.Label();
			this.textBoxWidth = new System.Windows.Forms.TextBox();
			this.labelHeight = new System.Windows.Forms.Label();
			this.textBoxHeight = new System.Windows.Forms.TextBox();
			this.buttonOK = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(24, 16);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(272, 16);
			this.label1.TabIndex = 0;
			this.label1.Text = "What tile size should this new tile collection have?";
			// 
			// radioButton24x24
			// 
			this.radioButton24x24.Location = new System.Drawing.Point(24, 40);
			this.radioButton24x24.Name = "radioButton24x24";
			this.radioButton24x24.Size = new System.Drawing.Size(104, 16);
			this.radioButton24x24.TabIndex = 1;
			this.radioButton24x24.TabStop = true;
			this.radioButton24x24.Text = "24 x 24";
			this.radioButton24x24.CheckedChanged += new System.EventHandler(this.radioButton24x24_CheckedChanged);
			// 
			// radioButton16x16
			// 
			this.radioButton16x16.Location = new System.Drawing.Point(24, 64);
			this.radioButton16x16.Name = "radioButton16x16";
			this.radioButton16x16.Size = new System.Drawing.Size(104, 16);
			this.radioButton16x16.TabIndex = 1;
			this.radioButton16x16.Text = "16 x 16";
			this.radioButton16x16.CheckedChanged += new System.EventHandler(this.radioButton16x16_CheckedChanged);
			// 
			// radioButtonCustom
			// 
			this.radioButtonCustom.Location = new System.Drawing.Point(24, 88);
			this.radioButtonCustom.Name = "radioButtonCustom";
			this.radioButtonCustom.Size = new System.Drawing.Size(64, 16);
			this.radioButtonCustom.TabIndex = 1;
			this.radioButtonCustom.Text = "Custom:";
			this.radioButtonCustom.CheckedChanged += new System.EventHandler(this.radioButtonCustom_CheckedChanged);
			// 
			// labelWidth
			// 
			this.labelWidth.Enabled = false;
			this.labelWidth.Location = new System.Drawing.Point(104, 89);
			this.labelWidth.Name = "labelWidth";
			this.labelWidth.Size = new System.Drawing.Size(40, 16);
			this.labelWidth.TabIndex = 2;
			this.labelWidth.Text = "Width:";
			// 
			// textBoxWidth
			// 
			this.textBoxWidth.Enabled = false;
			this.textBoxWidth.Location = new System.Drawing.Point(144, 86);
			this.textBoxWidth.Name = "textBoxWidth";
			this.textBoxWidth.Size = new System.Drawing.Size(48, 20);
			this.textBoxWidth.TabIndex = 2;
			this.textBoxWidth.Text = "";
			// 
			// labelHeight
			// 
			this.labelHeight.Enabled = false;
			this.labelHeight.Location = new System.Drawing.Point(208, 90);
			this.labelHeight.Name = "labelHeight";
			this.labelHeight.Size = new System.Drawing.Size(48, 16);
			this.labelHeight.TabIndex = 62;
			this.labelHeight.Text = "Height:";
			// 
			// textBoxHeight
			// 
			this.textBoxHeight.Enabled = false;
			this.textBoxHeight.Location = new System.Drawing.Point(254, 86);
			this.textBoxHeight.Name = "textBoxHeight";
			this.textBoxHeight.Size = new System.Drawing.Size(48, 20);
			this.textBoxHeight.TabIndex = 2;
			this.textBoxHeight.Text = "";
			// 
			// buttonOK
			// 
			this.buttonOK.Location = new System.Drawing.Point(136, 128);
			this.buttonOK.Name = "buttonOK";
			this.buttonOK.TabIndex = 8;
			this.buttonOK.Text = "OK";
			this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
			// 
			// TileSizeForm
			// 
			this.AcceptButton = this.buttonOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(336, 158);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.textBoxWidth,
																		  this.textBoxHeight,
																		  this.buttonOK,
																		  this.labelHeight,
																		  this.labelWidth,
																		  this.radioButtonCustom,
																		  this.radioButton16x16,
																		  this.radioButton24x24,
																		  this.label1});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "TileSizeForm";
			this.ShowInTaskbar = false;
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			this.Text = "Tile Size";
			this.ResumeLayout(false);

		}
		#endregion

		private void radioButton24x24_CheckedChanged(object sender, System.EventArgs e) {
			SelectRadioButton(radioButton24x24);
		}

		private void radioButton16x16_CheckedChanged(object sender, System.EventArgs e) {
			SelectRadioButton(radioButton16x16);		
		}

		private void radioButtonCustom_CheckedChanged(object sender, System.EventArgs e) {
			SelectRadioButton(radioButtonCustom);
			textBoxWidth.Focus();
		}

		private void buttonOK_Click(object sender, System.EventArgs e) {
			// If custom, validate

			if (radioButtonCustom.Checked) {
				int cx = int.Parse(textBoxWidth.Text);
				int cy = int.Parse(textBoxHeight.Text);
				if (cx <= 0 || cx > 64 || cy <= 0 || cy > 64) {
					MessageBox.Show(DocManager.GetFrameParent(), "Invalid custom tile size. Try again.");
					return;
				}
				m_sizTile = new Size(cx, cy);
			}

			DialogResult = DialogResult.OK;
		}

		public Size GetTileSize() {
			return m_sizTile;
		}
	}
}
