using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using SpiffCode;
using SpiffLib;

namespace acrunch
{
    /// <summary>
    /// Summary description for Class1.
    /// </summary>
    class App
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static int Main(string[] astrArgs) {

            if (astrArgs.Length < 2 || astrArgs.Length > 9) {
                Console.WriteLine("Usage:\nacrunch [-scale N] [-save] [-noscaleicon] [-noshrinkwrap] [-highlight <cxTile>] [-stats] <input.amx> <outdir>\n");
                return -1;
            }

            double nScale = 1.0;
            bool fSave = false;
            bool fNoShrinkWrap = true;
            bool fDumpStats = false;
            bool fScaleIcon = true;
            bool fMakeHighlight = false;
            int cxTile = 0;

            int iarg = 0;
            while (astrArgs[iarg][0] == '-') {
                switch (astrArgs[iarg]) {
                case "-scale":
                    nScale = double.Parse(astrArgs[++iarg]);
                    break;

                case "-noscaleicon":
                    fScaleIcon = false;
                    break;

                case "-save":
                    fSave = true;
                    break;

                case "-noshrinkwrap":
                    fNoShrinkWrap = true;
                    break;

                case "-stats":
                    fDumpStats = true;
                    break;

                case "-highlight":
                    fMakeHighlight = true;
                    cxTile = Int32.Parse(astrArgs[++iarg]);
                    break;
                }

                iarg++;
            }

#if false
            // Read in the palette

            string strFilePal = astrArgs[iarg++];
            Palette pal = new Palette(strFilePal);
            if (pal == null) {
                Console.WriteLine("Error: unable to read the palette file {0}\n", strFilePal);
                return -1;
            }
#endif

            // Read in the animation file (.amx)

            string strFileAmx = astrArgs[iarg++];
            AnimDoc doc = AnimDoc.Load(strFileAmx);
            if (doc == null) {
                Console.WriteLine("Error: unable to read animation file {0}\n", strFileAmx);
                return -1;
            }

            // Get directory

            string strDir = astrArgs[iarg++];

            // Reduce the Bitmaps down to the tightest rectangular boundary around the
            // non-transparent pixels.

            if (!fNoShrinkWrap)
                ShrinkWrap(doc);

            // Make highlight if asked. This uses existing images, so do this
            // before scaling.

            Strip stpHighlight = null;
            if (fMakeHighlight) {
                stpHighlight = MakeHighlightStrip(doc, cxTile, null);
            }

            // Scale if asked

            if (nScale != 1.0) {
                Scale(doc, nScale, null, fScaleIcon);
            }

            // Add in highlight strip
       
            if (stpHighlight != null) { 
                doc.StripSet.Add(stpHighlight);
                foreach (Frame fr in stpHighlight) {
                    foreach (BitmapPlacer plc in fr.BitmapPlacers) {
                        doc.XBitmapSet.Add(plc.XBitmap);
                    }
                }
            }

            // If dump stats, we're *only* dumping stats

            if (fDumpStats) {
                DumpStats(doc);
                return 0;
            }

            // Write the runtime animation file (.anir) and crunched bitmaps

            if (fSave)
                doc.Save(strDir + Path.DirectorySeparatorChar + Path.GetFileName(strFileAmx));
            else
                doc.WriteAnir(strDir, Path.GetFileNameWithoutExtension(strFileAmx));
            return 0;
        }

        static void DumpStats(AnimDoc doc) {
            Console.WriteLine("Filename: " + doc.FileName.ToLower());
            Console.WriteLine("FrameRate: " + doc.FrameRate);
            Console.WriteLine("Strip Count: " + doc.StripSet.Count);
            foreach (Strip stp in doc.StripSet) {
                Console.WriteLine("Strip Name: " + stp.Name);
                Console.WriteLine("Frame Count: " + stp.Count);
            }
        }

        static Strip MakeHighlightStrip(AnimDoc doc, int cxTile,
                Palette palFixed) {
            Strip stpHelp = doc.StripSet["help"];
            if (stpHelp == null) {
                return null;
            }

            // This does a deep copy
            Strip stpHighlight = (Strip)stpHelp.Clone();
            stpHighlight.Name = "highlight";

            // Figure out the scaling. It would be better to pass this in as
            // a parameter.

            Frame frT = stpHighlight[0];
            Rectangle rcUnion = new Rectangle();
            foreach (BitmapPlacer plc in frT.BitmapPlacers) {
                Rectangle rc = new Rectangle();
                rc.X = -plc.X;
                rc.Y = -plc.Y;
                rc.Width = plc.XBitmap.Width;
                rc.Height = plc.XBitmap.Height;
				if (rcUnion.IsEmpty) {
					rcUnion = rc;
				} else {
					rcUnion = Rectangle.Union(rcUnion, rc);
				}
            }
         
			// Needs to be 4 tiles high. Keep aspect ratio

            int cy = rcUnion.Height;
            if (cy < cxTile * 4) {
                cy = cxTile * 4;
            }
			double nScale = 1.0;
			if (cy > rcUnion.Height) {
				nScale = (double)cy / (double)rcUnion.Height;
			}

            // Scale

			if (nScale != 1.0) {
				foreach (Frame fr in stpHighlight) {
					foreach (BitmapPlacer plc in fr.BitmapPlacers) {
						plc.XBitmap.Bitmap = TBitmapTools.ScaleBitmap(
							plc.XBitmap.Bitmap, nScale, palFixed);
						plc.X = (int)Math.Round(plc.X * nScale);
						plc.Y = (int)Math.Round(plc.Y * nScale);
					}

					Point pt = new Point();
					pt.X = (int)Math.Round(fr.SpecialPoint.X * nScale);
					pt.Y = (int)Math.Round(fr.SpecialPoint.Y * nScale);
					fr.SpecialPoint = pt;
				}
			}

            return stpHighlight;
        }

        static void Scale(AnimDoc doc, double nScale, Palette palFixed, bool fScaleIcon) {
            if (nScale >= 1.5)
                doc.Hires = true;

            // If not scaling icon, make a clone of it so it doesn't get scaled,
            // then after scaling set this clone back in.

            Strip stpIcon = doc.StripSet["icon"];
            Strip stpIconClone = null;
            if (!fScaleIcon && stpIcon != null) {
                stpIconClone = (Strip)stpIcon.Clone();
            }

            // Scale all the bitmaps

            foreach (XBitmap xbm in doc.XBitmapSet) {
                // Scale

                //xbm.Bitmap = TBitmapTools.ScaleBitmap(xbm.Bitmap, nScale, palFixed);

                // Scale the points in the frames that use this bitmap

                foreach (Strip stp in doc.StripSet) {
                    foreach (Frame fr in stp) {
                        foreach (BitmapPlacer plc in fr.BitmapPlacers) {
                            if (plc.XBitmap == xbm) {
                                plc.X = (int)Math.Round(plc.X * nScale);
                                plc.Y = (int)Math.Round(plc.Y * nScale);
                            }
                        }
                    }
                }
            }

            // Scale all special points too

            foreach (Strip stp in doc.StripSet) {
                foreach (Frame fr in stp) {
                    Point pt = new Point();
                    pt.X = (int)Math.Round(fr.SpecialPoint.X * nScale);
                    pt.Y = (int)Math.Round(fr.SpecialPoint.Y * nScale);
                    fr.SpecialPoint = pt;
                }
            }

            // Put the strip icon back in if it shouldn't be scaled

            if (!fScaleIcon && stpIconClone != null) {
                // Patch in the clone and add the images to the XBitmapSet
                // (they are not added auto-magically).

                doc.StripSet[doc.StripSet.IndexOf(stpIcon)] = stpIconClone;
                foreach (Frame fr in stpIconClone) {
                    foreach (BitmapPlacer plc in fr.BitmapPlacers) {
                        doc.XBitmapSet.Add(plc.XBitmap);
                    }
                }
            }
        }

        static void ShrinkWrap(AnimDoc doc) {
            Color clrTransparent = Color.FromArgb(0xff, 0, 0xff);
            SolidBrush brTransparent = new SolidBrush(clrTransparent);

            foreach (XBitmap xbm in doc.XBitmapSet) {

                Bitmap bm = xbm.Bitmap;

                // OPT: this could be made faster by doing four independent edge scans

                int xL = bm.Width;
                int xR = 0;
                int yT = bm.Height;
                int yB = 0;
                for (int y = 0; y < bm.Height; y++) {
                    for (int x = 0; x < bm.Width; x++) {
                        Color clr = bm.GetPixel(x, y);
                        if (clr != clrTransparent) {
                            xL = Math.Min(xL, x);
                            xR = Math.Max(xR, x);
                            yT = Math.Min(yT, y);
                            yB = Math.Max(yB, y);
                        }
                    }
                }
                int cx = xR - xL + 1;
                int cy = yB - yT + 1;

                Bitmap bmT = new Bitmap(cx, cy, PixelFormat.Format24bppRgb);
                using (Graphics g = Graphics.FromImage(bmT)) {
                    Rectangle rcT = new Rectangle(xL, yT, cx, cy);
                    g.DrawImage(bm, 0, 0, rcT, GraphicsUnit.Pixel);
                }

                xbm.Bitmap = bmT;

                // Don't need this anymore

                bm.Dispose();

                // If the upper-left corner of the bitmap has been adjusted
                // we must adjust the origins of all Frames referencing it.

                if (xL != 0 || yT != 0) {
                    foreach (Strip stp in doc.StripSet) {
                        foreach (Frame fr in stp) {
                            foreach (BitmapPlacer plc in fr.BitmapPlacers) {
                                if (plc.XBitmap == xbm) {
                                    plc.X -= xL;
                                    plc.Y -= yT;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
