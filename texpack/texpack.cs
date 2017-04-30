using System;
using SpiffCode;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using SpiffLib;
using System.Linq;

namespace SpiffCode {

    class MainClass {

        public static void Main(string[] args) {

            if (args.Count() < 1 || args.Count() > 2) {
                Console.WriteLine("Usage: <in json> <out dir>");
                return;
            }

            string outdir = args[1];
            string infile = System.IO.File.ReadAllText(args[0]);
            InputJsonObject dict = (InputJsonObject)Json.DeserializeJson<InputJsonObject>(infile);

            // outtrcs is used later to generate an output json that 
            // includes info on all the images in all the packers

            List<TexRect> outtrcs = new List<TexRect>();

            foreach (InputJsonObject.Packer packerinfo in dict.packers) {
                Console.WriteLine("Processing: {0}", packerinfo.name);

                Packer packer = new Packer(
                    packerinfo.name,
                    packerinfo.width,
                    packerinfo.height,
                    packerinfo.search_resolution,
                    packerinfo.sort_type
                );

                // When a trc is added to packer, a black_ variant will
                // be added to packerBlack. Since packerBlack is compiled
                // with the same specs and same number of trcs with the same 
                // size, each image's black_ variant should be packed in the
                // same location. This allows the result of the packerBlack to
                // essentially be a black_ for the entire result of packer
                // NOTE: if this process fails, fcanPackBlackPacker should get set to false

                Packer packerBlack = new Packer(
                    String.Format("black_{0}", packerinfo.name),
                    packerinfo.width,
                    packerinfo.height,
                    packerinfo.search_resolution,
                    packerinfo.sort_type
                );
                bool fcanPackBlackPacker = true;

                foreach (InputJsonObject.Packer.Image imageinfo in packerinfo.images) {

                    // Paths should be relative to the art dir

                    string path = string.Format("{0}/{1}", dict.art_dir, imageinfo.path);
                    string pathBlack = null;
                    if (imageinfo.black != null)
                        pathBlack = string.Format("{0}/{1}", dict.art_dir, imageinfo.black);

                    // Load the bitmaps

                    if (!File.Exists(path)) {
                        Console.WriteLine("{0} does not exist!", path);
                        throw new Exception("Requested bitmap does not exist");
                    }
                    Bitmap bm = new Bitmap(path);
                    Bitmap bmBlack = null;
                    if (File.Exists(pathBlack)) {
                        bmBlack = new Bitmap(pathBlack);
                    }

                    // Scale BEFORE mapping transparency

                    if (imageinfo.scale != 1.0) {
                        
                        // NOTE: I think this scaling method was designed for 824 art

                        bm = TBitmapTools.ScaleBitmap(bm, imageinfo.scale, null);
                        if (bmBlack != null)
                            bmBlack = TBitmapTools.ScaleBitmap(bmBlack, imageinfo.scale, null);
                    }

                    // Old 8 bit art needs some processing

                    if (imageinfo.black == "8bit") {
                        bm = BitmapTools.MapBitmapTransparency(bm);
                        bm = BitmapTools.MapBitmapShadow(bm, dict.shadow_8bit);
                        bmBlack = BitmapTools.CreateBlackBitmap(bm);

                        // HACK: Shift the hue up by 18 so that it
                        // "matches" the 2432 hue and can be hued with
                        // along with the 2432 art

                        Bitmap bmSub = SideMap.Subtract(bm, bmBlack);
                        Bitmap bmHew = SideMap.ShiftHue(bmSub, 18);
                        bm = SideMap.Add(bmHew, bmBlack);
                    }

                    if (bmBlack != null) {
                        // It's okay to not have a black_
                        // But if we do, it better be the right size
                        if (bm.Width != bmBlack.Width || bm.Height != bmBlack.Height)
                            throw new Exception("Bitmap black_ must be the same size as the bitmap");
                    } else {
                        // A missing black means that a black_ variant
                        // of the packer's result can't be created
                        fcanPackBlackPacker = false;
                    }

                    // Caulcaute atlases

                    List<int> atlases = new List<int>();
                    if (packerinfo.hue_variants != null)
                        foreach (InputJsonObject.Packer.HueVariant hv in packerinfo.hue_variants)
                            atlases.Add(hv.index);

                    if (atlases.Count != 0 && atlases.Count != 3) {
                        Console.WriteLine("{0} hue variations found for {1}", atlases.Count, packerinfo.name);
                        Console.WriteLine("Atlas should have exacly 0 variations or exactly 3 hue variations");
                        throw new Exception("Incorrect number of hue variations for atlas");
                    }

                    if (atlases.Count == 3 && packerinfo.grayscale.name == null) {
                        Console.WriteLine("{0} must have grayscale (for side neutral)", packerinfo.name);
                        throw new Exception("Missing grayscale entry for sidemapped packer");
                    }

                    // 0 means the "base index" is used for all sides

                    if (atlases.Count == 0) {
                        for (int i = 0; i < 5; i++)
                            atlases.Add(packerinfo.index);
                    } else { // if 3
                        atlases.Add(packerinfo.index);
                        atlases.Add(packerinfo.grayscale.index);
                    }

                    // Sort atlases assuming that the index values for side0 to side4 are correspondingly smallest to largest

                    atlases.Sort();
                   
                    // Create trc

                    TexRect trc = new TexRect(imageinfo.name, bm, atlases.ToArray());
                    Rectangle rcClip = trc.CropTransparent();
                    packer.AddRect(trc);

                    // Force the black to be cropped with the same rect
                    // This keeps the black from getting a different size
                    // in the event that it has different sized margins

                    // Only bother to do this if we can still pack the black packer

                    if (bmBlack != null & fcanPackBlackPacker) {
                        TexRect trcBlack = new TexRect(String.Format("black_{0}", imageinfo.name), bmBlack, atlases.ToArray());
                        trcBlack.CropTransparent(rcClip);
                        packerBlack.AddRect(trcBlack);
                    }

                    // Queue trc for the out json

                    outtrcs.Add(trc);
                }

                // Compile the packer and output stats

                int rectsFailed = packer.Compile();
                Console.WriteLine("{0} rects not packed: {1}", packer.Name, rectsFailed);
                Console.WriteLine("{0} search steps spent: {1}", packer.Name, packer.SearchSteps);
                Bitmap bmp = packer.Image();
                Bitmap bmpBlack = null;

                // Go ahead and save

                bmp.Save(String.Format("{0}/{1}", outdir, packer.Name), ImageFormat.Png);

                // Only build the black packer if needed

                if (fcanPackBlackPacker && packerinfo.hue_variants != null) {
                    packerBlack.Compile();
                    bmpBlack = packerBlack.Image();

                    // hue shifts: subtract black, shift hue, add black, save

                    Bitmap bmpSub = SideMap.Subtract(bmp, bmpBlack);
                    foreach (InputJsonObject.Packer.HueVariant hv in packerinfo.hue_variants) {
                        Bitmap bmpNewHue = SideMap.ShiftHue(bmpSub, hv.hue);
                        Bitmap bmpResult = SideMap.Add(bmpNewHue, bmpBlack);
                        bmpResult.Save(String.Format("{0}/{1}", outdir, hv.name), ImageFormat.Png);
                    }

                    // grayscale

                    if (packerinfo.grayscale.name != null) {
                        Bitmap bmpGray = BitmapTools.CreateGrayscale(bmpSub);
                        Bitmap bmpResult = SideMap.Add(bmpGray, bmpBlack);
                        bmpResult.Save(String.Format("{0}/{1}", outdir, packerinfo.grayscale.name), ImageFormat.Png);
                    }

                } else if (packerinfo.hue_variants != null) {
                    Console.WriteLine("Can't create variant(s) for {0} becuase a black_ is missing", packerinfo.name);
                    throw new Exception("Missing black_ in packer that has hue variants");
                }
            }

            Console.WriteLine("Saving atlasmap.json");
            string json = Json.SerializeObject<Dictionary<string, TexRectJsonEntry>>(Json.CreateEntries(outtrcs));
            File.WriteAllText(String.Format("{0}/atlasmap.json", outdir), json);
        }

    } // class MainClass
} // namespace SpiffCode
