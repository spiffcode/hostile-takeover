using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using SpiffLib;

namespace m
{
	/// <summary>
	/// Summary description for Terrain.
	/// </summary>
	public class EditTerrainForm : System.Windows.Forms.Form {

		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private Brush[] m_abr = new Brush[s_aclrTerrain.Length];
		private System.Windows.Forms.Panel panel9;
		private System.Windows.Forms.Button button1;
		private m.FlowPanel panel1;
		Size m_sizTile;

		static Color[] s_aclrTerrain = {
				Color.FromArgb(0, 0, 0, 0), // Open
				Color.FromArgb(100, 255, 0, 0), // Blocked
				Color.FromArgb(100, 255, 255, 0), // Road
				Color.FromArgb(100, 0, 255, 0) // Scrabble
		};

		public EditTerrainForm(TemplateDoc tmpd) {
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_sizTile = tmpd.TileSize;
			for (int n = 0; n < m_abr.Length; n++)
				m_abr[n] = new SolidBrush(s_aclrTerrain[n]);

			Template[] atmpl = tmpd.GetTemplates();
			panel1.SuspendLayout();
			foreach (Template tmpl in atmpl) {
				PictureBox picb = new PictureBox();
				picb.Image = ConstructTerrainBitmap(tmpl);
				picb.SizeMode = PictureBoxSizeMode.AutoSize;
				picb.Tag = (Object)tmpl;
				picb.MouseDown += new MouseEventHandler(PictureBox_MouseDown);
				panel1.Controls.Add(picb);
			}
			panel1.ResumeLayout();
		}

		private Bitmap ConstructTerrainBitmap(Template tmpl) {
			Bitmap bm = new Bitmap(tmpl.Bitmap);
			Graphics g = Graphics.FromImage(bm);
			int ctx = tmpl.Bitmap.Width / m_sizTile.Width;
			int cty = tmpl.Bitmap.Height / m_sizTile.Height;
			for (int ty = 0; ty < cty; ty++) {
				for (int tx = 0; tx < ctx; tx++) {
					if (!tmpl.OccupancyMap[ty, tx])
						continue;
					int n = (int)tmpl.TerrainMap[ty, tx];
					g.FillRectangle(m_abr[n], tx * m_sizTile.Width, ty * m_sizTile.Height, m_sizTile.Width, m_sizTile.Height);
					bm.SetPixel(tx * m_sizTile.Width + m_sizTile.Width / 2, ty * m_sizTile.Height + m_sizTile.Height / 2, Color.White);
				}
			}

			return Misc.TraceEdges(bm, 1, Color.Azure);
		}

		private void PictureBox_MouseDown(Object obj, MouseEventArgs args) {
			PictureBox picb = (PictureBox)obj;
			Rectangle rcBounds = picb.ClientRectangle;
			rcBounds.Inflate(-1, -1);
			if (!rcBounds.Contains(args.X, args.Y))
				return;
			Template tmpl = (Template)picb.Tag;
			if ((Control.ModifierKeys & Keys.Control) != Keys.Control) {
				int tx = (args.X - 1) / m_sizTile.Width;
				int ty = (args.Y - 1) / m_sizTile.Height;
				if (!tmpl.OccupancyMap[ty, tx])
					return;
				TerrainTypes ter = tmpl.TerrainMap[ty, tx];
				ter += 1;
				if (ter == TerrainTypes.End)
					ter = TerrainTypes.Start;
				tmpl.TerrainMap[ty, tx] = ter;
			} else {
				TerrainTypes ter = TerrainTypes.Open;
				for (int ty = 0; ty < tmpl.OccupancyMap.GetLength(0); ty++) {
					for (int tx = 0; tx < tmpl.OccupancyMap.GetLength(1); tx++) {
						if  (tmpl.OccupancyMap[ty, tx]) {
							ter = tmpl.TerrainMap[ty, tx];
							break;
						}
					}
				}

				ter += 1;
				if (ter == TerrainTypes.End)
					ter = TerrainTypes.Start;

				for (int ty = 0; ty < tmpl.OccupancyMap.GetLength(0); ty++) {
					for (int tx = 0; tx < tmpl.OccupancyMap.GetLength(1); tx++) {
						if  (tmpl.OccupancyMap[ty, tx]) {
							tmpl.TerrainMap[ty, tx] = ter;
						}
					}
				}
			}
			picb.Image = ConstructTerrainBitmap(tmpl);
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
			this.panel1 = new m.FlowPanel();
			this.panel9 = new System.Windows.Forms.Panel();
			this.button1 = new System.Windows.Forms.Button();
			this.panel9.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.AutoScroll = true;
			this.panel1.BackColor = System.Drawing.Color.Black;
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(854, 564);
			this.panel1.TabIndex = 1;
			// 
			// panel9
			// 
			this.panel9.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.button1});
			this.panel9.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel9.Location = new System.Drawing.Point(0, 564);
			this.panel9.Name = "panel9";
			this.panel9.Size = new System.Drawing.Size(854, 48);
			this.panel9.TabIndex = 0;
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(8, 16);
			this.button1.Name = "button1";
			this.button1.TabIndex = 0;
			this.button1.Text = "Ok";
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// EditTerrainForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(854, 612);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panel1,
																		  this.panel9});
			this.Name = "EditTerrainForm";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show;
			this.Text = "Edit Terrain";
			this.panel9.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void button1_Click(object sender, System.EventArgs e) {
			DialogResult = DialogResult.OK;
		}
	}
}
