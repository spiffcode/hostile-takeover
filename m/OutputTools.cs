using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.IO;
using SpiffLib;
using System.Diagnostics;
using System.Text;
using m;

namespace m
{
    public class OutputTools
    {
		public static void ImportExportPdbs(string[] astr) {
			// Expand filespecs
			ArrayList alsFiles = new ArrayList();
			for (int n = 1; n < astr.Length; n++) {
				string strFileT = Path.GetFileName(astr[n]);
				string strDirT = Path.GetDirectoryName(astr[n]);
				if (strDirT == "")
					strDirT = ".";
				string[] astrFiles = Directory.GetFiles(strDirT, strFileT);
				foreach (string filename in astrFiles) {
					if (filename.EndsWith(".pdb")) {
						alsFiles.Add(filename);
					}
				}
			}

			// Import, then save each pdb
			foreach (string filename in alsFiles) {
				try {
					if (File.Exists(filename + "_new")) {
						Console.WriteLine("Exists: " + filename + "_new");
						continue;
					}
					ImportExpansionPdb(filename);
					LevelDocTemplate doctLevel = (LevelDocTemplate)DocManager.FindDocTemplate(typeof(LevelDoc));
					Document[] adoc = doctLevel.GetDocuments();
					SaveExpansionPdb(filename + "_new", adoc, "1.1");
					foreach (Document doc in adoc) {
						doc.SetModified(false);
					}
					doctLevel.CloseAllDocuments();
				} catch {
					Console.WriteLine("Error loading " + filename + ", skipping");
				}
			}
		}

		public static void ImportExpansionPdb(string strFile) {
			PdbPacker pdbp = new PdbPacker(strFile);

			ArrayList alsTileSets = new ArrayList();
			for (int i = 0; i < pdbp.Count; i++) {
				PdbPacker.File file = pdbp[i];
				if (!file.str.EndsWith(".lvl")) {
					continue;
				}

				// Load up the pieces

				Ini ini = Ini.LoadBinary(new MemoryStream(file.ab));
				string strTileMapFilename = ini["General"]["TileMap"].Value;
				TileMap tmap = TileMap.Load(new MemoryStream(pdbp[strTileMapFilename].ab));


				// First, tell the active LevelDoc not to switch its templates based on the following
				// template load

				LevelDoc lvldActive = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));
				if (lvldActive != null) {
					lvldActive.SwitchTemplatesEnabled = false;
				}

				// If the TileSet for this level is not yet available, load it now

				TemplateDoc tmpd = (TemplateDoc)DocManager.OpenDocument(tmap.Filename.Replace(".tset", ".tc"));
				TileSet tset = null;
				foreach (TileSet tsetT in alsTileSets) {
					if (tsetT.FileName == tmap.Filename) {
						tset = tsetT;
						break;
					}
				}
				if (tset == null) {
					tset = new TileSet(tmpd, tmap.Filename);
					alsTileSets.Add(tset);
				}
			
				// Re-enable template switching

				if (lvldActive != null) {
					lvldActive.SwitchTemplatesEnabled = true;
				}

				// Create a new level description, and deduce which templates are in it, with what visibility

				LevelDoc lvld = (LevelDoc)DocManager.NewDocument(typeof(LevelDoc), null);
				lvld.OutputFilename = file.str;
				ImportTileMap(tmap, tset, tmpd, lvld);

				// Walls are stored in the terrain map. Load them.
				string strTrmapFilename = ini["General"]["TerrainMap"].Value;
				TerrainMap trmap = TerrainMap.Load(new MemoryStream(pdbp[strTrmapFilename].ab));
				ImportWalls(trmap, lvld);

				// Load everything else
				lvld.LoadIni(ini);
			}
		}

		static void ImportWalls(TerrainMap trmap, LevelDoc lvld) {
			ArrayList alsmi = new ArrayList();
			for (int ty = 0; ty < trmap.Map.GetLength(0); ty++) {
				for (int tx = 0; tx < trmap.Map.GetLength(1); tx++) {
					if (trmap.Map[ty, tx] == TerrainTypes.Wall) {
						IMapItem mi = new Wall(100, lvld.Bounds.Left + tx, lvld.Bounds.Top + ty);
						alsmi.Add(mi);
					}
				}
			}
			lvld.AddMapItems((IMapItem[])alsmi.ToArray(typeof(IMapItem)));
		}
		
		class TemplatePos {
			public Template tmpl;
			public int txOrigin;
			public int tyOrigin;
			public bool[,] afMapped;

			public TemplatePos(Template tmpl, int txOrigin, int tyOrigin, bool[,] afMapped) {
				this.tmpl = tmpl;
				this.txOrigin = txOrigin;
				this.tyOrigin = tyOrigin;
				this.afMapped = afMapped;
			}
		}

		static void ImportTileMap(TileMap tmap, TileSet tset, TemplateDoc tmpd, LevelDoc lvld) {
			// The TileMap is a list of indexes into a tile set. A Tileset is a list of tiles compiled
			// from Templates. Reverse the tilemap into Templates, and set into lvld.

			bool[,] afCellTaken = new bool[64, 64];
			ArrayList alsTemplPos = new ArrayList();
			Template[] atmpl = tmpd.GetTemplates();
			for (int ty = 0; ty < tmap.Height; ty++) {
				for (int tx = 0; tx < tmap.Width; tx++) {
					// Cell mapped already?
					if (afCellTaken[ty, tx]) {
						continue;
					}

					// Cell not mapped. Create TemplatePos.
					int iTile = tmap.GetTileIndex(tx, ty);
					TileData tdata = tset.GetTileData(iTile);
					Template tmpl = atmpl[tdata.iTemplate];

					// Don't bother with background tiles
					if (tmpl == tmpd.GetBackgroundTemplate()) {
						continue;
					}

					int txOrigin = tx - tdata.txTemplate;
					int tyOrigin = ty - tdata.tyTemplate;
					bool[,] afMapped = new bool[tmpl.Cty, tmpl.Ctx];

					for (int tyTmpl = 0; tyTmpl < tmpl.Cty; tyTmpl++) {
						for (int txTmpl = 0; txTmpl < tmpl.Ctx; txTmpl++) {
							int txT = txOrigin + txTmpl;
							int tyT = tyOrigin + tyTmpl;
							if (txT < 0 || txT >= 64 || tyT < 0 || tyT >= 64) {
								continue;
							}
							if (afCellTaken[tyT, txT]) {
								continue;
							}
							int iTileT = tmap.GetTileIndex(txT, tyT);
							if (iTileT != -1) {
								TileData tdataT = tset.GetTileData(iTileT);
								if (tdataT.iTemplate != tdata.iTemplate) {
									continue;
								}
								if (tdataT.txTemplate != txTmpl || tdataT.tyTemplate != tyTmpl) {
									continue;
								}
							}
							afMapped[tyTmpl, txTmpl] = true;
							afCellTaken[tyT, txT] = true;
						}
					}
					alsTemplPos.Add(new TemplatePos(tmpl, txOrigin, tyOrigin, afMapped));
				}
			}

			// Figure out the bounds.

			Rectangle rcBounds = new Rectangle((64 - tmap.Width) / 2, (64 - tmap.Height) / 2,
					tmap.Width, tmap.Height);
			lvld.Bounds = rcBounds;

			// The list of TemplatePos's has been created. Add to LevelDoc.

			ArrayList alsTiles = new ArrayList();
			foreach (TemplatePos tpos in alsTemplPos) {
				Tile tile = new Tile(tpos.tmpl.Name,
						tpos.txOrigin + rcBounds.Left,
						tpos.tyOrigin + rcBounds.Top,
						tpos.afMapped, tpos.tmpl.OccupancyMap);
				alsTiles.Add(tile);
			}
			lvld.AddMapItems((IMapItem[])alsTiles.ToArray(typeof(IMapItem)));
		}

		public static void SaveExpansionPdb(string strFile, Document[] adoc, string strVersion) {
            // First save all level docs

			//annoying
            //DocManager.SaveAllModified(typeof(LevelDoc));

            // Remember active document

            LevelDoc lvldActive = (LevelDoc)DocManager.GetActiveDocument(typeof(LevelDoc));

            // Save documents adoc in an expansion .pdb. Expansion .pdbs need:
            // - .lvl, .tmap, .trmap, but not .tsets since these come from game .pdbs
            // - need a version.txt
            // - need an if demo trigger check "inserted" at save time
            // - .pdb needs WARI creator, ADD1 type
            // - data should be compressed for obfuscation purposes

            PdbPacker pdbp = new PdbPacker();

            // Add version.txt

            byte[] abT = new byte[strVersion.Length + 1];
            for (int n = 0; n < strVersion.Length; n++)
                abT[n] = (byte)strVersion[n];
            abT[abT.Length - 1] = 0;
            pdbp.Add(new PdbPacker.File("version.txt", abT));

            // Load res.h from embedded resource in prep for pre-process

            System.Reflection.Assembly ass = typeof(GobImage).Module.Assembly;
            Stream stmResDotH = ass.GetManifestResourceStream("m.EmbeddedResources." + "res.h");
            if (stmResDotH == null)
                throw new Exception("Cannot load res.h");

            // Compile levels

            Random rand = new Random();
            ArrayList alsTileSets = new ArrayList();
            foreach (LevelDoc lvld in adoc) {
                // Need to do this unfortunately; some of the "saving" code relies on what the "active" document
                // is!!

                DocManager.SetActiveDocument(typeof(LevelDoc), lvld);

                TemplateDoc tmpd = lvld.GetTemplateDoc();

                // Get appropriate TileSet, or make one if this map
                // uses a new tile collection

                TileSet tset = null;
                foreach (TileSet tsetT in alsTileSets) {
                    if (tsetT.TemplateDoc == tmpd) {
                        tset = tsetT;
                        break;
                    }
                }

                // Create new tile set if none found

                if (tset == null) {
                    tset = new TileSet(tmpd, tmpd.GetName() + ".tset");
                    alsTileSets.Add(tset);
                }

#if false
                // Generate base file name for this level (this is never user-visible, but it should be
                // as unique as possible

                char[] achBase = new char[16];
                for (int n = 0; n < achBase.Length; n++) {
                    int nT = rand.Next() % (26 + 10);
                    if (nT < 26) {
                        achBase[n] = (char)(nT + 97);
                    } else {
                        achBase[n] = (char)(nT + 48 - 26);
                    }
                }
                if (lvld.MaxPlayers > 1) {
                    achBase[0] = 'm';
                    achBase[1] = '_';
                }
                string strBase = new String(achBase);
#else
                // This isn't unique which can cause problems due to how packfiles work.
                // Could change packfile api to accept .pdb parameter.
                // Note1: set next mission action requires predictable filename
                // Note2: mission sorting is based on filename, not title
                // Could put lots of "support" in to fix these problems, or just ship
                // it like this.
                //
                // Hack: filename length 29
                // Maximum extension on filename: 7

                string strBase = lvld.Title;
                if (strBase.Length > 29 - 7)
                    strBase = strBase.Substring(0, 29 - 7);

                // If multiplayer, add "m_" to the name by losing the last two characters
                // so sort order is preserved

                if (lvld.MaxPlayers > 1)
                    strBase = "m_" + strBase.Substring(0, strBase.Length - 2);
#endif

                // Get tile map file for this level

                MemoryStream stmTmap = new MemoryStream();
                TileMap tmap = TileMap.CreateFromImage(tset, lvld.GetMapBitmap(tmpd.TileSize, tmpd, true), tmpd.TileSize);
                tmap.Save(stmTmap);
                string strTmap = strBase + ".tmap";
                pdbp.Add(new PdbPacker.File(strTmap, stmTmap.ToArray()));
                stmTmap.Close();

                // Get the terrain map file for this level

                MemoryStream stmTRmap = new MemoryStream();
                TerrainMap trmap = new TerrainMap(lvld.GetTerrainMap(tmpd.TileSize, tmpd, false));
                trmap.Save(stmTRmap);
                string strTRmap = strBase + ".trmap";
                pdbp.Add(new PdbPacker.File(strTRmap, stmTRmap.ToArray()));
                stmTRmap.Close();

                // Save .ini file for this level doc

                MemoryStream stmLvld = new MemoryStream();
                lvld.SaveIni(stmLvld, -1, strTmap, strTRmap, true);

                // Pre-process

                stmLvld.Seek(0, SeekOrigin.Begin);
                MemoryStream stmPreprocessed = Misc.PreprocessStream(stmLvld, stmResDotH);
                stmPreprocessed.Seek(0, SeekOrigin.Begin);
                Ini iniPreProcessed = new Ini(stmPreprocessed);

                MemoryStream stmLvldPreProcessedBinary = new MemoryStream();
                iniPreProcessed.SaveBinary(stmLvldPreProcessedBinary);
                stmLvldPreProcessedBinary.Close();

				string strLvlName = lvld.OutputFilename;
				if (strLvlName == null) {
					strLvlName = strBase + ".lvl";
				}
                pdbp.Add(new PdbPacker.File(strLvlName, stmLvldPreProcessedBinary.ToArray()));
                stmLvldPreProcessedBinary.Close();
            }
            stmResDotH.Close();

            // Restore active document

            if (lvldActive != null)
                DocManager.SetActiveDocument(typeof(LevelDoc), lvldActive);

            // Now save out pdb

            pdbp.Save(strFile, "WARI", "ADD2");
        }

        public static void MixMapImportSpecial(Theater theater, TemplateDoc tmpdCopyTerrain, string strFileSave) {
            TemplateDoc tmpd24 = (TemplateDoc)DocManager.NewDocument(typeof(TemplateDoc), new Object[] { new Size(24, 24) });
            MixTemplate[] amixt = MixSuck.LoadTemplates(theater);
            MixSuck.ImportTemplates(amixt, tmpd24);
            
            Template[] atmpl24 = tmpd24.GetTemplates();
            Template[] atmpl16 = tmpdCopyTerrain.GetTemplates();

            for (int n = 0; n < atmpl24.Length; n++) {
                Template tmpl24 = atmpl24[n];
                Template tmpl16 = atmpl16[n];
                tmpl24.TerrainMap = tmpl16.TerrainMap;
            }
            
            tmpd24.SetBackgroundTemplate(atmpl24[0]);
            tmpd24.SaveAs(strFileSave);
        }

        public static void MakePalette(string[] astr) {
            // -makepal 16 templates.tc palsize fixpalsize backgroundpalsize fixed.pal out.pal

            // tile size

            Size sizTile = new Size(0, 0);
            sizTile.Width = int.Parse(astr[1]);
            sizTile.Height = sizTile.Width;

            // Load template collection

            TemplateDoc tmpd = (TemplateDoc)DocManager.OpenDocument(astr[2]);

            // palette size

            int cPalEntries = int.Parse(astr[3]);

            // entries fixed

            int cPalEntriesFixed = int.Parse(astr[4]);

            // entries for background

            int cPalEntriesBackground = int.Parse(astr[5]);

            // fixed palette

            Palette palFixed = new Palette(astr[6]);

            // output palette

            string strFilePalOut = astr[7];

            // If this template collection already has a palette it has already been quantized; we don't
            // want that.

            if (tmpd.GetPalette() != null)
                new Exception("Template collection has already been quantized!");

            // Scale templates if needed

            if (sizTile.Width != tmpd.TileSize.Width || sizTile.Height != tmpd.TileSize.Height)
                TemplateTools.ScaleTemplates(tmpd, sizTile);

            // Quantize

            TemplateTools.QuantizeTemplates(tmpd, palFixed, cPalEntries, cPalEntriesFixed, cPalEntriesBackground);

            // Save the new palette out

            Palette palNew = tmpd.GetPalette();
            palNew.SaveJasc(strFilePalOut);
        }

        public static void ExportMixMaps(string[] astr) {
            // Expand filespecs
            ArrayList alsFiles = new ArrayList();
            for (int n = 1; n < astr.Length; n++) {
                string strFileT = Path.GetFileName(astr[n]);
                string strDirT = Path.GetDirectoryName(astr[n]);
                if (strDirT == "")
                    strDirT = ".";
                string[] astrFiles = Directory.GetFiles(strDirT, strFileT);
                alsFiles.AddRange(astrFiles);
            }

            foreach (string strFile in alsFiles) {
                LevelDoc lvld = (LevelDoc)DocManager.NewDocument(typeof(LevelDoc), null);
                Console.Write("Exporting " + strFile + " as ");
                string strFileExport = MixSuck.ImportExportMixMap(strFile, lvld);
                if (strFileExport == null) {
                    Console.Write("Error exporting!\n");
                } else {
                    Console.Write(strFileExport + "\n");
                }
                lvld.Dispose();
            }
        }

        public static void ExportImages(string[] astr) {
            // Get directory
            //string strDir = Path.GetFullPath(astr[1]);
            string strDir = ".";
            string strPrefix = astr[1];

            // Expand filespecs
            ArrayList alsFiles = new ArrayList();
            for (int n = 2; n < astr.Length; n++) {
                string strFileT = Path.GetFileName(astr[n]);
                string strDirT = Path.GetDirectoryName(astr[n]);
                if (strDirT == "")
                    strDirT = ".";
                string[] astrFiles = Directory.GetFiles(strDirT, strFileT);
                alsFiles.AddRange(astrFiles);
            }

            // Attempt to process these level docs
            foreach (string strFile in alsFiles) {
                Console.Write("Map of " + strFile + " -> ");
                LevelDoc lvld = (LevelDoc)DocManager.OpenDocument(strFile);
                if (lvld == null)
                    throw new Exception("Could not load level doc " + strFile);
                string strPng = strDir + Path.DirectorySeparatorChar + strPrefix + Path.GetFileName(strFile).Replace(".ld", ".png");
                Console.Write(strPng + "...");
                TemplateDoc tmpd = lvld.GetTemplateDoc();
                Bitmap bm = lvld.GetMapBitmap(tmpd.TileSize, tmpd, true);
                bm.Save(strPng, ImageFormat.Png);
                bm.Dispose();
                lvld.Dispose();
                Console.Write(" Done.\n");
            }
        }

        public static void ExportLevels(string[] astr, int nVersion) {
            // Get tile size

            Size sizTile = new Size(0, 0);
            sizTile.Width = int.Parse(astr[1]);
            sizTile.Height = sizTile.Width;

            // Get depth

            int nDepth = Int32.Parse(astr[2]);

            // Get background threshold

            double nAreaBackgroundThreshold = double.Parse(astr[3]);

            // Background luminance multiplier

            double nLuminanceMultBackground = double.Parse(astr[4]);

            // Background saturation multiplier

            double nSaturationMultBackground = double.Parse(astr[5]);

            // Foreground luminance multiplier

            double nLuminanceMultForeground = double.Parse(astr[6]);

            // Foreground saturation multiplier

            double nSaturationMultForeground = double.Parse(astr[7]);

            // Palette directory

            // string strPalDir = astr[8];

            // Get output directory

            string strDir = Path.GetFullPath(astr[8]);

            // Expand filespecs
            ArrayList alsFiles = new ArrayList();
            for (int n = 8; n < astr.Length; n++) {
                string strFileT = Path.GetFileName(astr[n]);
                string strDirT = Path.GetDirectoryName(astr[n]);
                if (strDirT == "")
                    strDirT = ".";
                string[] astrFiles = Directory.GetFiles(strDirT, strFileT);
                alsFiles.AddRange(astrFiles);
            }

            // Attempt to process these level doc
            ArrayList alsTileSets = new ArrayList();
            foreach (string strFile in alsFiles) {
                Console.Write("Loading " + strFile + "...");
                LevelDoc lvld = (LevelDoc)DocManager.OpenDocument(strFile);
                if (lvld == null)
                    throw new Exception("Could not load level doc " + strFile);
                Console.Write(" Done.\n");

                // Size this template collection if necessary

                TemplateDoc tmpd = lvld.GetTemplateDoc();
                if (sizTile.Width != tmpd.TileSize.Width || sizTile.Height != tmpd.TileSize.Height)
                    TemplateTools.ScaleTemplates(tmpd, sizTile);

                // Get appropriate TileSet, or make one if this map
                // uses a new tile collection
                TileSet tset = null;
                foreach (TileSet tsetT in alsTileSets) {
                    if (tsetT.TileCollectionFileName == Path.GetFileName(lvld.GetTemplateDoc().GetPath())) {
                        tset = tsetT;
                        break;
                    }
                }

                // Create new tile set if none found
                if (tset == null) {
                    string strTcPath = lvld.GetTemplateDoc().GetPath();
                    string strTcName = Path.GetFileNameWithoutExtension(strTcPath);
                    string strTcDir = Path.GetDirectoryName(strTcPath);
                    // string strT3 = strTcDir + Path.DirectorySeparatorChar + strPalDir + Path.DirectorySeparatorChar + strTcName;
                    tset = new TileSet(lvld.GetTemplateDoc(), Path.GetFileName(strTcPath), nDepth, sizTile);
                    alsTileSets.Add(tset);
                }

                // Get and save a tile map for this level
                string strTmap = Path.GetFileName(lvld.GetPath().Replace(".ld", ".tmap"));
                string strFileTmap = strDir + Path.DirectorySeparatorChar + strTmap;
                Console.Write("Creating & writing " + strFileTmap + "...");    
                TileMap tmap = TileMap.CreateFromImage(tset, lvld.GetMapBitmap(tmpd.TileSize, tmpd, true), tmpd.TileSize);
                tmap.Save(strFileTmap);
                Console.Write(" Done.\n");
                
                // Get and save terrain map for this level
                string strTrmap = Path.GetFileName(lvld.GetPath().Replace(".ld", ".trmap"));
                string strFileTrmap = strDir + Path.DirectorySeparatorChar + strTrmap;
                Console.Write("Creating & writing " + strFileTrmap + "...");
                TerrainMap trmap = new TerrainMap(lvld.GetTerrainMap(tmpd.TileSize, tmpd, false));
                trmap.Save(strFileTrmap);
                Console.Write(" Done.\n");

                // Save .ini for this level doc
                string strFileIni = strDir + Path.DirectorySeparatorChar + Path.GetFileName(strFile).Replace(".ld", ".lvl");
                Console.Write("Writing " + strFileIni + "...");
                lvld.SaveIni(strFileIni, nVersion, strTmap, strTrmap);
                Console.Write(" Done.\n");
                lvld.Dispose();
            }

            // Now write out tilesets
            foreach (TileSet tset in alsTileSets) {
                // Save tile set
                string strFileTset = strDir + Path.DirectorySeparatorChar + tset.FileName;
                Console.Write("Creating & writing " + strFileTset + ", " + tset.Count.ToString() + " tiles...");
                tset.Save(strFileTset);
                tset.SaveMini(strFileTset, nAreaBackgroundThreshold, nLuminanceMultBackground, nSaturationMultBackground, nLuminanceMultForeground, nSaturationMultForeground);
                Console.Write(" Done.\n");
            }
        }

        public static void ExportText(string[] astr) {
            // Expand filespecs
            ArrayList alsFiles = new ArrayList();
            for (int n = 1; n < astr.Length; n++) {
                string strFileT = Path.GetFileName(astr[n]);
                string strDirT = Path.GetDirectoryName(astr[n]);
                if (strDirT == "")
                    strDirT = ".";
                string[] astrFiles = Directory.GetFiles(strDirT, strFileT);
                alsFiles.AddRange(astrFiles);
            }

            Console.WriteLine("Exporting text from {0} files", alsFiles.Count);

            // Attempt to process these level docs

            foreach (string strFile in alsFiles) {
                Console.Write("Writing text of " + strFile + "...");
                LevelDoc lvld = (LevelDoc)DocManager.OpenDocument(strFile);
                if (lvld == null)
                    throw new Exception("Could not load level doc " + strFile);
                string strTextFile = "." + Path.DirectorySeparatorChar + Path.GetFileName(strFile).Replace(".ld", ".txt");
                string str = lvld.GetLevelText();
                str = str.Replace("\n", "\r\n");
                StreamWriter stmw = new StreamWriter(strTextFile);
                stmw.Write(str);
                stmw.Close();
                lvld.Dispose();
                Console.WriteLine(" done");
            }
        }

        public static void ImportText(string[] astr) {
            // Expand filespecs
            ArrayList alsFiles = new ArrayList();
            for (int n = 1; n < astr.Length; n++) {
                string strFileT = Path.GetFileName(astr[n]);
                string strDirT = Path.GetDirectoryName(astr[n]);
                if (strDirT == "")
                    strDirT = ".";
                string[] astrFiles = Directory.GetFiles(strDirT, strFileT);
                alsFiles.AddRange(astrFiles);
            }

            Console.WriteLine("Importing text from {0} files", alsFiles.Count);

            // Attempt to process these text files/level docs

            foreach (string strTextFile in alsFiles) {
                string strFile = "." + Path.DirectorySeparatorChar + Path.GetFileName(strTextFile).Replace(".txt", ".ld");
                Console.Write("Writing " + strTextFile + " to " + strFile + "...");
                LevelDoc lvld = (LevelDoc)DocManager.OpenDocument(strFile);
                if (lvld == null)
                    throw new Exception("Could not load level doc " + strFile);

                StreamReader stmr = new StreamReader(strTextFile);
                string str = stmr.ReadToEnd();
                stmr.Close();
                str = str.Replace("\r\n", "\n");

                int ichErrorPos;
                if (!lvld.SetLevelText(str, out ichErrorPos)) {
                    Console.WriteLine(" error at char " + ichErrorPos);
                } else {
                    lvld.Save();
                    Console.WriteLine(" saved");
                }
                lvld.Dispose();
            }
        }
    }

	class TerrainMap {
		private TerrainTypes[,] m_aterMap;

		public TerrainMap(TerrainTypes[,] aterMap) {
			m_aterMap = aterMap;
		}

		public void Save(string strFileTrmap) {
			Stream stm = new FileStream(strFileTrmap, FileMode.Create, FileAccess.Write, FileShare.None);
			Save(stm);
		}

		public void Save(Stream stm) {
			BinaryWriter bwtr = new BinaryWriter(stm);

			//struct TerrainMapHeader { // trmhdr
			//    word ctx;
			//    word cty;
			//};

			bwtr.Write(Misc.SwapUShort((ushort)m_aterMap.GetLength(1)));
			bwtr.Write(Misc.SwapUShort((ushort)m_aterMap.GetLength(0)));
			for (int ty = 0; ty < m_aterMap.GetLength(0); ty++) {
				for (int tx = 0; tx < m_aterMap.GetLength(1); tx++) {
					byte b = 0;
					switch (m_aterMap[ty, tx]) {
					default:
					case TerrainTypes.Open:
						b = 1;
						break;

					case TerrainTypes.Area:
						b = 0;
						break;

					case TerrainTypes.Wall:
						b = 2;
						break;

					case TerrainTypes.Blocked:
						b = 3;
						break;
					}
					bwtr.Write(b);
				}
			}
			bwtr.Close();
		}

		public static TerrainMap Load(MemoryStream stm) {
			BinaryReader brdr = new BinaryReader(stm);
			int ctx = (int)Misc.SwapUShort(brdr.ReadUInt16());
			int cty = (int)Misc.SwapUShort(brdr.ReadUInt16());
			TerrainTypes[,] map = new TerrainTypes[cty, ctx];
			for (int ty = 0; ty < cty; ty++) {
				for (int tx = 0; tx < ctx; tx++) {
					byte b = brdr.ReadByte();
					switch (b) {
					case 0: // Area
						map[ty, tx] = TerrainTypes.Area;
						break;

					case 1: // Open
						map[ty, tx] = TerrainTypes.Open;
						break;

					case 2: // Wall
						map[ty, tx] = TerrainTypes.Wall;
						break;

					case 3: // Blocked
						map[ty, tx] = TerrainTypes.Blocked;
						break;

					default: // Default to Open
						map[ty, tx] = TerrainTypes.Open;
						break;
					}
				}
			}
			brdr.Close();
			return new TerrainMap(map);
		}

		public TerrainTypes[,] Map {
			get {
				return m_aterMap;
			}
		}
    }

    class TileMap {
        int m_ctx;
        int m_cty;
        Size m_sizTile;
        int[,] m_aiTile;
        string m_strTSetFileName;

        public TileMap(string strTSetFileName, int ctx, int cty, Size sizTile) {
            m_strTSetFileName = strTSetFileName;
            m_ctx = ctx;
            m_cty = cty;
            m_sizTile = sizTile;
            m_aiTile = new int[cty, ctx];
        }

		public TileMap(string strTSetFileName, int ctx, int cty, int[,] aiTile, Size sizTile) {
			m_strTSetFileName = strTSetFileName;
			m_ctx = ctx;
			m_cty = cty;
			m_aiTile = aiTile;
			m_sizTile = sizTile;
		}

		public string Filename {
			get {
				return m_strTSetFileName;
			}
		}

		public int Width {
			get {
				return m_ctx;
			}
		}

		public int Height {
			get {
				return m_cty;
			}
		}

        public static TileMap CreateFromImage(TileSet tset, Bitmap bm, Size sizTile) {
            int ctx = bm.Width / sizTile.Width;
            int cty = bm.Height / sizTile.Height;
            TileMap tmap = new TileMap(tset.FileName, ctx, cty, sizTile);
            tmap.InitFromImage(tset, bm);
            return tmap;
        }

        public void InitFromImage(TileSet tset, Bitmap bm) {
            Color[] aclrTile = new Color[m_sizTile.Width * m_sizTile.Height];
            for (int ty = 0; ty < m_cty; ty++) {
                for (int tx = 0; tx < m_ctx; tx++) {
                    TileSet.ExtractTilePixels(bm, tx, ty, m_sizTile, ref aclrTile);
                    int index = tset.FindTileIndex(aclrTile);
                    // if (index == -1)
                        // throw new Exception("Couldn't find tile index!");
                    m_aiTile[ty, tx] = index;
                }
            }
        }

        public void Save(string strFileTmap) {
            Stream stm = new FileStream(strFileTmap, FileMode.Create, FileAccess.Write, FileShare.None);
            Save(stm);
        }

        public void Save(Stream stm) {
            BinaryWriter bwtr = new BinaryWriter(stm);

            // #define kcbFilename 28
            // struct TileMapHeader { // thdr
            //     char szFnTset[kcbFilename];
            //     word ctx;
            //    word cty;
            // };

            char[] szFnTset = new char[28];
            m_strTSetFileName.CopyTo(0, szFnTset, 0, m_strTSetFileName.Length);
            bwtr.Write(szFnTset);
            bwtr.Write(Misc.SwapUShort((ushort)m_ctx));
            bwtr.Write(Misc.SwapUShort((ushort)m_cty));

            for (int ty = 0; ty < m_cty; ty++) {
                for (int tx = 0; tx < m_ctx; tx++) {
                    bwtr.Write(Misc.SwapUShort((ushort)(m_aiTile[ty, tx] << 2)));
                }
            }
            bwtr.Close();
        }

		public static TileMap Load(MemoryStream stm) {
			BinaryReader brdr = new BinaryReader(stm);
			byte[] ab = brdr.ReadBytes(28);
			int i = 0;
			for (; i < ab.Length; i++) {
				if (ab[i] == 0) {
					break;
				}
			}
			string strFilename = Encoding.ASCII.GetString(ab, 0, i);
			int ctx = Misc.SwapUShort(brdr.ReadUInt16());
			int cty = Misc.SwapUShort(brdr.ReadUInt16());

			int [,] aiTile = new int[cty, ctx];
			for (int ty = 0; ty < cty; ty++) {
				for (int tx = 0; tx < ctx; tx++) {
					aiTile[ty, tx] = Misc.SwapUShort(brdr.ReadUInt16()) >> 2;
				}
			}
			brdr.Close();

			Size sizTile = new Size(24, 24); // hardwired
			return new TileMap(strFilename, ctx, cty, aiTile, sizTile);
		}

		public int GetTileIndex(int tx, int ty) {
			if (tx < 0 || tx >= m_aiTile.GetLength(1)) {
				return -1;
			}
			if (ty < 0 || ty >= m_aiTile.GetLength(0)) {
				return -1;
			}
			return m_aiTile[ty, tx];
		}
    }

    struct TileData {
        public int hash;
        public Color[] aclr;
		public int iTemplate;
		public int txTemplate;
		public int tyTemplate;
    }

    class TileSet {
        public string FileName;
        public string TileCollectionFileName;
        // public string PalBinFileName;
        public TemplateDoc TemplateDoc;
        private ArrayList m_alsTileData = new ArrayList();
        // private Palette m_pal = null;
        private static int s_cbFileMax = 32000;
        Size m_sizTile;

        public TileSet(TemplateDoc tmpd, string strFile) {
            TemplateDoc = tmpd;
            // m_pal = tmpd.GetPalette();
            m_sizTile = tmpd.TileSize;
            FileName = strFile.Replace(".tc", ".tset");
            SuckTemplates();
        }

        public TileSet(TemplateDoc tmpd, string strFile, int nDepth, Size sizTile) {
            TemplateDoc = tmpd;
            // m_pal = new Palette(strFilePal + "_" + nDepth.ToString() + "bpp.pal");
            TileCollectionFileName = strFile;
            FileName = strFile.Replace(".tc", ".tset");
            // PalBinFileName = Path.GetFileName(strFilePal) + ".palbin";
            m_sizTile = sizTile;
            SuckTemplates();
        }

        private void SuckTemplates() {
            // Suck all the tiles in

            Template[] atmpl = TemplateDoc.GetTemplates();
			int iTemplate = 0;
            foreach (Template tmpl in atmpl) {
                bool[,] afOccupancy = tmpl.OccupancyMap;
                for (int ty = 0; ty < afOccupancy.GetLength(0); ty++) {
                    for (int tx = 0; tx < afOccupancy.GetLength(1); tx++) {
                        if (!afOccupancy[ty, tx])
                            continue;
                        TileData tdata = new TileData();
						tdata.iTemplate = iTemplate;
						tdata.txTemplate = tx;
						tdata.tyTemplate = ty;
                        tdata.aclr = new Color[m_sizTile.Width * m_sizTile.Height];
                        ExtractTilePixels(tmpl.Bitmap, tx, ty, m_sizTile, ref tdata.aclr);
                        tdata.hash = HashTile(tdata.aclr);
                        m_alsTileData.Add(tdata);
                    }
                }
				iTemplate++;
            }
        }

        public static unsafe void ExtractTilePixels(Bitmap bm, int tx, int ty, Size sizTile, ref Color[] aclrTile) {
            Rectangle rcSrc = new Rectangle(tx * sizTile.Width, ty * sizTile.Height, sizTile.Width, sizTile.Height);
            BitmapData bmd = bm.LockBits(rcSrc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
            IntPtr p = bmd.Scan0;
            byte *pbBase = (byte *)p.ToPointer();
            for (int y = 0; y < sizTile.Height; y++) {
                for (int x = 0; x < sizTile.Width; x++) {
                    byte *pb = pbBase + y * bmd.Stride + x * 3;
                    aclrTile[y * sizTile.Width + x] = Color.FromArgb(pb[2], pb[1], pb[0]);
                }
            }
            bm.UnlockBits(bmd);
        }

        public int FindTileIndex(Color[] aclrTile) {
            int hash = HashTile(aclrTile);
            for (int n = 0; n < m_alsTileData.Count; n++) {
                TileData td = (TileData)m_alsTileData[n];
                if (hash == td.hash && CheckTileMatch(aclrTile, td.aclr))
                    return n;
            }
            return -1;
        }

        int HashTile(Color[] aclrTile) {
            int hash = 0;
            for (int n = 0; n < aclrTile.Length; n++) {
                Color clr = aclrTile[n];
                hash += (hash << 1) | (clr.R << 16) + (clr.G << 8) + clr.B;
            }
            return hash;
        }

        bool CheckTileMatch(Color[] aclr1, Color[] aclr2) {
            for (int n = 0; n < aclr1.Length; n++) {
                if (aclr1[n] != aclr2[n])
                    return false;
            }
            return true;
        }

        public void Save(string strFileTset) {
            // struct TileSetHeader { // tshdr
            //    ushort cTiles;
            //    ushort cxTile;
            //    ushort cyTile;
            // };

            int cbTile = m_sizTile.Width * m_sizTile.Height;
            int cTilesFit = (s_cbFileMax - 6) / cbTile;
            int cTilesLeft = m_alsTileData.Count;

            int nFile = 0;
            int nTile = 0;
            while (cTilesLeft != 0) {
                // Get filename
                string strT = strFileTset;
                if (nFile > 0)
                    strT = strFileTset.Replace(".tset", ".tset." + nFile.ToString());
                nFile++;

                // Open file for writing
                Stream stm = new FileStream(strT, FileMode.Create, FileAccess.Write, FileShare.None);
                BinaryWriter bwtr = new BinaryWriter(stm);

                // Figure out how many tiles will go in this file        
                int cTilesWrite = Math.Min(cTilesFit, cTilesLeft);

                // Write out the header
                bwtr.Write(Misc.SwapUShort((ushort)cTilesWrite));
                bwtr.Write(Misc.SwapUShort((ushort)m_sizTile.Width));
                bwtr.Write(Misc.SwapUShort((ushort)m_sizTile.Height));

                // Write out the tiles
                for (int n = 0; n < cTilesWrite; n++) {
                    int[] bytes = GetTileBytes(nTile);
                    for (int i = 0; i < bytes.Length; i++) {
                        bwtr.Write(bytes[i]);
                    }
                    nTile++;
                }

                // Next file
                bwtr.Close();
                cTilesLeft -= cTilesWrite;
            }
        }

        public void SaveMini(string strFileTset, double nAreaBackgroundThreshold, double nLuminanceMultBackground, double nSaturationMultBackground, double nLuminanceMultForeground, double nSaturationMultForeground) {
			// Now write out minimap tiles, 2x2 and 1x1

            Stream stmT = new FileStream(strFileTset.Replace(".tset", ".tsetmini"), FileMode.Create, FileAccess.Write, FileShare.None);
            BinaryWriter bwtrMini = new BinaryWriter(stmT);
            WriteMiniTiles(bwtrMini, TemplateDoc, 2, true, nAreaBackgroundThreshold, nLuminanceMultBackground, nSaturationMultBackground, nLuminanceMultForeground, nSaturationMultForeground);
            WriteMiniTiles(bwtrMini, TemplateDoc, 1, false, nAreaBackgroundThreshold, nLuminanceMultBackground, nSaturationMultBackground, nLuminanceMultForeground, nSaturationMultForeground);
            bwtrMini.Close();
        }

        void WriteMiniTiles(BinaryWriter bwtr, TemplateDoc tmpd, int cx, bool fNext, double nAreaBackgroundThreshold, double nLuminanceMultBackground, double nSaturationMultBackground, double nLuminanceMultForeground, double nSaturationMultForeground) {
            // struct MiniTileSetHeader { // mtshdr
            //  ushort offNext;
            //    ushort cTiles;
            //    ushort cxTile;
            //    ushort cyTile;
            // };
            
            ushort offNext = 0;
            if (fNext)
                offNext = (ushort)(8 + m_alsTileData.Count * cx * cx * 4);
            bwtr.Write(Misc.SwapUShort(offNext));
            bwtr.Write(Misc.SwapUShort((ushort)m_alsTileData.Count));
            bwtr.Write(Misc.SwapUShort((ushort)cx));
            bwtr.Write(Misc.SwapUShort((ushort)cx));

            // If a background template exists, use it to distinguish foreground from background objects for better minimaps

            Size sizTile = tmpd.TileSize;
            ArrayList alsColors = new ArrayList();
            Template tmplBackground = tmpd.GetBackgroundTemplate();
            if (tmplBackground != null && nAreaBackgroundThreshold >= 0.0) {
                // Get despeckled hue map of background, calc mean and
                // std dev for filtering purposes

                Bitmap bmHueBackground = TemplateTools.MakeHueMap(tmplBackground.Bitmap);
                TemplateTools.DespeckleGrayscaleBitmap(bmHueBackground, 9, 50);
                double nMean = TemplateTools.CalcGrayscaleMean(bmHueBackground);
                double nStdDev = TemplateTools.CalcGrayscaleStandardDeviation(bmHueBackground, nMean);

                // Go through each tile, first make a mask that'll delineate foreground from background

                Bitmap bmTile = new Bitmap(sizTile.Width, sizTile.Height);
                foreach (TileData td in m_alsTileData) {
                    // Need to turn data back into a bitmap - doh!

                    for (int y = 0; y < sizTile.Height; y++) {
                        for (int x = 0; x < sizTile.Width; x++) {
                            Color clr = (Color)td.aclr[y * sizTile.Width + x];
                            bmTile.SetPixel(x, y, clr);
                        }
                    }

                    // Create mask which'll replace background with transparent color (255, 0, 255)

                    Bitmap bmMask = TemplateTools.MakeHueMap(bmTile);
                    TemplateTools.DespeckleGrayscaleBitmap(bmMask, 9, 50);
                    TemplateTools.SubtractGrayscaleDistribution(bmMask, nMean, nStdDev);

                    // Now scale tile down to desired size, using mask as input

                    Bitmap bmScaled = TemplateTools.ScaleTemplateBitmap(bmTile, bmMask, cx, cx, nAreaBackgroundThreshold, nLuminanceMultBackground, nSaturationMultBackground, nLuminanceMultForeground, nSaturationMultForeground);

                    // Grab the data

                    for (int y = 0; y < cx; y++) {
                        for (int x = 0; x < cx; x++) {
                            alsColors.Add(bmScaled.GetPixel(x, y));
                        }
                    }
                    bmScaled.Dispose();
                    bmMask.Dispose();
                }
            } else {
                // No background template; just scale

                Bitmap bmTile = new Bitmap(sizTile.Width, sizTile.Height);
                foreach (TileData td in m_alsTileData) {
                    // Need to turn data back into a bitmap - doh!

                    for (int y = 0; y < sizTile.Height; y++) {
                        for (int x = 0; x < sizTile.Width; x++) {
                            Color clr = (Color)td.aclr[y * sizTile.Width + x];
                            bmTile.SetPixel(x, y, clr);
                        }
                    }

                    // Now scale tile down to desired size, using mask as input

                    Bitmap bmScaled = TemplateTools.ScaleTemplateBitmap(bmTile, null, cx, cx, 1.0, 1.0, 1.0, 1.0, 1.0);

                    // Grab the data

                    for (int y = 0; y < cx; y++) {
                        for (int x = 0; x < cx; x++) {
                            alsColors.Add(bmScaled.GetPixel(x, y));
                        }
                    }
                    bmScaled.Dispose();
                }
            }

            // Palette match and write results

            foreach (Color clr in alsColors) {
                bwtr.Write(((clr.R << 24) | (clr.G << 16) | (clr.B << 8) | clr.A));
            }
        }

        int[] GetTileBytes(int nTile) {
            int[] ab = new int[m_sizTile.Width * m_sizTile.Height];
            TileData tdata = (TileData)m_alsTileData[nTile];
            for (int n = 0; n < tdata.aclr.Length; n++) {
                ab[n] = (
                    (tdata.aclr[n].R << 24) | 
                    (tdata.aclr[n].G << 16) | 
                    (tdata.aclr[n].B << 8) | 
                    (tdata.aclr[n].A << 0)
                );
            }

            return ab;
        }

        public int Count {
            get {
                return m_alsTileData.Count;
            }
        }

		public TileData GetTileData(int iTile) {
			return (TileData)m_alsTileData[iTile];
		}
    }
}
