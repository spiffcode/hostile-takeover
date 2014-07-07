using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace m
{
	/// <summary>
	/// Summary description for TriggerPropForm.
	/// </summary>
	public class TriggerPropForm : System.Windows.Forms.Form
	{
		Trigger m_tgr;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPageConditions;
		private System.Windows.Forms.TabPage tabPageActions;
		private System.Windows.Forms.Button buttonOk;
		private System.Windows.Forms.Button buttonCancel;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.CheckedListBox checkedListBoxConditions;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button buttonNewCondition;
		private System.Windows.Forms.Button buttonModifyCondition;
		private System.Windows.Forms.Button buttonCopyCondition;
		private System.Windows.Forms.Button buttonDeleteCondition;
		private System.Windows.Forms.Button buttonMoveUpCondition;
		private System.Windows.Forms.Button buttonMoveDownCondition;
		private System.Windows.Forms.Button buttonMoveUpAction;
		private System.Windows.Forms.Button buttonDeleteAction;
		private System.Windows.Forms.Button buttonCopyAction;
		private System.Windows.Forms.Button buttonModifyAction;
		private System.Windows.Forms.Button buttonNewAction;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.CheckedListBox checkedListBoxActions;
		private System.Windows.Forms.Button buttonMoveDownAction;
		private System.Windows.Forms.CheckedListBox checkedListBoxSides;
		private System.Windows.Forms.TabPage tabPageSides;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public TriggerPropForm(Trigger tgr)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			m_tgr = tgr;
			InitConditionsListBox(-1);
			InitActionsListBox(-1);
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
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabPageSides = new System.Windows.Forms.TabPage();
			this.label1 = new System.Windows.Forms.Label();
			this.checkedListBoxSides = new System.Windows.Forms.CheckedListBox();
			this.tabPageConditions = new System.Windows.Forms.TabPage();
			this.buttonMoveUpCondition = new System.Windows.Forms.Button();
			this.buttonDeleteCondition = new System.Windows.Forms.Button();
			this.buttonCopyCondition = new System.Windows.Forms.Button();
			this.buttonModifyCondition = new System.Windows.Forms.Button();
			this.buttonNewCondition = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.checkedListBoxConditions = new System.Windows.Forms.CheckedListBox();
			this.buttonMoveDownCondition = new System.Windows.Forms.Button();
			this.tabPageActions = new System.Windows.Forms.TabPage();
			this.buttonMoveUpAction = new System.Windows.Forms.Button();
			this.buttonDeleteAction = new System.Windows.Forms.Button();
			this.buttonCopyAction = new System.Windows.Forms.Button();
			this.buttonModifyAction = new System.Windows.Forms.Button();
			this.buttonNewAction = new System.Windows.Forms.Button();
			this.label3 = new System.Windows.Forms.Label();
			this.checkedListBoxActions = new System.Windows.Forms.CheckedListBox();
			this.buttonMoveDownAction = new System.Windows.Forms.Button();
			this.buttonOk = new System.Windows.Forms.Button();
			this.buttonCancel = new System.Windows.Forms.Button();
			this.tabControl1.SuspendLayout();
			this.tabPageSides.SuspendLayout();
			this.tabPageConditions.SuspendLayout();
			this.tabPageActions.SuspendLayout();
			this.SuspendLayout();
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.AddRange(new System.Windows.Forms.Control[] {
																					  this.tabPageSides,
																					  this.tabPageConditions,
																					  this.tabPageActions});
			this.tabControl1.Location = new System.Drawing.Point(16, 16);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(480, 384);
			this.tabControl1.TabIndex = 0;
			this.tabControl1.SelectedIndexChanged += new System.EventHandler(this.tabControl1_SelectedIndexChanged);
			// 
			// tabPageSides
			// 
			this.tabPageSides.Controls.AddRange(new System.Windows.Forms.Control[] {
																					   this.label1,
																					   this.checkedListBoxSides});
			this.tabPageSides.Location = new System.Drawing.Point(4, 22);
			this.tabPageSides.Name = "tabPageSides";
			this.tabPageSides.Size = new System.Drawing.Size(472, 358);
			this.tabPageSides.TabIndex = 0;
			this.tabPageSides.Text = "Sides";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(16, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(208, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "For which sides will this trigger execute?";
			// 
			// checkedListBoxSides
			// 
			this.checkedListBoxSides.CheckOnClick = true;
			this.checkedListBoxSides.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.checkedListBoxSides.Location = new System.Drawing.Point(16, 37);
			this.checkedListBoxSides.Name = "checkedListBoxSides";
			this.checkedListBoxSides.Size = new System.Drawing.Size(352, 304);
			this.checkedListBoxSides.TabIndex = 0;
			this.checkedListBoxSides.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBoxSides_ItemCheck);
			// 
			// tabPageConditions
			// 
			this.tabPageConditions.Controls.AddRange(new System.Windows.Forms.Control[] {
																							this.buttonMoveUpCondition,
																							this.buttonDeleteCondition,
																							this.buttonCopyCondition,
																							this.buttonModifyCondition,
																							this.buttonNewCondition,
																							this.label2,
																							this.checkedListBoxConditions,
																							this.buttonMoveDownCondition});
			this.tabPageConditions.Location = new System.Drawing.Point(4, 22);
			this.tabPageConditions.Name = "tabPageConditions";
			this.tabPageConditions.Size = new System.Drawing.Size(472, 358);
			this.tabPageConditions.TabIndex = 1;
			this.tabPageConditions.Text = "Conditions";
			// 
			// buttonMoveUpCondition
			// 
			this.buttonMoveUpCondition.Location = new System.Drawing.Point(16, 323);
			this.buttonMoveUpCondition.Name = "buttonMoveUpCondition";
			this.buttonMoveUpCondition.Size = new System.Drawing.Size(168, 23);
			this.buttonMoveUpCondition.TabIndex = 6;
			this.buttonMoveUpCondition.Text = "Move &Up";
			this.buttonMoveUpCondition.Click += new System.EventHandler(this.buttonMoveUpCondition_Click);
			// 
			// buttonDeleteCondition
			// 
			this.buttonDeleteCondition.Location = new System.Drawing.Point(382, 133);
			this.buttonDeleteCondition.Name = "buttonDeleteCondition";
			this.buttonDeleteCondition.TabIndex = 5;
			this.buttonDeleteCondition.Text = "&Delete";
			this.buttonDeleteCondition.Click += new System.EventHandler(this.buttonDeleteCondition_Click);
			// 
			// buttonCopyCondition
			// 
			this.buttonCopyCondition.Location = new System.Drawing.Point(382, 101);
			this.buttonCopyCondition.Name = "buttonCopyCondition";
			this.buttonCopyCondition.TabIndex = 4;
			this.buttonCopyCondition.Text = "&Copy";
			this.buttonCopyCondition.Click += new System.EventHandler(this.buttonCopyCondition_Click);
			// 
			// buttonModifyCondition
			// 
			this.buttonModifyCondition.Location = new System.Drawing.Point(382, 69);
			this.buttonModifyCondition.Name = "buttonModifyCondition";
			this.buttonModifyCondition.TabIndex = 3;
			this.buttonModifyCondition.Text = "&Modify...";
			this.buttonModifyCondition.Click += new System.EventHandler(this.buttonModifyCondition_Click);
			// 
			// buttonNewCondition
			// 
			this.buttonNewCondition.Location = new System.Drawing.Point(382, 37);
			this.buttonNewCondition.Name = "buttonNewCondition";
			this.buttonNewCondition.TabIndex = 2;
			this.buttonNewCondition.Text = "&New...";
			this.buttonNewCondition.Click += new System.EventHandler(this.buttonNewCondition_Click);
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(16, 13);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(312, 16);
			this.label2.TabIndex = 1;
			this.label2.Text = "Conditions specified for this trigger:";
			// 
			// checkedListBoxConditions
			// 
			this.checkedListBoxConditions.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.checkedListBoxConditions.Location = new System.Drawing.Point(16, 37);
			this.checkedListBoxConditions.Name = "checkedListBoxConditions";
			this.checkedListBoxConditions.Size = new System.Drawing.Size(352, 274);
			this.checkedListBoxConditions.TabIndex = 0;
			this.checkedListBoxConditions.SelectedIndexChanged += new System.EventHandler(this.checkedListBoxConditions_SelectedIndexChanged);
			this.checkedListBoxConditions.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBoxConditions_ItemCheck);
			// 
			// buttonMoveDownCondition
			// 
			this.buttonMoveDownCondition.Location = new System.Drawing.Point(200, 323);
			this.buttonMoveDownCondition.Name = "buttonMoveDownCondition";
			this.buttonMoveDownCondition.Size = new System.Drawing.Size(168, 23);
			this.buttonMoveDownCondition.TabIndex = 6;
			this.buttonMoveDownCondition.Text = "Move D&own";
			this.buttonMoveDownCondition.Click += new System.EventHandler(this.buttonMoveDownCondition_Click);
			// 
			// tabPageActions
			// 
			this.tabPageActions.Controls.AddRange(new System.Windows.Forms.Control[] {
																						 this.buttonMoveUpAction,
																						 this.buttonDeleteAction,
																						 this.buttonCopyAction,
																						 this.buttonModifyAction,
																						 this.buttonNewAction,
																						 this.label3,
																						 this.checkedListBoxActions,
																						 this.buttonMoveDownAction});
			this.tabPageActions.Location = new System.Drawing.Point(4, 22);
			this.tabPageActions.Name = "tabPageActions";
			this.tabPageActions.Size = new System.Drawing.Size(472, 358);
			this.tabPageActions.TabIndex = 2;
			this.tabPageActions.Text = "Actions";
			// 
			// buttonMoveUpAction
			// 
			this.buttonMoveUpAction.Location = new System.Drawing.Point(16, 323);
			this.buttonMoveUpAction.Name = "buttonMoveUpAction";
			this.buttonMoveUpAction.Size = new System.Drawing.Size(168, 23);
			this.buttonMoveUpAction.TabIndex = 14;
			this.buttonMoveUpAction.Text = "Move &Up";
			this.buttonMoveUpAction.Click += new System.EventHandler(this.buttonMoveUpAction_Click);
			// 
			// buttonDeleteAction
			// 
			this.buttonDeleteAction.Location = new System.Drawing.Point(382, 133);
			this.buttonDeleteAction.Name = "buttonDeleteAction";
			this.buttonDeleteAction.TabIndex = 12;
			this.buttonDeleteAction.Text = "&Delete";
			this.buttonDeleteAction.Click += new System.EventHandler(this.buttonDeleteAction_Click);
			// 
			// buttonCopyAction
			// 
			this.buttonCopyAction.Location = new System.Drawing.Point(382, 101);
			this.buttonCopyAction.Name = "buttonCopyAction";
			this.buttonCopyAction.TabIndex = 11;
			this.buttonCopyAction.Text = "&Copy";
			this.buttonCopyAction.Click += new System.EventHandler(this.buttonCopyAction_Click);
			// 
			// buttonModifyAction
			// 
			this.buttonModifyAction.Location = new System.Drawing.Point(382, 69);
			this.buttonModifyAction.Name = "buttonModifyAction";
			this.buttonModifyAction.TabIndex = 10;
			this.buttonModifyAction.Text = "&Modify...";
			this.buttonModifyAction.Click += new System.EventHandler(this.buttonModifyAction_Click);
			// 
			// buttonNewAction
			// 
			this.buttonNewAction.Location = new System.Drawing.Point(382, 37);
			this.buttonNewAction.Name = "buttonNewAction";
			this.buttonNewAction.TabIndex = 9;
			this.buttonNewAction.Text = "&New...";
			this.buttonNewAction.Click += new System.EventHandler(this.buttonNewAction_Click);
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(16, 13);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(312, 16);
			this.label3.TabIndex = 8;
			this.label3.Text = "Actions specified for this trigger:";
			// 
			// checkedListBoxActions
			// 
			this.checkedListBoxActions.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.checkedListBoxActions.Location = new System.Drawing.Point(16, 37);
			this.checkedListBoxActions.Name = "checkedListBoxActions";
			this.checkedListBoxActions.Size = new System.Drawing.Size(352, 274);
			this.checkedListBoxActions.TabIndex = 7;
			this.checkedListBoxActions.SelectedIndexChanged += new System.EventHandler(this.checkedListBoxActions_SelectedIndexChanged);
			// 
			// buttonMoveDownAction
			// 
			this.buttonMoveDownAction.Location = new System.Drawing.Point(200, 323);
			this.buttonMoveDownAction.Name = "buttonMoveDownAction";
			this.buttonMoveDownAction.Size = new System.Drawing.Size(168, 23);
			this.buttonMoveDownAction.TabIndex = 13;
			this.buttonMoveDownAction.Text = "Move D&own";
			this.buttonMoveDownAction.Click += new System.EventHandler(this.buttonMoveDownAction_Click);
			// 
			// buttonOk
			// 
			this.buttonOk.Location = new System.Drawing.Point(155, 412);
			this.buttonOk.Name = "buttonOk";
			this.buttonOk.TabIndex = 1;
			this.buttonOk.Text = "Ok";
			this.buttonOk.Click += new System.EventHandler(this.buttonOk_Click);
			// 
			// buttonCancel
			// 
			this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonCancel.Location = new System.Drawing.Point(283, 412);
			this.buttonCancel.Name = "buttonCancel";
			this.buttonCancel.TabIndex = 1;
			this.buttonCancel.Text = "Cancel";
			this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
			// 
			// TriggerPropForm
			// 
			this.AcceptButton = this.buttonOk;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.buttonCancel;
			this.ClientSize = new System.Drawing.Size(512, 446);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.buttonOk,
																		  this.tabControl1,
																		  this.buttonCancel});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "TriggerPropForm";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Trigger Properties";
			this.Load += new System.EventHandler(this.TriggerPropForm_Load);
			this.tabControl1.ResumeLayout(false);
			this.tabPageSides.ResumeLayout(false);
			this.tabPageConditions.ResumeLayout(false);
			this.tabPageActions.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		void InitSidesListBox() {
			for (int n = 0; n < checkedListBoxSides.Items.Count; n++) {
				checkedListBoxSides.SetItemChecked(n, (m_tgr.Sides & (1 << n)) != 0);
			}
		}

		void InitConditionsListBox(int nSelectedIndex) {
			checkedListBoxConditions.Items.Clear();
			foreach (CaBase cab in m_tgr.Conditions)
				checkedListBoxConditions.Items.Add(cab, cab.Active);
			EnableConditionButtons();
			checkedListBoxConditions.SelectedIndex = nSelectedIndex;
			if (checkedListBoxConditions.SelectedIndex == -1 && checkedListBoxConditions.Items.Count != 0)
				checkedListBoxConditions.SelectedIndex = 0;
		}

		void InitActionsListBox(int nSelectedIndex) {
			checkedListBoxActions.Items.Clear();
			foreach (CaBase cab in m_tgr.Actions)
				checkedListBoxActions.Items.Add(cab, cab.Active);
			EnableActionButtons();
			checkedListBoxActions.SelectedIndex = nSelectedIndex;
			if (checkedListBoxActions.SelectedIndex == -1 && checkedListBoxActions.Items.Count != 0)
				checkedListBoxActions.SelectedIndex = 0;
		}

		private void buttonNewCondition_Click(object sender, System.EventArgs e) {
			CaBase cab = CaNew.DoModal(null, "New Condition", "Condition");
			if (cab != null) {
				m_tgr.Conditions.Add(cab);
				InitConditionsListBox(m_tgr.Conditions.Count - 1);
			}
		}

		private void buttonModifyCondition_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxConditions.SelectedIndex;
			if (n < 0)
				return;
			CaBase cab = (CaBase)m_tgr.Conditions[n];
			cab = CaNew.DoModal(cab.Clone(), "Modify Condition", "Condition");
			if (cab != null) {
				m_tgr.Conditions[n] = cab;
				InitConditionsListBox(n);
			}		
		}

		private void buttonNewAction_Click(object sender, System.EventArgs e) {
			CaBase cab = CaNew.DoModal(null, "New Action", "TriggerAction");
			if (cab != null) {
				m_tgr.Actions.Add(cab);
				InitActionsListBox(m_tgr.Actions.Count - 1);
			}
		}

		private void buttonModifyAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;
			if (n < 0)
				return;
			CaBase cab = (CaBase)m_tgr.Actions[n];
			cab = CaNew.DoModal(cab.Clone(), "Modify Action", "TriggerAction");
			if (cab != null) {
				m_tgr.Actions[n] = cab;
				InitActionsListBox(n);
			}
		}

		private void buttonOk_Click(object sender, System.EventArgs e) {
			if (!m_tgr.IsValid()) {
				MessageBox.Show(this, "Trigger not initialized correctly! (" + m_tgr.GetError() + ")");
				return;
			}
			DialogResult = DialogResult.OK;
		}

		private void buttonCancel_Click(object sender, System.EventArgs e) {
			DialogResult = DialogResult.Cancel;
		}

		private void checkedListBoxSides_ItemCheck(object sender, System.Windows.Forms.ItemCheckEventArgs e) {
			if (e.CurrentValue == CheckState.Unchecked && e.NewValue == CheckState.Checked) {
				m_tgr.Sides |= (1 << e.Index);
			}
			if (e.CurrentValue == CheckState.Checked && e.NewValue == CheckState.Unchecked) {
				m_tgr.Sides &= ~(1 << e.Index);
			}
		}

		private void tabControl1_SelectedIndexChanged(object sender, System.EventArgs e) {
			switch (tabControl1.SelectedIndex) {
			case 0:
				InitSidesListBox();
				break;

			case 1:
				InitConditionsListBox(-1);
				break;

			case 2:
				InitActionsListBox(-1);
				break;
			}
		}

		private void checkedListBoxConditions_ItemCheck(object sender, System.Windows.Forms.ItemCheckEventArgs e) {
			if (e.CurrentValue == CheckState.Unchecked && e.NewValue == CheckState.Checked) {
				CaBase cab = (CaBase)m_tgr.Conditions[e.Index];
				cab.Active = true;
			}
			if (e.CurrentValue == CheckState.Checked && e.NewValue == CheckState.Unchecked) {
				CaBase cab = (CaBase)m_tgr.Conditions[e.Index];
				cab.Active = false;
			}
		}

		private void checkedListBoxConditions_SelectedIndexChanged(object sender, System.EventArgs e) {
			EnableConditionButtons();
		}

		void EnableConditionButtons() {
			int n = checkedListBoxConditions.SelectedIndex;
			bool fItemSelected = (n >= 0);
			buttonModifyCondition.Enabled = fItemSelected;
			buttonCopyCondition.Enabled = fItemSelected;
			buttonDeleteCondition.Enabled = fItemSelected;
			buttonMoveUpCondition.Enabled = (n > 0);
			buttonMoveDownCondition.Enabled = (n < checkedListBoxConditions.Items.Count - 1);
		}

		private void checkedListBoxActions_SelectedIndexChanged(object sender, System.EventArgs e) {
			EnableActionButtons();
		}

		void EnableActionButtons() {
			int n = checkedListBoxActions.SelectedIndex;
			bool fItemSelected = (n >= 0);
			buttonModifyAction.Enabled = fItemSelected;
			buttonCopyAction.Enabled = fItemSelected;
			buttonDeleteAction.Enabled = fItemSelected;
			buttonMoveUpAction.Enabled = (n > 0);
			buttonMoveDownAction.Enabled = (n < checkedListBoxActions.Items.Count - 1);		
		}

		private void buttonCopyCondition_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxConditions.SelectedIndex;		
			if (n < 0)
				return;
			CaBase cab = (CaBase)m_tgr.Conditions[n];
			m_tgr.Conditions.Add(cab.Clone());
			InitConditionsListBox(m_tgr.Conditions.Count - 1);
		}

		private void buttonDeleteCondition_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxConditions.SelectedIndex;		
			if (n < 0)
				return;
			m_tgr.Conditions.RemoveAt(n);
			InitConditionsListBox(Math.Min(m_tgr.Conditions.Count - 1, n));		
		}

		private void buttonCopyAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;		
			if (n < 0)
				return;
			CaBase cab = (CaBase)m_tgr.Actions[n];
			m_tgr.Actions.Add(cab.Clone());
			InitActionsListBox(m_tgr.Actions.Count - 1);		
		}

		private void buttonDeleteAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;		
			if (n < 0)
				return;
			m_tgr.Actions.RemoveAt(n);
			InitActionsListBox(Math.Min(m_tgr.Actions.Count - 1, n));
		}

		private void buttonMoveUpCondition_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxConditions.SelectedIndex;		
			if (n <= 0)
				return;
			CaBase cab = (CaBase)m_tgr.Conditions[n];
			m_tgr.Conditions.RemoveAt(n);
			m_tgr.Conditions.Insert(n - 1, cab);
			InitConditionsListBox(n - 1);
		}

		private void buttonMoveDownCondition_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxConditions.SelectedIndex;		
			if (n < 0 || n >= checkedListBoxConditions.Items.Count - 1)
				return;
			CaBase cab = (CaBase)m_tgr.Conditions[n];
			m_tgr.Conditions.RemoveAt(n);
			m_tgr.Conditions.Insert(n + 1, cab);
			InitConditionsListBox(n + 1);		
		}

		private void buttonMoveUpAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;		
			if (n <= 0)
				return;
			CaBase cab = (CaBase)m_tgr.Actions[n];
			m_tgr.Actions.RemoveAt(n);
			m_tgr.Actions.Insert(n - 1, cab);
			InitActionsListBox(n - 1);
		}

		private void buttonMoveDownAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;		
			if (n < 0 || n >= checkedListBoxActions.Items.Count - 1)
				return;
			CaBase cab = (CaBase)m_tgr.Actions[n];
			m_tgr.Actions.RemoveAt(n);
			m_tgr.Actions.Insert(n + 1, cab);
			InitActionsListBox(n + 1);
		}

		private void TriggerPropForm_Load(object sender, System.EventArgs e) {
			// Have to do this initialization here because the sides checked
			// list box will get unchecked before display if we don't.

			checkedListBoxSides.DataSource = Enum.GetNames(typeof(Side));
			InitSidesListBox();
		}
	}
}
