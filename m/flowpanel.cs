using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;

using System.Windows.Forms;


namespace m {
	public class FlowPanel : Panel {
		private System.ComponentModel.IContainer components = null;
		public Size Spacing;

		public FlowPanel() {
			// This call is required by the Windows Form Designer.
			InitializeComponent();

			Spacing.Width = 1;
			Spacing.Height = 1;
			Layout += new LayoutEventHandler(FlowPanel_Layout);
		}

		public void RefreshScrollbar() {
			Size siz = Size;
			Size = new Size(siz.Width + 1, siz.Height);
			UpdateBounds();
			Size = siz;
			UpdateBounds();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing ) {
			if( disposing ) {
				if (components != null) {
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent() {
			// 
			// FlowPanel
			// 

		}
		#endregion
	
		void FlowPanel_Layout(object sender, LayoutEventArgs e) {
			PerformLayout(true);
		}

		int PerformLayout(bool fSetPosition) {
			int xItem = Spacing.Width + AutoScrollPosition.X;
			int yItem = Spacing.Height + AutoScrollPosition.Y;
			int cyTallest = 0;
			bool fAdded = false;
			foreach (Control ctl in Controls) {
				if (fAdded) {
					if (xItem + ctl.Width + Spacing.Width >= ClientSize.Width) {
						xItem = Spacing.Width;
						yItem += cyTallest + Spacing.Height;
						cyTallest = 0;
						fAdded = false;
					}
				}
				if (fSetPosition)
					ctl.Location = new Point(xItem, yItem);
				fAdded = true;
				if (ctl.Height > cyTallest)
					cyTallest = ctl.Height;
				xItem += ctl.Width + Spacing.Width;
			}
			return yItem + cyTallest;
		}
	}
}

