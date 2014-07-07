using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for StripsForm.
	/// </summary>
	public class StripsForm : System.Windows.Forms.Form
	{
		private AnimDoc m_doc;
		private StripSet m_stps;
		private Strip m_stpActive;
		private System.Windows.Forms.ListView lstv;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.MenuItem mniView;
		private System.Windows.Forms.MenuItem mniViewDetails;
		private System.Windows.Forms.MenuItem mniViewList;
		private System.Windows.Forms.MenuItem mniViewThumbnails;
		private System.Windows.Forms.ContextMenu mnuListView;
		private System.Windows.Forms.MenuItem mniNewStrip;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem mniDelete;
		private System.Windows.Forms.MenuItem mniRename;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem mniProperties;
		private System.Windows.Forms.MenuItem mniStripsSort;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public StripsForm(AnimDoc doc)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			// My constructor code

			Globals.ActiveDocumentChanged += new EventHandler(OnActiveDocumentChanged);
			Globals.ActiveStripChanged += new EventHandler(OnActiveStripChanged);
			m_doc = doc;
			m_stps = m_doc.StripSet;
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
			this.mniViewList = new System.Windows.Forms.MenuItem();
			this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
			this.mniView = new System.Windows.Forms.MenuItem();
			this.mniViewThumbnails = new System.Windows.Forms.MenuItem();
			this.mniViewDetails = new System.Windows.Forms.MenuItem();
			this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
			this.mniRename = new System.Windows.Forms.MenuItem();
			this.mnuListView = new System.Windows.Forms.ContextMenu();
			this.mniNewStrip = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.mniDelete = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.mniProperties = new System.Windows.Forms.MenuItem();
			this.lstv = new System.Windows.Forms.ListView();
			this.mniStripsSort = new System.Windows.Forms.MenuItem();
			this.SuspendLayout();
			// 
			// mniViewList
			// 
			this.mniViewList.Index = 0;
			this.mniViewList.Text = "List";
			this.mniViewList.Click += new System.EventHandler(this.mniViewList_Click);
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "# frames";
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
			this.columnHeader1.Width = 90;
			// 
			// mniRename
			// 
			this.mniRename.Index = 3;
			this.mniRename.Text = "Rename";
			this.mniRename.Click += new System.EventHandler(this.mniRename_Click);
			// 
			// mnuListView
			// 
			this.mnuListView.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						this.mniNewStrip,
																						this.menuItem2,
																						this.mniDelete,
																						this.mniRename,
																						this.menuItem1,
																						this.mniView,
																						this.mniStripsSort,
																						this.menuItem3,
																						this.mniProperties});
			this.mnuListView.Popup += new System.EventHandler(this.mnuListView_Popup);
			// 
			// mniNewStrip
			// 
			this.mniNewStrip.Index = 0;
			this.mniNewStrip.Text = "New Strip";
			this.mniNewStrip.Click += new System.EventHandler(this.mniNewStrip_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 1;
			this.menuItem2.Text = "-";
			// 
			// mniDelete
			// 
			this.mniDelete.Index = 2;
			this.mniDelete.Shortcut = System.Windows.Forms.Shortcut.Del;
			this.mniDelete.Text = "Delete";
			this.mniDelete.Click += new System.EventHandler(this.mniDelete_Click);
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 4;
			this.menuItem1.Text = "-";
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 7;
			this.menuItem3.Text = "-";
			// 
			// mniProperties
			// 
			this.mniProperties.Index = 8;
			this.mniProperties.Text = "&Properties";
			this.mniProperties.Click += new System.EventHandler(this.mniProperties_Click);
			// 
			// lstv
			// 
			this.lstv.AllowColumnReorder = true;
			this.lstv.AllowDrop = true;
			this.lstv.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																				   this.columnHeader1,
																				   this.columnHeader2});
			this.lstv.ContextMenu = this.mnuListView;
			this.lstv.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lstv.FullRowSelect = true;
			this.lstv.HideSelection = false;
			this.lstv.LabelEdit = true;
			this.lstv.MultiSelect = false;
			this.lstv.Name = "lstv";
			this.lstv.Size = new System.Drawing.Size(320, 309);
			this.lstv.TabIndex = 0;
			this.lstv.View = System.Windows.Forms.View.Details;
			this.lstv.ItemActivate += new System.EventHandler(this.lstv_ItemActivate);
			this.lstv.DoubleClick += new System.EventHandler(this.lstv_DoubleClick);
			this.lstv.DragDrop += new System.Windows.Forms.DragEventHandler(this.lstv_DragDrop);
			this.lstv.AfterLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.lstv_AfterLabelEdit);
			this.lstv.DragEnter += new System.Windows.Forms.DragEventHandler(this.lstv_DragEnter);
			this.lstv.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.lstv_ItemDrag);
			this.lstv.BeforeLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.lstv_BeforeLabelEdit);
			this.lstv.SelectedIndexChanged += new System.EventHandler(this.lstv_SelectedIndexChanged);
			// 
			// mniStripsSort
			// 
			this.mniStripsSort.Index = 6;
			this.mniStripsSort.Text = "&Sort";
			this.mniStripsSort.Click += new System.EventHandler(this.mniStripsSort_Click);
			// 
			// StripsForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(320, 309);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.lstv});
			this.Name = "StripsForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
			this.Text = "Strips";
			this.ResumeLayout(false);

		}
		#endregion

		private void OnActiveDocumentChanged(object obSender, EventArgs e) {
			m_doc = Globals.ActiveDocument;
			m_stps = m_doc.StripSet;
			RefreshView();
		}

		private void OnActiveStripChanged(object obSender, EventArgs e) {
			if (m_stpActive == Globals.ActiveStrip)
				return;

			m_stpActive = Globals.ActiveStrip;
			foreach (ListViewItem lvi in lstv.Items) {
				if (lvi.Tag == Globals.ActiveStrip) {
					lvi.Selected = true;
					break;
				}
			}
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

		private void mniViewDetails_Click(object sender, System.EventArgs e) {
			lstv.View = View.Details;
		}

		private void mniViewThumbnails_Click(object sender, System.EventArgs e) {
			lstv.View = View.LargeIcon;
		}

		private void mniViewList_Click(object sender, System.EventArgs e) {
			lstv.View = View.List;
		}

		private void RefreshView() {
			lstv.Items.Clear();
			if (m_stps == null)
				return;

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
			foreach (Strip stp in m_stps) {
				ListViewItem lvi = lstv.Items.Add(stp.Name);
				lvi.Tag = stp;

				if (stp == Globals.ActiveStrip)
					lvi.Selected = true;

				lvi.SubItems.Add(stp.Count.ToString());

				// UNDONE: cook up better thumbnails
				// UNDONE: have a 'blank' thumbnail for empty Strips

				if (stp.Count > 0) {
					if (stp[0].BitmapPlacers.Count > 0) {
                        XBitmap xbm = stp[0].BitmapPlacers[0].XBitmap;
						Bitmap bmThumb = xbm.MakeThumbnail(64, 64, false);
						lstv.LargeImageList.Images.Add(bmThumb);
						bmThumb = xbm.MakeThumbnail(16, 16, false);
						lstv.SmallImageList.Images.Add(bmThumb);
						lvi.ImageIndex = i++;
					}
				}
			}
		}

		private void mniNewStrip_Click(object sender, System.EventArgs e) {
			NewStrip();
		}

		public void NewStrip() {
			Strip stp = new Strip("untitled " + (m_stps.Count + 1).ToString());
			// UNDONE: undo
			m_stps.Add(stp);
			Globals.ActiveStrip = stp;
			m_doc.Dirty = true;
			RefreshView();
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

			Strip stp = (Strip)lstv.Items[e.Item].Tag;

			// No change or an invalid change

			if (e.Label == null || e.Label == "") {
				e.CancelEdit = true;
				return;
			}

			stp.Name = e.Label;
			m_doc.Dirty = true;
		}

		private void mnuListView_Popup(object sender, System.EventArgs e) {
			bool fAnySelected = lstv.SelectedItems.Count != 0;
			mniDelete.Enabled = fAnySelected;
			mniRename.Enabled = fAnySelected;
			mniProperties.Enabled = fAnySelected;
		}

		private void mniDelete_Click(object sender, System.EventArgs e) {
			Strip stp = (Strip)lstv.SelectedItems[0].Tag;
			int i = m_stps.IndexOf(stp);
			// UNDONE: undo
			m_stps.RemoveAt(i);
			lstv.Items.Remove(lstv.SelectedItems[0]);
			m_doc.Dirty = true;

			// Select a new Strip to be active
			
			i = Math.Min(i, m_stps.Count - 1);
			if (i >= 0)
				Globals.ActiveStrip = m_stps[i];
			else
				Globals.ActiveStrip = null;
		}

		private void mniRename_Click(object sender, System.EventArgs e) {
			MessageBox.Show(this, "Click on the name of the strip to select it then edit it in place.", 
				"AniMax");
		}

		private void lstv_ItemActivate(object sender, System.EventArgs e) {
//			m_doc.ActiveStrip = (Strip)lstv.SelectedItems[0].Tag;
		}

		private void lstv_DragEnter(object sender, System.Windows.Forms.DragEventArgs e) {
			if (e.Data.GetDataPresent(DataFormats.FileDrop))
				e.Effect = DragDropEffects.Copy;
		}

		private void lstv_DragDrop(object sender, System.Windows.Forms.DragEventArgs e) {
			string[] astrFiles = e.Data.GetData(DataFormats.FileDrop) as string[];
			if (astrFiles != null) {
				if (astrFiles.Length == 1)
					((MainForm)Globals.MainForm).CloseAndOpenDocument(astrFiles[0]);
				else
					MessageBox.Show(this, "Please, only drop one animation file.", "Too many files dropped", MessageBoxButtons.OK, MessageBoxIcon.Stop);
			}
#if false
			XBitmap[] axbm = e.Data.GetData(typeof(XBitmap[])) as XBitmap[];
			if (axbm == null)
				return;

			// Use the standard file naming convention to break the bitmaps down into
			// animations (e.g., *_fire_*_*), a single track (e.g., track 1), and a set
			// of TSprite keys (e.g., *_*_d_n)

			foreach (XBitmap xbm in axbm) {
				string strFile = xbm.FileName;
				string[] astr = strFile.Substring(strFile.LastIndexOf('\\') + 1).Split('_', '.');
				if (astr.Length != 5) {
					MessageBox.Show(this, String.Format("Warning: file {0} does not match the requisite naming pattern", strFile), 
							"AniMax");
					continue;
				}

				// Find the appropriate animation

				Strip stp = null;
				Frame fr = null;
				string strName = astr[1] + " " + astr[2];
				foreach (Strip stpT in m_stps) {
					if (stpT.Name == strName) {
						stp = stpT;
						fr = stp[0];
						break;
					}
				}

				// If one isn't found, create a new stpmation and empty track

				if (stp == null) {
					stp = new Strip(strName);
					m_stps.Add(stp);
					tspr = new TSprite("track 1");
					stp.Add(tspr);
				}

				Sprite spr = new Sprite(xbm, 0, 0);
				Time t = 0;
				if (tspr.End != Time.tUndefined)
					t = tspr.End + m_doc.FrameRate;
				tspr.SetValue(t, spr);
			}

			m_doc.Dirty = true;
			RefreshView();
#endif
		}

		private void lstv_SelectedIndexChanged(object sender, System.EventArgs e) {
			if (lstv.SelectedItems.Count == 0)
				return;
			Globals.ActiveStrip = (Strip)lstv.SelectedItems[0].Tag;
		}

		private void lstv_DoubleClick(object sender, System.EventArgs e) {
			((StripForm)Globals.StripForm).ShowStripProperties((Strip)lstv.SelectedItems[0].Tag);
		}

		private void mniProperties_Click(object sender, System.EventArgs e) {
			((StripForm)Globals.StripForm).ShowStripProperties((Strip)lstv.SelectedItems[0].Tag);
		}

		private void lstv_ItemDrag(object sender, System.Windows.Forms.ItemDragEventArgs e) {
			DoDragDrop(lstv.SelectedItems[0].Tag, DragDropEffects.All);
		}

		private void mniStripsSort_Click(object sender, System.EventArgs e) {
			m_stps.Sort();
			RefreshView();
		}
	}
}
