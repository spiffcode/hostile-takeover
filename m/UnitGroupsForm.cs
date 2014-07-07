using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Collections.Specialized;

namespace m
{
	/// <summary>
	/// Summary description for UnitGroupsForm.
	/// </summary>
	public class UnitGroupsForm : System.Windows.Forms.Form
	{
		private UnitGroupManager m_ugm;
		private UnitGroup m_ugSelected;
		private LevelDoc m_lvld;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Button buttonMoveUpAction;
		private System.Windows.Forms.Button buttonDeleteAction;
		private System.Windows.Forms.Button buttonCopyAction;
		private System.Windows.Forms.Button buttonModifyAction;
		private System.Windows.Forms.Button buttonNewAction;
		private System.Windows.Forms.CheckedListBox checkedListBoxActions;
		private System.Windows.Forms.Button buttonMoveDownAction;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.ListView listViewUnitGroups;
		private System.Windows.Forms.Button buttonNewUnitGroup;
		private System.Windows.Forms.Button buttonDeleteUnitGroup;
		private System.Windows.Forms.ListBox listBoxUnits;
		private System.Windows.Forms.Button buttonDeleteUnits;
		private System.Windows.Forms.Button buttonNewUnits;
		private System.Windows.Forms.Button buttonClose;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.ComboBox comboBoxSide;
		private System.Windows.Forms.CheckBox checkBoxForever;
		private System.Windows.Forms.ComboBox comboBoxAggressiveness;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.CheckBox checkBoxRandomGroup;
		private System.Windows.Forms.CheckBox checkBoxCreateAtLevelLoad;
		private System.Windows.Forms.CheckBox checkBoxReplaceDestroyedGroup;
		private System.Windows.Forms.CheckBox checkBoxSpawn;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.ComboBox comboBoxSpawnArea;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox textBoxHealth;
		private System.Windows.Forms.Button buttonAddUnit;
		private System.Windows.Forms.Button buttonSubtractUnit;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public UnitGroupsForm(LevelDoc lvld, UnitGroupManager ugm)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//

			m_ugm = ugm;
			m_lvld = lvld;
			m_ugm.ClearModified();
			InitUnitGroupsListBox();
			InitSideComboBox();
			InitAggressivenessComboBox();
			InitSpawnAreaComboBox();
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
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.checkBoxForever = new System.Windows.Forms.CheckBox();
			this.buttonMoveUpAction = new System.Windows.Forms.Button();
			this.buttonDeleteAction = new System.Windows.Forms.Button();
			this.buttonCopyAction = new System.Windows.Forms.Button();
			this.buttonModifyAction = new System.Windows.Forms.Button();
			this.buttonNewAction = new System.Windows.Forms.Button();
			this.checkedListBoxActions = new System.Windows.Forms.CheckedListBox();
			this.buttonMoveDownAction = new System.Windows.Forms.Button();
			this.buttonClose = new System.Windows.Forms.Button();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.buttonDeleteUnits = new System.Windows.Forms.Button();
			this.buttonNewUnits = new System.Windows.Forms.Button();
			this.listBoxUnits = new System.Windows.Forms.ListBox();
			this.buttonNewUnitGroup = new System.Windows.Forms.Button();
			this.buttonDeleteUnitGroup = new System.Windows.Forms.Button();
			this.listViewUnitGroups = new System.Windows.Forms.ListView();
			this.label1 = new System.Windows.Forms.Label();
			this.comboBoxSide = new System.Windows.Forms.ComboBox();
			this.comboBoxAggressiveness = new System.Windows.Forms.ComboBox();
			this.label2 = new System.Windows.Forms.Label();
			this.checkBoxRandomGroup = new System.Windows.Forms.CheckBox();
			this.checkBoxCreateAtLevelLoad = new System.Windows.Forms.CheckBox();
			this.checkBoxReplaceDestroyedGroup = new System.Windows.Forms.CheckBox();
			this.checkBoxSpawn = new System.Windows.Forms.CheckBox();
			this.label3 = new System.Windows.Forms.Label();
			this.comboBoxSpawnArea = new System.Windows.Forms.ComboBox();
			this.label4 = new System.Windows.Forms.Label();
			this.textBoxHealth = new System.Windows.Forms.TextBox();
			this.buttonAddUnit = new System.Windows.Forms.Button();
			this.buttonSubtractUnit = new System.Windows.Forms.Button();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
																					this.checkBoxForever,
																					this.buttonMoveUpAction,
																					this.buttonDeleteAction,
																					this.buttonCopyAction,
																					this.buttonModifyAction,
																					this.buttonNewAction,
																					this.checkedListBoxActions,
																					this.buttonMoveDownAction});
			this.groupBox1.Location = new System.Drawing.Point(152, 240);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(488, 280);
			this.groupBox1.TabIndex = 1;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Actions";
			// 
			// checkBoxForever
			// 
			this.checkBoxForever.Location = new System.Drawing.Point(400, 160);
			this.checkBoxForever.Name = "checkBoxForever";
			this.checkBoxForever.Size = new System.Drawing.Size(64, 32);
			this.checkBoxForever.TabIndex = 23;
			this.checkBoxForever.Text = "Repeat forever";
			this.checkBoxForever.CheckedChanged += new System.EventHandler(this.checkBoxForever_CheckedChanged);
			// 
			// buttonMoveUpAction
			// 
			this.buttonMoveUpAction.Location = new System.Drawing.Point(8, 248);
			this.buttonMoveUpAction.Name = "buttonMoveUpAction";
			this.buttonMoveUpAction.Size = new System.Drawing.Size(152, 23);
			this.buttonMoveUpAction.TabIndex = 22;
			this.buttonMoveUpAction.Text = "Move Up";
			this.buttonMoveUpAction.Click += new System.EventHandler(this.buttonMoveUpAction_Click);
			// 
			// buttonDeleteAction
			// 
			this.buttonDeleteAction.Location = new System.Drawing.Point(400, 112);
			this.buttonDeleteAction.Name = "buttonDeleteAction";
			this.buttonDeleteAction.TabIndex = 20;
			this.buttonDeleteAction.Text = "Delete";
			this.buttonDeleteAction.Click += new System.EventHandler(this.buttonDeleteAction_Click);
			// 
			// buttonCopyAction
			// 
			this.buttonCopyAction.Location = new System.Drawing.Point(400, 80);
			this.buttonCopyAction.Name = "buttonCopyAction";
			this.buttonCopyAction.TabIndex = 19;
			this.buttonCopyAction.Text = "Copy";
			this.buttonCopyAction.Click += new System.EventHandler(this.buttonCopyAction_Click);
			// 
			// buttonModifyAction
			// 
			this.buttonModifyAction.Location = new System.Drawing.Point(400, 48);
			this.buttonModifyAction.Name = "buttonModifyAction";
			this.buttonModifyAction.TabIndex = 18;
			this.buttonModifyAction.Text = "Modify...";
			this.buttonModifyAction.Click += new System.EventHandler(this.buttonModifyAction_Click);
			// 
			// buttonNewAction
			// 
			this.buttonNewAction.Location = new System.Drawing.Point(400, 16);
			this.buttonNewAction.Name = "buttonNewAction";
			this.buttonNewAction.TabIndex = 17;
			this.buttonNewAction.Text = "New...";
			this.buttonNewAction.Click += new System.EventHandler(this.buttonNewAction_Click);
			// 
			// checkedListBoxActions
			// 
			this.checkedListBoxActions.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.checkedListBoxActions.Location = new System.Drawing.Point(8, 16);
			this.checkedListBoxActions.Name = "checkedListBoxActions";
			this.checkedListBoxActions.Size = new System.Drawing.Size(384, 225);
			this.checkedListBoxActions.TabIndex = 15;
			this.checkedListBoxActions.SelectedIndexChanged += new System.EventHandler(this.checkedListBoxActions_SelectedIndexChanged);
			// 
			// buttonMoveDownAction
			// 
			this.buttonMoveDownAction.Location = new System.Drawing.Point(168, 248);
			this.buttonMoveDownAction.Name = "buttonMoveDownAction";
			this.buttonMoveDownAction.Size = new System.Drawing.Size(152, 23);
			this.buttonMoveDownAction.TabIndex = 21;
			this.buttonMoveDownAction.Text = "Move Down";
			this.buttonMoveDownAction.Click += new System.EventHandler(this.buttonMoveDownAction_Click);
			// 
			// buttonClose
			// 
			this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonClose.Location = new System.Drawing.Point(552, 8);
			this.buttonClose.Name = "buttonClose";
			this.buttonClose.TabIndex = 3;
			this.buttonClose.Text = "Close";
			this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.AddRange(new System.Windows.Forms.Control[] {
																					this.buttonAddUnit,
																					this.buttonDeleteUnits,
																					this.buttonNewUnits,
																					this.listBoxUnits,
																					this.buttonSubtractUnit});
			this.groupBox2.Location = new System.Drawing.Point(152, 104);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(488, 128);
			this.groupBox2.TabIndex = 4;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Units";
			// 
			// buttonDeleteUnits
			// 
			this.buttonDeleteUnits.Location = new System.Drawing.Point(400, 48);
			this.buttonDeleteUnits.Name = "buttonDeleteUnits";
			this.buttonDeleteUnits.TabIndex = 2;
			this.buttonDeleteUnits.Text = "Delete";
			this.buttonDeleteUnits.Click += new System.EventHandler(this.buttonDeleteUnits_Click);
			// 
			// buttonNewUnits
			// 
			this.buttonNewUnits.Location = new System.Drawing.Point(400, 16);
			this.buttonNewUnits.Name = "buttonNewUnits";
			this.buttonNewUnits.TabIndex = 1;
			this.buttonNewUnits.Text = "New...";
			this.buttonNewUnits.Click += new System.EventHandler(this.buttonNewUnits_Click);
			// 
			// listBoxUnits
			// 
			this.listBoxUnits.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.listBoxUnits.ItemHeight = 16;
			this.listBoxUnits.Location = new System.Drawing.Point(8, 16);
			this.listBoxUnits.Name = "listBoxUnits";
			this.listBoxUnits.Size = new System.Drawing.Size(384, 100);
			this.listBoxUnits.TabIndex = 0;
			this.listBoxUnits.SelectedIndexChanged += new System.EventHandler(this.listBoxUnits_SelectedIndexChanged);
			// 
			// buttonNewUnitGroup
			// 
			this.buttonNewUnitGroup.Location = new System.Drawing.Point(16, 456);
			this.buttonNewUnitGroup.Name = "buttonNewUnitGroup";
			this.buttonNewUnitGroup.TabIndex = 7;
			this.buttonNewUnitGroup.Text = "New...";
			this.buttonNewUnitGroup.Click += new System.EventHandler(this.buttonNewUnitGroup_Click);
			// 
			// buttonDeleteUnitGroup
			// 
			this.buttonDeleteUnitGroup.Location = new System.Drawing.Point(16, 488);
			this.buttonDeleteUnitGroup.Name = "buttonDeleteUnitGroup";
			this.buttonDeleteUnitGroup.TabIndex = 8;
			this.buttonDeleteUnitGroup.Text = "Delete";
			this.buttonDeleteUnitGroup.Click += new System.EventHandler(this.buttonDeleteUnitGroup_Click);
			// 
			// listViewUnitGroups
			// 
			this.listViewUnitGroups.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.listViewUnitGroups.FullRowSelect = true;
			this.listViewUnitGroups.HideSelection = false;
			this.listViewUnitGroups.LabelEdit = true;
			this.listViewUnitGroups.Location = new System.Drawing.Point(8, 8);
			this.listViewUnitGroups.MultiSelect = false;
			this.listViewUnitGroups.Name = "listViewUnitGroups";
			this.listViewUnitGroups.Size = new System.Drawing.Size(136, 440);
			this.listViewUnitGroups.Sorting = System.Windows.Forms.SortOrder.Ascending;
			this.listViewUnitGroups.TabIndex = 9;
			this.listViewUnitGroups.View = System.Windows.Forms.View.List;
			this.listViewUnitGroups.AfterLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.listViewUnitGroups_AfterLabelEdit);
			this.listViewUnitGroups.SelectedIndexChanged += new System.EventHandler(this.listViewUnitGroups_SelectedIndexChanged);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(152, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(32, 16);
			this.label1.TabIndex = 10;
			this.label1.Text = "Side:";
			// 
			// comboBoxSide
			// 
			this.comboBoxSide.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxSide.Location = new System.Drawing.Point(184, 7);
			this.comboBoxSide.Name = "comboBoxSide";
			this.comboBoxSide.Size = new System.Drawing.Size(80, 21);
			this.comboBoxSide.TabIndex = 11;
			this.comboBoxSide.SelectedIndexChanged += new System.EventHandler(this.comboBoxSide_SelectedIndexChanged);
			// 
			// comboBoxAggressiveness
			// 
			this.comboBoxAggressiveness.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxAggressiveness.Location = new System.Drawing.Point(368, 7);
			this.comboBoxAggressiveness.Name = "comboBoxAggressiveness";
			this.comboBoxAggressiveness.Size = new System.Drawing.Size(80, 21);
			this.comboBoxAggressiveness.TabIndex = 13;
			this.comboBoxAggressiveness.SelectedIndexChanged += new System.EventHandler(this.comboBoxAggressiveness_SelectedIndexChanged);
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(280, 8);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(88, 16);
			this.label2.TabIndex = 12;
			this.label2.Text = "Aggressiveness:";
			// 
			// checkBoxRandomGroup
			// 
			this.checkBoxRandomGroup.Location = new System.Drawing.Point(152, 32);
			this.checkBoxRandomGroup.Name = "checkBoxRandomGroup";
			this.checkBoxRandomGroup.Size = new System.Drawing.Size(104, 16);
			this.checkBoxRandomGroup.TabIndex = 14;
			this.checkBoxRandomGroup.Text = "Random Group";
			this.checkBoxRandomGroup.CheckedChanged += new System.EventHandler(this.checkBoxRandomGroup_CheckedChanged);
			// 
			// checkBoxCreateAtLevelLoad
			// 
			this.checkBoxCreateAtLevelLoad.Location = new System.Drawing.Point(264, 32);
			this.checkBoxCreateAtLevelLoad.Name = "checkBoxCreateAtLevelLoad";
			this.checkBoxCreateAtLevelLoad.Size = new System.Drawing.Size(128, 16);
			this.checkBoxCreateAtLevelLoad.TabIndex = 15;
			this.checkBoxCreateAtLevelLoad.Text = "Create at level load";
			this.checkBoxCreateAtLevelLoad.CheckedChanged += new System.EventHandler(this.checkBoxCreateAtLevelLoad_CheckedChanged);
			// 
			// checkBoxReplaceDestroyedGroup
			// 
			this.checkBoxReplaceDestroyedGroup.Location = new System.Drawing.Point(392, 32);
			this.checkBoxReplaceDestroyedGroup.Name = "checkBoxReplaceDestroyedGroup";
			this.checkBoxReplaceDestroyedGroup.Size = new System.Drawing.Size(152, 16);
			this.checkBoxReplaceDestroyedGroup.TabIndex = 16;
			this.checkBoxReplaceDestroyedGroup.Text = "Recreate if destroyed";
			this.checkBoxReplaceDestroyedGroup.CheckedChanged += new System.EventHandler(this.checkBoxReplaceDestroyedGroup_CheckedChanged);
			// 
			// checkBoxSpawn
			// 
			this.checkBoxSpawn.Location = new System.Drawing.Point(152, 56);
			this.checkBoxSpawn.Name = "checkBoxSpawn";
			this.checkBoxSpawn.Size = new System.Drawing.Size(120, 16);
			this.checkBoxSpawn.TabIndex = 17;
			this.checkBoxSpawn.Text = "Spawn, don\'t build";
			this.checkBoxSpawn.CheckedChanged += new System.EventHandler(this.checkBoxSpawn_CheckedChanged);
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(272, 56);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(72, 15);
			this.label3.TabIndex = 18;
			this.label3.Text = "Spawn Area:";
			// 
			// comboBoxSpawnArea
			// 
			this.comboBoxSpawnArea.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxSpawnArea.Location = new System.Drawing.Point(344, 54);
			this.comboBoxSpawnArea.Name = "comboBoxSpawnArea";
			this.comboBoxSpawnArea.Size = new System.Drawing.Size(160, 21);
			this.comboBoxSpawnArea.TabIndex = 19;
			this.comboBoxSpawnArea.SelectedIndexChanged += new System.EventHandler(this.comboBoxSpawnArea_SelectedIndexChanged);
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(152, 80);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(64, 16);
			this.label4.TabIndex = 20;
			this.label4.Text = "Health (%):";
			// 
			// textBoxHealth
			// 
			this.textBoxHealth.Location = new System.Drawing.Point(216, 78);
			this.textBoxHealth.Name = "textBoxHealth";
			this.textBoxHealth.Size = new System.Drawing.Size(40, 20);
			this.textBoxHealth.TabIndex = 21;
			this.textBoxHealth.Text = "100";
			this.textBoxHealth.TextChanged += new System.EventHandler(this.textBoxHealth_TextChanged);
			// 
			// buttonAddUnit
			// 
			this.buttonAddUnit.Location = new System.Drawing.Point(408, 80);
			this.buttonAddUnit.Name = "buttonAddUnit";
			this.buttonAddUnit.Size = new System.Drawing.Size(24, 23);
			this.buttonAddUnit.TabIndex = 3;
			this.buttonAddUnit.Text = "+";
			this.buttonAddUnit.Click += new System.EventHandler(this.buttonAddUnit_Click);
			// 
			// buttonSubtractUnit
			// 
			this.buttonSubtractUnit.Location = new System.Drawing.Point(440, 80);
			this.buttonSubtractUnit.Name = "buttonSubtractUnit";
			this.buttonSubtractUnit.Size = new System.Drawing.Size(24, 23);
			this.buttonSubtractUnit.TabIndex = 3;
			this.buttonSubtractUnit.Text = "–";
			this.buttonSubtractUnit.Click += new System.EventHandler(this.buttonSubtractUnit_Click);
			// 
			// UnitGroupsForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.buttonClose;
			this.ClientSize = new System.Drawing.Size(650, 528);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.textBoxHealth,
																		  this.label4,
																		  this.comboBoxSpawnArea,
																		  this.label3,
																		  this.checkBoxSpawn,
																		  this.checkBoxReplaceDestroyedGroup,
																		  this.checkBoxCreateAtLevelLoad,
																		  this.checkBoxRandomGroup,
																		  this.comboBoxAggressiveness,
																		  this.label2,
																		  this.comboBoxSide,
																		  this.label1,
																		  this.listViewUnitGroups,
																		  this.buttonDeleteUnitGroup,
																		  this.buttonNewUnitGroup,
																		  this.groupBox2,
																		  this.buttonClose,
																		  this.groupBox1});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "UnitGroupsForm";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Unit Groups";
			this.Closed += new System.EventHandler(this.UnitGroupsForm_Closed);
			this.groupBox1.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		// Action list management

		void InitActionsListBox(int nSelectedIndex) {
			checkedListBoxActions.Items.Clear();
			if (m_ugSelected != null) {
				foreach (CaBase cab in m_ugSelected.Actions)
					checkedListBoxActions.Items.Add(cab, cab.Active);
				checkedListBoxActions.SelectedIndex = nSelectedIndex;
				if (checkedListBoxActions.SelectedIndex == -1 && checkedListBoxActions.Items.Count != 0)
					checkedListBoxActions.SelectedIndex = 0;
			}
			EnableActionButtons();
		}

		private void buttonNewAction_Click(object sender, System.EventArgs e) {
			CaBase cab = CaNew.DoModal(null, "New Action", "UnitGroupAction");
			if (cab != null) {
				m_ugSelected.Actions.Add(cab);
				InitActionsListBox(m_ugSelected.Actions.Count - 1);
			}
		}

		private void buttonModifyAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;
			if (n < 0)
				return;
			CaBase cab = (CaBase)m_ugSelected.Actions[n];
			cab = CaNew.DoModal(cab.Clone(), "Modify Action", "UnitGroupAction");
			if (cab != null) {
				m_ugSelected.Actions[n] = cab;
				InitActionsListBox(n);
			}
		}

		private void checkedListBoxActions_SelectedIndexChanged(object sender, System.EventArgs e) {
			EnableActionButtons();
		}

		void EnableActionButtons() {
			int n = checkedListBoxActions.SelectedIndex;
			bool fItemSelected = (n >= 0);
			buttonNewAction.Enabled = m_ugSelected != null;
			buttonModifyAction.Enabled = fItemSelected;
			buttonCopyAction.Enabled = fItemSelected;
			buttonDeleteAction.Enabled = fItemSelected;
			buttonMoveUpAction.Enabled = (n > 0);
			buttonMoveDownAction.Enabled = (n < checkedListBoxActions.Items.Count - 1);		
		}

		private void buttonCopyAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;
			if (n < 0)
				return;
			CaBase cab = (CaBase)m_ugSelected.Actions[n];
			m_ugSelected.Actions.Add(cab.Clone());
			InitActionsListBox(m_ugSelected.Actions.Count - 1);		
		}

		private void buttonDeleteAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;		
			if (n < 0)
				return;
			m_ugSelected.Actions.RemoveAt(n);
			InitActionsListBox(Math.Min(m_ugSelected.Actions.Count - 1, n));
		}

		private void buttonMoveUpAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;		
			if (n <= 0)
				return;
			CaBase cab = (CaBase)m_ugSelected.Actions[n];
			m_ugSelected.Actions.RemoveAt(n);
			m_ugSelected.Actions.Insert(n - 1, cab);
			InitActionsListBox(n - 1);
		}

		private void buttonMoveDownAction_Click(object sender, System.EventArgs e) {
			int n = checkedListBoxActions.SelectedIndex;		
			if (n < 0 || n >= checkedListBoxActions.Items.Count - 1)
				return;
			CaBase cab = (CaBase)m_ugSelected.Actions[n];
			m_ugSelected.Actions.RemoveAt(n);
			m_ugSelected.Actions.Insert(n + 1, cab);
			InitActionsListBox(n + 1);
		}

		// UnitGroup management

		void InitUnitGroupsListBox() {
			listViewUnitGroups.Items.Clear();
			UnitGroup[] aug = m_ugm.GetUnitGroupList();
			bool fFirst = true;
			foreach (UnitGroup ug in aug) {
				ListViewItem lvi = new ListViewItem(ug.Name);
				lvi.Tag = ug;
				if (fFirst) {
					lvi.Selected = true;
					fFirst = false;
				}
				listViewUnitGroups.Items.Add(lvi);
			}
			EnableActionButtons();
			EnableUnitsButtons();
		}

		private void listViewUnitGroups_AfterLabelEdit(object sender, System.Windows.Forms.LabelEditEventArgs e) {
			if (e.Label != null && e.Label != "")
				((UnitGroup)listViewUnitGroups.Items[e.Item].Tag).Name = e.Label;
		}

		private void buttonNewUnitGroup_Click(object sender, System.EventArgs e) {
			int i = 1;
			for (; i < 100; i++) {
				bool fFound = false;
				for (int j = 0; j < listViewUnitGroups.Items.Count; j++) {
					if (listViewUnitGroups.Items[j].Text == "group" + i) {
						fFound = true;
						break;
					}
				}
				if (!fFound)
					break;
			}
			m_ugm.AddUnitGroup(new UnitGroup("group" + i));
			InitUnitGroupsListBox();
			SelectUnitGroup("group" + i);
		}

		private void SelectUnitGroup(string str) {
			for (int j = 0; j < listViewUnitGroups.Items.Count; j++) {
				if (listViewUnitGroups.Items[j].Text == str ) {
					listViewUnitGroups.Items[j].Selected = true;
					break;
				}
			}
		}

		private void buttonDeleteUnitGroup_Click(object sender, System.EventArgs e) {
			if (listViewUnitGroups.SelectedItems.Count == 0)
				return;
			ListViewItem lvi = listViewUnitGroups.SelectedItems[0];
			listViewUnitGroups.Items.Remove(lvi);
			m_ugm.RemoveUnitGroup((UnitGroup)lvi.Tag);
		}

		private void listViewUnitGroups_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (listViewUnitGroups.SelectedItems.Count == 0)
				m_ugSelected = null;
			else
				m_ugSelected = (UnitGroup)listViewUnitGroups.SelectedItems[0].Tag;
			InitUnitGroupPanel();
		}

		private void InitUnitGroupPanel() {
			InitUnitsListBox(-1);
			InitActionsListBox(-1);
			InitSideComboBox();
			InitAggressivenessComboBox();
			InitSpawnAreaComboBox();
			checkBoxForever.Checked = m_ugSelected == null ? false : m_ugSelected.LoopForever;
			checkBoxRandomGroup.Checked = m_ugSelected == null ? false : m_ugSelected.RandomGroup;
			checkBoxSpawn.Checked = m_ugSelected == null ? false : m_ugSelected.Spawn;
			comboBoxSpawnArea.Enabled = m_ugSelected == null ? false : m_ugSelected.Spawn;
			checkBoxCreateAtLevelLoad.Checked = m_ugSelected == null ? false : m_ugSelected.CreateAtLevelLoad;
			checkBoxReplaceDestroyedGroup.Checked = m_ugSelected == null ? false : m_ugSelected.ReplaceDestroyedGroup;
			textBoxHealth.Text = m_ugSelected == null ? "" : m_ugSelected.Health.ToString();
		}

		// Units list management

		void InitUnitsListBox(int nSelected) {
			listBoxUnits.Items.Clear();
			if (m_ugSelected != null) {

				foreach (UnitTypeAndCount utc in m_ugSelected.UnitTypeAndCounts)
					listBoxUnits.Items.Add(utc);
	#if false
				checkedListBoxUnits.SelectedIndex = nSelectedIndex;
				if (checkedListBoxUnits.SelectedIndex == -1 && checkedListBoxUnits.Items.Count != 0)
					checkedListBoxUnits.SelectedIndex = 0;
	#endif
				EnableUnitsButtons();
				if (nSelected != -1)
					listBoxUnits.SelectedIndex = nSelected;
			}
		}

		void EnableUnitsButtons() {
			int n = listBoxUnits.SelectedIndex;
			bool fItemSelected = (n >= 0);
			buttonNewUnits.Enabled = m_ugSelected != null;
			buttonDeleteUnits.Enabled = fItemSelected;
		}

		private void buttonNewUnits_Click(object sender, System.EventArgs e) {
			UnitTypeAndCountForm frm = new UnitTypeAndCountForm();
			if (frm.ShowDialog(this) == DialogResult.Cancel)
				return;
			UnitTypeAndCount[] autc = (UnitTypeAndCount[])m_ugSelected.UnitTypeAndCounts.ToArray(typeof(UnitTypeAndCount));
			bool fDuplicate = false;
			foreach (UnitTypeAndCount utc in autc) {
				if (utc.ut == frm.UnitType) {
					utc.c += frm.Count;
					fDuplicate = true;
					break;
				}
			}
			int n = -1;
			if (!fDuplicate)
				n = m_ugSelected.UnitTypeAndCounts.Add(new UnitTypeAndCount(frm.UnitType, frm.Count));
			m_ugm.SetModified();
			InitUnitsListBox(n);
		}

		private void buttonDeleteUnits_Click(object sender, System.EventArgs e) {
			int n = listBoxUnits.SelectedIndex;
			if (n >= 0) {
				m_ugSelected.UnitTypeAndCounts.RemoveAt(n);
				m_ugm.SetModified();
				InitUnitsListBox(-1);
			}
		}

		private void buttonAddUnit_Click(object sender, System.EventArgs e) {
			int n = listBoxUnits.SelectedIndex;
			if (n >= 0) {
				UnitTypeAndCount uct = (UnitTypeAndCount)m_ugSelected.UnitTypeAndCounts[n];
				uct.c++;
				m_ugm.SetModified();
				InitUnitsListBox(n);
			}
		}

		private void buttonSubtractUnit_Click(object sender, System.EventArgs e) {
			int n = listBoxUnits.SelectedIndex;
			if (n >= 0) {
				UnitTypeAndCount uct = (UnitTypeAndCount)m_ugSelected.UnitTypeAndCounts[n];
				if (uct.c > 1) {
					uct.c--;
					m_ugm.SetModified();
					InitUnitsListBox(n);
				}
			}
		}

		private void listBoxUnits_SelectedIndexChanged(object sender, System.EventArgs e) {
			EnableUnitsButtons();
		}

		private void checkBoxForever_CheckedChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null)
				m_ugSelected.LoopForever = checkBoxForever.Checked;
		}

		private void checkBoxCreateAtLevelLoad_CheckedChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null)
				m_ugSelected.CreateAtLevelLoad = checkBoxCreateAtLevelLoad.Checked;
		}

		private void checkBoxReplaceDestroyedGroup_CheckedChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null)
				m_ugSelected.ReplaceDestroyedGroup = checkBoxReplaceDestroyedGroup.Checked;
		}

		private void checkBoxRandomGroup_CheckedChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null)
				m_ugSelected.RandomGroup = checkBoxRandomGroup.Checked;
		}

		private void checkBoxSpawn_CheckedChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null)
				m_ugSelected.Spawn = checkBoxSpawn.Checked;
			comboBoxSpawnArea.Enabled = checkBoxSpawn.Checked;
		}

		private void textBoxHealth_TextChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null) {
				try {
					m_ugSelected.Health = int.Parse(textBoxHealth.Text);
				} catch {
					MessageBox.Show("Invalid Health value. Must be in the range from 0 to 100.", "M");
					textBoxHealth.Text = m_ugSelected.Health.ToString();
				}
			}
		}

		// Side combobox management

		void InitSideComboBox() {
			comboBoxSide.Items.Clear();
			comboBoxSide.Items.Add(Helper.GetDisplayName(typeof(Side), "side1"));
			comboBoxSide.Items.Add(Helper.GetDisplayName(typeof(Side), "side2"));
			comboBoxSide.Items.Add(Helper.GetDisplayName(typeof(Side), "side3"));
			comboBoxSide.Items.Add(Helper.GetDisplayName(typeof(Side), "side4"));

			if (m_ugSelected != null)
				comboBoxSide.SelectedIndex = (int)m_ugSelected.Side - 1;
		}

		private void comboBoxSide_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null)
				m_ugSelected.Side = (Side)(comboBoxSide.SelectedIndex + 1);
		}

		// Aggressiveness combobox management

		void InitAggressivenessComboBox() {
			comboBoxAggressiveness.Items.Clear();
			string[] astr = Enum.GetNames(typeof(Aggressiveness));
			foreach (string str in astr)
				comboBoxAggressiveness.Items.Add(str);

			if (m_ugSelected != null)
				comboBoxAggressiveness.SelectedIndex = (int)m_ugSelected.Aggressiveness;
		}

		private void comboBoxAggressiveness_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null)
				m_ugSelected.Aggressiveness = (Aggressiveness)comboBoxAggressiveness.SelectedIndex;
		}

		// Spawn Area combobox management

		void InitSpawnAreaComboBox() {
			comboBoxSpawnArea.Items.Clear();
			StringCollection strc = CaTypeArea.GetAreaNames();
			for (int i = CaTypeArea.VirtualAreaCount; i < strc.Count; i++)
				comboBoxSpawnArea.Items.Add(strc[i]);

			if (m_ugSelected != null) {
				int i = -1;
				if (m_ugSelected.SpawnArea != null)
					i = comboBoxSpawnArea.Items.IndexOf(m_ugSelected.SpawnArea);
				if (i == -1)
					comboBoxSpawnArea.ResetText();
				else
					comboBoxSpawnArea.SelectedIndex = i;
			}
		}

		private void comboBoxSpawnArea_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (m_ugSelected != null)
				m_ugSelected.SpawnArea = (string)comboBoxSpawnArea.SelectedItem;
		}

		private void buttonClose_Click(object sender, System.EventArgs e) {
			Close();
		}

		private void UnitGroupsForm_Closed(object sender, System.EventArgs e) {
			if (m_ugm.IsModified())
				m_lvld.SetModified(true);
		}
	}
}
