using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Data;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace m
{
	public class LevelView : System.Windows.Forms.UserControl
	{
		System.ComponentModel.Container components = null;
		Bitmap m_bm;
		LevelDoc m_lvld;
		Rectangle m_rcDragStart = Rectangle.Empty;
		System.Windows.Forms.ContextMenu m_contextMenu;
		System.Windows.Forms.MenuItem menuItemRemove;
		IMapItem m_miContextMenu;
		Point m_ptDragSelectAnchor = new Point(0, 0);
		Rectangle m_rcDragSelect;
		bool m_fDragSelect;
		bool m_fJustSelected;
		float m_xRatioView = 0.0f;
		float m_yRatioView = 0.0f;
		float m_flScale = 1.0f;
		TemplateDoc m_tmpd = null;
		LayerFlags m_lyrf = LayerFlags.Default;
		Bitmap m_bmDrag = null;
		private System.Windows.Forms.MenuItem menuItemSelectSame;
		Rectangle m_rcDragBoundsLast = Rectangle.Empty;
		Point m_ptMouseDown;
		IMapItem m_miCapturedMouse = null;

		public event EventHandler ScaleChanged;

		public LevelView() {
			m_lvld = null;
		}

		public void SetDocument(Document doc) {
			// This call is required by the Windows.Forms Form Designer.

			InitializeComponent();

			// Add any initialization after the InitForm call

			m_lvld = (LevelDoc)doc;

			// Create the bitmap

			AutoScroll = true;
			VScroll = true;
			HScroll = true;
			CreateBitmap();

			// Init view position

			InitPosition();

			// Draw it initially

			m_lvld.Draw(m_bm, null, GetTileSize(), GetTemplateDoc(), m_lyrf);

			// Need to know these events

			m_lvld.ImageChanged += new LevelDoc.ImageChangedHandler(LevelDoc_ImageChanged);
			m_lvld.ItemsRemoved += new LevelDoc.ItemsRemovedHandler(LevelDoc_ItemsRemoved);

			// Check size

			CheckSize();
		}

		public void SetScale(float flScale) {
			if (flScale < 0.25f)
				flScale = 0.25f;
			if (flScale == m_flScale)
				return;
			m_flScale = flScale;
			ResetClientSize();
			DrawLevelImage(null);
			if (ScaleChanged != null)
				ScaleChanged(this, null);
		}

		public float GetScale() {
			return m_flScale;
		}

		Point WorldToView(Point ptWorld) {
			int xView = (int)((float)ptWorld.X * m_flScale);
			int yView = (int)((float)ptWorld.Y * m_flScale);
			return new Point(xView, yView);
		}

		Point ViewToWorld(Point ptView) {
			int xWorld = (int)((float)ptView.X / m_flScale);
			int yWorld = (int)((float)ptView.Y / m_flScale);
			return new Point(xWorld, yWorld);
		}

		Size WorldToViewSize(Size sizWorld) {
			Point ptWorld = new Point(sizWorld.Width, sizWorld.Height);
			Point ptView = WorldToView(ptWorld);
			return new Size(ptView.X, ptView.Y);
		}

		Size ViewToWorldSize(Size sizView) {
			Point ptView = new Point(sizView.Width, sizView.Height);
			Point ptWorld = ViewToWorld(ptView);
			return new Size(ptWorld.X, ptWorld.Y);
		}

		void InitPosition() {
			int txWorld = m_lvld.Bounds.X - 1;
			if (txWorld < 0)
				txWorld = 0;
			int tyWorld = m_lvld.Bounds.Y - 1;
			if (tyWorld < 0)
				tyWorld = 0;
			Size sizTile = GetTileSize();
			int xWorld = txWorld * sizTile.Width;
			int yWorld = tyWorld * sizTile.Height;
			m_xRatioView = (float)xWorld / (float)m_bm.Width;
			m_yRatioView = (float)yWorld / (float)m_bm.Height;
			AutoScrollPosition = WorldToView(new Point(xWorld, yWorld));
		}

		protected unsafe override void WndProc(ref Message m) {
			Point ptWorldScrollPos;
			switch (m.Msg) {
			// #define WM_HSCROLL                      0x0114
			case 0x114:
				base.WndProc(ref m);
				ptWorldScrollPos = ViewToWorld(AutoScrollPosition);
				m_xRatioView = (float)-ptWorldScrollPos.X / (float)m_bm.Width;
				break;

			// #define WM_VSCROLL                      0x0115
			// #define WM_MOUSEWHEEL                   0x020A
			case 0x20a:
			case 0x115:
				base.WndProc(ref m);
				ptWorldScrollPos = ViewToWorld(AutoScrollPosition);
				m_yRatioView = (float)-ptWorldScrollPos.Y / (float)m_bm.Height;
				break;

			default:
				base.WndProc(ref m);
				break;
			}
		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			m_lvld.ImageChanged -= new LevelDoc.ImageChangedHandler(LevelDoc_ImageChanged);
			m_lvld.ItemsRemoved -= new LevelDoc.ItemsRemovedHandler(LevelDoc_ItemsRemoved);

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
			this.m_contextMenu = new System.Windows.Forms.ContextMenu();
			this.menuItemRemove = new System.Windows.Forms.MenuItem();
			this.menuItemSelectSame = new System.Windows.Forms.MenuItem();
			// 
			// m_contextMenu
			// 
			this.m_contextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						  this.menuItemRemove,
																						  this.menuItemSelectSame});
			// 
			// menuItemRemove
			// 
			this.menuItemRemove.Index = 0;
			this.menuItemRemove.Text = "Remove";
			this.menuItemRemove.Click += new System.EventHandler(this.menuItemRemove_Click);
			// 
			// menuItemSelectSame
			// 
			this.menuItemSelectSame.Index = 1;
			this.menuItemSelectSame.Text = "Select Same";
			this.menuItemSelectSame.Click += new System.EventHandler(this.menuItemSelectSame_Click);
			// 
			// LevelView
			// 
			this.AllowDrop = true;
			this.BackColor = System.Drawing.Color.Black;
			this.Name = "LevelView";
			this.Size = new System.Drawing.Size(368, 288);
			this.Resize += new System.EventHandler(this.LevelView_Resize);
			this.DragEnter += new System.Windows.Forms.DragEventHandler(this.LevelView_DragEnter);
			this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.LevelView_MouseUp);
			this.Paint += new System.Windows.Forms.PaintEventHandler(this.LevelView_Paint);
			this.DragLeave += new System.EventHandler(this.LevelView_DragLeave);
			this.DragDrop += new System.Windows.Forms.DragEventHandler(this.LevelView_DragDrop);
			this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.LevelView_KeyDown);
			this.DragOver += new System.Windows.Forms.DragEventHandler(this.LevelView_DragOver);
			this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.LevelView_MouseMove);
			this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.LevelView_MouseDown);

		}
		#endregion

		public void SetTemplateDoc(TemplateDoc tmpd) {
			if (tmpd == m_tmpd)
				return;

			if (m_tmpd != null)
				m_tmpd.BackgroundChanged -= new TemplateDoc.BackgroundChangedHandler(TemplateDoc_BackgroundChangedHandler);			

			m_tmpd = tmpd;

			if (m_tmpd != null)
				m_tmpd.BackgroundChanged += new TemplateDoc.BackgroundChangedHandler(TemplateDoc_BackgroundChangedHandler);

			UpdateImage();
		}

		void TemplateDoc_BackgroundChangedHandler(TemplateDoc tmpd) {
			UpdateImage();
		}

		public void SetLayerFlags(LayerFlags lyrf) {
			// If already set to this, return

			if (lyrf == m_lyrf)
				return;

			// Remove the appropriate items from the current selection

			ArrayList alsmiSelected = m_lvld.Selection;

			for (int imi = 0; imi < alsmiSelected.Count; ) {
				IMapItem mi = (IMapItem)alsmiSelected[imi];
				if (mi is Tile) {
					if ((lyrf & LayerFlags.Templates) == 0) {
						alsmiSelected.RemoveAt(imi);
						continue;
					}
				} else if (mi is Area) {
					if ((lyrf & LayerFlags.Areas) == 0) {
						alsmiSelected.RemoveAt(imi);
						continue;
					}
				} else {
					if ((lyrf & LayerFlags.Gobs) == 0) {
						alsmiSelected.RemoveAt(imi);
						continue;
					}
				}
				imi++;
			}
			m_lvld.Selection = alsmiSelected;

			// Set new flags and redraw

			m_lyrf = lyrf;
			Redraw();
		}

		public LayerFlags GetLayerFlags() {
			return m_lyrf;
		}

		TemplateDoc GetTemplateDoc() {
			if (m_tmpd != null)
				return m_tmpd;
			return m_lvld.GetTemplateDoc();
		}

		Size GetTileSize() {
			TemplateDoc tmpd = GetTemplateDoc();
			if (tmpd == null)
				return new Size(24, 24);
			return tmpd.TileSize;
		}

		private void LevelView_Paint(object sender, System.Windows.Forms.PaintEventArgs e) {
			if (m_lvld == null)
				return;
			DrawLevelImage(e.Graphics);
		}

		protected override void OnPaintBackground(System.Windows.Forms.PaintEventArgs e) {
			if (this.DesignMode)
				base.OnPaintBackground(e);
		}

		private void Redraw() {
			m_lvld.Draw(m_bm, null, GetTileSize(), GetTemplateDoc(), m_lyrf);
			using (Graphics gWin = CreateGraphics())
				DrawLevelImage(gWin);
		}

		void DrawLevelImage(Graphics g) {
			Graphics gT = g;
			if (g == null)
				gT = CreateGraphics();
			Rectangle rcSrcWorld = new Rectangle(0, 0, m_bm.Width, m_bm.Height);
			Rectangle rcDstView = new Rectangle(AutoScrollPosition, WorldToViewSize(m_bm.Size));
			gT.InterpolationMode = InterpolationMode.NearestNeighbor;
			gT.PixelOffsetMode = PixelOffsetMode.Half;
			gT.DrawImage(m_bm, rcDstView, rcSrcWorld, GraphicsUnit.Pixel);
			if (g == null)
				gT.Dispose();
		}

		void CreateBitmap() {
			Size sizTile = GetTileSize();
			if (m_bm != null)
				m_bm.Dispose();
			m_bm = new Bitmap(m_lvld.Width * sizTile.Width, m_lvld.Height * sizTile.Height, PixelFormat.Format24bppRgb);
			ResetClientSize();
		}

		void ResetClientSize() {
			AutoScrollMinSize = WorldToViewSize(m_bm.Size);

			// Force it to recalc where it thinks the client size should be based on frame decorations,
			// and send a Resize event which'll cause us to recalc the client area

			Size sizSav = Size;
			Size sizT = new Size(sizSav.Width + 1, sizSav.Height + 1);
			Size = sizT;
			UpdateBounds();
			Size = sizSav;
			UpdateBounds();
		}

		private void LevelDoc_ImageChanged() {
			UpdateImage();
		}

		void UpdateImage() {
			Size sizTile = GetTileSize();
			if (sizTile.Width * Width != m_bm.Width || sizTile.Height * Height != m_bm.Height) {
				CreateBitmap();
			}
			Redraw();
		}

		private void LevelDoc_ItemsRemoved(IMapItem[] ami) {
			ArrayList alsmiSelected = m_lvld.Selection;
			foreach (IMapItem mi in ami) {
				alsmiSelected.Remove(mi);
				if (m_miContextMenu == mi)
					m_miContextMenu = null;
				if (m_miCapturedMouse == mi)
					m_miCapturedMouse = null;
				break;
			}
			m_lvld.Selection = alsmiSelected;
		}

		private void LevelView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
			m_fJustSelected = false;

			m_ptMouseDown = ViewToWorld(new Point(e.X - AutoScrollPosition.X, e.Y - AutoScrollPosition.Y));

			// Select / clear items
			IMapItem mi = m_lvld.HitTest(m_ptMouseDown.X, m_ptMouseDown.Y, GetTileSize(), GetTemplateDoc(), m_lyrf);
			if (e.Button == MouseButtons.Left) {
				if (mi == null) {
					// Clear selection
					m_lvld.Selection = new ArrayList();
					Globals.PropertyGrid.SelectedObject = null;
					m_ptDragSelectAnchor.X = m_ptMouseDown.X;
					m_ptDragSelectAnchor.Y = m_ptMouseDown.Y;
					m_rcDragSelect = new Rectangle(m_ptDragSelectAnchor, new Size(0, 0));
					m_fDragSelect = true;
				} else {
					m_rcDragStart.X = m_ptMouseDown.X - 5;
					m_rcDragStart.Y = m_ptMouseDown.Y - 5;
					m_rcDragStart.Width = 10;
					m_rcDragStart.Height = 10;
					
					// Add to selection. Extend if control is down
					ArrayList alsmiSelected = m_lvld.Selection;
					if (!alsmiSelected.Contains(mi)) {
						if ((Control.ModifierKeys & Keys.Control) != Keys.Control)
							alsmiSelected.Clear();
						alsmiSelected.Add(mi);
						Globals.PropertyGrid.SelectedObjects = (IMapItem[])alsmiSelected.ToArray(typeof(IMapItem));
						m_fJustSelected = true;
						m_lvld.Selection = alsmiSelected;
					}

					if (mi.OnMouseDown(e, m_ptMouseDown, GetTileSize(), GetTemplateDoc()))
						m_miCapturedMouse = mi;
				}
				Redraw();
			} else if (e.Button == MouseButtons.Right) {
				if (mi != null) {
					// If this is not already selected, then clear selection and add this
					if (!m_lvld.Selection.Contains(mi)) {
						ArrayList als = new ArrayList();
						als.Add(mi);
						m_lvld.Selection = als;
						Globals.PropertyGrid.SelectedObject = mi;
						Redraw();
					}

					if (mi.OnMouseDown(e, m_ptMouseDown, GetTileSize(), GetTemplateDoc())) {
						m_miCapturedMouse = mi;
						return;
					}

					m_miContextMenu = mi;
					m_contextMenu.Show(this, new Point(e.X, e.Y));
				}
			}
		}

		private Rectangle GetBoundingRect(IMapItem[] ami) {
			Rectangle rc = new Rectangle();
			TemplateDoc tmpd = GetTemplateDoc();
			Size sizTile = GetTileSize();
			foreach (IMapItem mi in ami) {
				int x = (int)(mi.tx * sizTile.Width);
				int y = (int)(mi.ty * sizTile.Height);
				rc = UnionRect(rc, mi.GetBoundingRectAt(x, y, sizTile, tmpd));
			}
			return rc;
		}

		private Rectangle GetSelectRect(int x, int y) {
			int cx = x - m_ptDragSelectAnchor.X;
			if (cx < 0)
				cx = -cx;
			int cy = y - m_ptDragSelectAnchor.Y;
			if (cy < 0)
				cy = -cy;
			int xT = x;
			if (m_ptDragSelectAnchor.X < x)
				xT = m_ptDragSelectAnchor.X;
			int yT = y;
			if (m_ptDragSelectAnchor.Y < y)
				yT = m_ptDragSelectAnchor.Y;
			return new Rectangle(xT, yT, cx, cy);
		}

		private Rectangle UnionRect(Rectangle rc1, Rectangle rc2) {
			if (rc1.IsEmpty)
				return new Rectangle(rc2.Location, rc2.Size);
			if (rc2.IsEmpty)
				return new Rectangle(rc1.Location, rc1.Size);
			int xLeft = rc1.Left < rc2.Left ? rc1.Left : rc2.Left;
			int yTop = rc1.Top < rc2.Top ? rc1.Top : rc2.Top;
			int xRight = rc1.Right > rc2.Right ? rc1.Right : rc2.Right;
			int yBottom = rc1.Bottom > rc2.Bottom ? rc1.Bottom : rc2.Bottom;
			return new Rectangle(xLeft, yTop, xRight - xLeft, yBottom - yTop);
		}

		private void LevelView_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e) {
			Point ptMouse = ViewToWorld(new Point(e.X - AutoScrollPosition.X, e.Y - AutoScrollPosition.Y));
			TemplateDoc tmpd = GetTemplateDoc();
			Size sizTile = GetTileSize();

			// Update status bar

			int tx = ptMouse.X / sizTile.Width;
			int ty = ptMouse.Y / sizTile.Height;
			Globals.StatusBar.Text = String.Format("Coords: {0}, {1}", tx, ty);

			// If we're dragging a selection, handle it here

			if (m_fDragSelect) {
				DragSelectExtend(e);
				return;
			}

			// Send input to MapItem, see if it wants it

			IMapItem mi = m_miCapturedMouse;
			if (mi == null)
				mi = m_lvld.HitTest(ptMouse.X, ptMouse.Y, sizTile, tmpd, m_lyrf);
			if (mi != null) {
				if (mi.OnMouseMove(e, ptMouse, sizTile, tmpd))
					return;
			}

			// We're not extending a selection. Perhaps we're initiating a drag drop operation
			// Check initiation conditions

			if (m_lvld.Selection.Count == 0)
				return;
			if (MouseButtons != MouseButtons.Left)
				return;
			if (m_rcDragStart.Contains(ptMouse.X, ptMouse.Y))
				return;

			PerformDragDrop(e, m_ptMouseDown);
		}

		private void LevelView_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e) {
			Point ptMouse = ViewToWorld(new Point(e.X - AutoScrollPosition.X, e.Y - AutoScrollPosition.Y));

			// If a MapItem has captured the mouse, give it a crack at the event

			IMapItem miT = m_miCapturedMouse;
			m_miCapturedMouse = null;
			if (miT != null) {
				if (miT.OnMouseUp(e, ptMouse, GetTileSize(), GetTemplateDoc()))
					return;
			}

			if (e.Button != MouseButtons.Left)
				return;
			if (m_fDragSelect) {
				m_fDragSelect = false;
				Graphics gWin = CreateGraphics();
				Rectangle rcSrcWorld = new Rectangle(m_rcDragSelect.Left, m_rcDragSelect.Top, m_rcDragSelect.Width, m_rcDragSelect.Height);
				rcSrcWorld.Inflate(1, 1);
				Rectangle rcDstView = new Rectangle(WorldToView(rcSrcWorld.Location), WorldToViewSize(rcSrcWorld.Size));
				rcDstView.Offset(AutoScrollPosition);				
				gWin.InterpolationMode = InterpolationMode.NearestNeighbor;
				gWin.PixelOffsetMode = PixelOffsetMode.Half;
				gWin.DrawImage(m_bm, rcDstView, rcSrcWorld, GraphicsUnit.Pixel);
				gWin.Dispose();
			}

			// Clear selected placement?
			if (m_fJustSelected) {
				m_fJustSelected = false;
				return;
			}
			IMapItem mi = m_lvld.HitTest(ptMouse.X, ptMouse.Y, GetTileSize(), GetTemplateDoc(), m_lyrf);
			if (mi == null)
				return;
			if ((Control.ModifierKeys & Keys.Control) != Keys.Control)
				return;
			ArrayList alsmiSelected = m_lvld.Selection;
            if (!alsmiSelected.Contains(mi))
				return;
			alsmiSelected.Remove(mi);
			m_lvld.Selection = alsmiSelected;
			Globals.PropertyGrid.SelectedObject = null;
			Redraw();
		}

		private void LevelView_DragOver(object sender, System.Windows.Forms.DragEventArgs e) {
			// As the drag occurs, we show what is being dragged. This is done by drawing
			// a properly aligned image on top of the view image as the drag occurs.

			// Get the data. It's not what we want then don't allow

			if (!e.Data.GetDataPresent(typeof(LevelData))) {
				e.Effect = DragDropEffects.None;
				return;
			}
			LevelData ldat = (LevelData)e.Data.GetData(typeof(LevelData));

			// Figure out where we want to place these map items

			PointF ptOrigin;
			Point ptClient = PointToClient(new Point(e.X, e.Y));
			Point ptMouse = ViewToWorld(new Point(ptClient.X - AutoScrollPosition.X, ptClient.Y - AutoScrollPosition.Y));
			PointF[] aptPlace = GetPlacementPoints2(ptMouse, ldat, out ptOrigin);

			// Calc the bounding rect of the map items

			Size sizTile = GetTileSize();
			Rectangle rcDragBoundsNew = GetBoundingRect(ldat.ami);
			rcDragBoundsNew.Offset((int)(ptOrigin.X * sizTile.Width), (int)(ptOrigin.Y * sizTile.Height));

			// If have an old drag image to erase, the drawing bounds needs to include it too so it
			// gets erased during this update

			Rectangle rcUnion = UnionRect(rcDragBoundsNew, m_rcDragBoundsLast);

			// See if our drag compose buffer is suitably sized. If not, recreate it

			if (m_bmDrag == null || m_bmDrag.Width < rcUnion.Width || m_bmDrag.Height < rcUnion.Height) {
				if (m_bmDrag != null)
					m_bmDrag.Dispose();
				m_bmDrag = new Bitmap(rcUnion.Width + rcUnion.Width / 2, rcUnion.Height + rcUnion.Height / 2, PixelFormat.Format24bppRgb);
			}

			// Copy this rectangle from the map bitmap into our drag compose buffer

			Graphics gMem = Graphics.FromImage(m_bmDrag);
			gMem.DrawImage(m_bm, 0, 0, rcUnion, GraphicsUnit.Pixel);

			// Now draw in map items
			
			TemplateDoc tmpd = GetTemplateDoc();
			for (LayerType layer = LayerType.Start; layer < LayerType.End; layer++) {
				for (int imi = 0; imi < ldat.ami.Length; imi++) {
					int x = (int)(aptPlace[imi].X * sizTile.Width);
					int y = (int)(aptPlace[imi].Y * sizTile.Height);
					ldat.ami[imi].Draw(gMem, x - rcUnion.X, y - rcUnion.Y, sizTile, tmpd, layer, true);
				}
			}

			// Put it on screen

			Graphics gWin = CreateGraphics();
			Rectangle rcSrcWorld = new Rectangle(0, 0, rcUnion.Width, rcUnion.Height);
			Rectangle rcDstView = new Rectangle(WorldToView(rcUnion.Location), WorldToViewSize(rcUnion.Size));
			rcDstView.Offset(AutoScrollPosition.X, AutoScrollPosition.Y);
			gWin.InterpolationMode = InterpolationMode.NearestNeighbor;
			gWin.PixelOffsetMode = PixelOffsetMode.Half;
			gWin.DrawImage(m_bmDrag, rcDstView, rcSrcWorld, GraphicsUnit.Pixel);
			gMem.Dispose();
			gWin.Dispose();

			// Remember old bounds

			m_rcDragBoundsLast = rcDragBoundsNew;
		}

		private void LevelView_DragEnter(object sender, System.Windows.Forms.DragEventArgs e) {
			m_rcDragBoundsLast = Rectangle.Empty;
			if (!e.Data.GetDataPresent(typeof(LevelData))) {
				e.Effect = DragDropEffects.None;
				return;
			}
			e.Effect = DragDropEffects.Copy | DragDropEffects.Move;
		}

		private void LevelView_DragLeave(object sender, System.EventArgs e) {
			// Update the on-screen drag image so that it gets erased properly
			// Copy appropriate part of the backing image

			Graphics gMem = Graphics.FromImage(m_bmDrag);
			gMem.DrawImage(m_bm, 0, 0, m_rcDragBoundsLast, GraphicsUnit.Pixel);

			// Now draw in on screen in the appropriate spot

			Graphics gWin = CreateGraphics();
			Rectangle rcSrcWorld = new Rectangle(0, 0, m_rcDragBoundsLast.Width, m_rcDragBoundsLast.Height);
			Rectangle rcDstView = new Rectangle(WorldToView(m_rcDragBoundsLast.Location), WorldToViewSize(m_rcDragBoundsLast.Size));
			rcDstView.Offset(AutoScrollPosition);
			gWin.InterpolationMode = InterpolationMode.NearestNeighbor;
			gWin.PixelOffsetMode = PixelOffsetMode.Half;
			gWin.DrawImage(m_bmDrag, rcDstView, rcSrcWorld, GraphicsUnit.Pixel);
			gMem.Dispose();
			gWin.Dispose();

			// Not being used any more

			m_bmDrag.Dispose();
			m_bmDrag = null;
			m_rcDragBoundsLast = Rectangle.Empty;
		}

		private void LevelView_DragDrop(object sender, System.Windows.Forms.DragEventArgs e) {
			// Get the data. It's not what we want then don't allow

			if (!e.Data.GetDataPresent(typeof(LevelData))) {
				e.Effect = DragDropEffects.None;
				return;
			}
			LevelData ldat = (LevelData)e.Data.GetData(typeof(LevelData));

			// Place map items

			Point ptClient = PointToClient(new Point(e.X, e.Y));
			Point ptMouse = ViewToWorld(new Point(ptClient.X - AutoScrollPosition.X, ptClient.Y - AutoScrollPosition.Y));
			PlaceMapItems(ptMouse, ldat);
			e.Effect = e.AllowedEffect;
		}

		void PlaceMapItems(Point ptMouse, LevelData ldat) {
			// Figure out where we want to place these map items

			PointF ptOrigin;
			PointF[] aptPlace = GetPlacementPoints2(ptMouse, ldat, out ptOrigin);

			// Set their positions

			IMapItem[] ami = new IMapItem[ldat.ami.Length];
			for (int imi = 0; imi < ldat.ami.Length; imi++) {
				ami[imi] = (IMapItem)ldat.ami[imi].Clone();
				ami[imi].tx = aptPlace[imi].X;
				ami[imi].ty = aptPlace[imi].Y;
			}

			// Add them to the level, make them selected

			m_lvld.AddMapItems(ami);
			ArrayList alsmiSelected = new ArrayList();
			alsmiSelected.AddRange(ami);
			m_lvld.Selection = alsmiSelected;
			Globals.PropertyGrid.SelectedObjects = (Object[])alsmiSelected.ToArray(typeof(Object));
		}

		PointF[] GetPlacementPoints2(Point ptMouse, LevelData ldat, out PointF ptOrigin) {
			Size sizTile = GetTileSize();
			double txOrigin = (ptMouse.X / (double)sizTile.Width) - ldat.txMouse;
			if (txOrigin < 0)
				txOrigin = 0;
			double tyOrigin = (ptMouse.Y / (double)sizTile.Height) - ldat.tyMouse;
			if (tyOrigin < 0)
				tyOrigin = 0;

			// Keep the new positions on the grid
			
			txOrigin = Math.Floor((txOrigin + ldat.Grid.Width / 2) / ldat.Grid.Width) * ldat.Grid.Width;
			tyOrigin = Math.Floor((tyOrigin + ldat.Grid.Height / 2) / ldat.Grid.Height) * ldat.Grid.Height;

			PointF[] aptPlace = new PointF[ldat.ami.Length];
			for (int imi = 0; imi < ldat.ami.Length; imi++) {
				double tx = txOrigin + ldat.ami[imi].tx;
				if (tx > m_lvld.Width - 1)
					tx = m_lvld.Width - 1;
				double ty = tyOrigin + ldat.ami[imi].ty;
				if (ty > m_lvld.Height - 1)
					ty = m_lvld.Height - 1;
				aptPlace[imi] = new PointF((float)tx, (float)ty);
			}

			ptOrigin = new PointF((float)txOrigin, (float)tyOrigin);
			return aptPlace;
		}

		LevelData PrepareLevelData(int x, int y, Size sizTile, IMapItem[] ami) {
			// Figure out relative spacing

			double txMin = double.MaxValue;
			double tyMin = double.MaxValue;
			foreach (IMapItem mi in ami) {
				if (mi.tx < txMin)
					txMin = mi.tx;
				if (mi.ty < tyMin)
					tyMin = mi.ty;
			}

			LevelData ldat = new LevelData();
			ldat.Grid.Width = 0.0000001f;
			ldat.Grid.Height = 0.0000001f;
			
			for (int imi = 0; imi < ami.Length; imi++) {
				IMapItem mi = (IMapItem)ami[imi];

				// Keep track of the maximum gridding required by the various items
				// NOTE: all the grids must be evenly divisible into the largest grid
				// or final placement of the items will have the indivisible ones
				// realigning themselves.

				ldat.Grid.Width = Math.Max(ldat.Grid.Width, mi.Grid.Width);
				ldat.Grid.Height = Math.Max(ldat.Grid.Height, mi.Grid.Height);
			}

			// Offset tx/yMin so the clones will be grid-aligned

			txMin -= Math.IEEERemainder(txMin, ldat.Grid.Width);
			tyMin -= Math.IEEERemainder(tyMin, ldat.Grid.Height);

			// Now clone the map items and readjust tile coordinates
			// for origin of 0,0 (or as close as we can get and still be grid-aligned)

			ldat.ami = new IMapItem[ami.Length];
			for (int imi = 0; imi < ami.Length; imi++) {
				IMapItem mi = (IMapItem)ami[imi].Clone();
				ldat.ami[imi] = mi;
				mi.tx -= txMin;
				mi.ty -= tyMin;
			}

			// Figure out mouse position relative to this origin

			int xOrigin = (int)(txMin * sizTile.Width);
			int yOrigin = (int)(tyMin * sizTile.Height);
			ldat.txMouse = (double)(x - xOrigin) / (double)sizTile.Width;
			ldat.tyMouse = (double)(y - yOrigin) / (double)sizTile.Height;

			// All done

			return ldat;
		}

		void PerformDragDrop(MouseEventArgs e, Point ptMouse) {
			// Remember what mi are selected. We may be deleting this if this is a move operation

			ArrayList alsmiSelected = m_lvld.Selection;
			IMapItem[] amiMove = (IMapItem[])alsmiSelected.ToArray(typeof(IMapItem));
			
			// Prepare a data object for drag drop

			LevelData ldat = PrepareLevelData(ptMouse.X, ptMouse.Y, GetTileSize(), (IMapItem[])alsmiSelected.ToArray(typeof(IMapItem)));

			// Normal operation is a move unless control key is press in which case it is
			// a copy

			DragDropEffects eff = DragDropEffects.Move;
			if ((Control.ModifierKeys & Keys.Control) == Keys.Control)
				eff = DragDropEffects.Copy;

			// Perform the drag drop. If it was cancelled, nothing to do.

			DragDropEffects effActual = DoDragDrop(ldat, eff);
			if (effActual == DragDropEffects.None)
				return;

			// If a move actually did occur, then remove the originals

			if ((effActual & DragDropEffects.Move) != 0) {
				m_lvld.RemoveMapItems(amiMove);
			}
		}

		void DragSelectExtend(MouseEventArgs e) {
			// Select all the mi inside

			Point ptMouse = ViewToWorld(new Point(e.X - AutoScrollPosition.X, e.Y - AutoScrollPosition.Y));
			Size sizTile = GetTileSize();
			TemplateDoc tmpd = GetTemplateDoc();
			Rectangle rcDragSelectNew = GetSelectRect(ptMouse.X, ptMouse.Y);
			ArrayList alsmiSelected = m_lvld.HitTest(rcDragSelectNew, GetTileSize(), GetTemplateDoc(), m_lyrf);
			Rectangle rcSelect = new Rectangle();
			ArrayList alsmiSelectedOld = m_lvld.Selection;
			if (!alsmiSelectedOld.Equals(alsmiSelected)) {
				rcSelect = GetBoundingRect((IMapItem[])alsmiSelectedOld.ToArray(typeof(IMapItem)));
				m_lvld.Selection = alsmiSelected;
				rcSelect = UnionRect(rcSelect, GetBoundingRect((IMapItem[])alsmiSelected.ToArray(typeof(IMapItem))));
				m_lvld.Draw(m_bm, null, sizTile, tmpd, m_lyrf);
				Globals.PropertyGrid.SelectedObjects = (Object[])alsmiSelected.ToArray(typeof(Object));
			}

			// Fill a buffer from background
			Rectangle rcDragUnion = UnionRect(rcSelect, UnionRect(rcDragSelectNew, m_rcDragSelect));

			// Expand by 1 so that the boundary is definitely inside. Problems with portions of
			// the drag boundary not erasing when scaled due to rounding errors

			rcDragUnion.Inflate(1, 1);
#if false
			if (rcDragUnion.IsEmpty)
				return;
#else
			if (rcDragUnion.Width == 0 || rcDragUnion.Height == 0)
				return;
#endif
			Bitmap bm = new Bitmap(rcDragUnion.Width, rcDragUnion.Height);
			Graphics gMem = Graphics.FromImage(bm);
			gMem.DrawImage(m_bm, 0, 0, rcDragUnion, GraphicsUnit.Pixel);

			// Draw in the drag selection
			Pen pen = new Pen(new SolidBrush(Color.Red));
			gMem.DrawRectangle(pen, rcDragSelectNew.X - rcDragUnion.X, rcDragSelectNew.Y - rcDragUnion.Y, rcDragSelectNew.Width - 1, rcDragSelectNew.Height - 1);
			gMem.Dispose();

			// Drag onto the screen
			Graphics gWinT = CreateGraphics();
			Rectangle rcSrcWorld = new Rectangle(0, 0, bm.Width, bm.Height);
			Point ptViewDst = WorldToView(rcDragUnion.Location);
			ptViewDst.Offset(AutoScrollPosition.X, AutoScrollPosition.Y);
			Rectangle rcDstView = new Rectangle(ptViewDst, WorldToViewSize(rcDragUnion.Size));
			gWinT.InterpolationMode = InterpolationMode.NearestNeighbor;
			gWinT.PixelOffsetMode = PixelOffsetMode.Half;
			gWinT.DrawImage(bm, rcDstView, rcSrcWorld, GraphicsUnit.Pixel);
			gWinT.Dispose();
			bm.Dispose();
			m_rcDragSelect = rcDragSelectNew;
		}

		private void LevelView_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e) {
			switch (e.KeyCode) {
			case Keys.Delete:
				Delete();
				break;

			case Keys.C:
				if ((Control.ModifierKeys & Keys.Control) != 0)
					Copy();
				break;

			case Keys.X:
				if ((Control.ModifierKeys & Keys.Control) != 0)
					Cut();
				break;

			case Keys.V:
				if ((Control.ModifierKeys & Keys.Control) != 0)
					Paste();
				break;

			case Keys.OemMinus:
				if ((Control.ModifierKeys & Keys.Control) != 0) {
					SetScale(1.0f);
				} else {
					SetScale(m_flScale - 0.25f);
				}
				break;

			case Keys.Oemplus:
				if ((Control.ModifierKeys & Keys.Control) != 0) {
					SetScale(1.0f);
				} else {
					SetScale(m_flScale + 0.25f);
				}
				break;
			}
		}

		public void Delete() {
			RemoveSelectedItems();
		}

		public void Copy() {
			ArrayList alsmiSelected = m_lvld.Selection;
			if (alsmiSelected.Count != 0) {
				IMapItem[] ami = (IMapItem[])alsmiSelected.ToArray(typeof(IMapItem));
				Rectangle rc = GetBoundingRect(ami);
				LevelData ldat = PrepareLevelData(rc.X + rc.Width / 2, rc.Y + rc.Height / 2, GetTileSize(), ami);
				//Clipboard.SetDataObject(ldat); doesn't work for some reason
				Clipboard.SetDataObject(ldat.ami);
			}
		}

		public void Cut() {
			if (m_lvld.Selection.Count != 0) {
				Copy();
				RemoveSelectedItems();
			}
		}

		public void Paste() {
			// IMapItems present?

			IDataObject data = Clipboard.GetDataObject();
			if (!data.GetDataPresent(typeof(IMapItem[])))
				return;

			// For some reason the clipboard data object won't return a LevelData

			LevelData ldat = new LevelData();
			ldat.ami = (IMapItem[])data.GetData(typeof(IMapItem[]));
			Rectangle rc = GetBoundingRect(ldat.ami);
			Size sizTile = GetTileSize();
			ldat.txMouse = (double)rc.Width / (double)sizTile.Width;
			ldat.tyMouse = (double)rc.Height / (double)sizTile.Height;

			ldat.Grid.Width = 0.0000001f;
			ldat.Grid.Height = 0.0000001f;
			foreach (IMapItem mi in ldat.ami) {
				ldat.Grid.Width = Math.Max(mi.Grid.Width, ldat.Grid.Width);
				ldat.Grid.Height = Math.Max(mi.Grid.Height, ldat.Grid.Height);
			}

			// Place map items

			Point ptViewCenter = new Point(ClientSize.Width / 2, ClientSize.Height / 2);
			Point ptMouse = ViewToWorld(new Point(ptViewCenter.X - AutoScrollPosition.X, ptViewCenter.Y - AutoScrollPosition.Y));
			PlaceMapItems(ptMouse, ldat);
		}

		public void RemoveSelectedItems() {
			IMapItem[] ami = (IMapItem[])m_lvld.Selection.ToArray(typeof(IMapItem));
			m_lvld.Selection = new ArrayList();
			Globals.PropertyGrid.SelectedObject = null;
			m_lvld.RemoveMapItems(ami);
		}

		public void CheckSize() {
			SuspendLayout();
			Size sizBitmapView = WorldToViewSize(m_bm.Size);
			if (ClientSize.Width > sizBitmapView.Width)
				Width += sizBitmapView.Width - ClientSize.Width;
			if (ClientSize.Height > sizBitmapView.Height)
				Height += sizBitmapView.Height - ClientSize.Height;
			ResumeLayout(false);
		}

		private void LevelView_Resize(object sender, System.EventArgs e) {
			// Resize client area around m_bm

			CheckSize();

			// Reposition scrollbars to last mid point

			Size sizBitmapView = WorldToViewSize(m_bm.Size);
			int xViewMap = (int)(m_xRatioView * (float)sizBitmapView.Width);
			if (ClientSize.Width >= sizBitmapView.Width)
				xViewMap = 0;
			int yViewMap = (int)(m_yRatioView * (float)sizBitmapView.Height);
			if (ClientSize.Height >= sizBitmapView.Height)
				yViewMap = 0;
			AutoScrollPosition = new Point(xViewMap, yViewMap);
		}

		private void menuItemSelectSame_Click(object sender, System.EventArgs e) {
			ArrayList alsmiSelected = new ArrayList();
			foreach (IMapItem mi in m_lvld.MapItems) {
				if (mi.GetType() != m_miContextMenu.GetType())
					continue;
				if (m_miContextMenu is Tile) {
					if (mi is Tile) {
						if (((Tile)mi).Name != ((Tile)m_miContextMenu).Name)
							continue;
					}
				}
				alsmiSelected.Add(mi);
			}
			m_lvld.Selection = alsmiSelected;
			Redraw();
		}

		private void menuItemRemove_Click(object sender, System.EventArgs e) {
			RemoveSelectedItems();
		}
	}

	public class LevelData {
		public double txMouse;
		public double tyMouse;
		public IMapItem[] ami;
		public SizeF Grid;
	}
}
