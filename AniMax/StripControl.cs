using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for StripControl.
	/// </summary>
	public class StripControl : System.Windows.Forms.UserControl
	{
		private bool m_fSpaceDown = false;
		private int m_ifrInsertionPoint = -1;
		private bool m_fFrameRecentering = false;
		private Point m_ptDown;
		private Point m_ptOffset;
		private Point m_ptInitialOffset;
		private int m_cxFrame = 100;
		private bool m_fButtonDown = false;
		private bool m_fFrameResizing = false;
		private Strip m_stp;
		private System.Windows.Forms.HScrollBar sb;
		private System.Windows.Forms.ContextMenu mnu;
		private System.Windows.Forms.MenuItem mniDelete;
		private System.Windows.Forms.MenuItem mniInsert;
		private System.Windows.Forms.MenuItem mniCut;
		private System.Windows.Forms.MenuItem mniCopy;
		private System.Windows.Forms.MenuItem mniPaste;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem mniProperties;
		private System.Windows.Forms.Timer tmrScroll;
		private System.ComponentModel.IContainer components;

		public StripControl()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// My initialization

			ResizeRedraw = true;
			SetStyle(ControlStyles.Selectable, true);
			
			m_ptOffset = new Point(0, 0);
			Globals.ActiveFrameChanged += new EventHandler(OnInvalidatingChange);
			Globals.ActiveFrameCountChanged += new EventHandler(OnInvalidatingChange);
			Globals.GridChanged += new EventHandler(OnInvalidatingChange);
			Globals.SideColorMappingOnChanged += new EventHandler(OnInvalidatingChange);
			Globals.ShowOriginPointChanged += new EventHandler(OnInvalidatingChange);
			Globals.ShowSpecialPointChanged += new EventHandler(OnInvalidatingChange);
			Globals.KeyPress += new KeyPressEventHandler(StripControl_KeyPress);
			Globals.FrameContentChanged += new EventHandler(OnInvalidatingChange);
			Globals.KeyDown += new KeyEventHandler(OnGlobalKeyDown);
			Globals.KeyUp += new KeyEventHandler(OnGlobalKeyUp);
			Globals.StripScaleChanged += new EventHandler(OnInvalidatingChange);
			mnu.Popup += new EventHandler(StripControl_Popup);
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
			this.sb = new System.Windows.Forms.HScrollBar();
			this.mnu = new System.Windows.Forms.ContextMenu();
			this.mniCut = new System.Windows.Forms.MenuItem();
			this.mniCopy = new System.Windows.Forms.MenuItem();
			this.mniPaste = new System.Windows.Forms.MenuItem();
			this.mniDelete = new System.Windows.Forms.MenuItem();
			this.mniInsert = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.mniProperties = new System.Windows.Forms.MenuItem();
			this.tmrScroll = new System.Windows.Forms.Timer(this.components);
			this.SuspendLayout();
			// 
			// sb
			// 
			this.sb.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.sb.Location = new System.Drawing.Point(0, 79);
			this.sb.Name = "sb";
			this.sb.Size = new System.Drawing.Size(335, 16);
			this.sb.TabIndex = 0;
			this.sb.ValueChanged += new System.EventHandler(this.sb_ValueChanged);
			// 
			// mnu
			// 
			this.mnu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																				this.mniCut,
																				this.mniCopy,
																				this.mniPaste,
																				this.mniDelete,
																				this.mniInsert,
																				this.menuItem1,
																				this.mniProperties});
			// 
			// mniCut
			// 
			this.mniCut.Index = 0;
			this.mniCut.Shortcut = System.Windows.Forms.Shortcut.CtrlX;
			this.mniCut.Text = "Cu&t";
			this.mniCut.Click += new System.EventHandler(this.mniCut_Click);
			// 
			// mniCopy
			// 
			this.mniCopy.Index = 1;
			this.mniCopy.Shortcut = System.Windows.Forms.Shortcut.CtrlC;
			this.mniCopy.Text = "&Copy";
			this.mniCopy.Click += new System.EventHandler(this.mniCopy_Click);
			// 
			// mniPaste
			// 
			this.mniPaste.Index = 2;
			this.mniPaste.Shortcut = System.Windows.Forms.Shortcut.CtrlV;
			this.mniPaste.Text = "&Paste";
			this.mniPaste.Click += new System.EventHandler(this.mniPaste_Click);
			// 
			// mniDelete
			// 
			this.mniDelete.Index = 3;
			this.mniDelete.Shortcut = System.Windows.Forms.Shortcut.Del;
			this.mniDelete.Text = "&Delete";
			this.mniDelete.Click += new System.EventHandler(this.mniDelete_Click);
			// 
			// mniInsert
			// 
			this.mniInsert.Index = 4;
			this.mniInsert.Shortcut = System.Windows.Forms.Shortcut.Ins;
			this.mniInsert.Text = "&Insert";
			this.mniInsert.Click += new System.EventHandler(this.mniInsert_Click);
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 5;
			this.menuItem1.Text = "-";
			// 
			// mniProperties
			// 
			this.mniProperties.Index = 6;
			this.mniProperties.Text = "Properties";
			// 
			// tmrScroll
			// 
			this.tmrScroll.Interval = 150;
			this.tmrScroll.Tick += new System.EventHandler(this.tmrScroll_Tick);
			// 
			// StripControl
			// 
			this.AllowDrop = true;
			this.ContextMenu = this.mnu;
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.sb});
			this.DockPadding.Bottom = 1;
			this.DockPadding.Right = 1;
			this.Name = "StripControl";
			this.Size = new System.Drawing.Size(336, 96);
			this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.StripControl_KeyPress);
			this.DragEnter += new System.Windows.Forms.DragEventHandler(this.StripControl_DragEnter);
			this.DragDrop += new System.Windows.Forms.DragEventHandler(this.StripControl_DragDrop);
			this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.StripControl_KeyDown);
			this.DragOver += new System.Windows.Forms.DragEventHandler(this.StripControl_DragOver);
			this.ResumeLayout(false);

		}
		#endregion

		public Strip Strip {
			set {
				// Our Undo isn't robust enough to be maintained across strip
				// activations so we flush it to be sure strip A's undos aren't
				// applied to strip B.

				UndoManager.Flush();

				m_stp = value;
				RecalcScrollbar();
				Invalidate();
			}
			get {
				return m_stp;
			}
		}

		public int FrameWidth {
			set {
				m_cxFrame = value;
				RecalcScrollbar();
				Invalidate();
			}
		}

		public event FrameOffsetEventHandler FrameOffsetChanged;

		private void OnInvalidatingChange(object obSender, EventArgs e) {
			RecalcScrollbar();
			Invalidate();
		}

		public void OnFrameOffsetChanged(object obSender, FrameOffsetEventArgs e) {
			m_ptOffset.X = e.X;
			m_ptOffset.Y = e.Y;
			Invalidate();
		}

		protected override void OnPaintBackground(System.Windows.Forms.PaintEventArgs e) {
			if (m_stp == null || m_stp.Count == 0)
				base.OnPaintBackground(e);
		}

		protected override void OnPaint(System.Windows.Forms.PaintEventArgs e) {
			if (m_stp == null || m_stp.Count == 0)
				return;

			Frame.DrawArgs drwa = new Frame.DrawArgs();
			drwa.clrBackground = this.BackColor;
			drwa.fDrawBackground = true;
			drwa.fMapSideColors = Globals.SideColorMappingOn;
			drwa.fShowGrid = Globals.GridOn;
			drwa.cxGrid = Globals.GridWidth;
			drwa.cyGrid = Globals.GridHeight;
			drwa.fShowOrigin = Globals.ShowOriginPoint;
			drwa.fShowSpecialPoint = Globals.ShowSpecialPoint;
			drwa.nScale = Globals.StripScale;

			Pen penInactive = new Pen(Color.Black);
			Pen penActive = new Pen(Color.Red);
			Font fnt = new Font("Arial", 8);
			Brush brBlack = new SolidBrush(Color.Black);

			int cyFrame = sb.Top - 1;

			Bitmap bmT = new Bitmap(ClientRectangle.Width, cyFrame + 1);
			Graphics gT = Graphics.FromImage(bmT);
			gT.Clear(drwa.clrBackground);

			int ifr = sb.Value / m_cxFrame;
			int x = -(sb.Value % m_cxFrame);

			for (; x < ClientRectangle.Width && ifr < m_stp.Count; x += m_cxFrame, ifr++) {

				// Draw a frame around the Frame

				gT.DrawRectangle(penInactive, x, 0, m_cxFrame, cyFrame);

				// Set up a clip box so the scaled frame won't draw anywhere we don't want it to

				gT.SetClip(new Rectangle(x + 1, 1, m_cxFrame - 1, cyFrame - 1));

				// Draw the scaled Frame

				Frame fr = m_stp[ifr];
				Rectangle rc = new Rectangle(x, 0, m_cxFrame, cyFrame);
				fr.Draw(gT, rc, drwa, m_ptOffset);

				// Draw the frame's hold count

				if (fr.HoldCount != 0) {
					string strT = fr.HoldCount.ToString();
					SizeF sizf = e.Graphics.MeasureString(strT, fnt);
					gT.DrawString(strT, fnt, brBlack, x + m_cxFrame - sizf.Width - 1, cyFrame - fnt.Height);
				}

				// Clear out our special clip region

				gT.ResetClip();
			}

			// Draw a special frame around the active Frame

            for (int ifrT = m_stp.ActiveFrame; ifrT < m_stp.ActiveFrame +
                    m_stp.ActiveFrameCount; ifrT++) {
                Rectangle rcT = new Rectangle((ifrT * m_cxFrame) - sb.Value, 0, m_cxFrame, cyFrame);
                gT.DrawRectangle(penActive, rcT);
                rcT.Inflate(-1, -1);
                gT.DrawRectangle(penActive, rcT);
            }

			// Draw the insertion point, if any

			if (m_ifrInsertionPoint != -1) {
				Brush br = new SolidBrush(Color.Red);
				int xT = (m_ifrInsertionPoint * m_cxFrame) - sb.Value;
				gT.FillRectangle(br, xT, 0, 1, cyFrame);
				gT.FillRectangle(br, xT - 2, 0, 5, 1);
				gT.FillRectangle(br, xT - 1, 1, 3, 1);
				gT.FillRectangle(br, xT - 1, cyFrame - 2, 3, 1);
				gT.FillRectangle(br, xT - 2, cyFrame - 1, 5, 1);
			}

			// Copy the buffered Strip to the screen

			e.Graphics.DrawImageUnscaled(bmT, 0, 0);

			gT.Dispose();
			bmT.Dispose();
		}

		protected override void OnMouseMove(System.Windows.Forms.MouseEventArgs e) {
			if (m_stp == null || m_stp.Count == 0)
				return;

			if (m_fButtonDown) {
				if (m_fFrameResizing) {
					Cursor = Cursors.VSplit;
					m_cxFrame = Math.Max(4, e.X + sb.Value);
					RecalcScrollbar();
					Invalidate();
				} else if (m_fFrameRecentering) {
					m_ptOffset.X = m_ptInitialOffset.X + (e.X - m_ptDown.X) / Globals.StripScale;
					m_ptOffset.Y = m_ptInitialOffset.Y + (e.Y - m_ptDown.Y) / Globals.StripScale;
					Invalidate();
					Update();

					// Notify anyone who cares that we've changed the Frame offset

					if (FrameOffsetChanged != null)
						FrameOffsetChanged(this, new FrameOffsetEventArgs(m_ptOffset.X, m_ptOffset.Y));
				} else {
					m_fButtonDown = false;		// because drag drop will capture the up
					DoDragDrop(m_stp[m_stp.ActiveFrame], DragDropEffects.Move | DragDropEffects.Copy | DragDropEffects.Scroll);
				}
			} else {
				if (e.X >= m_cxFrame - sb.Value - 1 && e.X <= m_cxFrame - sb.Value + 1)
					Cursor = Cursors.VSplit;
				else
					Cursor = m_fSpaceDown ? Globals.HandCursor : Cursors.Default;
			}
		}

		protected override void OnMouseDown(System.Windows.Forms.MouseEventArgs e) {
			Focus();

			if (m_stp == null || m_stp.Count == 0)
				return;

			if (e.Button == MouseButtons.Left) {
				m_fButtonDown = true;

				// Is user clicking on the frame border?

				if (e.X >= m_cxFrame - sb.Value - 1 && e.X <= m_cxFrame - sb.Value + 1) {
					m_fFrameResizing = true;

				// No, activate the frame they clicked

				} else {
					int ifr = (e.X + sb.Value) / m_cxFrame;
					if (ifr < m_stp.Count) {
                        if ((ModifierKeys & Keys.Shift) == 0) {
                            m_stp.ActiveFrame = ifr;
                        } else {
                            if (ifr >= m_stp.ActiveFrame) {
                                m_stp.ActiveFrameCount = ifr -
                                        m_stp.ActiveFrame + 1;
                            } else {
                                int cfr = m_stp.ActiveFrame - ifr + 1;
                                m_stp.ActiveFrame = ifr;
                                m_stp.ActiveFrameCount = cfr;
                            }
                        }

						// If spacebar is held down then prepare to recenter the frame.

						if (m_fSpaceDown) {
							m_fFrameRecentering = true;
							m_ptInitialOffset = m_ptOffset;
							m_ptDown = new Point(e.X, e.Y);
							Cursor = Globals.GrabCursor;
						}
					}
				}
			} else if (e.Button == MouseButtons.Right) {
				int ifr = (e.X + sb.Value) / m_cxFrame;
				if (ifr < m_stp.Count) {
					if (m_stp.ActiveFrame != ifr)
						m_stp.ActiveFrame = ifr;
					mnu.Show(this, new Point(e.X, e.Y));
				}
			}
		}

		protected override void OnMouseUp(System.Windows.Forms.MouseEventArgs e) {
			if (e.Button == MouseButtons.Left) {
				m_fButtonDown = false;
				m_fFrameResizing = false;
				m_fFrameRecentering = false;
			}
		}

		protected override void OnResize(EventArgs e) {
			base.OnResize(e);
			RecalcScrollbar();
		}

		private void StripControl_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e) {
			switch (e.KeyChar) {
			case '[': 
				{
					int ifr = m_stp.ActiveFrame - 1;
					if (ifr < 0)
						ifr = m_stp.Count - 1;
					m_stp.ActiveFrame = ifr;
					e.Handled = true;
				}
				break;

			case ']': 
				{
					int ifr = m_stp.ActiveFrame + 1;
					if (ifr >= m_stp.Count)
						ifr = 0;
					m_stp.ActiveFrame = ifr;
					e.Handled = true;
				}
				break;
			}
		}

		// UNDONE: move _KeyPress handler into this
		// UNDONE: this doesn't work for some reason. Somebody is snarfing the arrow keys (but not page up,etc)
		private void StripControl_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e) {
			switch (e.KeyCode) {
			case Keys.Left: 
				{
					int ifr = m_stp.ActiveFrame - 1;
					if (ifr < 0)
						ifr = m_stp.Count - 1;
					m_stp.ActiveFrame = ifr;
					e.Handled = true;
				}
				break;

			case Keys.Right: 
				{
					int ifr = m_stp.ActiveFrame + 1;
					if (ifr >= m_stp.Count)
						ifr = 0;
					m_stp.ActiveFrame = ifr;
					e.Handled = true;
				}
				break;
			}
		}

		private void OnGlobalKeyDown(object obSender, KeyEventArgs e) {
			if (e.KeyCode == Keys.Space && !m_fSpaceDown) {
				Cursor = Globals.HandCursor;
				m_fSpaceDown = true;
			}
		}
		
		private void OnGlobalKeyUp(object obSender, KeyEventArgs e) {
			if (e.KeyCode == Keys.Space) {
				Cursor = Cursors.Default;
				m_fSpaceDown = false;
			}
		}

		private void StripControl_Popup(object sender, System.EventArgs e) {
			mniPaste.Enabled = Clipboard.GetDataObject().GetDataPresent(typeof(Frame));
		}

		private void StripControl_DragEnter(object sender, System.Windows.Forms.DragEventArgs e) {
			if (e.Data.GetData(typeof(Frame)) == null || (e.KeyState & 8) != 0)
				e.Effect = DragDropEffects.Copy;
			else
				e.Effect = DragDropEffects.Move;

			tmrScroll.Start();
		}

		private void StripControl_DragOver(object sender, System.Windows.Forms.DragEventArgs e) {
			if (e.Data.GetData(typeof(Frame)) == null || (e.KeyState & 8) != 0)
				e.Effect = DragDropEffects.Copy;
			else
				e.Effect = DragDropEffects.Move;

			// Find the closest between-frames border and draw an insertion cursor there

			Point pt = PointToClient(new Point(e.X, e.Y));
			SetInsertionPoint(pt.X);
		}

		private void StripControl_DragDrop(object sender, System.Windows.Forms.DragEventArgs e) {
			tmrScroll.Stop();
			Focus();

			// Drop from Bitmaps window

			XBitmap[] axbm = e.Data.GetData(typeof(XBitmap[])) as XBitmap[];
			if (axbm != null) {
				if (m_stp == null)
					((StripsForm)Globals.StripsForm).NewStrip();

				int ifrFirst = m_ifrInsertionPoint;
				int ifrInsert = ifrFirst;

				UndoManager.BeginGroup();

				foreach (XBitmap xbm in axbm) {
					Frame frT = new Frame();
					frT.BitmapPlacers.Add(new BitmapPlacer());
					frT.BitmapPlacers[0].XBitmap = xbm;
					frT.BitmapPlacers[0].X = xbm.Width / 2;
					frT.BitmapPlacers[0].Y = xbm.Height / 2;

					InsertFrame(m_stp, ifrInsert++, frT, true);
				}

				UndoManager.EndGroup();

				m_stp.ActiveFrame = ifrFirst;
			}

			// Drop from Strip window

			// UNDONE: wrap the Frame in a data structure that includes Strip and ifr
			// and clean this up

			Frame fr = e.Data.GetData(typeof(Frame)) as Frame;
			if (fr != null) {
				if (m_ifrInsertionPoint != -1) {
					if ((ModifierKeys & Keys.Control) == 0) {

						// Don't try to move the frame immediately before or after itself

						if (m_ifrInsertionPoint == m_stp.ActiveFrame || 
								m_ifrInsertionPoint == m_stp.ActiveFrame + 1) {
							m_ifrInsertionPoint = -1;
							Invalidate();
							return;
						}
					}

					UndoManager.BeginGroup();

					// Insert at new place

					int ifrOld = m_stp.ActiveFrame;
					fr = (Frame)m_stp[ifrOld].Clone();
					InsertFrame(m_stp, m_ifrInsertionPoint, fr, true);
					m_stp.ActiveFrame = m_ifrInsertionPoint;

					// If the control key is held the frame is duplicated, not moved

					if ((ModifierKeys & Keys.Control) == 0) {
						// Remove from old place

						if (m_ifrInsertionPoint <= ifrOld)
							ifrOld++;
						DeleteFrame(m_stp, ifrOld, true);
					}

					UndoManager.EndGroup();
				}
			}

			m_ifrInsertionPoint = -1;
			Invalidate();
		}

		private void RecalcScrollbar() {
			if (m_stp == null) {
				sb.Enabled = false;
			} else {
				int cxScroll = m_stp.Count * m_cxFrame;
				if (cxScroll <= ClientRectangle.Width) {
					sb.Value = 0;
					sb.Enabled = false;
				} else {
					sb.Enabled = true;
					sb.Maximum = cxScroll;
					if (sb.Value > cxScroll - ClientRectangle.Width)
						sb.Value = cxScroll - ClientRectangle.Width + 1;
					sb.LargeChange = ClientRectangle.Width;
					sb.SmallChange = m_cxFrame;
				}
			}
		}

		private void SetInsertionPoint(int x) {
			x += sb.Value;
			if (x < 0)
				x = 0;
			else if (x > m_cxFrame * m_stp.Count)
				x = m_cxFrame * m_stp.Count;
			int ifr = x / m_cxFrame;
			if (x % m_cxFrame >= m_cxFrame / 2)
				ifr++;
			m_ifrInsertionPoint = ifr;
			Invalidate();
		}

		private void sb_ValueChanged(object sender, System.EventArgs e) {
			Invalidate();
		}

		private void mniInsert_Click(object sender, System.EventArgs e) {
			if (m_stp == null)
				return;
			InsertFrame(m_stp, m_stp.ActiveFrame, new Frame(), true);
			m_stp.ActiveFrame = m_stp.ActiveFrame;
		}

		private void mniDelete_Click(object sender, System.EventArgs e) {
			DeleteFrame(m_stp, m_stp.ActiveFrame, true);
		}

		private void mniCopy_Click(object sender, System.EventArgs e) {
			Frame fr = (Frame)m_stp[m_stp.ActiveFrame].Clone();
			Clipboard.SetDataObject(fr);
		}

		private void mniCut_Click(object sender, System.EventArgs e) {
			mniCopy_Click(sender, e);
			mniDelete_Click(sender, e);
		}

		private void mniPaste_Click(object sender, System.EventArgs e) {
			IDataObject dta = Clipboard.GetDataObject();
			if (dta.GetDataPresent(typeof(Frame))) {
				Frame fr = (Frame)dta.GetData(typeof(Frame));
				InsertFrame(m_stp, m_stp.ActiveFrame, fr, true);
			}
		}

		private void UndoDelete(object[] aobArgs) {
			((Strip)aobArgs[0]).Insert((int)aobArgs[1], (Frame)aobArgs[2]);
			((Strip)aobArgs[0]).ActiveFrame = (int)aobArgs[3];
			Invalidate();
			RecalcScrollbar();
		}

		private void UndoInsert(object[] aobArgs) {
			DeleteFrame((Strip)aobArgs[0], (int)aobArgs[1], false);
			((Strip)aobArgs[0]).ActiveFrame = (int)aobArgs[2];
			Invalidate();
			RecalcScrollbar();
		}

		private void InsertFrame(Strip stp, int ifr, Frame fr, bool fUndoable) {
			if (fUndoable)
				UndoManager.AddUndo(new UndoDelegate(UndoInsert), new Object [] { stp, ifr, stp.ActiveFrame });
			stp.Insert(ifr, fr);
			Globals.ActiveDocument.Dirty = true;
			Invalidate();
			RecalcScrollbar();
		}

		private void DeleteFrame(Strip stp, int ifr, bool fUndoable) {
			if (fUndoable)
				UndoManager.AddUndo(new UndoDelegate(UndoDelete), new object[] { stp, ifr, stp[ifr], stp.ActiveFrame});
			stp.RemoveAt(ifr);
			Globals.ActiveDocument.Dirty = true;
			if (stp.ActiveFrame >= stp.Count)
				stp.ActiveFrame = stp.Count - 1;
			RecalcScrollbar();
		}

		private void tmrScroll_Tick(object sender, System.EventArgs e) {
			if (m_ifrInsertionPoint == -1)
				return;

			Point ptMouse = PointToClient(Control.MousePosition);
			int v;
			if (ptMouse.X < 0) {
				v = Math.Max(0, sb.Value - m_cxFrame);
			} else if (ptMouse.X >= ClientRectangle.Width) {
				v = Math.Min((m_cxFrame * m_stp.Count) - ClientRectangle.Width + 1, sb.Value + m_cxFrame);
			} else {
				return;
			}

			if (v < sb.Minimum) {
				v = sb.Minimum;
			}
			if (v > sb.Maximum) {
				v = sb.Maximum;
			}
			sb.Value = v;
			SetInsertionPoint(ptMouse.X);
		}
	}

	public delegate void FrameOffsetEventHandler(object sender, FrameOffsetEventArgs e);

	public class FrameOffsetEventArgs : EventArgs {
		public int X;
		public int Y;

		public FrameOffsetEventArgs(int x, int y) {
			X = x;
			Y = y;
		}
	}
}
