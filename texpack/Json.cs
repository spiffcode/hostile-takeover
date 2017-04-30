using System;
using System.Collections.Generic;
using System.Web.Script.Serialization;

namespace SpiffCode {
    
    public static class Json {

        public static TexRectJsonEntry CreateEntry(TexRect trc) {
            return new TexRectJsonEntry(trc.X, trc.Y, trc.Width, trc.Height, trc.WidthOrig, trc.HeightOrig, trc.LeftCropped, trc.TopCropped, trc.Atlases);
        }

        public static Dictionary<string, TexRectJsonEntry> CreateEntries(List<TexRect> trcs) {
            Dictionary<string, TexRectJsonEntry> dict = new Dictionary<string, TexRectJsonEntry>();

            foreach (TexRect trc in trcs) {
                dict.Add(trc.Name, CreateEntry(trc));
            }
            return dict;
        }

        public static string SerializeObject<T>(object obj) {
            return new JavaScriptSerializer().Serialize(obj);
        }

        public static object DeserializeJson<T>(string Json) {
            return new JavaScriptSerializer().Deserialize<T>(Json);
        }
    }

    // The serialization object for the output json
        
    public class TexRectJsonEntry {
        public int x { get; set; }          // x coordinate in atlas
        public int y { get; set; }          // y coordinate in atlas
        public int cx { get; set; }         // width in atlas
        public int cy { get; set; }         // height in atlas
        public int cx_orig { get; set; }    // width before croping transparent margins
        public int cy_orig { get; set; }    // height before croping transparent margins
        public int cc_left { get; set; }    // size cropped from left
        public int cc_top { get; set; }     // size cropped from top
        public int[] atlases { get; set; }  // the index of atlas for the side corresponding with the array index

        public TexRectJsonEntry(int x, int y, int cx, int cy, int cxOrig, int cyOrig, int ccLeft, int ccTop, int[] atlases) {
            this.x = x;
            this.y = y;
            this.cx = cx;
            this.cy = cy;
            this.cx_orig = cxOrig;
            this.cy_orig = cyOrig;
            this.cc_left = ccLeft;
            this.cc_top = ccTop;
            this.atlases = atlases;
        }
    }

    // The deserialization object for the input json

    public struct InputJsonObject {

        public struct Packer {

            public struct Grayscale {
                public int index { get; set; }
                public string name { get; set; }
            }

            public struct Image {
                public string black { get; set; }
                public string name { get; set; }
                public string path { get; set; }
                public float scale { get; set; }
            }

            public struct HueVariant {
                public int hue { get; set; }
                public int index { get; set; }
                public string name { get; set; }
            }

            public Grayscale grayscale { get; set; }
            public int height { get; set; }
            public HueVariant[] hue_variants { get; set; }
            public Image[] images { get; set; }
            public int index { get; set; }
            public string name { get; set; }
            public int search_resolution { get; set; }
            public int sort_type { get; set; }
            public int width { get; set; }
        }

        public string art_dir { get; set; }
        public Packer[] packers { get; set; }
        public int shadow_8bit { get; set; }
    }

} // namespace SpiffCode
