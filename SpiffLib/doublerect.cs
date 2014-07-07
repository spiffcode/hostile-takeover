using System;

namespace SpiffLib {

    public struct DoubleRect {
        public DoubleRect(double leftT, double topT, double rightT, double bottomT) {
            left = leftT;
            top = topT;
            right = rightT;
            bottom = bottomT;
        }

        public double Width {
            get {
                return right - left;
            }
        }

        public double Height {
            get {
                return bottom - top;
            }
        }

        public void Intersect(DoubleRect drc) {
            left = Math.Max(left, drc.left);
            right = Math.Min(right, drc.right);
            if (left < right) {
                top = Math.Max(top, drc.top);
                bottom = Math.Min(bottom, drc.bottom);
                if (top < bottom)
                    return;
            }
            left = 0.0;
            right = 0.0;
            top = 0.0;
            bottom = 0.0;
        }

        public double left;
        public double top;
        public double right;
        public double bottom;
    }
}

