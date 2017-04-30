using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace SpiffCode {

    public class TexRect : Rect {
        private String m_name;
        private Bitmap m_bm;
        private int[] m_atlases;

        private int m_leftCropped;
        private int m_topCropped;
        private int m_widthOrig;
        private int m_heightOrig;

        public Bitmap Bitmap {
            get { return m_bm; }
            private set { m_bm = value; }
        }

        public String Name {
            get { return m_name; }
            private set { m_name = value; }
        }

        public int Atlas {
            get { return m_atlases[1]; }
        }

        public int[] Atlases {
            get { return m_atlases; }
            private set { m_atlases = value; }
        }

        public int LeftCropped {
            get { return m_leftCropped; }
            private set { m_leftCropped = value; }
        }

        public int TopCropped {
            get { return m_topCropped; }
            private set { m_topCropped = value; }
        }

        public int WidthOrig {
            get { return m_widthOrig; }
            private set { m_widthOrig = value; }
        }

        public int HeightOrig {
            get { return m_heightOrig; }
            private set { m_heightOrig = value; }
        }

        public TexRect(Bitmap bm) {
            Width = bm.Width;
            Height = bm.Height;
            Bitmap = bm;
        }

        public TexRect(String name, Bitmap bm, int[] atlases) {
            m_name = name;
            Bitmap = bm;
            m_atlases = atlases;

            Width = bm.Width;
            Height = bm.Height;
            WidthOrig = Width;
            HeightOrig = Height;
        }

        public Rectangle CropTransparent() {
            if (Bitmap == null)
                return Rectangle.Empty;
            
            int[] c = CropTransparent(Bitmap);
            int left = c[0];
            int top = c[1];
            int right = c[2];
            int bottom = c[3];

            int width = right - left;
            int height = bottom - top;

            if (width == 0 && height == 0)
                return Rectangle.Empty;

            Bitmap bmDst = new Bitmap(width, height);
            Graphics g = Graphics.FromImage(bmDst);
            Rectangle rcClip = new Rectangle(left, top, width, height);
            g.DrawImage(Bitmap, 0, 0, rcClip, GraphicsUnit.Pixel);
            Bitmap = bmDst;

            Width = width;
            Height = height;

            LeftCropped = left;
            TopCropped = top;

            return rcClip;
        }

        public Rectangle CropTransparent(Rectangle rcClip) {
            Bitmap bmDst = new Bitmap(rcClip.Width, rcClip.Height);
            Graphics g = Graphics.FromImage(bmDst);
            g.DrawImage(Bitmap, 0, 0, rcClip, GraphicsUnit.Pixel);
            Bitmap = bmDst;

            Width = rcClip.Width;
            Height = rcClip.Height;

            LeftCropped = rcClip.X;
            TopCropped = rcClip.Y;

            return rcClip;
        }

        private int[] CropTransparent(Bitmap bm) {
            int left = 0, top = 0, right = 0, bottom = 0;
            bool fDone = false;

            // left
            fDone = false;
            for (int x = 0; x < bm.Width && !fDone; x++) {
                for (int y = 0; y < bm.Height && !fDone; y++) {
                    Color pixel = bm.GetPixel(x, y);
                    if (pixel.A == 0)
                        continue;

                    left = x;
                    fDone = true;
                }
            }

            // top
            fDone = false;
            for (int y = 0; y < bm.Height && !fDone; y++) {
                for (int x = 0; x < bm.Width && !fDone; x++) {
                    Color pixel = bm.GetPixel(x, y);
                    if (pixel.A == 0)
                        continue;

                    top = y;
                    fDone = true;
                }
            }

            // right
            fDone = false;
            for (int x = bm.Width - 1; x >= 0 && !fDone; x--) {
                for (int y = bm.Height - 1; y >= 0 && !fDone; y--) {
                    Color pixel = bm.GetPixel(x, y);
                    if (pixel.A == 0)
                        continue;

                    right = x + 1;
                    fDone = true;
                }
            }

            // bottom
            fDone = false;
            for (int y = bm.Height - 1; y >= 0 && !fDone; y--) {
                for (int x = bm.Width - 1; x >= 0 && !fDone; x--) {
                    Color pixel = bm.GetPixel(x, y);
                    if (pixel.A == 0)
                        continue;

                    bottom = y + 1;
                    fDone = true;
                }
            }

            return new int[] { left, top, right, bottom };
        }

    } // class TexRect

} // namespace SpiffCode
