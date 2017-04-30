using System;

namespace SpiffCode {

    // www.programmingalgorithms.com/algorithm/hsl-to-rgb
    // www.programmingalgorithms.com/algorithm/rgb-to-hsl

    public struct RGB {
        private byte _r;
        private byte _g;
        private byte _b;

        public RGB(byte r, byte g, byte b) {
            this._r = r;
            this._g = g;
            this._b = b;
        }

        public byte R {
            get { return this._r; }
            set { this._r = value; }
        }

        public byte G {
            get { return this._g; }
            set { this._g = value; }
        }

        public byte B {
            get { return this._b; }
            set { this._b = value; }
        }

        public bool Equals(RGB rgb) {
            return (this.R == rgb.R) && (this.G == rgb.G) && (this.B == rgb.B);
        }
    }

    public struct HSL {
        private int _h;
        private float _s;
        private float _l;

        public HSL(int h, float s, float l) {
            this._h = h;
            this._s = s;
            this._l = l;
        }

        public int H {
            get { return this._h; }
            set { this._h = value; }
        }

        public float S {
            get { return this._s; }
            set { this._s = value; }
        }

        public float L {
            get { return this._l; }
            set { this._l = value; }
        }

        public bool Equals(HSL hsl) {
            return (this.H == hsl.H) && (this.S == hsl.S) && (this.L == hsl.L);
        }
    }

   
    public static class ColorSys {
        
        public static HSL RGBToHSL(RGB rgb) {
            HSL hsl = new HSL();

            float r = (rgb.R / 255.0f);
            float g = (rgb.G / 255.0f);
            float b = (rgb.B / 255.0f);

            float min = Math.Min(Math.Min(r, g), b);
            float max = Math.Max(Math.Max(r, g), b);
            float delta = max - min;

            hsl.L = (max + min) / 2;

            if (delta == 0) {
                hsl.H = 0;
                hsl.S = 0.0f;
            } else {
                hsl.S = (hsl.L <= 0.5) ? (delta / (max + min)) : (delta / (2 - max - min));

                float hue;

                if (r == max) {
                    hue = ((g - b) / 6) / delta;
                } else if (g == max) {
                    hue = (1.0f / 3) + ((b - r) / 6) / delta;
                } else {
                    hue = (2.0f / 3) + ((r - g) / 6) / delta;
                }

                if (hue < 0)
                    hue += 1;
                if (hue > 1)
                    hue -= 1;

                hsl.H = (int)(hue * 360);
            }

            return hsl;
        }

        public static RGB HSLToRGB(HSL hsl) {
            byte r = 0;
            byte g = 0;
            byte b = 0;

            if (hsl.S == 0) {
                r = g = b = (byte)(hsl.L * 255);
            } else {
                float v1, v2;
                float hue = (float)hsl.H / 360;

                v2 = (hsl.L < 0.5) ? (hsl.L * (1 + hsl.S)) : ((hsl.L + hsl.S) - (hsl.L * hsl.S));
                v1 = 2 * hsl.L - v2;

                r = (byte)(255 * HueToRGB(v1, v2, hue + (1.0f / 3)));
                g = (byte)(255 * HueToRGB(v1, v2, hue));
                b = (byte)(255 * HueToRGB(v1, v2, hue - (1.0f / 3)));
            }

            return new RGB(r, g, b);
        }

        private static float HueToRGB(float v1, float v2, float vH) {
            if (vH < 0)
                vH += 1;

            if (vH > 1)
                vH -= 1;

            if ((6 * vH) < 1)
                return (v1 + (v2 - v1) * 6 * vH);

            if ((2 * vH) < 1)
                return v2;

            if ((3 * vH) < 2)
                return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

            return v1;
        }

    } // class ColorSys

} // namespace SpiffCode
