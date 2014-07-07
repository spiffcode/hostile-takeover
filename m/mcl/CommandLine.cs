using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.IO;
using SpiffLib;
using System.Diagnostics;
using m;

namespace m
{
	class CommandLine
	{
		public static void Main(string[] astr)
		{
			new Globals();
			new DocManager();

			DocManager.AddTemplate(new LevelDocTemplate(null, null));
			DocManager.AddTemplate(new TemplateDocTemplate());

			switch (astr[0]) {
			case "-mixmaps":
				OutputTools.ExportMixMaps(astr);
				break;

			case "-levels":
				OutputTools.ExportLevels(astr, 0);
				break;

			case "-images":
				OutputTools.ExportImages(astr);
				break;

			case "-makepal":
				OutputTools.MakePalette(astr);
				break;

			case "-special":
				char sep = Path.DirectorySeparatorChar;
				string root = sep + "ht" + sep + "data" + sep;
				OutputTools.MixMapImportSpecial(Theater.Desert, (TemplateDoc)DocManager.OpenDocument(root + "desert.tc"), root + "desert24.tc");
				OutputTools.MixMapImportSpecial(Theater.Temperate, (TemplateDoc)DocManager.OpenDocument(root + "temperate.tc"), root + "temperate24.tc");
				break;

			case "-exporttext":
				OutputTools.ExportText(astr);
				break;

			case "-importtext":
				OutputTools.ImportText(astr);
				break;

			case "-testimport":
				OutputTools.ImportExportPdbs(astr);
				break;
			}
		}
	}
}
