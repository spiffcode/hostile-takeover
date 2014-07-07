using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for PreviewPanel.
	/// </summary>
	public class PreviewPanel : System.Windows.Forms.UserControl
	{
		private bool m_fLoop = false;
		private System.Windows.Forms.NumericUpDown nudHoldCount;
		private SpiffCode.PreviewControl ctlPreview;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.CheckBox ckbSpecialPoint;
		private System.Windows.Forms.Button btnStop;
		private System.Windows.Forms.Button btnPlay;
		private System.Windows.Forms.CheckBox ckbTogglePlay;
		private System.Windows.Forms.Timer tmrAnim;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.TrackBar trkbScale;
		private System.ComponentModel.IContainer components;

		public PreviewPanel(AnimDoc doc)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			Globals.ActiveStripChanged += new EventHandler(OnActiveStripChanged);
			Globals.ActiveFrameChanged += new EventHandler(OnActiveFrameChanged);
			trkbScale.Value = Globals.PreviewScale - 1;
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

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(PreviewPanel));
			this.nudHoldCount = new System.Windows.Forms.NumericUpDown();
			this.ctlPreview = new SpiffCode.PreviewControl();
			this.panel1 = new System.Windows.Forms.Panel();
			this.trkbScale = new System.Windows.Forms.TrackBar();
			this.ckbSpecialPoint = new System.Windows.Forms.CheckBox();
			this.btnStop = new System.Windows.Forms.Button();
			this.btnPlay = new System.Windows.Forms.Button();
			this.ckbTogglePlay = new System.Windows.Forms.CheckBox();
			this.tmrAnim = new System.Windows.Forms.Timer(this.components);
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			((System.ComponentModel.ISupportInitialize)(this.nudHoldCount)).BeginInit();
			this.panel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.trkbScale)).BeginInit();
			this.SuspendLayout();
			// 
			// nudHoldCount
			// 
			this.nudHoldCount.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.nudHoldCount.BackColor = System.Drawing.SystemColors.Control;
			this.nudHoldCount.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.nudHoldCount.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.nudHoldCount.InterceptArrowKeys = false;
			this.nudHoldCount.Location = new System.Drawing.Point(0, 248);
			this.nudHoldCount.Name = "nudHoldCount";
			this.nudHoldCount.ReadOnly = true;
			this.nudHoldCount.Size = new System.Drawing.Size(32, 15);
			this.nudHoldCount.TabIndex = 2;
			this.nudHoldCount.TabStop = false;
			this.nudHoldCount.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.toolTip1.SetToolTip(this.nudHoldCount, "Hold Count");
			this.nudHoldCount.ValueChanged += new System.EventHandler(this.nudHoldCount_ValueChanged);
			// 
			// ctlPreview
			// 
			this.ctlPreview.AllowDrop = true;
			this.ctlPreview.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ctlPreview.Mode = SpiffCode.PreviewControlMode.RepositionBitmap;
			this.ctlPreview.Name = "ctlPreview";
			this.ctlPreview.Size = new System.Drawing.Size(264, 264);
			this.ctlPreview.TabIndex = 3;
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.SystemColors.Control;
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.trkbScale,
																				 this.ckbSpecialPoint,
																				 this.nudHoldCount});
			this.panel1.Dock = System.Windows.Forms.DockStyle.Right;
			this.panel1.Location = new System.Drawing.Point(264, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(32, 264);
			this.panel1.TabIndex = 4;
			// 
			// trkbScale
			// 
			this.trkbScale.Location = new System.Drawing.Point(0, 27);
			this.trkbScale.Maximum = 20;
			this.trkbScale.Name = "trkbScale";
			this.trkbScale.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.trkbScale.Size = new System.Drawing.Size(45, 85);
			this.trkbScale.TabIndex = 4;
			this.trkbScale.TickFrequency = 2;
			this.trkbScale.TickStyle = System.Windows.Forms.TickStyle.TopLeft;
			this.toolTip1.SetToolTip(this.trkbScale, "Zoom Level");
			this.trkbScale.Value = 1;
			this.trkbScale.Scroll += new System.EventHandler(this.trkbScale_Scroll);
			// 
			// ckbSpecialPoint
			// 
			this.ckbSpecialPoint.Appearance = System.Windows.Forms.Appearance.Button;
			this.ckbSpecialPoint.Image = ((System.Drawing.Bitmap)(resources.GetObject("ckbSpecialPoint.Image")));
			this.ckbSpecialPoint.Location = new System.Drawing.Point(4, 1);
			this.ckbSpecialPoint.Name = "ckbSpecialPoint";
			this.ckbSpecialPoint.Size = new System.Drawing.Size(24, 24);
			this.ckbSpecialPoint.TabIndex = 3;
			this.toolTip1.SetToolTip(this.ckbSpecialPoint, "Special Point Tool");
			this.ckbSpecialPoint.CheckedChanged += new System.EventHandler(this.ckbSpecialPoint_CheckedChanged);
			// 
			// btnStop
			// 
			this.btnStop.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.btnStop.Image = ((System.Drawing.Bitmap)(resources.GetObject("btnStop.Image")));
			this.btnStop.Location = new System.Drawing.Point(192, 240);
			this.btnStop.Name = "btnStop";
			this.btnStop.Size = new System.Drawing.Size(24, 24);
			this.btnStop.TabIndex = 11;
			this.toolTip1.SetToolTip(this.btnStop, "Stop animation");
			this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
			// 
			// btnPlay
			// 
			this.btnPlay.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.btnPlay.Image = ((System.Drawing.Bitmap)(resources.GetObject("btnPlay.Image")));
			this.btnPlay.Location = new System.Drawing.Point(216, 240);
			this.btnPlay.Name = "btnPlay";
			this.btnPlay.Size = new System.Drawing.Size(24, 24);
			this.btnPlay.TabIndex = 10;
			this.toolTip1.SetToolTip(this.btnPlay, "Play animation (once through)");
			this.btnPlay.Click += new System.EventHandler(this.btnPlay_Click);
			// 
			// ckbTogglePlay
			// 
			this.ckbTogglePlay.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.ckbTogglePlay.Appearance = System.Windows.Forms.Appearance.Button;
			this.ckbTogglePlay.Image = ((System.Drawing.Bitmap)(resources.GetObject("ckbTogglePlay.Image")));
			this.ckbTogglePlay.Location = new System.Drawing.Point(240, 240);
			this.ckbTogglePlay.Name = "ckbTogglePlay";
			this.ckbTogglePlay.Size = new System.Drawing.Size(24, 24);
			this.ckbTogglePlay.TabIndex = 9;
			this.toolTip1.SetToolTip(this.ckbTogglePlay, "Play animation (loop forever)");
			this.ckbTogglePlay.Click += new System.EventHandler(this.ckbTogglePlay_CheckedChanged);
			// 
			// tmrAnim
			// 
			this.tmrAnim.Interval = 80;
			this.tmrAnim.Tick += new System.EventHandler(this.tmrAnim_Tick);
			// 
			// PreviewPanel
			// 
			this.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(192)), ((System.Byte)(192)), ((System.Byte)(255)));
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.btnStop,
																		  this.btnPlay,
																		  this.ckbTogglePlay,
																		  this.ctlPreview,
																		  this.panel1});
			this.Name = "PreviewPanel";
			this.Size = new System.Drawing.Size(296, 264);
			((System.ComponentModel.ISupportInitialize)(this.nudHoldCount)).EndInit();
			this.panel1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.trkbScale)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		public Control PreviewControl {
			get {
				return ctlPreview;
			}
		}

		private void OnActiveStripChanged(object obSender, EventArgs e) {
			if (Globals.ActiveStrip != null)
				tmrAnim.Interval = 80 * (Globals.ActiveStrip.DefHoldCount + 1);
		}

		private void OnActiveFrameChanged(object obSender, EventArgs e) {
			if (Globals.ActiveStrip != null && Globals.ActiveStrip.Count != 0)
				nudHoldCount.Value = Globals.ActiveStrip[Globals.ActiveFrame].HoldCount;
		}

		private void nudHoldCount_ValueChanged(object sender, System.EventArgs e) {
			if (Globals.ActiveStrip == null)
				return;

			Globals.ActiveStrip[Globals.ActiveFrame].HoldCount = (int)nudHoldCount.Value;

			// UNDONE: this is a hack. Determine the best way to do this kind 
			// of thing.

			Globals.StripControl.Invalidate();
		}

		private void ckbSpecialPoint_CheckedChanged(object sender, System.EventArgs e) {
			if (ckbSpecialPoint.Checked)
				ctlPreview.Mode = PreviewControlMode.SetSpecialPoint;
			else
				ctlPreview.Mode = PreviewControlMode.RepositionBitmap;
			ctlPreview.Invalidate();
		}

		private void ckbTogglePlay_CheckedChanged(object sender, System.EventArgs e) {
			m_fLoop = !m_fLoop;
			if (m_fLoop)
				tmrAnim.Start();
			else
				tmrAnim.Stop();
		}

		private void btnPlay_Click(object sender, System.EventArgs e) {
			Globals.ActiveStrip.ActiveFrame = 0;
			m_fLoop = false;
			ckbTogglePlay.Checked = false;
			tmrAnim.Start();
		}

		private void btnStop_Click(object sender, System.EventArgs e) {
			tmrAnim.Stop();
			ckbTogglePlay.Checked = false;
			m_fLoop = false;
		}

		private void tmrAnim_Tick(object sender, System.EventArgs e) {
			Strip stp = Globals.ActiveStrip;
			if (stp == null)
				return;

			int ifr = stp.ActiveFrame + 1;
			if (ifr >= stp.Count) {
				ifr = 0;
				if (!m_fLoop) {
					tmrAnim.Stop();
				}
			}
			tmrAnim.Interval = (stp[ifr].HoldCount * 80) + (80 * (Globals.ActiveStrip.DefHoldCount + 1));
			stp.ActiveFrame = ifr;
		}

		private void trkbScale_Scroll(object sender, System.EventArgs e) {
			Globals.PreviewScale = trkbScale.Value + 1;
			ctlPreview.Invalidate();
		}
	}
}
