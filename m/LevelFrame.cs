using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Diagnostics;

namespace m
{
	public class LevelFrame : System.Windows.Forms.Form, ICommandTarget
	{
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.Splitter splitter;
		Document m_doc;
		LevelViewParent m_viewTop;
		LevelViewParent m_viewBottom;
		float m_nSplitRatio;
		static ArrayList s_alsFrames = new ArrayList();
		bool m_fNoRecurse = false;

		public LevelFrame(Form frmParent, Document doc, Type typeView) {
			InitializeComponent();

			LevelDocTemplate doct = (LevelDocTemplate)DocManager.FindDocTemplate(typeof(LevelDoc));
			doct.DocActive += new LevelDocTemplate.DocActiveHandler(LevelDocTemplate_DocActive);

			m_doc = doc;

			doc.PathChanged += new Document.PathChangedHandler(Document_PathChanged);
			doc.ModifiedChanged += new Document.ModifiedChangedHandler(Document_ModifiedChanged);
			doc.OpenCountChanged += new Document.OpenCountChangedHandler(Document_OpenCountChanged);
			((LevelDoc)doc).NameChanged += new LevelDoc.NameChangedHandler(LevelDoc_NameChanged);
			
			// Parent this and create panes

			MdiParent = frmParent;
			ChangePanes(2);

			// See if the top most mdi frame is maximized. If so, maximize this too
			// If no window around, maximize

			bool fMaximize = true;
			if (frmParent.ActiveMdiChild != null) {
				if (frmParent.ActiveMdiChild.WindowState != FormWindowState.Maximized)
					fMaximize = false;
			}
			if (fMaximize)
				WindowState = FormWindowState.Maximized;

			// Set Title

			s_alsFrames.Add(this);
			SetTitle();

			// If this doc is active, this is the new command target

			if (m_doc == DocManager.GetActiveDocument(typeof(LevelDoc)))
				DocManager.SetCommandTarget(this);

			// Show

			Show();
		}

		public void DispatchCommand(Command cmd) {
            if (m_viewTop.ContainsFocus) {
				m_viewTop.DispatchCommand(cmd);
			}
			if (m_viewBottom.ContainsFocus) {
				m_viewBottom.DispatchCommand(cmd);
			}
		}

		void Document_ModifiedChanged(Document doc, bool fModified) {
			SetTitle();
		}

		void LevelDoc_NameChanged(LevelDoc lvld) {
			SetTitle();
		}

		void Document_PathChanged(Document doc) {
			SetTitle();
		}

		void Document_OpenCountChanged(Document doc) {
			SetTitle();
		}

		void SetTitle() {
			// Set the window title

			string strTitle = m_doc.GetName();
			string strPath = m_doc.GetPath();
			if (strPath != null)
				strTitle += " (" + strPath + ")";
			if (m_doc.GetOpenCount() > 1)
				strTitle += ":" + (s_alsFrames.IndexOf(this) + 1);
			if (m_doc.IsModified())
				strTitle += "*";
			Text = strTitle;
		}

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
			this.splitter = new System.Windows.Forms.Splitter();
			this.SuspendLayout();
			// 
			// splitter
			// 
			this.splitter.BackColor = System.Drawing.Color.PapayaWhip;
			this.splitter.Dock = System.Windows.Forms.DockStyle.Top;
			this.splitter.MinExtra = 17;
			this.splitter.MinSize = 0;
			this.splitter.Name = "splitter";
			this.splitter.Size = new System.Drawing.Size(634, 5);
			this.splitter.TabIndex = 1;
			this.splitter.TabStop = false;
			this.splitter.SplitterMoved += new System.Windows.Forms.SplitterEventHandler(this.splitter_SplitterMoved);
			this.splitter.DoubleClick += new System.EventHandler(this.splitter_DoubleClick);
			// 
			// LevelFrame
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.BackColor = System.Drawing.SystemColors.AppWorkspace;
			this.ClientSize = new System.Drawing.Size(634, 360);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.splitter});
			this.Name = "LevelFrame";
			this.Text = "LevelFrame";
			this.Closing += new System.ComponentModel.CancelEventHandler(this.LevelFrame_Closing);
			this.SizeChanged += new System.EventHandler(this.LevelFrame_SizeChanged);
			this.ResumeLayout(false);

		}
		#endregion

		public Document GetDocument() {
			return m_doc;
		}

		void ChangePanes(int nPaneStateNew) {
			// If starting, create panes and views

			if (m_viewTop == null && m_viewBottom == null) {
				// Bottom full size

				m_viewBottom = new LevelViewParent();
				m_viewBottom.SetDocument(m_doc);
				m_viewBottom.Dock = DockStyle.Fill;
				Controls.Add(m_viewBottom);

				// Top

				m_viewTop = new LevelViewParent();
				m_viewTop.SetDocument(m_doc);
				m_viewTop.Dock = DockStyle.Top;
				Controls.Add(m_viewTop);

				// Set right order for auto formatting

				Controls.SetChildIndex(m_viewBottom, 0);
				Controls.SetChildIndex(splitter, 1);
				Controls.SetChildIndex(m_viewTop, 2);

				// If state 2 then top pane is zero height

				if (nPaneStateNew == 2)
					splitter.SplitPosition = 0;
				return;
			}

			// If closing remove panes and views

			if (nPaneStateNew == 0) {
				m_viewTop.Dispose();
				Controls.Remove(m_viewTop);
				m_viewTop = null;
				m_viewBottom.Dispose();
				Controls.Remove(m_viewBottom);
				m_viewBottom = null;
				return;
			}

			// Have both panes.

			switch (nPaneStateNew) {
			case -2:
				// Expand the top pane: Make top pane the bottom pane,
				// then make top pane size 0

				LevelViewParent viewT = m_viewTop;
				m_viewTop = m_viewBottom;
				m_viewBottom = viewT;

				// Set right order for auto formatting

				m_viewTop.Dock = DockStyle.Top;
				m_viewTop.Height = 0;
				m_viewBottom.Dock = DockStyle.Fill;
				splitter.SplitPosition = 0;
				Controls.SetChildIndex(m_viewBottom, 0);
				Controls.SetChildIndex(splitter, 1);
				Controls.SetChildIndex(m_viewTop, 2);
				break;

			case 2:
				// Expand the bottom pane: Make top pane 0 size

				splitter.SplitPosition = 0;
				break;

			case 3:
				// Expand the top pane

				splitter.SplitPosition = ClientSize.Height / 2;
				break;
			}
		}

		private void splitter_DoubleClick(object sender, System.EventArgs e) {
			if (splitter.SplitPosition == 0) {
				ChangePanes(3);
			} else {
				ChangePanes(2);
			}
		}

		private void splitter_SplitterMoved(object sender, System.Windows.Forms.SplitterEventArgs e) {
			// If splitter at bottom, exchange top pane with bottom and close
			// out top.

			if (ClientSize.Height - splitter.SplitPosition <= splitter.MinExtra + 10) {
				if (!m_fNoRecurse) {
					m_fNoRecurse = true;
					ChangePanes(-2);
					m_fNoRecurse = false;
				}
			}

			// Save away size ratio

			m_nSplitRatio = (float)splitter.SplitPosition / (float)ClientSize.Height;
		}

		private void LevelFrame_Closing(object sender, System.ComponentModel.CancelEventArgs e) {
			s_alsFrames.Remove(this);
			if (!m_doc.Close()) {
				e.Cancel = true;
				return;
			}
			ChangePanes(0);
		}

		private void LevelFrame_SizeChanged(object sender, System.EventArgs e) {
			splitter.SplitPosition = (int)((float)ClientSize.Height * m_nSplitRatio);
		}

		// For zorder control when another doc closes

		void LevelDocTemplate_DocActive(Document doc) {
			if (m_doc == doc)
				BringToFront();
		}

		// For z order control when this doc gets clicked on.
		// Activated event doesn't work consistently
		// MdiChildActivate on parent doesn't give details about the activation
		// and is before the activation
		// This is the resulting hack:

		public struct WINDOWPOS {
			public IntPtr hwnd;
			public IntPtr hwndInsertAfter;
			public int x;
			public int y;
			public int cx;
			public int cy;
			public uint flags;
		}

		protected unsafe override void WndProc(ref Message m) {
			switch (m.Msg) {
			// #define WM_WINDOWPOSCHANGED 0x0047
			// #define SWP_NOZORDER 0x0004
			case 0x47:
				WINDOWPOS *ppos = (WINDOWPOS *)m.LParam;
				if (ppos == null || (ppos->flags & 4) == 0) {
					DocManager.SetActiveDocument(typeof(LevelDoc), m_doc);
					DocManager.SetCommandTarget(this);
				}
				break;

			// #define WM_NCACTIVATE                   0x0086
			case 0x86:
				if (((ushort)m.WParam) != 0)
					DocManager.SetCommandTarget(this);
				break;
			}

			base.WndProc(ref m);
		}
	}
}
