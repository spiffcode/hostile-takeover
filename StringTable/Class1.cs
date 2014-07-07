using System;
using System.IO;
using System.Collections;
using System.Text.RegularExpressions;

namespace StringTable
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			//
			// TODO: Add code to start application here
			//

			string sFileNameIn = null;
			string sFileNameOut = null;

			if (args.Length != 2) {
				Console.WriteLine("StringTable <FileNameIn> <FileNameOut>");
			} else {
				sFileNameIn = args[0];
				sFileNameOut = args[1];
			}
			Save(sFileNameIn, sFileNameOut);
		}

		public static bool Save(string strFileIn, string strFileOut)
		{
			// Read it

			TextReader tr;
			try 
			{
				tr = new StreamReader(strFileIn);
			} 
			catch 
			{
				return false;
			}

			// Find the line that starts with "enum"

			while (true) {
				string strT = tr.ReadLine();
				if (strT == null) 
				{
					tr.Close();
					return false;
				}
				if (strT.StartsWith("enum"))
					break;
			}

			// Read in all the strings
			
			ArrayList alsStrings = new ArrayList();
			alsStrings.Clear();
			while (true) {
				string strT = tr.ReadLine();
				if (strT == null)
					break;
				if (strT == "};")
					break;
				Regex rex = new Regex(@"^\s*(?<name0>[a-zA-Z_0-9]+)\,\s*\/\/\s*(?<string>.*)\s*$");
				Match mat = rex.Match(strT);
				string strValue = mat.Groups["string"].Value;
				if (strValue.Length != 0)
					alsStrings.Add(strValue);
			}
			tr.Close();

			// Write directory

			Stream stm = new FileStream(strFileOut, FileMode.Create, FileAccess.Write, FileShare.None);
			BinaryWriter bwtr = new BinaryWriter(stm);
			int offStart = alsStrings.Count * 2;
			foreach (string strT in alsStrings) {
				// Write the current string's offset

				bwtr.Write(SwapUShort((ushort)offStart));

				// Calc next index

				offStart += strT.Length + 1;
			}

			// Write strings, length preceeded

			foreach (string strT in alsStrings) {
				bwtr.Write((byte)strT.Length);
				foreach (char ch in strT)
					bwtr.Write((byte)ch);
			}
				
			// Done

			bwtr.Close();
			return true;
		}


		public static ushort SwapUShort(ushort us) {
			int n = (int)us;
			return (ushort)(((n >> 8) & 0x00ff) | ((n << 8) & 0xff00));
		}
	}
}
