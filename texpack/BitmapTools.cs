using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;

namespace SpiffCode {
    public static class BitmapTools {
        
        public static Bitmap CreateBlackBitmap(Bitmap bm) {

            Color[] aclrSide = new Color[] {
                // side colors
                Color.FromArgb(0, 116, 232),
                Color.FromArgb(0, 96, 196),
                Color.FromArgb(0, 64, 120),
                Color.FromArgb(0, 48, 92),
                Color.FromArgb(0, 32, 64),
            };

            // Return a new bitmap where with all 
            // side color pixels are changed to black

            Bitmap bmNew = (Bitmap)bm.Clone();
            if (bmNew.PixelFormat != PixelFormat.Format32bppArgb)
                bmNew = ConvertBitmapTo32bpp(bmNew);
            
            for (int y = 0; y < bmNew.Height; y++) {
                for (int x = 0; x < bmNew.Width; x++) {
                    Color pixel = bm.GetPixel(x, y);
                    if (aclrSide.Contains(pixel)) {
                        bmNew.SetPixel(x, y, Color.FromArgb(0, 0, 0));
                    }
                }
            }
            return bmNew;
        }

        public static Bitmap MapBitmapTransparency(Bitmap bm) {
            Color clrPink = Color.FromArgb(255, 0, 255);

            // Map all the pink to actual alpha channel

            Bitmap bmNew = (Bitmap)bm.Clone();
            if (bmNew.PixelFormat != PixelFormat.Format32bppArgb)
                bmNew = ConvertBitmapTo32bpp(bmNew);
            
            for (int y = 0; y < bmNew.Height; y++) {
                for (int x = 0; x < bmNew.Width; x++) {
                    Color pixel = bmNew.GetPixel(x, y);
                    if (pixel == clrPink) {
                        bmNew.SetPixel(x, y, Color.FromArgb(0, 0, 0, 0));
                    }
                }
            }
            return bmNew;
        }

        public static Bitmap MapBitmapShadow(Bitmap bm, int shadow) {
            Color clrShadow = Color.FromArgb(156, 212, 248);

            // Map shadow bits to black with inputted alpha

            Bitmap bmNew = (Bitmap)bm.Clone();
            if (bmNew.PixelFormat != PixelFormat.Format32bppArgb)
                bmNew = ConvertBitmapTo32bpp(bmNew);
            
            for (int y = 0; y < bmNew.Height; y++) {
                for (int x = 0; x < bmNew.Width; x++) {
                    Color pixel = bmNew.GetPixel(x, y);
                    if (pixel == clrShadow) {
                        bmNew.SetPixel(x, y, Color.FromArgb(shadow, 0, 0, 0));
                    }
                }
            }
            return bmNew;
        }

        public static Bitmap CreateGrayscale(Bitmap bm) {
            Bitmap bmNew = (Bitmap)bm.Clone();
            if (bmNew.PixelFormat != PixelFormat.Format32bppArgb)
                bmNew = ConvertBitmapTo32bpp(bmNew);

            for (int x = 0; x < bmNew.Width; x++) {
                for (int y = 0; y < bmNew.Height; y++) {
                    Color pixel = bmNew.GetPixel(x, y);
                    int grey = (int)(pixel.R * 0.3 + pixel.G * 0.59 + pixel.B * 0.11);
                    bmNew.SetPixel(x, y, Color.FromArgb(pixel.A, grey, grey, grey));
                }
            }
            return bmNew;
        }

        public static Bitmap ConvertBitmapTo32bpp(Bitmap bm) {
            var bmTmp = new Bitmap(bm.Width, bm.Height, PixelFormat.Format32bppArgb);
            using (var g = Graphics.FromImage(bmTmp))
                g.DrawImage(bm, new Rectangle(0, 0, bm.Width, bm.Height));
            return bmTmp;
        }

    } // BitmapTools

} // namespace SpiffCode
