using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace m
{
	/// <summary>
	/// Summary description for CaTypeUnitTypesForm.
	/// </summary>
	public class CaTypeUnitTypesForm : System.Windows.Forms.Form
	{
		private UnitMask m_um;
		private System.Windows.Forms.CheckedListBox checkedListBox;
		private System.Windows.Forms.Button buttonOK;
		private System.Windows.Forms.Button buttonStructures;
		private System.Windows.Forms.Button buttonInfantry;
		private System.Windows.Forms.Button buttonMobileUnits;
		private System.Windows.Forms.Button buttonBuilders;
		private System.Windows.Forms.Button buttonVehicles;
		private System.Windows.Forms.Button buttonAll;
		private System.Windows.Forms.Button buttonNone;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public CaTypeUnitTypesForm(UnitMask um)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			m_um = um;

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
			this.checkedListBox = new System.Windows.Forms.CheckedListBox();
			this.buttonOK = new System.Windows.Forms.Button();
			this.buttonStructures = new System.Windows.Forms.Button();
			this.buttonInfantry = new System.Windows.Forms.Button();
			this.buttonMobileUnits = new System.Windows.Forms.Button();
			this.buttonBuilders = new System.Windows.Forms.Button();
			this.buttonVehicles = new System.Windows.Forms.Button();
			this.buttonAll = new System.Windows.Forms.Button();
			this.buttonNone = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// checkedListBox
			// 
			this.checkedListBox.CheckOnClick = true;
			this.checkedListBox.Location = new System.Drawing.Point(8, 8);
			this.checkedListBox.Name = "checkedListBox";
			this.checkedListBox.Size = new System.Drawing.Size(184, 289);
			this.checkedListBox.Sorted = true;
			this.checkedListBox.TabIndex = 0;
			// 
			// buttonOK
			// 
			this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.buttonOK.Location = new System.Drawing.Point(200, 272);
			this.buttonOK.Name = "buttonOK";
			this.buttonOK.Size = new System.Drawing.Size(80, 23);
			this.buttonOK.TabIndex = 1;
			this.buttonOK.Text = "OK";
			this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
			// 
			// buttonStructures
			// 
			this.buttonStructures.Location = new System.Drawing.Point(200, 104);
			this.buttonStructures.Name = "buttonStructures";
			this.buttonStructures.Size = new System.Drawing.Size(80, 23);
			this.buttonStructures.TabIndex = 2;
			this.buttonStructures.Text = "Structures";
			this.buttonStructures.Click += new System.EventHandler(this.buttonStructures_Click);
			// 
			// buttonInfantry
			// 
			this.buttonInfantry.Location = new System.Drawing.Point(200, 8);
			this.buttonInfantry.Name = "buttonInfantry";
			this.buttonInfantry.Size = new System.Drawing.Size(80, 23);
			this.buttonInfantry.TabIndex = 3;
			this.buttonInfantry.Text = "Infantry";
			this.buttonInfantry.Click += new System.EventHandler(this.buttonInfantry_Click);
			// 
			// buttonMobileUnits
			// 
			this.buttonMobileUnits.Location = new System.Drawing.Point(200, 72);
			this.buttonMobileUnits.Name = "buttonMobileUnits";
			this.buttonMobileUnits.Size = new System.Drawing.Size(80, 23);
			this.buttonMobileUnits.TabIndex = 4;
			this.buttonMobileUnits.Text = "Mobile Units";
			this.buttonMobileUnits.Click += new System.EventHandler(this.buttonMobileUnits_Click);
			// 
			// buttonBuilders
			// 
			this.buttonBuilders.Location = new System.Drawing.Point(200, 136);
			this.buttonBuilders.Name = "buttonBuilders";
			this.buttonBuilders.Size = new System.Drawing.Size(80, 23);
			this.buttonBuilders.TabIndex = 5;
			this.buttonBuilders.Text = "Builders";
			this.buttonBuilders.Click += new System.EventHandler(this.buttonBuilders_Click);
			// 
			// buttonVehicles
			// 
			this.buttonVehicles.Location = new System.Drawing.Point(200, 40);
			this.buttonVehicles.Name = "buttonVehicles";
			this.buttonVehicles.Size = new System.Drawing.Size(80, 23);
			this.buttonVehicles.TabIndex = 6;
			this.buttonVehicles.Text = "Vehicles";
			this.buttonVehicles.Click += new System.EventHandler(this.buttonVehicles_Click);
			// 
			// buttonAll
			// 
			this.buttonAll.Location = new System.Drawing.Point(200, 168);
			this.buttonAll.Name = "buttonAll";
			this.buttonAll.Size = new System.Drawing.Size(80, 23);
			this.buttonAll.TabIndex = 7;
			this.buttonAll.Text = "All";
			this.buttonAll.Click += new System.EventHandler(this.buttonAll_Click);
			// 
			// buttonNone
			// 
			this.buttonNone.Location = new System.Drawing.Point(200, 200);
			this.buttonNone.Name = "buttonNone";
			this.buttonNone.Size = new System.Drawing.Size(80, 23);
			this.buttonNone.TabIndex = 8;
			this.buttonNone.Text = "None";
			this.buttonNone.Click += new System.EventHandler(this.buttonNone_Click);
			// 
			// CaTypeUnitTypesForm
			// 
			this.AcceptButton = this.buttonOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(290, 304);
			this.ControlBox = false;
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.buttonNone,
																		  this.buttonAll,
																		  this.buttonVehicles,
																		  this.buttonBuilders,
																		  this.buttonMobileUnits,
																		  this.buttonInfantry,
																		  this.buttonStructures,
																		  this.buttonOK,
																		  this.checkedListBox});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "CaTypeUnitTypesForm";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Unit Types";
			this.ResumeLayout(false);

		}
		#endregion


		private void InitUnitTypesListBox() {
			checkedListBox.Items.Clear();
			for (int i = 0; i < (int)UnitType.kutMax; i++) 
				checkedListBox.Items.Add(new UnitTypeWrapper((UnitType)i), ((uint)m_um & (1 << i)) != 0);
		}

		private void buttonOK_Click(object sender, System.EventArgs e) {
			m_um = 0;
			for (int i = 0; i < checkedListBox.CheckedItems.Count; i++) {
				UnitType ut = ((UnitTypeWrapper)checkedListBox.CheckedItems[i]).ut;
				m_um = (UnitMask)(((uint)m_um) | (uint)(1 << (int)ut));
			}
		}

		private void buttonInfantry_Click(object sender, System.EventArgs e) {
			m_um = UnitMask.kumInfantry;
			InitUnitTypesListBox();
		}

		private void buttonVehicles_Click(object sender, System.EventArgs e) {
			m_um = UnitMask.kumVehicles;
			InitUnitTypesListBox();
		}

		private void buttonMobileUnits_Click(object sender, System.EventArgs e) {
			m_um = UnitMask.kumMobileUnits;
			InitUnitTypesListBox();
		}

		private void buttonStructures_Click(object sender, System.EventArgs e) {
			m_um = UnitMask.kumStructures;
			InitUnitTypesListBox();
		}

		private void buttonBuilders_Click(object sender, System.EventArgs e) {
			m_um = UnitMask.kumFactories;
			InitUnitTypesListBox();
		}

		private void buttonAll_Click(object sender, System.EventArgs e) {
			m_um = UnitMask.kumAll;
			InitUnitTypesListBox();
		}

		private void buttonNone_Click(object sender, System.EventArgs e) {
			m_um = UnitMask.kumNone;
			InitUnitTypesListBox();
		}
	
		public UnitMask UnitMask {
			get {
				return m_um;
			}
		}
	}
}
