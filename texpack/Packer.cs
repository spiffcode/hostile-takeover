using System;
using System.Collections.Generic;
using SpiffCode;
using System.Drawing;
using System.Linq;

namespace SpiffCode {

    public class Packer {

        // Various ways to sort rects by before packing

        public const int SORT_TYPE_AREA = 0;
        public const int SORT_TYPE_PERIMETER = 1;
        public const int SORT_TYPE_WIDTH = 2;
        public const int SORT_TYPE_HEIGHT = 3;
        public const int SORT_TYPE_MAX_WH = 4;
        private int m_sortType;

        private List<TexRect> m_rects;
        private List<TexRect> m_packedRects;
        private int m_width;
        private int m_height;

        private int m_searchStep;
        private int m_areaCovered;
        private int m_searchResolution;

        private string m_name;

        public List<TexRect> PackedRects {
            get { return m_packedRects; }
            private set { m_packedRects = value; }
        }

        public int AreaCovered {
            get { return m_areaCovered; }
            private set { m_areaCovered = value; }
        }

        public int SearchSteps {
            get { return m_searchStep; }
            private set { m_searchStep = value; }
        }

        public String Name {
            get { return m_name; }
            private set { m_name = value; }
        }

        public int SortType {
            get { return m_sortType; }
            private set { m_sortType = value; }
        }

        public Packer(String name, int width, int height, int searchResolution, int sortType) {
            m_width = width;
            m_height = height;
            m_searchStep = 0;
            m_areaCovered = 0;
            m_searchResolution = searchResolution;
            Name = name;
            SortType = IsValidSortType(sortType) ? sortType : SORT_TYPE_AREA;

            m_rects = new List<TexRect> { };
            PackedRects = new List<TexRect> { };
        }

        public static bool IsValidSortType(int sortType) {
            switch (sortType) {
                case SORT_TYPE_AREA:
                case SORT_TYPE_PERIMETER:
                case SORT_TYPE_WIDTH:
                case SORT_TYPE_HEIGHT:
                case SORT_TYPE_MAX_WH:
                return true;
                default:
                    return false;
            }
        }

        public void AddRect(TexRect rc) {
            m_rects.Add(rc);
        }

        public int Compile() {
            SortRects();
            BuildTree(new Rect(0, 0, m_width, m_height));

            // Return number of rects that didn't fit
            return m_rects.Count;
        }

        public Bitmap Image() {
            Bitmap bmDst = new Bitmap(m_width, m_height);
            Graphics g = Graphics.FromImage(bmDst);

            for (int i = 0; i < PackedRects.Count; i++) {
                TexRect rc = PackedRects[i];
                Rectangle rcClip = new Rectangle(0, 0, rc.Width, rc.Height);
                g.DrawImage(rc.Bitmap, rc.X, rc.Y, rcClip, GraphicsUnit.Pixel);
            }
            return bmDst;
        }

        private void SortRects() {

            switch (SortType) {
                case Packer.SORT_TYPE_AREA:
                    m_rects.Sort((x, y) => x.Area() < y.Area() ? -1 : x.Area() > y.Area() ? 1 : 0);
                    break;

                case Packer.SORT_TYPE_PERIMETER:
                    m_rects.Sort((x, y) => x.Perimeter() < y.Perimeter() ? -1 :
                         x.Perimeter() > y.Perimeter() ? 1 : 0);
                    break;

                case Packer.SORT_TYPE_WIDTH:
                    m_rects.Sort((x, y) => x.Width < y.Width ? -1 : x.Width > y.Width ? 1 : 0);
                    break;

                case Packer.SORT_TYPE_HEIGHT:
                    m_rects.Sort((x, y) => x.Height < y.Height ? -1 : x.Height > y.Height ? 1 : 0);
                    break;

                case Packer.SORT_TYPE_MAX_WH:
                    m_rects.Sort((x, y) => 
                         Math.Max(x.Width, x.Height) < Math.Max(y.Width, y.Height) ? -1 : 
                         Math.Max(x.Width, x.Height) > Math.Max(y.Width, y.Height) ? 1 : 0);
                    break;

                default:
                    throw new Exception("Invalid packer sort type");
            }
          
            // Reverse so it's largest to smallest
            m_rects.Reverse();
        }

        private void BuildTree(Rect freeSpace) {
            if (freeSpace.Width <= 0 || freeSpace.Height <= 0)
                return;
            
            if (m_rects.Count == 0)
                return;

            int rectIndex = 0;
            bool fDone = false;
            int step = Math.Max(rectIndex + m_rects.Count / m_searchResolution, 1);
            while (!fDone) {
                if (m_rects[rectIndex].Width <= freeSpace.Width && m_rects[rectIndex].Height <= freeSpace.Height) {
                    fDone = true;
                } else {
                    SearchSteps += 1;
                    rectIndex += step;
                    if (rectIndex >= m_rects.Count)
                        return;
                }
            }

            // Move the rect from m_rects to m_packedRects

            TexRect rc = m_rects[rectIndex];
            m_rects.RemoveAt(rectIndex);
            PackedRects.Add(rc);

            // Set the rect x,y

            rc.X = freeSpace.X;
            rc.Y = freeSpace.Y;

            AreaCovered += rc.Area();

            // Determine cutting direction(horizontal or vertical)
            // Split current node

            if (freeSpace.Width - rc.Height > freeSpace.Height - rc.Width) {
                // cut into two nodes side-by-side
                // Shrink first node of spit nodes
                // call BuildTree() for each new node
                BuildTree(new Rect(freeSpace.X, freeSpace.Y + rc.Height, rc.Width, freeSpace.Height - rc.Height));
                BuildTree(new Rect(freeSpace.X + rc.Width, freeSpace.Y, freeSpace.Width - rc.Width, freeSpace.Height));
            } else {
                // cut into two nodes one on top of the other
                // Shrink first node of spit nodes
                // call BuildTree() for each new node
                BuildTree(new Rect(freeSpace.X + rc.Width, freeSpace.Y, freeSpace.Width - rc.Width, rc.Height));
                BuildTree(new Rect(freeSpace.X, freeSpace.Y + rc.Height, freeSpace.Width, freeSpace.Height - rc.Height));
            }
        }

    } // class Packer
} // namespace SpiffCode
