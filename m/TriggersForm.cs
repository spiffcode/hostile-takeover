using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace m
{
	/// <summary>
	/// Summary description for TriggersForm.
	/// </summary>
	public class TriggersForm : System.Windows.Forms.Form
	{
		static Rectangle s_rcBounds = new Rectangle();
		TriggerManager m_tgrm;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Button buttonClose;
		private System.Windows.Forms.Button buttonNewTrigger;
		private System.Windows.Forms.Button buttonModifyTrigger;
		private System.Windows.Forms.Button buttonLoadTriggers;
		private System.Windows.Forms.Button buttonSaveTriggers;
		private System.Windows.Forms.Button buttonCopyTrigger;
		private System.Windows.Forms.Button buttonDeleteTrigger;
		private System.Windows.Forms.Button buttonMoveUpTrigger;
		private System.Windows.Forms.Button buttonMoveDownTrigger;
		private System.Windows.Forms.ListBox listBoxSides;
		private System.Windows.Forms.ListBox listBoxTriggers;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public TriggersForm(TriggerManager tgrm)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			if (!s_rcBounds.IsEmpty)
				SetBounds(s_rcBounds.Left, s_rcBounds.Top, s_rcBounds.Width, s_rcBounds.Height, BoundsSpecified.All);
			else {
				Rectangle rcScreen = Screen.GetWorkingArea(this);
				SetBounds((rcScreen.Width - Bounds.Width) / 2, (rcScreen.Height - Bounds.Height) / 2, 0, 0, BoundsSpecified.Location);
			}

			m_tgrm = tgrm;
			InitSidesListBox(null);
			InitTriggersListBox(null);
			EnableButtons();
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
			this.listBoxSides = new System.Windows.Forms.ListBox();
			this.label1 = new System.Windows.Forms.Label();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.buttonMoveUpTrigger = new System.Windows.Forms.Button();
			this.buttonMoveDownTrigger = new System.Windows.Forms.Button();
			this.buttonNewTrigger = new System.Windows.Forms.Button();
			this.listBoxTriggers = new System.Windows.Forms.ListBox();
			this.buttonModifyTrigger = new System.Windows.Forms.Button();
			this.buttonCopyTrigger = new System.Windows.Forms.Button();
			this.buttonDeleteTrigger = new System.Windows.Forms.Button();
			this.buttonLoadTriggers = new System.Windows.Forms.Button();
			this.buttonSaveTriggers = new System.Windows.Forms.Button();
			this.buttonClose = new System.Windows.Forms.Button();
			this.groupBox1.SuspendLayout();
			this.SuspendLayout();
			// 
			// listBoxSides
			// 
			this.listBoxSides.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.listBoxSides.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.listBoxSides.ItemHeight = 16;
			this.listBoxSides.Location = new System.Drawing.Point(16, 24);
			this.listBoxSides.Name = "listBoxSides";
			this.listBoxSides.Size = new System.Drawing.Size(528, 68);
			this.listBoxSides.TabIndex = 0;
			this.listBoxSides.SelectedIndexChanged += new System.EventHandler(this.listBoxSides_SelectedIndexChanged);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(16, 7);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(336, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "Sides with triggers:";
			// 
			// groupBox1
			// 
			this.groupBox1.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.groupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
																					this.buttonMoveUpTrigger,
																					this.buttonMoveDownTrigger,
																					this.buttonNewTrigger,
																					this.listBoxTriggers,
																					this.buttonModifyTrigger,
																					this.buttonCopyTrigger,
																					this.buttonDeleteTrigger,
																					this.buttonLoadTriggers,
																					this.buttonSaveTriggers});
			this.groupBox1.Location = new System.Drawing.Point(8, 104);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(672, 528);
			this.groupBox1.TabIndex = 2;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Triggers";
			// 
			// buttonMoveUpTrigger
			// 
			this.buttonMoveUpTrigger.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
			this.buttonMoveUpTrigger.Location = new System.Drawing.Point(8, 494);
			this.buttonMoveUpTrigger.Name = "buttonMoveUpTrigger";
			this.buttonMoveUpTrigger.Size = new System.Drawing.Size(184, 23);
			this.buttonMoveUpTrigger.TabIndex = 8;
			this.buttonMoveUpTrigger.Text = "Move &Up";
			this.buttonMoveUpTrigger.Click += new System.EventHandler(this.buttonMoveUpTrigger_Click);
			// 
			// buttonMoveDownTrigger
			// 
			this.buttonMoveDownTrigger.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
			this.buttonMoveDownTrigger.Location = new System.Drawing.Point(200, 494);
			this.buttonMoveDownTrigger.Name = "buttonMoveDownTrigger";
			this.buttonMoveDownTrigger.Size = new System.Drawing.Size(184, 23);
			this.buttonMoveDownTrigger.TabIndex = 7;
			this.buttonMoveDownTrigger.Text = "Move D&own";
			this.buttonMoveDownTrigger.Click += new System.EventHandler(this.buttonMoveDownTrigger_Click);
			// 
			// buttonNewTrigger
			// 
			this.buttonNewTrigger.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
			this.buttonNewTrigger.Location = new System.Drawing.Point(552, 16);
			this.buttonNewTrigger.Name = "buttonNewTrigger";
			this.buttonNewTrigger.Size = new System.Drawing.Size(104, 23);
			this.buttonNewTrigger.TabIndex = 1;
			this.buttonNewTrigger.Text = "&New...";
			this.buttonNewTrigger.Click += new System.EventHandler(this.buttonNewTrigger_Click);
			// 
			// listBoxTriggers
			// 
			this.listBoxTriggers.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.listBoxTriggers.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(113)), ((System.Byte)(111)), ((System.Byte)(100)));
			this.listBoxTriggers.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawVariable;
			this.listBoxTriggers.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.listBoxTriggers.IntegralHeight = false;
			this.listBoxTriggers.Location = new System.Drawing.Point(8, 16);
			this.listBoxTriggers.Name = "listBoxTriggers";
			this.listBoxTriggers.Size = new System.Drawing.Size(528, 472);
			this.listBoxTriggers.TabIndex = 0;
			this.listBoxTriggers.DoubleClick += new System.EventHandler(this.listBoxTriggers_DoubleClick);
			this.listBoxTriggers.MeasureItem += new System.Windows.Forms.MeasureItemEventHandler(this.listBoxTriggers_MeasureItem);
			this.listBoxTriggers.DrawItem += new System.Windows.Forms.DrawItemEventHandler(this.listBoxTriggers_DrawItem);
			this.listBoxTriggers.SelectedIndexChanged += new System.EventHandler(this.listBoxTriggers_SelectedIndexChanged);
			// 
			// buttonModifyTrigger
			// 
			this.buttonModifyTrigger.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
			this.buttonModifyTrigger.Location = new System.Drawing.Point(552, 48);
			this.buttonModifyTrigger.Name = "buttonModifyTrigger";
			this.buttonModifyTrigger.Size = new System.Drawing.Size(104, 23);
			this.buttonModifyTrigger.TabIndex = 1;
			this.buttonModifyTrigger.Text = "&Modify...";
			this.buttonModifyTrigger.Click += new System.EventHandler(this.buttonModifyTrigger_Click);
			// 
			// buttonCopyTrigger
			// 
			this.buttonCopyTrigger.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
			this.buttonCopyTrigger.Location = new System.Drawing.Point(552, 80);
			this.buttonCopyTrigger.Name = "buttonCopyTrigger";
			this.buttonCopyTrigger.Size = new System.Drawing.Size(104, 23);
			this.buttonCopyTrigger.TabIndex = 1;
			this.buttonCopyTrigger.Text = "&Copy";
			this.buttonCopyTrigger.Click += new System.EventHandler(this.buttonCopyTrigger_Click);
			// 
			// buttonDeleteTrigger
			// 
			this.buttonDeleteTrigger.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
			this.buttonDeleteTrigger.Location = new System.Drawing.Point(552, 112);
			this.buttonDeleteTrigger.Name = "buttonDeleteTrigger";
			this.buttonDeleteTrigger.Size = new System.Drawing.Size(104, 23);
			this.buttonDeleteTrigger.TabIndex = 1;
			this.buttonDeleteTrigger.Text = "&Delete";
			this.buttonDeleteTrigger.Click += new System.EventHandler(this.buttonDeleteTrigger_Click);
			// 
			// buttonLoadTriggers
			// 
			this.buttonLoadTriggers.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
			this.buttonLoadTriggers.Location = new System.Drawing.Point(552, 160);
			this.buttonLoadTriggers.Name = "buttonLoadTriggers";
			this.buttonLoadTriggers.Size = new System.Drawing.Size(104, 23);
			this.buttonLoadTriggers.TabIndex = 1;
			this.buttonLoadTriggers.Text = "&Load Triggers";
			this.buttonLoadTriggers.Click += new System.EventHandler(this.buttonLoadTriggers_Click);
			// 
			// buttonSaveTriggers
			// 
			this.buttonSaveTriggers.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
			this.buttonSaveTriggers.Location = new System.Drawing.Point(552, 192);
			this.buttonSaveTriggers.Name = "buttonSaveTriggers";
			this.buttonSaveTriggers.Size = new System.Drawing.Size(104, 23);
			this.buttonSaveTriggers.TabIndex = 1;
			this.buttonSaveTriggers.Text = "&Save Triggers";
			this.buttonSaveTriggers.Click += new System.EventHandler(this.buttonSaveTriggers_Click);
			// 
			// buttonClose
			// 
			this.buttonClose.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
			this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonClose.Location = new System.Drawing.Point(560, 24);
			this.buttonClose.Name = "buttonClose";
			this.buttonClose.Size = new System.Drawing.Size(104, 23);
			this.buttonClose.TabIndex = 3;
			this.buttonClose.Text = "Close";
			this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
			// 
			// TriggersForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.buttonClose;
			this.ClientSize = new System.Drawing.Size(688, 638);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.buttonClose,
																		  this.groupBox1,
																		  this.label1,
																		  this.listBoxSides});
			this.Name = "TriggersForm";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
			this.Text = "Triggers";
			this.groupBox1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private class NameSide {
			private string m_strName;
			public Side side;

			public NameSide(Side side) {
				this.side = side;
				m_strName = Helper.GetDisplayName(typeof(Side), side.ToString());
			}

			override public string ToString() {
				return m_strName;
			}
		}

		void InitSidesListBox(Trigger tgr) {
			Side[] aside = m_tgrm.GetTriggerSides();
			if (aside.Length == 0) {
				listBoxSides.Items.Clear();
				return;
			}
			int n = listBoxSides.SelectedIndex;
			Side sideSelected;
			if (n >= 0) {
				sideSelected = ((NameSide)listBoxSides.Items[n]).side;
//				sideSelected = (Side)Enum.Parse(typeof(Side), (string)listBoxSides.Items[n]);
			} else {
				sideSelected = aside[0];
			}
			listBoxSides.Items.Clear();
			foreach (Side side in aside)
				listBoxSides.Items.Add(new NameSide(side));
//				listBoxSides.Items.Add(side.ToString());

			if (tgr != null) {
				if ((tgr.Sides & m_tgrm.SideToMask(sideSelected)) == 0) {
					foreach (Side side in aside) {
						if ((m_tgrm.SideToMask(side) & tgr.Sides) != 0) {
							sideSelected = side;
							break;
						}
					}
				}
			}
			listBoxSides.SelectedIndex = Array.IndexOf(aside, sideSelected);
		}

		int GetSideMaskSelected() {
			int nSide = listBoxSides.SelectedIndex;
			if (nSide < 0)
				return 0;
			return m_tgrm.SideToMask(GetSideSelected());
		}

		Side GetSideSelected() {
			Side[] aside = m_tgrm.GetTriggerSides();
			return aside[listBoxSides.SelectedIndex];
		}

		void InitTriggersListBox(Trigger tgrSelect) {
			listBoxTriggers.Items.Clear();
			int nSide = listBoxSides.SelectedIndex;
			if (nSide < 0)
				return;
			Trigger[] atgr = m_tgrm.GetTriggerList(GetSideSelected());
			foreach (Trigger tgr in atgr)
				listBoxTriggers.Items.Add(tgr);
			if (tgrSelect == null) {
				if (listBoxTriggers.SelectedIndex == -1 && listBoxTriggers.Items.Count != 0)
					listBoxTriggers.SelectedIndex = 0;
			} else {
				listBoxTriggers.SelectedIndex = Array.IndexOf(atgr, tgrSelect);
			}
		}

		private void buttonClose_Click(object sender, System.EventArgs e) {
			DialogResult = DialogResult.OK;
		}

		private void buttonNewTrigger_Click(object sender, System.EventArgs e) {
			Trigger tgr = new Trigger();
			TriggerPropForm frm = new TriggerPropForm(tgr);
			if (frm.ShowDialog() == DialogResult.OK) {
				m_tgrm.AddTrigger(tgr);
				InitSidesListBox(tgr);
				InitTriggersListBox(tgr);
			}
		}

		private void buttonModifyTrigger_Click(object sender, System.EventArgs e) {
			int n = listBoxTriggers.SelectedIndex;
			if (n < 0)
				return;
			Trigger[] atgr = m_tgrm.GetTriggerList(GetSideSelected());
			Trigger tgr = atgr[n].Clone();
			TriggerPropForm frm = new TriggerPropForm(tgr);
			if (frm.ShowDialog() == DialogResult.OK) {
				m_tgrm.ModifyTrigger(atgr[n], tgr);
				InitSidesListBox(tgr);
				InitTriggersListBox(tgr);
			}
		}

		private void listBoxTriggers_MeasureItem(object sender, System.Windows.Forms.MeasureItemEventArgs e) {
			Trigger[] atgr = m_tgrm.GetTriggerList(GetSideSelected());
			int n = e.Index;
			Trigger tgr = atgr[n];
			int cy = (tgr.Actions.Count + tgr.Conditions.Count + 2) * listBoxTriggers.Font.Height + 4;
			// BUGBUG: listbox seems to have a problem with item heights > 255
			e.ItemHeight = Math.Min(cy, 255);
		}

		private void listBoxTriggers_DrawItem(object sender, System.Windows.Forms.DrawItemEventArgs e) {
			int n = e.Index;
			if (n == -1)
				return;

			Trigger[] atgr = m_tgrm.GetTriggerList(GetSideSelected());
			Trigger trg = atgr[n];
			Color clr = (e.State & DrawItemState.Selected) != 0 ? Color.FromArgb(178, 180, 191) : Color.White;
			e.Graphics.FillRectangle(new SolidBrush(clr), e.Bounds.Left + 2, e.Bounds.Top + 2, e.Bounds.Width - 4, e.Bounds.Height - 4);
			int y = e.Bounds.Top + 2;
			int x = e.Bounds.Left + 2;
			int cyFont = e.Font.Height;
			Brush br = new SolidBrush(e.ForeColor);
			e.Graphics.DrawString("CONDITIONS:", e.Font, br, x, y);
			y += cyFont;
			foreach (CaBase cab in trg.Conditions) {
				e.Graphics.DrawString(" - " + cab.ToString(), e.Font, br, x, y);
				y += cyFont;
			}
			e.Graphics.DrawString("ACTIONS:", e.Font, br, x, y);
			y += cyFont;
			foreach (CaBase cab in trg.Actions) {
				e.Graphics.DrawString(" - " + cab.ToString(), e.Font, br, x, y);
				y += cyFont;
			}
		}

		private void buttonCopyTrigger_Click(object sender, System.EventArgs e) {
			int n = listBoxTriggers.SelectedIndex;
			if (n < 0)
				return;
			Trigger[] atgr = m_tgrm.GetTriggerList(GetSideSelected());
			Trigger tgr = atgr[n].Clone();
			m_tgrm.AddTrigger(tgr);
			InitTriggersListBox(tgr);
		}

		private void buttonDeleteTrigger_Click(object sender, System.EventArgs e) {
			int n = listBoxTriggers.SelectedIndex;
			if (n < 0)
				return;
			Trigger[] atgr = m_tgrm.GetTriggerList(GetSideSelected());
			m_tgrm.RemoveTrigger(atgr[n]);
			InitSidesListBox(null);
			InitTriggersListBox(null);
		}

		private void buttonLoadTriggers_Click(object sender, System.EventArgs e) {
		
		}

		private void buttonSaveTriggers_Click(object sender, System.EventArgs e) {
		
		}

		private void buttonMoveUpTrigger_Click(object sender, System.EventArgs e) {
			int n = listBoxTriggers.SelectedIndex;
			if (n <= 0)
				return;
			Trigger[] atgr = m_tgrm.GetTriggerList(GetSideSelected());
			m_tgrm.MoveUpTrigger(GetSideSelected(), atgr[n]);
			InitTriggersListBox(atgr[n]);
		}

		private void buttonMoveDownTrigger_Click(object sender, System.EventArgs e) {
			int n = listBoxTriggers.SelectedIndex;
			if (n < 0 || n >= listBoxTriggers.Items.Count - 1)
				return;
			Trigger[] atgr = m_tgrm.GetTriggerList(GetSideSelected());
			m_tgrm.MoveDownTrigger(GetSideSelected(), atgr[n]);
			InitTriggersListBox(atgr[n]);
		}

		void EnableButtons() {
			int n = listBoxTriggers.SelectedIndex;
			bool fSelected = (n >= 0);
			buttonModifyTrigger.Enabled = fSelected;
			buttonCopyTrigger.Enabled = fSelected;
			buttonDeleteTrigger.Enabled = fSelected;
			buttonMoveUpTrigger.Enabled = (n > 0);
			buttonMoveDownTrigger.Enabled = (n < listBoxTriggers.Items.Count - 1);
			buttonLoadTriggers.Enabled = false; // (listBoxTriggers.Items.Count != 0);
			buttonSaveTriggers.Enabled = false; // (listBoxTriggers.Items.Count != 0);
		}

		private void listBoxTriggers_SelectedIndexChanged(object sender, System.EventArgs e) {
			EnableButtons();
		}

		private void listBoxSides_SelectedIndexChanged(object sender, System.EventArgs e) {
			InitTriggersListBox(null);
		}

		private void listBoxTriggers_DoubleClick(object sender, System.EventArgs e) {
			buttonModifyTrigger_Click(sender, e);
		}

		protected override void OnClosed(System.EventArgs e) {
			s_rcBounds = Bounds;
			base.OnClosed(e);
		}
	}
}
