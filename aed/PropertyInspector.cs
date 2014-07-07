using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace AED
{
	/// <summary>
	/// Summary description for PropertyInspector.
	/// </summary>
	public class PropertyInspector : System.Windows.Forms.Form {
		private System.Windows.Forms.PropertyGrid prpg;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="obInspectee"></param>
		public PropertyInspector(object obInspectee) {
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			prpg.SelectedObject = obInspectee;
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing ) {
			if( disposing ) {
				if(components != null) {
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
		private void InitializeComponent() {
			this.prpg = new System.Windows.Forms.PropertyGrid();
			this.SuspendLayout();
			// 
			// prpg
			// 
			this.prpg.CommandsVisibleIfAvailable = true;
			this.prpg.Dock = System.Windows.Forms.DockStyle.Fill;
			this.prpg.LargeButtons = false;
			this.prpg.LineColor = System.Drawing.SystemColors.ScrollBar;
			this.prpg.Name = "prpg";
			this.prpg.Size = new System.Drawing.Size(280, 429);
			this.prpg.TabIndex = 0;
			this.prpg.Text = "propertyGrid1";
			this.prpg.ViewBackColor = System.Drawing.SystemColors.Window;
			this.prpg.ViewForeColor = System.Drawing.SystemColors.WindowText;
			// 
			// PropertyInspector
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(280, 429);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.prpg});
			this.Name = "PropertyInspector";
			this.Text = "Property Inspector";
			this.ResumeLayout(false);

		}
		#endregion

		public event PropertyValueChangedEventHandler PropertyValueChanged {
			add {
				prpg.PropertyValueChanged += value;
			}
			remove {
				prpg.PropertyValueChanged -= value;
			}
		}
	}
}
