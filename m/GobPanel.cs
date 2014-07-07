using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.Drawing.Imaging;

namespace m
{
	/// <summary>
	/// Summary description for GobPanel.
	/// </summary>
	public class GobPanel : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.ComboBox comboSide;
		private FlowPanel flowPanel;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public GobPanel()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// Populate sides combo

			string[] astr = Helper.GetDisplayNames(typeof(Side));
			comboSide.Items.AddRange(astr);
			comboSide.SelectedIndex = 0;

//			comboSide.DataSource = Enum.GetNames(typeof(Side));

			// Fill

			FillGobPanel();
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
			this.comboSide = new System.Windows.Forms.ComboBox();
			this.flowPanel = new m.FlowPanel();
			this.SuspendLayout();
			// 
			// comboSide
			// 
			this.comboSide.Dock = System.Windows.Forms.DockStyle.Top;
			this.comboSide.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboSide.Name = "comboSide";
			this.comboSide.Size = new System.Drawing.Size(208, 21);
			this.comboSide.TabIndex = 0;
			this.comboSide.SelectedIndexChanged += new System.EventHandler(this.comboSide_SelectedIndexChanged);
			// 
			// flowPanel
			// 
			this.flowPanel.BackColor = System.Drawing.Color.DarkKhaki;
			this.flowPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.flowPanel.Location = new System.Drawing.Point(0, 21);
			this.flowPanel.Name = "flowPanel";
			this.flowPanel.Size = new System.Drawing.Size(208, 451);
			this.flowPanel.TabIndex = 1;
			// 
			// GobPanel
			// 
			this.AutoScroll = true;
			this.BackColor = System.Drawing.Color.Black;
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.flowPanel,
																		  this.comboSide});
			this.Name = "GobPanel";
			this.Size = new System.Drawing.Size(208, 472);
			this.ResumeLayout(false);

		}
		#endregion

		private void ChangeSide(Control ctl, Side side) {
			if (!(ctl is PictureBox))
				return;

			PictureBox picb = (PictureBox)ctl;
			Unit unit = picb.Tag as Unit;
			if (unit == null)
				return;

			unit.Side = side;
			picb.Image = unit.GetBitmap(new Size(16, 16), null);
		}

		void FillGobPanel() {
			flowPanel.SuspendLayout();
			flowPanel.Controls.Clear();

			// If no plugins are enabled then add the default (HT) MapItems

			if (Globals.Plugins.Count == 0) {
				flowPanel.Controls.Add(CreatePictureBox(new Headquarters(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Radar(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new ResearchCenter(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new VehicleTransportStation(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Reactor(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new HumanResourceCenter(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Processor(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Warehouse(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new MachineGunTower(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new RocketTower(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new MobileHeadquarters(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new GalaxMiner(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new LightTank(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new MediumTank(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new MachineGunVehicle(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new RocketVehicle(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Artillery(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new ShortRangeInfantry(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new LongRangeInfantry(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new TakeoverSpecialist(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Andy(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Fox(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(0, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(1, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(2, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(3, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(4, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(5, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(6, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(7, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Galaxite(8, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Area(2, 2)));
				flowPanel.Controls.Add(CreatePictureBox(new Wall(15)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("RocketArtifact", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Tree", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Tree1", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Tree2", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Tree3", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Tree4", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Tree5", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Tree6", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Tree7", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Plant", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Plant1", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Plant2", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Plant3", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Plant4", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Plant5", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Scenery("Rocks", 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Replicator(Side.sideNeutral, 0, 0)));
				flowPanel.Controls.Add(CreatePictureBox(new Activator
					(Side.sideNeutral, 0, 0)));

			// Otherwise let the plugins add their own MapItems

			} else {
				foreach (IPlugin plug in Globals.Plugins) {
					IMapItem[] ami = plug.GetMapItems();
					foreach (IMapItem mi in ami) {
						flowPanel.Controls.Add(CreatePictureBox(mi));
					}
				}
			}
			flowPanel.ResumeLayout();
		}

		PictureBox CreatePictureBox(IMapItem mi) {
			PictureBox picb = new PictureBox();
			picb.Image = mi.GetBitmap(new Size(16, 16), null);
			picb.SizeMode = PictureBoxSizeMode.AutoSize;
			picb.Tag = (Object)mi;
			picb.MouseDown += new MouseEventHandler(PictureBox_MouseDown);
			return picb;
		}

		private void comboSide_SelectedIndexChanged(object sender, System.EventArgs e) {
			Side side = (Side)comboSide.SelectedIndex;

#if false
			if (m_ctlSelected != null) {
				ChangeSide(m_ctlSelected, side);
				OnMapItemSelectionChanged((IMapItem)m_ctlSelected.Tag);
			}
#endif

			foreach (PictureBox picb in flowPanel.Controls) {
				ChangeSide(picb, side);
			}
		}

		private void PictureBox_MouseDown(Object sender, MouseEventArgs e) {
			Control ctlSelected = (Control)sender;

			// Start drag drop

			LevelData ldat = new LevelData();
			IMapItem mi = (IMapItem)ctlSelected.Tag;
			ldat.ami = new IMapItem[] { mi };
			ldat.txMouse = e.X / 16.0;
			ldat.tyMouse = e.Y / 16.0;
			ldat.Grid.Width = mi.Grid.Width;
			ldat.Grid.Height = mi.Grid.Height;
			DoDragDrop(ldat, DragDropEffects.Copy);
		}
	}
}
