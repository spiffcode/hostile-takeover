using System;
using System.IO;
using System.Drawing;
using SpiffLib;

namespace palbin
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1 {
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static int Main(string[] astrArgs) {
			if (astrArgs.Length != 2) {
				Usage();
				return 1;
			}

			Palette pal;
			try {
				pal = new Palette(astrArgs[0]);
			} catch (Exception ex) {
				Console.WriteLine(ex);
				return 1;
			}

			BinaryWriter binw = new BinaryWriter(new FileStream(astrArgs[1], FileMode.Create));
			binw.Write((byte)((pal.Length & 0xff00) >> 8));
			binw.Write((byte)(pal.Length & 0xff));
			
			for (int i = 0; i < pal.Length; i++) {
				binw.Write(pal[i].R);
				binw.Write(pal[i].G);
				binw.Write(pal[i].B);
			}

			binw.Close();
			
			return 0;
		}

		static void Usage() {
			Console.WriteLine("Usage:");
			Console.WriteLine("palbin <jasc.pal | photoshop.act> palette.palbin");
		}
	}
}
