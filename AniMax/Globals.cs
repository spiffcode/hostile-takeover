using System;
using System.Windows.Forms;
using System.Reflection;
using System.IO;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for Globals.
	/// </summary>
	public class Globals
	{
		private static AnimDoc s_doc;
		private static Strip s_stpActive = null;
		private static int s_ifrActive = 0;
		private static int s_cfrActive = 1;
		private static int s_msFrameRate = 80;
		private static int s_nPreviewScale = 4;
		private static int s_nStripScale = 1;
		private static bool s_fSideColorMappingOn = true;
		private static bool s_fGridOn = true;
		private static bool s_fShowOriginPoint = false;
		private static bool s_fShowSpecialPoint = false;
		private static AnimDoc s_docNull;
		private static Control s_ctlStrip;
		private static Control s_ctlPreview;
		private static Form s_frmStrips;
		private static Form s_frmStrip;
		private static Form s_frmMain;
		private static Cursor s_crsrHand;
		private static Cursor s_crsrGrab;
		private static int s_nTileSize = 32;
		private static int s_cxGrid = 32;
		private static int s_cyGrid = 32;

		public static event EventHandler ActiveDocumentChanged;

		public static AnimDoc ActiveDocument {
			get {
				return s_doc;
			}
			set {
				// Keep track of the ActiveDoc's ActiveStrip

				if (s_doc != null)
					s_doc.ActiveStripChanged -= new EventHandler(OnActiveStripChanged);

				s_doc = value;
				if (ActiveDocumentChanged != null)
					ActiveDocumentChanged(null, EventArgs.Empty);
				ActiveStrip = s_doc == null ? null : s_doc.ActiveStrip;

				if (s_doc != null)
					s_doc.ActiveStripChanged += new EventHandler(OnActiveStripChanged);
			}
		}

		private static void OnActiveStripChanged(object obSender, EventArgs e) {
			ActiveStrip = s_doc.ActiveStrip;
		}

		public static event EventHandler ActiveStripChanged;

		public static Strip ActiveStrip {
			get {
				return s_stpActive;
			}
			set {
				// Keep track of the ActiveStrip's ActiveFrame

				if (s_stpActive != null) {
					s_stpActive.ActiveFrameChanged -= new EventHandler(OnActiveFrameChanged);
					s_stpActive.ActiveFrameCountChanged -= new EventHandler(OnActiveFrameCountChanged);
                }

				s_stpActive = value;
				if (ActiveStripChanged != null)
					ActiveStripChanged(null, EventArgs.Empty);
				ActiveFrame = s_stpActive == null ? 0 : s_stpActive.ActiveFrame;

				if (s_stpActive != null) {
					s_stpActive.ActiveFrameChanged += new EventHandler(OnActiveFrameChanged);
					s_stpActive.ActiveFrameCountChanged += new EventHandler(OnActiveFrameCountChanged);
                }
			}
		}

		private static void OnActiveFrameChanged(object obSender, EventArgs e) {
			ActiveFrame = s_stpActive.ActiveFrame;
		}

		private static void OnActiveFrameCountChanged(object obSender, EventArgs e) {
			ActiveFrameCount = s_stpActive.ActiveFrameCount;
		}

		public static event EventHandler ActiveFrameChanged;

		public static int ActiveFrame {
			get {
				return s_ifrActive;
			}
			set {
				s_ifrActive = value;
				if (ActiveFrameChanged != null)
					ActiveFrameChanged(null, EventArgs.Empty);
			}
		}

        public static event EventHandler ActiveFrameCountChanged;

		public static int ActiveFrameCount {
			get {
				return s_cfrActive;
			}
			set {
				s_cfrActive = value;
				if (ActiveFrameCountChanged != null)
					ActiveFrameCountChanged(null, EventArgs.Empty);
			}
		}

		public static int FrameRate {
			get {
				return s_msFrameRate;
			}
			set {
				s_msFrameRate = value;
			}
		}

		public static event EventHandler GridChanged;

		public static bool GridOn {
			get {
				return s_fGridOn;
			}
			set {
				s_fGridOn = value;
				if (GridChanged != null)
					GridChanged(null, EventArgs.Empty);
			}
		}

		public static int GridWidth {
			get {
				return s_cxGrid;
			}
			set {
				s_cxGrid = value;
				if (GridChanged != null)
					GridChanged(null, EventArgs.Empty);
			}
		}

		public static int GridHeight {
			get {
				return s_cyGrid;
			}
			set {
				s_cyGrid = value;
				if (GridChanged != null)
					GridChanged(null, EventArgs.Empty);
			}
		}

		public static event EventHandler TileSizeChanged;

		public static int TileSize {
			set {
                s_nTileSize = value;
                GridWidth = s_nTileSize;
                GridHeight = s_nTileSize;
                if (s_nTileSize < 24) {
                    StripScale = 2;
                } else {
                    StripScale = 1;
                }
				if (TileSizeChanged != null)
					TileSizeChanged(null, EventArgs.Empty);
			}
			get {
				return s_nTileSize;
			}
		}

		public static Cursor HandCursor {
			get {
				if (s_crsrHand == null) {
					Assembly ass = Assembly.GetAssembly(typeof(Globals));
//					string[] astr = ass.GetManifestResourceNames();
					Stream stm = ass.GetManifestResourceStream("SpiffCode.Resources.hand.cur");
					s_crsrHand = new Cursor(stm);
					stm.Close();
				}
				return s_crsrHand;
			}
		}

		public static Cursor GrabCursor {
			get {
				if (s_crsrGrab == null) {
					Assembly ass = Assembly.GetAssembly(typeof(Globals));
					Stream stm = ass.GetManifestResourceStream("SpiffCode.Resources.grab.cur");
					s_crsrGrab = new Cursor(stm);
					stm.Close();
				}
				return s_crsrGrab;
			}
		}

		public static event EventHandler SideColorMappingOnChanged;

		public static bool SideColorMappingOn {
			get {
				return s_fSideColorMappingOn;
			}
			set {
				s_fSideColorMappingOn = value;
				if (SideColorMappingOnChanged != null)
					SideColorMappingOnChanged(null, EventArgs.Empty);
			}
		}

		public static event EventHandler ShowOriginPointChanged;

		public static bool ShowOriginPoint {
			get {
				return s_fShowOriginPoint;
			}
			set {
				s_fShowOriginPoint = value;
				if (ShowOriginPointChanged != null)
					ShowOriginPointChanged(null, EventArgs.Empty);
			}
		}

		public static event EventHandler ShowSpecialPointChanged;

		public static bool ShowSpecialPoint {
			get {
				return s_fShowSpecialPoint;
			}
			set {
				s_fShowSpecialPoint = value;
				if (ShowSpecialPointChanged != null)
					ShowSpecialPointChanged(null, EventArgs.Empty);
			}
		}

		public static event EventHandler PreviewScaleChanged;

		public static int PreviewScale {
			get {
				return s_nPreviewScale;
			}
			set {
				s_nPreviewScale = value;
				if (PreviewScaleChanged != null)
					PreviewScaleChanged(null, EventArgs.Empty);
			}
		}

		public static event EventHandler StripScaleChanged;

		public static int StripScale {
			get {
				return s_nStripScale;
			}
			set {
				s_nStripScale = value;
				if (StripScaleChanged != null)
					StripScaleChanged(null, EventArgs.Empty);
			}
		}

		public static AnimDoc NullDocument {
			get {
				return s_docNull;
			}
			set {
				s_docNull = value;
			}
		}

		public static Control StripControl {
			get {
				return s_ctlStrip;
			}
			set {
				s_ctlStrip = value;
			}
		}

		public static Control PreviewControl {
			get {
				return s_ctlPreview;
			}
			set {
				s_ctlPreview = value;
			}
		}

		public static Form StripsForm {
			get {
				return s_frmStrips;
			}
			set {
				s_frmStrips = value;
			}
		}

		public static Form StripForm {
			get {
				return s_frmStrip;
			}
			set {
				s_frmStrip = value;
			}
		}

		public static Form MainForm {
			get {
				return s_frmMain;
			}
			set {
				s_frmMain = value;
			}
		}

		public static event KeyPressEventHandler KeyPress;

		// UNDONE: this will send to every handler even after one has handled
		// the event.

		public static void OnKeyPress(Object sender, KeyPressEventArgs e) {
			if (KeyPress != null)
				KeyPress(sender, e);
		}

		public static event KeyEventHandler KeyDown;

		public static void OnKeyDown(Object sender, KeyEventArgs e) {
			if (KeyDown != null)
				KeyDown(sender, e);
		}

		public static event KeyEventHandler KeyUp;

		public static void OnKeyUp(Object sender, KeyEventArgs e) {
			if (KeyUp != null)
				KeyUp(sender, e);
		}

		public static event EventHandler FrameContentChanged;

		// UNDONE: this will send to every handler even after one has handled
		// the event.

		public static void OnFrameContentChanged(Object sender, EventArgs e) {
			if (FrameContentChanged != null)
				FrameContentChanged(sender, e);
		}
	}
}

