using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for WallPreviewForm.
	/// </summary>
	public class WallPreviewForm : System.Windows.Forms.Form
	{
		private const int kctxMap = 64;
		private const int kctyMap = 64;
		private bool m_fMouseDown = false;
		private bool m_fSet;

		private bool[,] m_mpfWall = new bool[kctxMap, kctyMap];

		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public WallPreviewForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			m_mpfWall[5, 5] = true;
			m_mpfWall[6, 5] = true;
			m_mpfWall[6, 6] = true;
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
			// 
			// WallPreviewForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(368, 334);
			this.Name = "WallPreviewForm";
			this.Text = "Wall Preview";

		}
		#endregion

		protected override void OnPaint(System.Windows.Forms.PaintEventArgs e) {
			Graphics g = e.Graphics;
			int nScale = Globals.PreviewScale;
			Bitmap bmDst = new Bitmap(Width, Height);
			Graphics gDst = Graphics.FromImage(bmDst);

			// Draw background (if enabled)

//			if (drwa.fDrawBackground)
			gDst.Clear(BackColor);

			// Draw grid (if enabled)

			if (true) {
				Brush br = new SolidBrush(Color.FromArgb(256 / 3, 255, 255, 255));
				for (int x = 0; x < bmDst.Width; x += Globals.GridWidth)
					gDst.FillRectangle(br, x, 0, 1, bmDst.Height);

				for (int y = 0; y < bmDst.Height; y += Globals.GridHeight)
					gDst.FillRectangle(br, 0, y, bmDst.Width, 1);
			}

			BitmapData bmdDst = bmDst.LockBits(new Rectangle(0, 0, bmDst.Width, bmDst.Height), 
					ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);

			for (int ty = 0; ty < Height / Globals.GridHeight; ty++) {
				for (int tx = 0; tx < Width / Globals.GridWidth; tx++) {
					if (m_mpfWall[tx, ty]) {
						int ifrm = 0;
						if (ty > 0)
							ifrm |= m_mpfWall[tx, ty - 1] ? 1 : 0;
						if (tx < kctxMap - 1)
							ifrm |= m_mpfWall[tx + 1, ty] ? 2 : 0;
						if (ty < kctyMap - 1)
							ifrm |= m_mpfWall[tx, ty + 1] ? 4 : 0;
						if (tx > 0)
							ifrm |= m_mpfWall[tx - 1, ty] ? 8 : 0;

						XBitmap xbm = Globals.ActiveStrip[ifrm].BitmapPlacers[0].XBitmap;
						xbm.SuperBlt(0, 0, bmdDst,
                            tx * Globals.GridWidth, ty * Globals.GridHeight,
                            xbm.Bitmap.Width, xbm.Bitmap.Height,
                            Globals.SideColorMappingOn);
					}
				}
			}

			bmDst.UnlockBits(bmdDst);

			// Force a nice simple fast old-school stretchblt

			InterpolationMode imOld = g.InterpolationMode;
			g.InterpolationMode = InterpolationMode.NearestNeighbor;

			// NOTE: _without_ this the first row and column are only scaled by half!

			PixelOffsetMode pomOld = g.PixelOffsetMode;
			g.PixelOffsetMode = PixelOffsetMode.Half;

			// StretchBlt the temporary composite to the passed-in Graphic
				
			g.DrawImage(bmDst, 0, 0, bmDst.Width * nScale, bmDst.Height * nScale);

			g.PixelOffsetMode = pomOld;
			g.InterpolationMode = imOld;

//			e.Graphics.DrawImage(bmDst, 0, 0, Width, Height);
			bmDst.Dispose();
			gDst.Dispose();
		}

		protected override void OnMouseDown(System.Windows.Forms.MouseEventArgs e) {
			int nScale = Globals.PreviewScale;
			m_fMouseDown = true;

			int tx = e.X / (Globals.GridWidth * nScale);
			int ty = e.Y / (Globals.GridHeight * nScale);
			m_fSet = !m_mpfWall[tx, ty];
			m_mpfWall[tx, ty] = m_fSet;
			Invalidate();
		}

		protected override void OnMouseMove(System.Windows.Forms.MouseEventArgs e) {
			if (!m_fMouseDown)
				return;
			int nScale = Globals.PreviewScale;

			int tx = e.X / (Globals.GridWidth * nScale);
			int ty = e.Y / (Globals.GridHeight * nScale);
			m_mpfWall[tx, ty] = m_fSet;
			Invalidate();
		}

		protected override void OnPaintBackground(System.Windows.Forms.PaintEventArgs pevent) {
		}

		protected override void OnMouseUp(System.Windows.Forms.MouseEventArgs e) {
			m_fMouseDown = false;
		}

	}
}
