using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for BitmapsForm.
	/// </summary>
	public class BitmapsForm : System.Windows.Forms.Form
	{
		private AnimDoc m_doc;
		private XBitmapSet m_xbms;
		private System.Windows.Forms.ListView lstv;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.ColumnHeader columnHeader5;
		private System.Windows.Forms.ContextMenu mnuListView;
		private System.Windows.Forms.MenuItem mniViewList;
		private System.Windows.Forms.MenuItem mniViewDetails;
		private System.Windows.Forms.MenuItem mniViewThumbnails;
		private System.Windows.Forms.MenuItem mniView;
		private System.Windows.Forms.MenuItem mniAddBitmap;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.OpenFileDialog openFileDialog;
		private System.Windows.Forms.MenuItem mniDelete;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem mniRename;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public BitmapsForm(AnimDoc doc)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			// My constructor code

			Globals.ActiveDocumentChanged += new EventHandler(OnActiveDocumentChanged);
			m_doc = doc;
			if (m_doc != null)
				m_xbms = doc.XBitmapSet;
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
			this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.mniViewList = new System.Windows.Forms.MenuItem();
			this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
			this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
			this.mniView = new System.Windows.Forms.MenuItem();
			this.mniViewThumbnails = new System.Windows.Forms.MenuItem();
			this.mniViewDetails = new System.Windows.Forms.MenuItem();
			this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
			this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
			this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
			this.mnuListView = new System.Windows.Forms.ContextMenu();
			this.mniAddBitmap = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.mniDelete = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.lstv = new System.Windows.Forms.ListView();
			this.mniRename = new System.Windows.Forms.MenuItem();
			this.SuspendLayout();
			// 
			// openFileDialog
			// 
			this.openFileDialog.DefaultExt = "png";
			this.openFileDialog.Filter = "All bitmaps (*.bmp,*.jpg,*.png,*.gif,*.tif)|*.png;*.tif;*.gif;*.jpg;*.bmp|All fil" +
				"es|*.*";
			this.openFileDialog.Multiselect = true;
			this.openFileDialog.Title = "Add Bitmaps";
			// 
			// mniViewList
			// 
			this.mniViewList.Index = 0;
			this.mniViewList.Text = "List";
			this.mniViewList.Click += new System.EventHandler(this.mniViewList_Click);
			// 
			// columnHeader3
			// 
			this.columnHeader3.Text = "Height";
			this.columnHeader3.Width = 44;
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Width";
			this.columnHeader2.Width = 40;
			// 
			// mniView
			// 
			this.mniView.Index = 5;
			this.mniView.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mniViewList,
																					this.mniViewThumbnails,
																					this.mniViewDetails});
			this.mniView.Text = "View";
			this.mniView.Popup += new System.EventHandler(this.mniView_Popup);
			// 
			// mniViewThumbnails
			// 
			this.mniViewThumbnails.Index = 1;
			this.mniViewThumbnails.Text = "Thumbnails";
			this.mniViewThumbnails.Click += new System.EventHandler(this.mniViewThumbnails_Click);
			// 
			// mniViewDetails
			// 
			this.mniViewDetails.Index = 2;
			this.mniViewDetails.Text = "Details";
			this.mniViewDetails.Click += new System.EventHandler(this.mniViewDetails_Click);
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Name";
			this.columnHeader1.Width = 125;
			// 
			// columnHeader5
			// 
			this.columnHeader5.Text = "Date Modified";
			this.columnHeader5.Width = 120;
			// 
			// columnHeader4
			// 
			this.columnHeader4.Text = "Size";
			this.columnHeader4.Width = 46;
			// 
			// mnuListView
			// 
			this.mnuListView.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						this.mniAddBitmap,
																						this.menuItem3,
																						this.mniDelete,
																						this.mniRename,
																						this.menuItem2,
																						this.mniView});
			this.mnuListView.Popup += new System.EventHandler(this.mnuListView_Popup);
			// 
			// mniAddBitmap
			// 
			this.mniAddBitmap.Index = 0;
			this.mniAddBitmap.Text = "Add Bitmap...";
			this.mniAddBitmap.Click += new System.EventHandler(this.mniAddBitmap_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 1;
			this.menuItem3.Text = "-";
			// 
			// mniDelete
			// 
			this.mniDelete.Index = 2;
			this.mniDelete.Shortcut = System.Windows.Forms.Shortcut.Del;
			this.mniDelete.Text = "Delete";
			this.mniDelete.Click += new System.EventHandler(this.mniDelete_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 4;
			this.menuItem2.Text = "-";
			// 
			// lstv
			// 
			this.lstv.AllowColumnReorder = true;
			this.lstv.AllowDrop = true;
			this.lstv.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																				   this.columnHeader1,
																				   this.columnHeader2,
																				   this.columnHeader3,
																				   this.columnHeader4,
																				   this.columnHeader5});
			this.lstv.ContextMenu = this.mnuListView;
			this.lstv.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstv.FullRowSelect = true;
			this.lstv.LabelEdit = true;
			this.lstv.Name = "lstv";
			this.lstv.Size = new System.Drawing.Size(312, 389);
			this.lstv.Sorting = System.Windows.Forms.SortOrder.Ascending;
			this.lstv.TabIndex = 0;
			this.lstv.View = System.Windows.Forms.View.Details;
			this.lstv.DragDrop += new System.Windows.Forms.DragEventHandler(this.lstv_DragDrop);
			this.lstv.AfterLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.lstv_AfterLabelEdit);
			this.lstv.DragEnter += new System.Windows.Forms.DragEventHandler(this.lstv_DragEnter);
			this.lstv.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.lstv_ItemDrag);
			this.lstv.BeforeLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.lstv_BeforeLabelEdit);
			// 
			// mniRename
			// 
			this.mniRename.Index = 3;
			this.mniRename.Text = "Rename";
			// 
			// BitmapsForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(312, 389);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.lstv});
			this.Name = "BitmapsForm";
			this.ShowInTaskbar = false;
			this.Text = "Bitmaps";
			this.ResumeLayout(false);

		}
		#endregion

		private void OnActiveDocumentChanged(object obSender, EventArgs e) {
			m_doc = Globals.ActiveDocument;
			m_xbms = m_doc != null ? m_doc.XBitmapSet : null;
			RefreshView();
		}

		private void mnuListView_Popup(object sender, System.EventArgs e) {
			mniDelete.Enabled = lstv.SelectedItems.Count != 0;
		}

		private void mniViewDetails_Click(object sender, System.EventArgs e) {
			lstv.View = View.Details;
		}

		private void mniViewThumbnails_Click(object sender, System.EventArgs e) {
			lstv.View = View.LargeIcon;
		}

		private void mniViewList_Click(object sender, System.EventArgs e) {
			lstv.View = View.List;
		}

		private void mniView_Popup(object sender, System.EventArgs e) {
			mniViewList.Checked = false;
			mniViewThumbnails.Checked = false;
			mniViewDetails.Checked = false;
			
			switch (lstv.View) {
			case View.List:
				mniViewList.Checked = true;
				break;

			case View.LargeIcon:
				mniViewThumbnails.Checked = true;
				break;

			case View.Details:
				mniViewDetails.Checked = true;
				break;
			}
		}

		private void lstv_DragEnter(object sender, System.Windows.Forms.DragEventArgs e) {
			bool fFileNamePresent = e.Data.GetDataPresent(DataFormats.FileDrop);
			if (fFileNamePresent)
				e.Effect = DragDropEffects.All;
			else
				e.Effect = DragDropEffects.None;
		}

		private void lstv_DragDrop(object sender, System.Windows.Forms.DragEventArgs e) {
			bool fFileNamePresent = e.Data.GetDataPresent(DataFormats.FileDrop);
			if (fFileNamePresent) {
				string[] astr = (string[])e.Data.GetData(DataFormats.FileDrop);
				foreach (string strT in astr) {
					m_xbms.Add(strT);
				}
				m_doc.Dirty = true;
			}
			RefreshView();
		}

		private void RefreshView() {
			lstv.Items.Clear();

			// Prep LargeImageList

			if (lstv.LargeImageList != null)
				lstv.LargeImageList.Dispose();
			lstv.LargeImageList = new ImageList();
			lstv.LargeImageList.ColorDepth = ColorDepth.Depth32Bit;
			lstv.LargeImageList.ImageSize = new Size(64, 64);

			// Prep SmallImageList

			if (lstv.SmallImageList != null)
				lstv.SmallImageList.Dispose();
			lstv.SmallImageList = new ImageList();
			lstv.SmallImageList.ColorDepth = ColorDepth.Depth32Bit;
			lstv.SmallImageList.ImageSize = new Size(16, 16);

			int i = 0;
			foreach (XBitmap xbm in m_xbms) {
				ListViewItem lvi = lstv.Items.Add(Path.GetFileName(xbm.FileName));
				lvi.Tag = xbm;
				lvi.SubItems.Add(xbm.Width.ToString());
				lvi.SubItems.Add(xbm.Height.ToString());
				try {
					FileInfo fili = new FileInfo(xbm.FileName);
					lvi.SubItems.Add(fili.Length.ToString("#,##0"));
					lvi.SubItems.Add(fili.LastWriteTime.ToShortDateString() + " " + fili.LastWriteTime.ToShortTimeString());
				} catch (FileNotFoundException) {
					// UNDONE: deal with this!
				}

				// Make a better thumbnail

				Bitmap bmLarge = xbm.MakeThumbnail(64, 64, false);

				lstv.LargeImageList.Images.Add(bmLarge);
				lstv.SmallImageList.Images.Add(bmLarge);
				lvi.ImageIndex = i++;
			}
		}

		private void mniAddBitmap_Click(object sender, System.EventArgs e) {
			if (openFileDialog.ShowDialog() != DialogResult.OK)
				return;

			foreach (string strFileName in openFileDialog.FileNames) {
				m_xbms.Add(strFileName);
			}
			m_doc.Dirty = true;
			RefreshView();
		}

		private void mniDelete_Click(object sender, System.EventArgs e) {
			XBitmap xbm = (XBitmap)lstv.SelectedItems[0].Tag;
			int i = m_xbms.IndexOf(xbm);
			// UNDONE: undo
			m_xbms.RemoveAt(i);
			lstv.Items.Remove(lstv.SelectedItems[0]);
			m_doc.Dirty = true;
		}

		private void lstv_ItemDrag(object sender, System.Windows.Forms.ItemDragEventArgs e) {
			XBitmap[] axbm = new XBitmap[lstv.SelectedItems.Count];
			for (int i = 0; i < lstv.SelectedItems.Count; i++)
				axbm[i] = (XBitmap)lstv.SelectedItems[i].Tag;
			DoDragDrop(axbm, DragDropEffects.All);
		}

		private void lstv_BeforeLabelEdit(object sender, System.Windows.Forms.LabelEditEventArgs e) {
			// We do this so the user can press the del key while in label editing mode
			// without it being intercepted by the context menu and used to delete the
			// entire Strip

			lstv.ContextMenu = null;
		}

		private void lstv_AfterLabelEdit(object sender, System.Windows.Forms.LabelEditEventArgs e) {

			// Restore the context menu so the command keys, etc will work again

			lstv.ContextMenu = mnuListView;

			XBitmap xbm = (XBitmap)lstv.Items[e.Item].Tag;

			// No change or an invalid change

			if (e.Label == null || e.Label == "") {
				e.CancelEdit = true;
				return;
			}

			xbm.FileName = e.Label;
			m_doc.Dirty = true;
		}
	}
}
