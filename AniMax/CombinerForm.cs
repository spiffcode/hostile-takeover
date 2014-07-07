using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for CombinerForm.
	/// </summary>
	public class CombinerForm : System.Windows.Forms.Form
	{
		private Point m_ptOffset;
		private FrameControl[] m_afrc = new FrameControl[20];
		private Label[] m_albl = new Label[20];
		private Strip[] m_astp = new Strip[20];
		private int[] m_aifr = new int[20];
		private int m_iSelected = 0;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private SpiffCode.FrameControl frc2;
		private SpiffCode.FrameControl frc1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public CombinerForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_afrc[0] = frc1;
			m_afrc[1] = frc2;
			m_albl[0] = label1;
			m_albl[1] = label2;

			Globals.ActiveDocumentChanged += new EventHandler(OnInvalidatingChange);
			Globals.ActiveFrameChanged += new EventHandler(OnInvalidatingChange);
			Globals.PreviewScaleChanged += new EventHandler(OnInvalidatingChange);
			Globals.GridChanged += new EventHandler(OnInvalidatingChange);
			Globals.SideColorMappingOnChanged += new EventHandler(OnInvalidatingChange);
			Globals.ShowOriginPointChanged += new EventHandler(OnInvalidatingChange);
			Globals.ShowSpecialPointChanged += new EventHandler(OnInvalidatingChange);
			((StripControl)Globals.StripControl).FrameOffsetChanged += 
					new FrameOffsetEventHandler(OnFrameOffsetChanged);
			Globals.FrameContentChanged += new EventHandler(OnInvalidatingChange);
			m_ptOffset = new Point(0, 0);
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
			this.label2 = new System.Windows.Forms.Label();
			this.frc2 = new SpiffCode.FrameControl();
			this.frc1 = new SpiffCode.FrameControl();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 80);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(80, 24);
			this.label1.TabIndex = 2;
			this.label1.Text = "<empty>";
			this.label1.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(96, 80);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(80, 24);
			this.label2.TabIndex = 3;
			this.label2.Text = "<empty>";
			this.label2.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// frc2
			// 
			this.frc2.AllowDrop = true;
			this.frc2.BorderColor = System.Drawing.Color.Black;
			this.frc2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.frc2.Frame = null;
			this.frc2.FrameIndex = 0;
			this.frc2.Location = new System.Drawing.Point(96, 8);
			this.frc2.Name = "frc2";
			this.frc2.OffsetPoint = new System.Drawing.Point(0, 0);
			this.frc2.Size = new System.Drawing.Size(80, 72);
			this.frc2.Strip = null;
			this.frc2.TabIndex = 4;
			this.frc2.Text = "frameControl1";
			this.frc2.MouseDown += new System.Windows.Forms.MouseEventHandler(this.frc2_MouseDown);
			// 
			// frc1
			// 
			this.frc1.AllowDrop = true;
			this.frc1.BorderColor = System.Drawing.Color.Black;
			this.frc1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.frc1.Frame = null;
			this.frc1.FrameIndex = 0;
			this.frc1.Location = new System.Drawing.Point(8, 8);
			this.frc1.Name = "frc1";
			this.frc1.OffsetPoint = new System.Drawing.Point(0, 0);
			this.frc1.Size = new System.Drawing.Size(80, 72);
			this.frc1.Strip = null;
			this.frc1.TabIndex = 5;
			this.frc1.Text = "frameControl1";
			this.frc1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.frc1_MouseDown);
			// 
			// CombinerForm
			// 
			this.AutoScale = false;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.BackColor = System.Drawing.Color.FromArgb(((System.Byte)(172)), ((System.Byte)(235)), ((System.Byte)(172)));
			this.ClientSize = new System.Drawing.Size(280, 302);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.frc1,
																		  this.frc2,
																		  this.label2,
																		  this.label1});
			this.Name = "CombinerForm";
			this.Text = "Combiner";
			this.ResumeLayout(false);

		}
		#endregion

		private void OnInvalidatingChange(object obSender, EventArgs e) {
			SelectStrip(m_iSelected, Globals.ActiveStrip, Globals.ActiveFrame);
			if (Globals.ActiveStrip != null && Globals.ActiveStrip.Count != 0)
				m_afrc[m_iSelected].Frame = Globals.ActiveStrip[Globals.ActiveFrame];
		}

		public void OnFrameOffsetChanged(object obSender, FrameOffsetEventArgs e) {
			m_ptOffset.X = e.X;
			m_ptOffset.Y = e.Y;
			frc1.OffsetPoint = m_ptOffset;
			frc2.OffsetPoint = m_ptOffset;
			Invalidate();
		}

		private void RefreshView() {
			Invalidate();
		}

		override protected void OnResize(EventArgs e) {
			Invalidate();
			base.OnResize(e);
		}

		protected override void OnPaintBackground(System.Windows.Forms.PaintEventArgs e) {
		}

		protected override void OnPaint(System.Windows.Forms.PaintEventArgs e) {
			if (Globals.ActiveStrip == null || Globals.ActiveStrip.Count == 0) {
				e.Graphics.FillRectangle(new SolidBrush(BackColor), e.ClipRectangle);
				return;
			}

			Frame.DrawArgs drwa = new Frame.DrawArgs();
			drwa.clrBackground = BackColor;
			drwa.fDrawBackground = true;
			drwa.fMapSideColors = Globals.SideColorMappingOn;
			drwa.fShowGrid = Globals.GridOn;
			drwa.cxGrid = Globals.GridWidth;
			drwa.cyGrid = Globals.GridHeight;
			drwa.fShowOrigin = Globals.ShowOriginPoint;
			drwa.fShowSpecialPoint = Globals.ShowSpecialPoint;
			drwa.nScale = Globals.PreviewScale;

			Rectangle rcClient = new Rectangle(0, 0, ClientRectangle.Width, ClientRectangle.Height);

			Graphics g = e.Graphics;

			int xCenter = rcClient.Width / 2;
			int yCenter = rcClient.Height / 2;

			int cxT = ((rcClient.Width + drwa.nScale - 1) / drwa.nScale) + 2;
			int cyT = ((rcClient.Height + drwa.nScale - 1) / drwa.nScale) + 2;
			int xCenterT = cxT / 2;
			int yCenterT = cyT / 2;

			// Create a temporary bitmap for compositing the grid, frames, origin indicator, etc into

			using (Bitmap bmT = new Bitmap(cxT, cyT)) {
				Point ptSpecial = new Point(0, 0);

				for (int i = 0; i < m_astp.Length; i++) {

					// Draw the frame and its indicators (grid, center point, special point, etc)

					Strip stp = m_astp[i];
					if (stp == null)
						continue;

					Frame fr = stp[m_aifr[i]];
					Point ptOffset = new Point(m_ptOffset.X + ptSpecial.X, m_ptOffset.Y + ptSpecial.Y);
					fr.DrawUnscaled(bmT, cxT, cyT, drwa, ptOffset);
					
					drwa.fDrawBackground = false;
					drwa.fShowGrid = false;

					ptSpecial = fr.SpecialPoint;
				}

				// Force a nice simple fast old-school stretchblt

				InterpolationMode imOld = g.InterpolationMode;
				g.InterpolationMode = InterpolationMode.NearestNeighbor;

				// NOTE: _without_ this the first row and column are only scaled by half!

				PixelOffsetMode pomOld = g.PixelOffsetMode;
				g.PixelOffsetMode = PixelOffsetMode.Half;

				// StretchBlt the temporary composite to the passed-in Graphic
				
				g.DrawImage(bmT, rcClient.Left - ((xCenterT * drwa.nScale) - xCenter), 
						rcClient.Top - ((yCenterT * drwa.nScale) - yCenter), 
						cxT * drwa.nScale, cyT * drwa.nScale);

				// Restore the Graphics' state

				g.PixelOffsetMode = pomOld;
				g.InterpolationMode = imOld;
			}
		}

		private void SelectStrip(int i, Strip stp, int ifr) {
			// Deselect any selected strip's FrameControl

			for (int j = 0; j < m_astp.Length; j++) {
				if (m_afrc[j] != null)
					m_afrc[j].BorderColor = Color.Black;
			}

			m_iSelected = i;
			m_afrc[m_iSelected].BorderColor = Color.Red;

			m_astp[m_iSelected] = stp;
			m_aifr[m_iSelected] = stp == null ? 0 : stp.ActiveFrame;
			m_afrc[m_iSelected].Frame = stp == null ? null : stp.Count == 0 ? null : stp[stp.ActiveFrame];
			m_albl[m_iSelected].Text = stp == null ? "<empty>" : stp.Name;
			Invalidate();
		}

		private void frc1_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
			SelectStrip(0, m_astp[0], m_aifr[0]);
			Globals.ActiveStrip = m_astp[m_iSelected];
			Globals.ActiveFrame = m_aifr[m_iSelected];
		}

		private void frc2_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
			SelectStrip(1, m_astp[1], m_aifr[1]);
			Globals.ActiveStrip = m_astp[m_iSelected];
			Globals.ActiveFrame = m_aifr[m_iSelected];
		}
	}
}
