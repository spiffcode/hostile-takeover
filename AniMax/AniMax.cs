using System;
using System.Windows.Forms;

namespace SpiffCode
{
	/// <summary>
	/// Summary description for AniMax.
	/// </summary>
	public class AniMax
	{
		private static Form s_frmMain;

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] astrArgs) {
			string strOpenFileName = null;
			if (astrArgs.Length != 0)
				strOpenFileName = astrArgs[0];

			Globals.NullDocument = new AnimDoc(Globals.TileSize,
                    Globals.FrameRate);
			Globals.NullDocument.Dirty = false;
			Globals.ActiveDocument = Globals.NullDocument;

			s_frmMain = new MainForm(strOpenFileName);
#if true
			Application.Run(s_frmMain);
#else
			try {
				Application.Run(s_frmMain);
			} catch (Exception ex) {
				MessageBox.Show(ex.ToString());
			}
#endif
		}
	}
}
