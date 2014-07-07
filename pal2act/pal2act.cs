using System;
using System.IO;
using SpiffLib;

namespace pal2act
{
	/// <summary>
	/// Summary description for App.
	/// </summary>
	class App
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static int Main(string[] astrArgs)
		{
			if (astrArgs.Length < 1) {
				Console.WriteLine("pal2act usage:\npal2act <palette.pal> [out.act]");
				return -1;
			}
			string strIn = astrArgs[0];

			string strOut;
			if (astrArgs.Length < 2) {
				strOut = Path.ChangeExtension(strIn, ".act");
			} else {
				strOut = astrArgs[1];
			}

			Palette pal = new Palette(strIn);
			pal.SavePhotoshopAct(strOut);

			return 0;
		}
	}
}
