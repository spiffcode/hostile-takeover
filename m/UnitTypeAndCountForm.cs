using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace m
{
	/// <summary>
	/// Summary description for UnitTypeAndCountForm.
	/// </summary>
	public class UnitTypeAndCountForm : System.Windows.Forms.Form
	{
		public UnitType m_ut;
		public int m_c;
		private System.Windows.Forms.Button buttonOK;
		private System.Windows.Forms.Button buttonCancel;
		private System.Windows.Forms.ListBox listBoxUnitTypes;
		private System.Windows.Forms.NumericUpDown nudCount;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public UnitTypeAndCountForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			InitUnitTypesListBox();
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
			this.listBoxUnitTypes = new System.Windows.Forms.ListBox();
			this.buttonOK = new System.Windows.Forms.Button();
			this.buttonCancel = new System.Windows.Forms.Button();
			this.nudCount = new System.Windows.Forms.NumericUpDown();
			((System.ComponentModel.ISupportInitialize)(this.nudCount)).BeginInit();
			this.SuspendLayout();
			// 
			// listBoxUnitTypes
			// 
			this.listBoxUnitTypes.Location = new System.Drawing.Point(8, 8);
			this.listBoxUnitTypes.Name = "listBoxUnitTypes";
			this.listBoxUnitTypes.Size = new System.Drawing.Size(152, 160);
			this.listBoxUnitTypes.TabIndex = 0;
			this.listBoxUnitTypes.DoubleClick += new System.EventHandler(this.listBoxUnitTypes_DoubleClick);
			this.listBoxUnitTypes.SelectedIndexChanged += new System.EventHandler(this.listBoxUnitTypes_SelectedIndexChanged);
			// 
			// buttonOK
			// 
			this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.buttonOK.Enabled = false;
			this.buttonOK.Location = new System.Drawing.Point(8, 208);
			this.buttonOK.Name = "buttonOK";
			this.buttonOK.TabIndex = 1;
			this.buttonOK.Text = "OK";
			this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
			// 
			// buttonCancel
			// 
			this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonCancel.Location = new System.Drawing.Point(88, 208);
			this.buttonCancel.Name = "buttonCancel";
			this.buttonCancel.TabIndex = 2;
			this.buttonCancel.Text = "Cancel";
			// 
			// nudCount
			// 
			this.nudCount.Location = new System.Drawing.Point(8, 176);
			this.nudCount.Minimum = new System.Decimal(new int[] {
																	 1,
																	 0,
																	 0,
																	 0});
			this.nudCount.Name = "nudCount";
			this.nudCount.Size = new System.Drawing.Size(152, 20);
			this.nudCount.TabIndex = 4;
			this.nudCount.Value = new System.Decimal(new int[] {
																   1,
																   0,
																   0,
																   0});
			// 
			// UnitTypeAndCountForm
			// 
			this.AcceptButton = this.buttonOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.buttonCancel;
			this.ClientSize = new System.Drawing.Size(170, 240);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.nudCount,
																		  this.buttonCancel,
																		  this.buttonOK,
																		  this.listBoxUnitTypes});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "UnitTypeAndCountForm";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Unit Type And Count";
			((System.ComponentModel.ISupportInitialize)(this.nudCount)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		private void InitUnitTypesListBox() {
			UnitMask um = UnitMask.kumMobileUnits | UnitMask.kumAndy | UnitMask.kumFox;
			uint umT = 1;
			for (int i = 0; i < 32; i++) {
				if (((uint)um & umT) != 0) {
					UnitType ut = (UnitType)i;
					listBoxUnitTypes.Items.Add(new UnitTypeWrapper(ut));
				}
				umT <<= 1;
			}
		}

		private void buttonOK_Click(object sender, System.EventArgs e) {
			m_ut = ((UnitTypeWrapper)listBoxUnitTypes.SelectedItem).ut;
			m_c = (int)nudCount.Value;
		}

		private void listBoxUnitTypes_DoubleClick(object sender, System.EventArgs e) {
			buttonOK_Click(sender, e);
			DialogResult = DialogResult.OK;
		}

		private void listBoxUnitTypes_SelectedIndexChanged(object sender, System.EventArgs e) {
			buttonOK.Enabled = listBoxUnitTypes.SelectedIndex >= 0;
		}

		public UnitType UnitType {
			get {
				return m_ut;
			}
		}

		public int Count {
			get {
				return m_c;
			}
		}
	}

	public class UnitTypeWrapper {
		public UnitType ut;
		public UnitTypeWrapper(UnitType ut) {
			this.ut = ut;
		}

		override public string ToString() {
			return Helper.GetDisplayName(typeof(UnitType), ut.ToString());
		}
	}
}
