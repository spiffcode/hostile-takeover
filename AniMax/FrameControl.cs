using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for FrameControl.
	/// </summary>
	public class FrameControl : System.Windows.Forms.Control
	{
		private Point m_ptOffset;
		private Strip m_stp;
		private Frame m_fr;
		private int m_ifr;
		private int m_nScale = 2;
		private BorderStyle m_bdrs = BorderStyle.None;
		private Color m_clrBorder = Color.Black;

		public FrameControl()
		{
			m_ptOffset = new Point(0, 0);
		}

		protected override void OnPaintBackground(System.Windows.Forms.PaintEventArgs e) {
		}

		protected override void OnPaint(PaintEventArgs pe)
		{
			if (m_fr == null) {
				pe.Graphics.FillRectangle(new SolidBrush(BackColor), pe.ClipRectangle);
			} else {
				Frame.DrawArgs drwa = new Frame.DrawArgs();
				drwa.clrBackground = BackColor;
				drwa.fDrawBackground = true;
				drwa.fMapSideColors = Globals.SideColorMappingOn;
				drwa.fShowGrid = false;
				drwa.fShowOrigin = Globals.ShowOriginPoint;
				drwa.fShowSpecialPoint = Globals.ShowSpecialPoint;
				drwa.nScale = m_nScale;

				m_fr.Draw(pe.Graphics, ClientRectangle, drwa, m_ptOffset);
			}

			switch (m_bdrs) {
			case BorderStyle.FixedSingle:
				Rectangle rc = ClientRectangle;
				rc.Width--;
				rc.Height--;
				pe.Graphics.DrawRectangle(new Pen(m_clrBorder), rc);
				break;

			case BorderStyle.Fixed3D:
				ControlPaint.DrawBorder3D(pe.Graphics, ClientRectangle, Border3DStyle.Raised);
				break;
			}

			// Calling the base class OnPaint
//			base.OnPaint(pe);
		}

		public Strip Strip {
			get {
				return m_stp;
			}
			set {
				m_stp = value;
				Invalidate();
			}
		}

		public Frame Frame {
			get {
				return m_fr;
			}
			set {
				m_fr = value;
				Invalidate();
			}
		}

		public BorderStyle BorderStyle {
			get {
				return m_bdrs;
			}
			set {
				m_bdrs = value;
				Invalidate();
			}
		}

		public Color BorderColor {
			get {
				return m_clrBorder;
			}
			set {
				m_clrBorder = value;
				if (m_bdrs != BorderStyle.None)
					Invalidate();
			}
		}

		public int FrameIndex {
			get {
				return m_ifr;
			}
			set {
				m_ifr = value;
				if (m_stp != null) {
					m_fr = m_stp[m_ifr];
					Invalidate();
				}
			}
		}

		public Point OffsetPoint {
			get {
				return m_ptOffset;
			}
			set {
				m_ptOffset = value;
				Invalidate();
			}
		}
	}
}
