using System.Windows.Forms;
using System.Drawing;

namespace SpiffCode {
	public class ScToggleButton : CheckBox {
		new public bool CanFocus {
			get {
				return false;
			}
		}

		override protected bool ShowFocusCues {
			get {
				return false;
			}
		}

		new public bool IsDefault {
			get {
				base.IsDefault = false;
				return false;
			}
			set {
				base.IsDefault = false;
			}
		}
	}

	public class ScBorder : Control {
		override protected void OnPaint(PaintEventArgs e) {
			ControlPaint.DrawBorder(e.Graphics, ClientRectangle, 
					Color.Black, 0, ButtonBorderStyle.None,
					SystemColors.ControlDark, 1, ButtonBorderStyle.Solid,
					Color.Black, 0, ButtonBorderStyle.None,
					SystemColors.ControlLight, 1, ButtonBorderStyle.Solid);
		}
	}
}
