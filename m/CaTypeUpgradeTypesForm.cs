using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace m
{
	/// <summary>
	/// Summary description for CaTypeUpgradeTypesForm.
	/// </summary>
	public class CaTypeUpgradeTypesForm : System.Windows.Forms.Form
	{
		private UpgradeMask m_upgm;
		private System.Windows.Forms.CheckedListBox checkedListBox;
		private System.Windows.Forms.Button buttonOK;
		private System.Windows.Forms.Button buttonAll;
		private System.Windows.Forms.Button buttonNone;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public CaTypeUpgradeTypesForm(UpgradeMask upgm)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_upgm = upgm;

			InitUpgradeTypesListBox();
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
			// buttonAll
			// 
			this.buttonAll.Location = new System.Drawing.Point(200, 8);
			this.buttonAll.Name = "buttonAll";
			this.buttonAll.Size = new System.Drawing.Size(80, 23);
			this.buttonAll.TabIndex = 7;
			this.buttonAll.Text = "All";
			this.buttonAll.Click += new System.EventHandler(this.buttonAll_Click);
			// 
			// buttonNone
			// 
			this.buttonNone.Location = new System.Drawing.Point(200, 40);
			this.buttonNone.Name = "buttonNone";
			this.buttonNone.Size = new System.Drawing.Size(80, 23);
			this.buttonNone.TabIndex = 8;
			this.buttonNone.Text = "None";
			this.buttonNone.Click += new System.EventHandler(this.buttonNone_Click);
			// 
			// CaTypeUpgradeTypesForm
			// 
			this.AcceptButton = this.buttonOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(290, 304);
			this.ControlBox = false;
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.buttonNone,
																		  this.buttonAll,
																		  this.buttonOK,
																		  this.checkedListBox});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "CaTypeUpgradeTypesForm";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Upgrade Types";
			this.ResumeLayout(false);

		}
		#endregion

		private void InitUpgradeTypesListBox() {
			checkedListBox.Items.Clear();
			for (int i = 0; i < (int)UpgradeType.kupgtMax; i++) 
				checkedListBox.Items.Add(new UpgradeTypeWrapper((UpgradeType)i), ((uint)m_upgm & (1 << i)) != 0);
		}

		private void buttonOK_Click(object sender, System.EventArgs e) {
			m_upgm = 0;
			for (int i = 0; i < checkedListBox.CheckedItems.Count; i++) {
				UpgradeType upgt = ((UpgradeTypeWrapper)checkedListBox.CheckedItems[i]).upgt;
				m_upgm = (UpgradeMask)(((uint)m_upgm) | (uint)(1 << (int)upgt));
			}
		}

		private void buttonAll_Click(object sender, System.EventArgs e) {
			m_upgm = UpgradeMask.kupgmAll;
			InitUpgradeTypesListBox();
		}

		private void buttonNone_Click(object sender, System.EventArgs e) {
			m_upgm = UpgradeMask.kupgmNone;
			InitUpgradeTypesListBox();
		}
	
		public UpgradeMask UpgradeMask {
			get {
				return m_upgm;
			}
		}
	}

	public class UpgradeTypeWrapper {
		public UpgradeType upgt;
		public UpgradeTypeWrapper(UpgradeType upgt) {
			this.upgt = upgt;
		}

		override public string ToString() {
			return Helper.GetDisplayName(typeof(UpgradeType), upgt.ToString());
		}
	}
}
