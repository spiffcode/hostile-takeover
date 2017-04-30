using System;
using System.Drawing;
using System.Drawing.Imaging;

namespace SpiffCode {

    public static class SideMap {

        public static Bitmap Subtract(Bitmap bm, Bitmap bmBlack) {
            if (bm == null || bmBlack == null)
                return null;
            if (bm.Width != bmBlack.Width || bm.Height != bmBlack.Height)
                return null;

            Bitmap bmr = new Bitmap(bm.Width, bm.Height);

            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color p0 = bm.GetPixel(x, y);
                    Color p1 = bmBlack.GetPixel(x, y);
                    int r, g, b, a;

                    r = p0.R - p1.R;
                    if (r < 0)
                        r = 0;
                    g = p0.G - p1.G;
                    if (g < 0)
                        g = 0;
                    b = p0.B - p1.B;
                    if (b < 0)
                        b = 0;
                    a = p0.A;

                    bmr.SetPixel(x, y, Color.FromArgb(a, r, g, b));
                }
            }

            return bmr;
        }

        public static Bitmap Add(Bitmap bm, Bitmap bmBlack) {
            if (bm == null || bmBlack == null)
                return null;
            if (bm.Width != bmBlack.Width || bm.Height != bmBlack.Height)
                return null;

            Bitmap bmr = new Bitmap(bm.Width, bm.Height);

            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color p0 = bm.GetPixel(x, y);
                    Color p1 = bmBlack.GetPixel(x, y);
                    int r, g, b, a;

                    r = p0.R + p1.R;
                    if (r > 255)
                        r = 255;
                    g = p0.G + p1.G;
                    if (g > 255)
                        g = 255;
                    b = p0.B + p1.B;
                    if (b > 255)
                        b = 255;
                    a = p0.A;

                    bmr.SetPixel(x, y, Color.FromArgb(a, r, g, b));
                }
            }

            return bmr;
        }

        public static Bitmap ShiftHue(Bitmap bm, int shift) {
            if (bm == null)
                return null;

            if (shift == 0)
                return bm;

            Bitmap bmr = new Bitmap(bm.Width, bm.Height);

            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color p0 = bm.GetPixel(x, y);

                    HSL hsl = ColorSys.RGBToHSL(new RGB(p0.R, p0.G, p0.B));
                    hsl.H = hsl.H + shift;
                    RGB rgb = ColorSys.HSLToRGB(hsl);

                    bmr.SetPixel(x, y, Color.FromArgb(p0.A, rgb.R, rgb.G, rgb.B));
                }
            }

            return bmr;
        }

    } // class SideMap

} // namespace SpiffCode
