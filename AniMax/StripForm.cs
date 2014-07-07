using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using Crownwood.Magic.Docking;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for StripForm.
	/// </summary>
	public class StripForm : System.Windows.Forms.Form
	{
		private Content m_tnt;
		private AnimDoc m_doc;
		private SpiffCode.StripControl stpc;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.CheckBox ckbToggleGrid;
		private System.Windows.Forms.CheckBox ckbToggleSideColor;
		private System.Windows.Forms.CheckBox ckbToggleOriginPoint;
		private System.Windows.Forms.CheckBox ckbToggleSpecialPoint;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.Button button1;
		private System.ComponentModel.IContainer components;

		public StripForm(AnimDoc doc)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			// My constructor code

			Globals.ActiveDocumentChanged += new EventHandler(OnActiveDocumentChanged);
			Globals.ActiveStripChanged += new EventHandler(OnActiveStripChanged);
			Globals.TileSizeChanged += new EventHandler(OnTileSizeChanged);
			Globals.StripControl = stpc;
			m_doc = doc;
			ckbToggleGrid.Checked = Globals.GridOn;
			ckbToggleSideColor.Checked = Globals.SideColorMappingOn;
			ckbToggleOriginPoint.Checked = Globals.ShowOriginPoint;
			ckbToggleSpecialPoint.Checked = Globals.ShowSpecialPoint;
			RefreshView();
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
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(StripForm));
			this.stpc = new SpiffCode.StripControl();
			this.panel1 = new System.Windows.Forms.Panel();
			this.ckbToggleSpecialPoint = new System.Windows.Forms.CheckBox();
			this.ckbToggleOriginPoint = new System.Windows.Forms.CheckBox();
			this.ckbToggleSideColor = new System.Windows.Forms.CheckBox();
			this.ckbToggleGrid = new System.Windows.Forms.CheckBox();
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			this.button1 = new System.Windows.Forms.Button();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// stpc
			// 
			this.stpc.AllowDrop = true;
			this.stpc.BackColor = System.Drawing.SystemColors.Control;
			this.stpc.Dock = System.Windows.Forms.DockStyle.Fill;
			this.stpc.DockPadding.Bottom = 1;
			this.stpc.DockPadding.Right = 1;
			this.stpc.Location = new System.Drawing.Point(0, 26);
			this.stpc.Name = "stpc";
			this.stpc.Size = new System.Drawing.Size(648, 100);
			this.stpc.Strip = null;
			this.stpc.TabIndex = 0;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.button1);
			this.panel1.Controls.Add(this.ckbToggleSpecialPoint);
			this.panel1.Controls.Add(this.ckbToggleOriginPoint);
			this.panel1.Controls.Add(this.ckbToggleSideColor);
			this.panel1.Controls.Add(this.ckbToggleGrid);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(648, 26);
			this.panel1.TabIndex = 1;
			// 
			// ckbToggleSpecialPoint
			// 
			this.ckbToggleSpecialPoint.Appearance = System.Windows.Forms.Appearance.Button;
			this.ckbToggleSpecialPoint.Image = ((System.Drawing.Image)(resources.GetObject("ckbToggleSpecialPoint.Image")));
			this.ckbToggleSpecialPoint.Location = new System.Drawing.Point(52, 0);
			this.ckbToggleSpecialPoint.Name = "ckbToggleSpecialPoint";
			this.ckbToggleSpecialPoint.Size = new System.Drawing.Size(24, 24);
			this.ckbToggleSpecialPoint.TabIndex = 9;
			this.toolTip1.SetToolTip(this.ckbToggleSpecialPoint, "Toggle Special Point on/off");
			this.ckbToggleSpecialPoint.CheckedChanged += new System.EventHandler(this.ckbToggleSpecialPoint_CheckedChanged);
			// 
			// ckbToggleOriginPoint
			// 
			this.ckbToggleOriginPoint.Appearance = System.Windows.Forms.Appearance.Button;
			this.ckbToggleOriginPoint.Image = ((System.Drawing.Image)(resources.GetObject("ckbToggleOriginPoint.Image")));
			this.ckbToggleOriginPoint.Location = new System.Drawing.Point(26, 0);
			this.ckbToggleOriginPoint.Name = "ckbToggleOriginPoint";
			this.ckbToggleOriginPoint.Size = new System.Drawing.Size(24, 24);
			this.ckbToggleOriginPoint.TabIndex = 4;
			this.toolTip1.SetToolTip(this.ckbToggleOriginPoint, "Toggle Origin Point on/off");
			this.ckbToggleOriginPoint.CheckedChanged += new System.EventHandler(this.ckbToggleOriginPoint_CheckedChanged);
			// 
			// ckbToggleSideColor
			// 
			this.ckbToggleSideColor.Appearance = System.Windows.Forms.Appearance.Button;
			this.ckbToggleSideColor.Image = ((System.Drawing.Image)(resources.GetObject("ckbToggleSideColor.Image")));
			this.ckbToggleSideColor.Location = new System.Drawing.Point(78, 0);
			this.ckbToggleSideColor.Name = "ckbToggleSideColor";
			this.ckbToggleSideColor.Size = new System.Drawing.Size(24, 24);
			this.ckbToggleSideColor.TabIndex = 1;
			this.toolTip1.SetToolTip(this.ckbToggleSideColor, "Toggle side color preview on/off");
			this.ckbToggleSideColor.CheckedChanged += new System.EventHandler(this.ckbToggleSideColor_CheckedChanged);
			// 
			// ckbToggleGrid
			// 
			this.ckbToggleGrid.Appearance = System.Windows.Forms.Appearance.Button;
			this.ckbToggleGrid.Image = ((System.Drawing.Image)(resources.GetObject("ckbToggleGrid.Image")));
			this.ckbToggleGrid.Location = new System.Drawing.Point(0, 0);
			this.ckbToggleGrid.Name = "ckbToggleGrid";
			this.ckbToggleGrid.Size = new System.Drawing.Size(24, 24);
			this.ckbToggleGrid.TabIndex = 0;
			this.toolTip1.SetToolTip(this.ckbToggleGrid, "Toggle Grid on/off");
			this.ckbToggleGrid.CheckedChanged += new System.EventHandler(this.ckbToggleGrid_CheckedChanged);
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(104, 0);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(27, 24);
			this.button1.TabIndex = 11;
			this.button1.Text = "TS";
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// StripForm
			// 
			this.AutoScale = false;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(648, 126);
			this.Controls.Add(this.stpc);
			this.Controls.Add(this.panel1);
			this.Name = "StripForm";
			this.Text = "Frames";
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		public Content Content {
			set {
				m_tnt = value;
			}
		}
		
		private void OnActiveDocumentChanged(object obSender, EventArgs e) {
			m_doc = Globals.ActiveDocument;
			RefreshView();
		}

		private void OnActiveStripChanged(object obSender, EventArgs e) {
			RefreshView();
		}

		private void RefreshView() {
			stpc.Strip = Globals.ActiveStrip;
			string strT = Globals.ActiveStrip == null ? "(none)" : Globals.ActiveStrip.Name;
			Text = "Frames - " + strT;
			if (m_tnt != null)
				m_tnt.Title = Text;
		}

		private void ckbToggleGrid_CheckedChanged(object sender, System.EventArgs e) {
			Globals.GridOn = ckbToggleGrid.Checked;
		}

		private void ckbToggleSideColor_CheckedChanged(object sender, System.EventArgs e) {
			Globals.SideColorMappingOn = ckbToggleSideColor.Checked;
		}

		private void ckbToggleOriginPoint_CheckedChanged(object sender, System.EventArgs e) {
			Globals.ShowOriginPoint = ckbToggleOriginPoint.Checked;
		}

		private void ckbToggleSpecialPoint_CheckedChanged(object sender, System.EventArgs e) {
			Globals.ShowSpecialPoint = ckbToggleSpecialPoint.Checked;
		}

#if false
		private void ckbToggleHires_CheckedChanged(object sender, System.EventArgs e) {
			bool fHires = ckbToggleHires.Checked;
			if (Globals.Hires != fHires)
				Globals.Hires = fHires;
			if (Globals.ActiveDocument.Hires != fHires)
				Globals.ActiveDocument.Hires = fHires;
		}
#endif

		private void OnTileSizeChanged(object obSender, EventArgs e) {
#if false
			ckbToggleHires.Checked = Globals.Hires;
#endif
		}

		private void btnProperties_Click(object sender, System.EventArgs e) {
			ShowStripProperties(stpc.Strip);
		}

		public void ShowStripProperties(Strip stp) {
			Form frm = new StripProperties(stp);
			if (frm.ShowDialog(this) == DialogResult.Cancel)
				return;

			// UNDONE: animation interval must be updated if Strip.DefHoldCount changes
//			if (stp == stpc.Strip)
//				tmrAnim.Interval = 80 + (80 * stpc.Strip.DefHoldCount);
		}

		private void button1_Click(object sender, System.EventArgs e) {
            int nTSNew = 32;
            switch (Globals.TileSize) {
            case 16:
                nTSNew = 24;
                break;
            case 24:
                nTSNew = 32;
                break;
            case 32:
                nTSNew = 16;
                break;
            }
            if (Globals.TileSize != nTSNew) {
                Globals.TileSize = nTSNew;
            }
            if (Globals.ActiveDocument.TileSize != nTSNew) {
                Globals.ActiveDocument.TileSize = nTSNew;
            }
		}
	}
}
