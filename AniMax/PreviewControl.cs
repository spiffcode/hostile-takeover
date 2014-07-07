using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for PreviewControl.
	/// </summary>
	public class PreviewControl : System.Windows.Forms.UserControl
	{
		private bool m_fFrameRecentering = false;
		private bool m_fSpaceDown = false;
		private bool m_fAllFrames;
		private bool m_fRepositionBitmaps = false;
		private BitmapPlacer m_plcSelected = null;
		private Point m_ptPreSetSpecialPoint;
		private bool m_fSetSpecialPoint = false;
		private PreviewControlMode m_mode = PreviewControlMode.RepositionBitmap;
#if false
		private bool m_fOnionSkin = true;
#endif
		private bool m_fDragging = false;
		private Point m_ptDragStart;
		private Point[] m_aptPreDragBitmap;
		private Point m_ptOffset;
		private Point m_ptInitialOffset;
		private System.Windows.Forms.Label label1;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public PreviewControl()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// My initialization

			ResizeRedraw = true;
			Globals.ActiveDocumentChanged += new EventHandler(OnActiveDocumentChanged);
			Globals.ActiveFrameChanged += new EventHandler(OnInvalidatingChange);
			Globals.GridChanged += new EventHandler(OnInvalidatingChange);
			Globals.SideColorMappingOnChanged += new EventHandler(OnInvalidatingChange);
			Globals.ShowOriginPointChanged += new EventHandler(OnInvalidatingChange);
			Globals.ShowSpecialPointChanged += new EventHandler(OnInvalidatingChange);
			Globals.FrameContentChanged += new EventHandler(OnInvalidatingChange);
			Globals.KeyDown += new KeyEventHandler(OnGlobalKeyDown);
			Globals.KeyUp += new KeyEventHandler(OnGlobalKeyUp);
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

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.label1 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(56, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "Preview";
			// 
			// PreviewControl
			// 
			this.AllowDrop = true;
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label1});
			this.Name = "PreviewControl";
			this.DragEnter += new System.Windows.Forms.DragEventHandler(this.PreviewControl_DragEnter);
			this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.PreviewControl_MouseUp);
			this.DragDrop += new System.Windows.Forms.DragEventHandler(this.PreviewControl_DragDrop);
			this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.PreviewControl_MouseMove);
			this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.PreviewControl_MouseDown);
			this.ResumeLayout(false);

		}
		#endregion

		public event FrameOffsetEventHandler FrameOffsetChanged;

		public PreviewControlMode Mode {
			set {
				m_mode = value;
			}
			get {
				return m_mode;
			}
		}

		private void OnGlobalKeyDown(object obSender, KeyEventArgs e) {
			if (e.KeyCode == Keys.Space) {
				Cursor = m_fFrameRecentering ? Globals.GrabCursor : Globals.HandCursor;
				m_fSpaceDown = true;
			}
		}
		
		private void OnGlobalKeyUp(object obSender, KeyEventArgs e) {
			if (e.KeyCode == Keys.Space) {
				if (!m_fFrameRecentering)
					Cursor = Cursors.Default;
				m_fSpaceDown = false;
			}
		}

		private void OnActiveDocumentChanged(object obSender, EventArgs e) {
			Invalidate();
		}

		public void OnFrameOffsetChanged(object obSender, FrameOffsetEventArgs e) {
			m_ptOffset.X = e.X;
			m_ptOffset.Y = e.Y;
			Invalidate();
		}

		private void OnInvalidatingChange(object obSender, EventArgs e) {
			Invalidate();
		}

		protected override void OnPaint(System.Windows.Forms.PaintEventArgs e) {
			Frame.DrawArgs drwa = new Frame.DrawArgs();
			drwa.clrBackground = BackColor;
			drwa.fDrawBackground = true;
			drwa.fMapSideColors = Globals.SideColorMappingOn;
			drwa.fShowGrid = Globals.GridOn;
			drwa.cxGrid = Globals.GridWidth;
			drwa.cyGrid = Globals.GridHeight;
			drwa.fShowOrigin = Globals.ShowOriginPoint;
			drwa.fShowSpecialPoint = Globals.ShowSpecialPoint || (m_mode == PreviewControlMode.SetSpecialPoint);
			drwa.nScale = Globals.PreviewScale;

			if (Globals.ActiveStrip != null && Globals.ActiveStrip.Count != 0) {
				Frame fr;
				Rectangle rc = new Rectangle(0, 0, ClientRectangle.Width, ClientRectangle.Height);

#if false
				if (m_fOnionSkin && Globals.ActiveFrame > 0) {
					drwa.fShowGrid = false;
					drwa.fShowOrigin = false;
					drwa.fShowSpecialPoint = false;
					drwa.nAlpha = 128;
					fr = Globals.ActiveStrip[Globals.ActiveFrame - 1];
					fr.Draw(e.Graphics, rc, drwa, m_ptOffset);
				}
#endif
				fr = Globals.ActiveStrip[Globals.ActiveFrame];
				fr.Draw(e.Graphics, rc, drwa, m_ptOffset);

#if false
				if (m_fOnionSkin && Globals.ActiveFrame < Globals.ActiveStrip.Count - 1) {
					fr = Globals.ActiveStrip[Globals.ActiveFrame + 1];
					fr.Draw(e.Graphics, rc, drwa, m_ptOffset);
				}
#endif

#if false
				// Draw selection rectangle

				if (m_plcSelected != null) {
					Point ptUpperLeft = WxyFromFxy(FxyFromBxy(m_plcSelected, new Point(0, 0)));
					Point ptLowerRight = WxyFromFxy(FxyFromBxy(m_plcSelected, 
							new Point(m_plcSelected.XBitmap.Width, m_plcSelected.XBitmap.Height)));
					e.Graphics.DrawRectangle(new Pen(Color.Black),
							ptUpperLeft.X, ptUpperLeft.Y, ptLowerRight.X - ptUpperLeft.X, ptLowerRight.Y - ptUpperLeft.Y);
				}
#endif
			} else {
				e.Graphics.FillRectangle(new SolidBrush(BackColor), e.ClipRectangle);
			}
		}

		protected override void OnPaintBackground(System.Windows.Forms.PaintEventArgs e) {
			if (Globals.ActiveStrip == null)
				base.OnPaintBackground(e);
		}

		private Point WxyFromFxy(Point ptFrame) {
			return new Point(((ptFrame.X + m_ptOffset.X) * Globals.PreviewScale) + (ClientRectangle.Width / 2),
					((ptFrame.Y + m_ptOffset.Y) * Globals.PreviewScale) + (ClientRectangle.Height / 2));
		}

		private Point FxyFromBxy(BitmapPlacer plc, Point ptBitmap) {
			return new Point(ptBitmap.X - plc.X, ptBitmap.Y - plc.Y);
		}

		private Point FxyFromWxy(Point ptWindow) {
			Rectangle rcClient = ClientRectangle;
			int xCenter = rcClient.Width / 2;
			int yCenter = rcClient.Height / 2;

			int dx = ptWindow.X - xCenter;
			if (dx < 0)
				dx -= Globals.PreviewScale;
			int dy = ptWindow.Y - yCenter;
			if (dy < 0)
				dy -= Globals.PreviewScale;
			return new Point((dx / Globals.PreviewScale) - m_ptOffset.X, (dy / Globals.PreviewScale) - m_ptOffset.Y);
		}

		private Point BxyFromFxy(BitmapPlacer plc, Point ptFrame) {
			return new Point(ptFrame.X + plc.X, ptFrame.Y + plc.Y);
		}

		private BitmapPlacer HitTest(int wx, int wy) {
			Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
			Point fpt = FxyFromWxy(new Point(wx, wy));

			foreach (BitmapPlacer plc in Globals.ActiveStrip[Globals.ActiveFrame].BitmapPlacers) {
				Point bpt = BxyFromFxy(plc, fpt);
				if (bpt.X < 0 || bpt.Y < 0 || bpt.X >= plc.XBitmap.Width || bpt.Y >= plc.XBitmap.Height)
					continue;

				if (plc.XBitmap.Bitmap.GetPixel(bpt.X, bpt.Y) != clrTransparent)
					return plc;
			}

			return null;
		}

		private void PreviewControl_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
			Strip stp = Globals.ActiveStrip;
			if (stp == null)
				return;

			if (m_fSpaceDown) {
				m_fFrameRecentering = true;
				m_fDragging = true;
				m_ptDragStart = FxyFromWxy(new Point(e.X, e.Y));
				m_ptInitialOffset = m_ptOffset;
				Cursor = Globals.GrabCursor;
				return;
			}

			switch (m_mode) {
			case PreviewControlMode.SetSpecialPoint:
				{
					m_fDragging = true;
					m_fSetSpecialPoint = true;
					Point ptT = FxyFromWxy(new Point(e.X, e.Y));
					Frame fr = stp[Globals.ActiveFrame];
					m_ptPreSetSpecialPoint = fr.SpecialPoint;
					fr.SpecialPoint = ptT;
					Globals.ActiveDocument.Dirty = true;
					Globals.OnFrameContentChanged(this, new EventArgs());
				}
				break;

			case PreviewControlMode.RepositionBitmap:
				{
					BitmapPlacer plc = HitTest(e.X, e.Y);
					if (plc == null)
						break;
					m_plcSelected = plc;

					m_fDragging = true;
					m_fAllFrames = (ModifierKeys & Keys.Shift) != 0;

					// UNDONE: across frame operations don't really make sense until we have Tracks

					m_aptPreDragBitmap = new Point[stp.Count];
					for (int ifr = 0; ifr < stp.Count; ifr++) {
						Frame fr = stp[ifr];
						if (fr.BitmapPlacers.Count > 0) {
							m_aptPreDragBitmap[ifr].X = fr.BitmapPlacers[0].X;
							m_aptPreDragBitmap[ifr].Y = fr.BitmapPlacers[0].Y;
						}
					}

					// Special handling for active frame

					m_aptPreDragBitmap[stp.ActiveFrame].X = m_plcSelected.X;
					m_aptPreDragBitmap[stp.ActiveFrame].Y = m_plcSelected.Y;

					m_ptDragStart = FxyFromWxy(new Point(e.X, e.Y));
				}
				break;
			}
		}

		private void PreviewControl_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e) {
			if (!m_fDragging)
				return;

			Strip stp = Globals.ActiveStrip;
			if (stp == null)
				return;

			if (m_fFrameRecentering) {
				m_ptOffset = m_ptInitialOffset;
				Point ptNew = FxyFromWxy(new Point(e.X, e.Y));
				m_ptOffset.X = m_ptInitialOffset.X + (ptNew.X - m_ptDragStart.X);
				m_ptOffset.Y = m_ptInitialOffset.Y + (ptNew.Y - m_ptDragStart.Y);
				Invalidate();
				Update();

				// Notify anyone who cares that we've changed the Frame offset

				if (FrameOffsetChanged != null)
					FrameOffsetChanged(this, new FrameOffsetEventArgs(m_ptOffset.X, m_ptOffset.Y));
				return;
			}

			switch (m_mode) {
			case PreviewControlMode.SetSpecialPoint: 
				{
					Point ptT = FxyFromWxy(new Point(e.X, e.Y));
					Frame fr = stp[Globals.ActiveFrame];
					fr.SpecialPoint = ptT;
					Globals.ActiveDocument.Dirty = true;
					Globals.OnFrameContentChanged(this, new EventArgs());
				}
				break;

			case PreviewControlMode.RepositionBitmap: 
				{
					Point ptT = FxyFromWxy(new Point(e.X, e.Y));
                    int ipl = stp[Globals.ActiveFrame].BitmapPlacers.
                            Index(m_plcSelected);
                    int ifr;
                    int cfr;
                    if (m_fAllFrames) {
                        ifr = 0;
                        cfr = stp.Count;
                    } else {
                        ifr = Globals.ActiveFrame;
                        cfr = Globals.ActiveFrameCount;
                    }

                    for (int ifrT = ifr; ifrT < ifr + cfr; ifrT++) {
                        int iplT = ipl;
                        if (iplT >= stp[ifrT].BitmapPlacers.Count) {
                            iplT = 0;
                        }
                        BitmapPlacer plc = stp[ifrT].BitmapPlacers[iplT];
                        plc.X = m_aptPreDragBitmap[ifrT].X + m_ptDragStart.X -
                                ptT.X;
                        plc.Y = m_aptPreDragBitmap[ifrT].Y + m_ptDragStart.Y -
                                ptT.Y;
                    }
					m_fRepositionBitmaps = true;
					Globals.ActiveDocument.Dirty = true;
					Globals.OnFrameContentChanged(this, new EventArgs());
				}
				break;
			}
		}

		private void PreviewControl_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e) {
			m_fDragging = false;

			if (m_fFrameRecentering) {
				Cursor = Cursors.Default;
				m_fFrameRecentering = false;
			}

			if (m_fRepositionBitmaps) {
				m_fRepositionBitmaps = false;
				Strip stp = Globals.ActiveStrip;

				UndoManager.BeginGroup();
				for (int ifr = 0; ifr < stp.Count; ifr++) {
					if (ifr != stp.ActiveFrame) {
						if (stp[ifr].BitmapPlacers.Count > 0) {
							UndoManager.AddUndo(new UndoDelegate(UndoSetBitmapPosition), 
								new Object[] { stp[ifr].BitmapPlacers[0], m_aptPreDragBitmap[ifr].X, m_aptPreDragBitmap[ifr].Y });
						}
					}
				}
				UndoManager.AddUndo(new UndoDelegate(UndoSetBitmapPosition), 
						new Object[] { m_plcSelected, m_aptPreDragBitmap[stp.ActiveFrame].X, m_aptPreDragBitmap[stp.ActiveFrame].Y });
				UndoManager.EndGroup();
			}

			if (m_fSetSpecialPoint) {
				m_fSetSpecialPoint = false;
				UndoManager.AddUndo(new UndoDelegate(UndoSetSpecialPoint), 
						new Object[] { Globals.ActiveStrip[Globals.ActiveFrame], m_ptPreSetSpecialPoint.X, m_ptPreSetSpecialPoint.Y });
			}
		}

		private void UndoSetBitmapPosition(object[] aobArgs) {
			BitmapPlacer plc = (BitmapPlacer)aobArgs[0];
			plc.X = (int)aobArgs[1];
			plc.Y = (int)aobArgs[2];
			Globals.ActiveDocument.Dirty = true;
			Globals.OnFrameContentChanged(this, new EventArgs());
		}

		private void UndoSetSpecialPoint(object[] aobArgs) {
			Frame fr = (Frame)aobArgs[0];
			fr.SpecialPoint = new Point((int)aobArgs[1], (int)aobArgs[2]);
			Globals.ActiveDocument.Dirty = true;
			Globals.OnFrameContentChanged(this, new EventArgs());
		}

		private void PreviewControl_DragEnter(object sender, System.Windows.Forms.DragEventArgs e) {
			XBitmap[] axbm = (XBitmap[])e.Data.GetData(typeof(XBitmap[]));
			if (axbm == null)
				e.Effect = DragDropEffects.None;
			else
				e.Effect = DragDropEffects.Copy;
		}

		private void PreviewControl_DragDrop(object sender, System.Windows.Forms.DragEventArgs e) {
			XBitmap[] axbm = (XBitmap[])e.Data.GetData(typeof(XBitmap[]));
			if (axbm == null)
				return;

			BitmapPlacerList plcl = Globals.ActiveStrip[Globals.ActiveFrame].BitmapPlacers;
			foreach (XBitmap xbm in axbm) {
				BitmapPlacer plc = new BitmapPlacer();
				plc.X = 0;
				plc.Y = 0;
				plc.XBitmap = xbm;
				plcl.Insert(0, plc);
			}

			Globals.ActiveDocument.Dirty = true;
			Globals.OnFrameContentChanged(this, new EventArgs());
		}
	}

	public enum PreviewControlMode {
		SetSpecialPoint,
		RepositionBitmap,
	}
}
