using System;
using System.IO;
using System.Collections;
using System.Collections.Specialized;
using System.Drawing;
using System.Drawing.Imaging;
using SpiffLib;

namespace TemplateExtractor
{
	/// <summary>
	/// Summary description for App.
	/// </summary>
	class App
	{
		static string gstrTemplateBitmap;
		static string gstrTerrainBitmap;
		static string gstrColorsBitmap;
		static string gstrTemplateNames;
		static string gstrOutputPrefix;
		static string gstrTileCollection;
		static int gcxTile = 24, gcyTile = 24;
		static BitmapData gbmd;
		static Color gclrTransparent;
		static int[,] gaCells;

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static unsafe int Main(string[] astrArgs)
		{
#if false
			while (true) {
				float h, s, l;
				float r, g, b;
				string str = Console.ReadLine();
				r = float.Parse(str);
				str = Console.ReadLine();
				g = float.Parse(str);
				str = Console.ReadLine();
				b = float.Parse(str);

				//			r = .5f; g = .5f; b = 1.0f;
				Misc.RgbToHsl(r, g, b, &h, &s, &l);
				Console.WriteLine("r: {3}, g: {4}, b: {5} -> h: {0}, s: {1}, l: {2}", h, s, l, r, g, b);

				Misc.HslToRgb(h, s, l, &r, &g, &b);
				Console.WriteLine("r: {3}, g: {4}, b: {5} <- h: {0}, s: {1}, l: {2}", h, s, l, r, g, b);
			}

			return 0;
#endif
			// Command-line argument processing

			if (astrArgs.Length == 0) {
				PrintHelp();
				return 0;
			}

			for (int i = 0; i < astrArgs.Length; i++) {
				switch (astrArgs[i]) {
				case "-?":
					PrintHelp();
					return 0;

				case "-n":
					gstrTemplateNames = astrArgs[++i];
					break;

				case "-tc":
					gstrTileCollection = astrArgs[++i];
					break;

				case "-art":
					gstrTemplateBitmap = astrArgs[++i];
					break;

				case "-ter":
					gstrTerrainBitmap = astrArgs[++i];
					break;

				case "-colors":
					gstrColorsBitmap = astrArgs[++i];
					break;
    
                case "-ts":
                    gcxTile = int.Parse(astrArgs[++i]);
                    gcyTile = gcxTile;
                    break;

				default:
					Console.WriteLine("Error: invalid argument '{0}'", astrArgs[i]);
					return 1;
				}
			}

			Color clrBlocked = Color.FromArgb(255, 0, 0);
			Bitmap bmTerrain = null;
			if (gstrTerrainBitmap != null)
				bmTerrain = new Bitmap(gstrTerrainBitmap);

			Bitmap bmColors = null;
			if (gstrColorsBitmap != null)
				bmColors = new Bitmap(gstrColorsBitmap);

			if (gstrTemplateBitmap == null) {
				Console.WriteLine("Error: A valid source bitmap must be specified");
				return -1;
			}

			if (gstrOutputPrefix == null)
				gstrOutputPrefix = Path.GetFileNameWithoutExtension(gstrTemplateBitmap);

			Console.WriteLine("Reading {0}", gstrTemplateBitmap);

			Bitmap bmFile = null;
			try {
				bmFile = new Bitmap(gstrTemplateBitmap);
			} catch {
				Console.WriteLine("Error: {0} is not a recognized bitmap file", gstrTemplateBitmap);
				return -1;
			}

			// Using DrawImage to copy subrectangles of a loaded bitmap doesn't work reliably
			// due to GDI+'s desire to scale based on Bitmap.Horizontal/VerticalResolution so
			// we must create a neutral resolution bitmap before doing any DrawImage'ing from it.

			Console.WriteLine("Creating neutral resolution source bitmap", gstrTemplateBitmap);

			Bitmap bm = SpiffLib.Misc.NormalizeBitmap(bmFile);
			bmFile.Dispose();

			// The upper-left-most pixel defines the transparent color
		
			gclrTransparent = bm.GetPixel(0, 0);

			// 

			int ctx = bm.Width / gcxTile;
			int cty = bm.Height / gcyTile;

			gaCells = new int[ctx, cty];
			for (int j = 0; j  < cty; j++) {
				for (int i = 0; i < ctx; i++) {
					gaCells[i, j] = -1;		// -1 = empty
				}
			}
			ArrayList alTemplates = new ArrayList();
	
            Console.WriteLine("Scanning for templates", alTemplates.Count);

			// Lock down bits for speed

			Rectangle rc = new Rectangle(0, 0, bm.Width, bm.Height);
			gbmd = bm.LockBits(rc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
			byte *pbBase = (byte *)gbmd.Scan0.ToPointer();

			for (int y = gcyTile, ty = 1; y < bm.Height - gcyTile; y += gcyTile, ty++) {
				for (int x = gcxTile, tx = 1; x < bm.Width - gcxTile; x += gcxTile, tx++) {
					byte *pb = pbBase + y * gbmd.Stride + x * 3;
					Color clr = Color.FromArgb(pb[2], pb[1], pb[0]);
					if (clr != gclrTransparent && gaCells[tx, ty] == -1) {
						ArrayList alTiles = new ArrayList();
						FloodFill(pbBase, alTiles, tx, ty);
						alTemplates.Add(alTiles);
					}
				}
			}

			bm.UnlockBits(gbmd);

			StreamReader stmrTemplateNames = null;
			if (gstrTemplateNames != null)
				stmrTemplateNames = new StreamReader(gstrTemplateNames);

			Console.WriteLine("Extracting {0} templates", alTemplates.Count);

			m.DocManager.AddTemplate(new m.TemplateDocTemplate());
			m.TemplateDoc tmpd = (m.TemplateDoc)m.DocManager.NewDocument(typeof(m.TemplateDoc), new Object[] { new Size(gcxTile, gcyTile) });
			m.Template[] atmpl = new m.Template[alTemplates.Count];

			// Create a new bitmap for each template. Empty cells of the template are 
			// filled with the transparent color.

			int cTiles = 0;
			int nTemplate = 0;
			foreach (ArrayList alTiles in alTemplates) {
				cTiles += alTiles.Count;

				// Found boundary of template

				int txMax = -1, tyMax = -1;
				int txMin = 99999, tyMin = 99999;

				foreach (Point pt in alTiles) {
					if (pt.X < txMin)
						txMin = pt.X;
					if (pt.X > txMax)
						txMax = pt.X;
					if (pt.Y < tyMin)
						tyMin = pt.Y;
					if (pt.Y > tyMax)
						tyMax = pt.Y;
				}

				// 

				ctx = txMax + 1 - txMin;
				cty = tyMax + 1 - tyMin;
				int cx =  ctx * gcxTile;
				int cy =  cty * gcyTile;

				Bitmap bmTemplate = new Bitmap(cx, cy, PixelFormat.Format24bppRgb);
				Graphics gTemplate = Graphics.FromImage(bmTemplate);

				Rectangle rcSrc = new Rectangle(txMin * gcxTile, tyMin * gcyTile, cx, cy);
				gTemplate.DrawImage(bm, 0, 0, rcSrc, GraphicsUnit.Pixel);

				string strT;
				if (gstrTemplateNames != null) {
					strT = stmrTemplateNames.ReadLine();
					if (strT == null) // end of template names reached
						strT = String.Format("template{0:0#}", nTemplate);
				} else {
					strT = String.Format("{0}{1:0#}", gstrOutputPrefix, nTemplate);
				}

				if (gstrTileCollection == null) {
					bmTemplate.Save(strT + ".png", ImageFormat.Png);
					bmTemplate.Dispose();
				} else {
					m.Template tmpl = new m.Template(tmpd, bmTemplate, strT);
					if (bmTerrain != null) {
						tmpl.TerrainMap = new m.TerrainTypes[cty, ctx];
						for (int j = 0; j < cty; j++) {
							for (int i = 0; i < ctx; i++) {
								if (bmTerrain.GetPixel((txMin + i) * gcxTile, (tyMin + j) * gcyTile) == clrBlocked) 
									tmpl.TerrainMap[j, i] = m.TerrainTypes.Blocked;
								else
									tmpl.TerrainMap[j, i] = m.TerrainTypes.Open;
							}
						}
					}
					if (bmColors != null) {
						Color[] aclr = new Color[4] {
														Color.FromArgb(114, 167, 48), // grass
														Color.FromArgb(81, 142, 118), // cliff
														Color.FromArgb(1, 172, 254), // water
														Color.FromArgb(174, 168, 99), // road
						};
						tmpl.TerrainColors = new m.TerrainColors[cty * 2, ctx * 2];
						for (int yT = 0; yT < cty * 2; yT++) {
							for (int xT = 0; xT < ctx * 2; xT++) {
								Color clr = bmColors.GetPixel(txMin * gcxTile + xT * gcxTile / 2, tyMin * gcyTile + yT * gcyTile / 2);
								m.TerrainColors tclr = m.TerrainColors.Grass;
								for (int i = 0; i < aclr.Length; i++) {
									if (clr == aclr[i]) {
										tclr = (m.TerrainColors)i;
										break;
									}
								}
								tmpl.TerrainColors[yT, xT] = tclr;
							}
						}
					}

					if (strT.ToLower() == "background")
						tmpd.SetBackgroundTemplate(tmpl);

					atmpl[nTemplate] = tmpl;
				}

				gTemplate.Dispose();

				nTemplate++;
			}

			if (stmrTemplateNames != null)
				stmrTemplateNames.Close();

			if (gstrTileCollection != null) {
				Console.WriteLine("Writing templates to {0}", gstrTileCollection);

				tmpd.AddTemplates(atmpl);
				tmpd.SaveAs(gstrTileCollection);
			}

			Console.WriteLine("{0} tiles total in {1} templates ({2:###,###,###} bytes)", cTiles, alTemplates.Count, cTiles * gcxTile * gcyTile);

			return 0;
		}

		static unsafe private void FloodFill(byte *pbBase, ArrayList alTiles, int tx, int ty) {
			// Has this cell already been filled?

			if (gaCells[tx, ty] != -1)
				return;

			byte *pb = pbBase + (ty * gcyTile) * gbmd.Stride + (tx * gcxTile) * 3;
			Color clr = Color.FromArgb(pb[2], pb[1], pb[0]);
			if (clr == gclrTransparent)
				return;

			gaCells[tx, ty] = 1;
			alTiles.Add(new Point(tx, ty));

			FloodFill(pbBase, alTiles, tx - 1, ty - 1);
			FloodFill(pbBase, alTiles, tx, ty - 1);
			FloodFill(pbBase, alTiles, tx + 1, ty - 1);
			FloodFill(pbBase, alTiles, tx - 1, ty);
			FloodFill(pbBase, alTiles, tx + 1, ty);
			FloodFill(pbBase, alTiles, tx - 1, ty + 1);
			FloodFill(pbBase, alTiles, tx, ty + 1);
			FloodFill(pbBase, alTiles, tx + 1, ty + 1);
		}

		//

		static void PrintHelp() {
			Console.WriteLine(
					"Usage: TemplateExtractor -art <source bitmap> [-n names file] [-tc template collection] [-ter terrain bitmap]\n" +
					"-art source bitmap: bitmap file containing templates to be processed.\n" +
					"-n names file: file containing template names, one per line\n" +
					"-tc template collection: name of template collection to output (instead of .pngs)\n" +
					"-ter terrain bitmap: bitmap file containing terrain info to be processed.");
		}
	}
}
