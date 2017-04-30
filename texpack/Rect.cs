using System;
using SpiffCode;
using System.Drawing;

namespace SpiffCode {

    public class Rect {
        private int m_x;
        private int m_y;
        private int m_width;
        private int m_height;

        public int X {
            get { return m_x; }
            set { m_x = value; }
        }

        public int Y {
            get { return m_y; }
            set { m_y = value; }
        }

        public int Width {
            get { return m_width; }
            protected set { m_width = value; }
        }

        public int Height {
            get { return m_height; }
            protected set { m_height = value; }
        }

        public Rect() {
            m_x = 0;
            m_y = 0;
            m_width = 0;
            m_height = 0;
        }

        public Rect(int x, int y, int width, int height) {
            X = x;
            Y = y;
            Width = width;
            Height = height;
        }

        public int Area() {
            return Width * Height;
        }

        public int Perimeter() {
            return (Width * 2) + (Height * 2);
        }
    }

} // namespace SpiffCode
