using System;
using System.Collections;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using SpiffLib;

namespace m
{
    /// <summary>
    /// Summary description for TemplateTools.
    /// </summary>
    public class TemplateTools
    {
        public TemplateTools()
        {
            //
            // TODO: Add constructor logic here
            //
        }

        public static void ScaleTemplates(TemplateDoc tmpd, Size sizTile) {
            // Scale templates

            Template[] atmpl = tmpd.GetTemplates();
            Template tmplBackground = tmpd.GetBackgroundTemplate();
            tmpd.RemoveTemplates(atmpl);
            foreach (Template tmpl in atmpl)
                ScaleTemplate(tmpl, tmpd.TileSize, sizTile);
            tmpd.TileSize = sizTile;
            tmpd.AddTemplates(atmpl);
            tmpd.SetBackgroundTemplate(tmplBackground);
        }

#if false
        static void ScaleTemplate(Template tmpl, Size sizTileSrc, Size sizTileDst) {
            bool [,] afOccupancySrc = tmpl.OccupancyMap;
            int ctx = afOccupancySrc.GetLength(1);
            int cty = afOccupancySrc.GetLength(0);
            Bitmap bmDst = new Bitmap(ctx * sizTileDst.Width, cty * sizTileDst.Height);
            Graphics gDst = Graphics.FromImage(bmDst);
            gDst.Clear(Color.FromArgb(255, 0, 255));
            for (int tx = 0; tx < ctx; tx++) {
                for (int ty = 0; ty < cty; ty++) {
                    if (!afOccupancySrc[ty, tx])
                        continue;
                    Rectangle rcSrc = new Rectangle(new Point(tx * sizTileSrc.Width, ty * sizTileSrc.Height), sizTileSrc);
                    Rectangle rcDst = new Rectangle(new Point(tx * sizTileDst.Width, ty * sizTileDst.Height), sizTileDst);
                    gDst.DrawImage(tmpl.Bitmap, rcDst, rcSrc, GraphicsUnit.Pixel);
                }
            }
            gDst.Dispose();
            bmDst.MakeTransparent(Color.FromArgb(255, 0, 255));
            tmpl.Bitmap = bmDst;
        }
#endif

        static void ScaleTemplate(Template tmpl, Size sizTileSrc, Size sizTileDst) {
            bool [,] afOccupancySrc = tmpl.OccupancyMap;
            int ctx = afOccupancySrc.GetLength(1);
            int cty = afOccupancySrc.GetLength(0);
            Bitmap bmDst = new Bitmap(ctx * sizTileDst.Width, cty * sizTileDst.Height);
            Graphics gDst = Graphics.FromImage(bmDst);
            gDst.Clear(Color.FromArgb(255, 0, 255));
            for (int tx = 0; tx < ctx; tx++) {
                for (int ty = 0; ty < cty; ty++) {
                    if (!afOccupancySrc[ty, tx])
                        continue;
                    Rectangle rcSrc = new Rectangle(new Point(tx * sizTileSrc.Width, ty * sizTileSrc.Height), sizTileSrc);
                    Rectangle rcDst = new Rectangle(new Point(tx * sizTileDst.Width, ty * sizTileDst.Height), sizTileDst);
                    ScaleTile(tmpl.Bitmap, rcSrc, bmDst, rcDst);
                }
            }
            gDst.Dispose();
            MakeTransparent(bmDst);

            tmpl.Bitmap = bmDst;
        }

        // HACK: Mono's MakeTransparent doesn't get the job done so we map the transparent color ourselves.
        public static void MakeTransparent(Bitmap bm) {
            bm.MakeTransparent(Color.FromArgb(255, 0, 255));
            Color clrTransparentMarker = Color.FromArgb(0, 255, 0, 255);
            Color clrTransparent = Color.FromArgb(0, 0, 0, 0);
            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color clr = bm.GetPixel(x, y);
                    if (clr == clrTransparentMarker)
                        bm.SetPixel(x, y, clrTransparent);
                }
            }
        }

        static void ScaleTile(Bitmap bmSrc, Rectangle rcSrc, Bitmap bmDst, Rectangle rcDst) {
            double nWidthRatio = (double)rcSrc.Width / (double)rcDst.Width;
            double nHeightRatio = (double)rcSrc.Height / (double)rcDst.Height;
            for (int yDst = rcDst.Top; yDst < rcDst.Bottom; yDst++) {
                for (int xDst = rcDst.Left; xDst < rcDst.Right; xDst++) {
                    double xLeftSample = (double)xDst * nWidthRatio;
                    double yTopSample = (double)yDst * nHeightRatio;
		    double xRightSample = (double)(xDst + 1) * nWidthRatio;
		    double yBottomSample = (double)(yDst + 1) * nHeightRatio;
                    DoubleRect drcSample = new DoubleRect(
                        xLeftSample, yTopSample, xRightSample, yBottomSample);
                    Color clrSample = SampleBitmap(bmSrc, drcSample);
                    bmDst.SetPixel(xDst, yDst, clrSample);
                }
            }
        }

        static Color SampleBitmap(Bitmap bm, DoubleRect drc) {
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;

            double nAreaTotal = drc.Width * drc.Height;
            for (int y = (int)Math.Floor(drc.top); y < (int)Math.Ceiling(drc.bottom); y++) {
                for (int x = (int)Math.Floor(drc.left); x < (int)Math.Ceiling(drc.right); x++) {
                    DoubleRect drcPixel = new DoubleRect(x, y, x + 1, y + 1);
                    drcPixel.Intersect(drc);
                    double nArea = drcPixel.Width * drcPixel.Height;
                    double nPercent = nArea / nAreaTotal;
                    Color clr = bm.GetPixel(x, y);
                    r += (double)clr.R / 255.0 * nPercent;
                    g += (double)clr.G / 255.0 * nPercent;
                    b += (double)clr.B / 255.0 * nPercent;
                }
            }
            return Color.FromArgb((int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
        }

        public static TemplateDoc CloneTemplateDoc(TemplateDoc tmpdSrc) {
            // This should probably be on ICloneable::Clone() on TemplateDoc

            TemplateDoc tmpdDst = (TemplateDoc)DocManager.NewDocument(typeof(TemplateDoc), new Object[] { tmpdSrc.TileSize });
            DocManager.SetActiveDocument(typeof(TemplateDoc), tmpdSrc);

            Template[] atmplSrc = tmpdSrc.GetTemplates();
            Template tmplSrcBackground = tmpdSrc.GetBackgroundTemplate();
            Template tmplDstBackground = null;
            ArrayList alsTmplDst = new ArrayList();
            foreach (Template tmplSrc in atmplSrc) {
                Template tmplDst = new Template(tmpdDst, tmplSrc.Name);
                tmplDst.OccupancyMap = tmplSrc.OccupancyMap;
                tmplDst.TerrainMap = tmplSrc.TerrainMap;
                tmplDst.Bitmap = (Bitmap)tmplSrc.Bitmap.Clone();
                alsTmplDst.Add(tmplDst);
                if (tmplSrc == tmplSrcBackground)
                    tmplDstBackground = tmplDst;
            }
            if (tmplDstBackground != null)
                tmpdDst.SetBackgroundTemplate(tmplDstBackground);
            tmpdDst.AddTemplates((Template[])alsTmplDst.ToArray(typeof(Template)));
            Palette palSrc = tmpdSrc.GetPalette();
            if (palSrc != null)
                tmpdDst.SetPalette(palSrc, false);

            return tmpdDst;
        }
    
        public static void QuantizeTemplates(TemplateDoc tmpd, Palette palFixed, int cPalEntries, int cPalEntriesFixed, int cPalEntriesBackground) {
            // Load the fixed palette. The result will be 24 bit templates normalized to a palette of 256 colors,
            // the "fixed" palette plus a quantized palette.

            if (palFixed == null) {
                palFixed = Palette.OpenDialog(null);
                if (palFixed == null) {
                    MessageBox.Show(DocManager.GetFrameParent(), "Must have the fixed color palette to continue!");
                    return;
                }

                switch (palFixed.Length) {
                case 16:
                    cPalEntries = 16;
                    cPalEntriesFixed = 16;
                    cPalEntriesBackground = 0;
                    break;

                case 256:
                    cPalEntries = 256;
                    cPalEntriesFixed = 128;
                    cPalEntriesBackground = 32;
                    break;
                }
            }

            // Quantize loop. Designed to make optimal use of the lower 128 fixed colors

            // Quantize background separately from foreground
    
            Template tmplBackground = tmpd.GetBackgroundTemplate();
            if (tmplBackground != null && cPalEntriesBackground != 0) {
                // Create a despeckled hue map of the background. We'll use this to 
                // subtract background from foreground so we can quantize foreground separately

                Bitmap bmHueBackground = MakeHueMap(tmplBackground.Bitmap);
                DespeckleGrayscaleBitmap(bmHueBackground, 9, 50);

                // Calc mean and standard deviation for filtering purposes

                double nMean = CalcGrayscaleMean(bmHueBackground);
                double nStdDev = CalcGrayscaleStandardDeviation(bmHueBackground, nMean);

                // Add extract & quantize the background pixels

                ArrayList alsColorsBackground = new ArrayList();
                AddTemplateColors(alsColorsBackground, tmplBackground.Bitmap);
                palFixed = QuantizeColors(alsColorsBackground, palFixed, cPalEntriesFixed + cPalEntriesBackground, cPalEntriesFixed);
                cPalEntriesFixed += cPalEntriesBackground;

                // Now extract foreground pixels by first subtracting background pixels

                ArrayList alsColorsForeground = new ArrayList();
                Template[] atmpl = tmpd.GetTemplates();
                foreach (Template tmpl in atmpl) {
                    if (tmpl == tmplBackground)
                        continue;
                    Bitmap bmT = MakeHueMap(tmpl.Bitmap);
                    DespeckleGrayscaleBitmap(bmT, 9, 50);
                    SubtractGrayscaleDistribution(bmT, nMean, nStdDev);
                    for (int y = 0; y < bmT.Height; y++) {
                        for (int x = 0; x < bmT.Width; x++) {
                            Color clr = bmT.GetPixel(x, y);
                            if (clr != Color.FromArgb(255, 0, 255))
                                bmT.SetPixel(x, y, tmpl.Bitmap.GetPixel(x, y));
                        }
                    }
                    AddTemplateColors(alsColorsForeground, bmT);
                }

                // Now quantize foreground pixels
                // Set the palette and color match

                Palette palNew = QuantizeColors(alsColorsForeground, palFixed, cPalEntries, cPalEntriesFixed);
                tmpd.SetPalette(palNew, true);
            } else {
                // No background template; just quantize everything together

                Template[] atmpl = tmpd.GetTemplates();
                ArrayList alsColors = new ArrayList();
                foreach (Template tmpl in atmpl)
                    AddTemplateColors(alsColors, tmpl.Bitmap);

                // Now quantize foreground pixels
                // Set the palette and color match

                Palette palNew = QuantizeColors(alsColors, palFixed, cPalEntries, cPalEntriesFixed);
                tmpd.SetPalette(palNew, true);
            }
        }

        static void AddTemplateColors(ArrayList alsColors, Bitmap bm) {
            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color clr = bm.GetPixel(x, y);
                    if (clr == Color.FromArgb(255, 0, 255))
                        continue;
                    alsColors.Add(clr);
                }
            }
        }

        static Palette QuantizeColors(ArrayList alsColors, Palette palFixed, int cPalEntries, int cPalEntriesFixed) {
            Palette palNew = null;
            while (true) {
                palNew = QuantizeColors2(alsColors, palFixed, cPalEntries, cPalEntriesFixed);
                ArrayList alsColorsNew = new ArrayList();
                foreach (Color clr in alsColors) {
                    int iclr = palNew.FindClosestEntry(clr);
                    if (iclr >= cPalEntriesFixed)
                        alsColorsNew.Add(clr);
                }
                if (alsColorsNew.Count == alsColors.Count)
                    break;
                alsColors = alsColorsNew;
            }
            return palNew;
        }

        static Palette QuantizeColors2(ArrayList alsColors, Palette palFixed, int cPalEntries, int cPalEntriesFixed) {
            // If no quantization needed (4 bit grayscale), return

            if (cPalEntriesFixed >= cPalEntries)
                return palFixed;

            MedianCut mcut = new MedianCut(alsColors);
            mcut.convert(cPalEntries - cPalEntriesFixed);
            Palette palUpper = mcut.GetPalette();
            palUpper.Pad(cPalEntries, Color.FromArgb(255, 0, 255));

            Color[] aclr = new Color[cPalEntries];
            for (int iclr = 0; iclr < cPalEntriesFixed; iclr++)
                aclr[iclr] = palFixed[iclr];
            for (int iclr = cPalEntriesFixed; iclr < cPalEntries; iclr++) {
                Color clr = palUpper[iclr - cPalEntriesFixed];
                Color clrT = Color.FromArgb(clr.R & 0xfc, clr.G & 0xfc, clr.B & 0xfc);
                aclr[iclr] = clrT;
            }
            return new Palette(aclr);
        }

        public static Bitmap MakeHueMap(Bitmap bm) {
            // Make grayscale image based on input image's hue

            Bitmap bmHue = new Bitmap(bm);
            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color clr = bm.GetPixel(x, y);
                    if (clr == Color.FromArgb(255, 0, 255)) {
                        bmHue.SetPixel(x, y, clr);
                        continue;
                    }
                    int nHue = (int)(clr.GetHue() / 360.0 * 255.0);
                    bmHue.SetPixel(x, y, Color.FromArgb(nHue, nHue, nHue));
                }
            }

            return bmHue;
        }

        public static void DespeckleGrayscaleBitmap(Bitmap bm, int cSamples, int nThreshold) {
            int[] anHues = new int[cSamples];
            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color clr = bm.GetPixel(x, y);
                    if (clr == Color.FromArgb(255, 0, 255))
                        continue;
                    int nHue = PatternSampleGrayscaleBitmap(bm, x, y, nThreshold, anHues);
                    bm.SetPixel(x, y, Color.FromArgb(nHue, nHue, nHue));
                }
            }
        }

        public static int PatternSampleGrayscaleBitmap(Bitmap bm, int x, int y, int nThreshold, int[] anSamples) {
            // Get the average - only needed for edge situations including edges next
            // to transparent color

            int cSamples = 0;
            int nTotal = 0;
            int nAverage = 0;

            int cSamplesEdge = (int)Math.Sqrt(anSamples.Length);
            if (cSamplesEdge < 3)
                cSamplesEdge = 3;
            int nDelta = cSamplesEdge / 2;
            for (int yT = -nDelta ; yT <= nDelta; yT++) {
                for (int xT = -nDelta; xT <= nDelta; xT++) {
                    if (xT == 0 && yT == 0)
                        continue;
                    if (x + xT >= 0 && x + xT < bm.Width && y + yT >= 0 && y + yT < bm.Height) {
                        Color clr = bm.GetPixel(x + xT, y + yT);
                        if (clr == Color.FromArgb(255, 0, 255))
                            continue;
                        cSamples++;
                        nTotal += clr.R;
                    }
                }
            }
            nAverage = nTotal / cSamples;

            // 5 sample filter (cross shape)

            int nSample = 0;
            if (anSamples.Length == 5) {
                for (int yT = -1 ; yT <= 1; yT++) {
                    for (int xT = -1; xT <= 1; xT++) {
                        // If one of the corners, don't collect a sample

                        if (xT != 0 && yT != 0)
                            continue;

                        // If off an edge, use the average

                        if (x + xT < 0 || x + xT >= bm.Width || y + yT < 0 || y + yT >= bm.Height) {
                            anSamples[nSample++] = nAverage;
                            continue;
                        }

                        // If transparent color, use average

                        Color clr = bm.GetPixel(x + xT, y + yT);
                        if (clr == Color.FromArgb(255, 0, 255)) {
                            anSamples[nSample++] = nAverage;
                            continue;
                        }
                        
                        // Get pixel

                        anSamples[nSample++] = clr.R;
                    }
                }
            } else {
                // NxN sample filter. Needs to be odd, 3x3, 5x5, etc.

                for (int yT = -nDelta; yT <= nDelta; yT++) {
                    for (int xT = -nDelta; xT <= nDelta; xT++) {
                        // If off an edge, use the average

                        if (x + xT < 0 || x + xT >= bm.Width || y + yT < 0 || y + yT >= bm.Height) {
                            anSamples[nSample++] = nAverage;
                            continue;
                        }

                        // If transparent color, use average

                        Color clr = bm.GetPixel(x + xT, y + yT);
                        if (clr == Color.FromArgb(255, 0, 255)) {
                            anSamples[nSample++] = nAverage;
                            continue;
                        }
                        
                        // Get pixel

                        anSamples[nSample++] = clr.R;
                    }
                }
            }

            // Sort values

            Array.Sort(anSamples);

            // Only accept median if the difference between it and the input pixel is above
            // the specified threshold

            int nInput = bm.GetPixel(x, y).R;
            int nMedian = anSamples[anSamples.Length / 2];
            if (Math.Abs(nMedian - nInput) < nThreshold)
                return nInput;
            return nMedian;
        }

        public static double CalcGrayscaleStandardDeviation(Bitmap bm, double nMean) {
            // 68% within 1 standard deviation from mean
            // 95% within 2 standard deviations from mean
            // 99.7% within 3 standard deviations from mean
            
            double nSigma = 0.0;
            int cCounted = 0;
            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color clr = bm.GetPixel(x, y);
                    if (clr == Color.FromArgb(255, 0, 255))
                        continue;
                    cCounted++;
                    double nDiff = clr.R - nMean;
                    nSigma += nDiff * nDiff;
                }
            }

            return Math.Sqrt(nSigma / cCounted);
        }

        public static double CalcGrayscaleMean(Bitmap bm) {
            double nHueTotal = 0.0;
            int cCounted = 0;
            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color clr = bm.GetPixel(x, y);
                    if (clr == Color.FromArgb(255, 0, 255))
                        continue;
                    cCounted++;
                    nHueTotal += clr.R;
                }
            }
            return nHueTotal / cCounted;
        }

        public static void SubtractGrayscaleDistribution(Bitmap bm, double nMean, double nStdDev) {
            double nMin = nMean - nStdDev * 2.0;
            double nMax = nMean + nStdDev * 2.0;

            for (int y = 0; y < bm.Height; y++) {
                for (int x = 0; x < bm.Width; x++) {
                    Color clr = bm.GetPixel(x, y);
                    if (nMin <= (double)clr.R && (double)clr.R <= nMax)
                        bm.SetPixel(x, y, Color.FromArgb(255, 0, 255));
                }
            }
        }

        public static Bitmap ScaleTemplateBitmap(Bitmap bm, Bitmap bmMask, int cxNew, int cyNew, double nAreaBackgroundThreshold, double nLuminanceMultBackground, double nSaturationMultBackground, double nLuminanceMultForeground, double nSaturationMultForeground) {
            Bitmap bmNew = new Bitmap(cxNew, cyNew);
            for (int y = 0; y < bmNew.Height; y++) {
                for (int x = 0; x < bmNew.Width; x++) {
                    double xLeft = (double)x * (double)bm.Width / (double)bmNew.Width;
                    double yTop = (double)y * (double)bm.Height / (double)bmNew.Height;
                    double xRight = (double)(x + 1) * (double)bm.Width / (double)bmNew.Width;
                    if (xRight > bm.Width)
                        xRight = bm.Width;
                    double yBottom = (double)(y + 1) * (double)bm.Height / (double)bmNew.Height;
                    if (yBottom > bm.Height)
                        yBottom = bm.Height;
                    DoubleRect drc = new DoubleRect(xLeft, yTop, xRight, yBottom);
                    Color clrSample = SampleTemplateBitmap(bm, bmMask, drc, nAreaBackgroundThreshold, nLuminanceMultBackground, nSaturationMultBackground, nLuminanceMultForeground, nSaturationMultForeground);
                    bmNew.SetPixel(x, y, clrSample);
                }
            }

            return bmNew;        
        }

        unsafe static Color SampleTemplateBitmap(Bitmap bm, Bitmap bmMask, DoubleRect drc, double nAreaBackgroundThreshold, double nLuminanceMultBackground, double nSaturationMultBackground, double nLuminanceMultForeground, double nSaturationMultForeground) {
            // First classify as foreground or background. Calc percentages
            
            double nAreaTotal = drc.Width * drc.Height;
            double nAreaBackground = 0.0;
            if (bmMask != null) {
                for (int y = (int)Math.Floor(drc.top); y < (int)Math.Ceiling(drc.bottom); y++) {
                    for (int x = (int)Math.Floor(drc.left); x < (int)Math.Ceiling(drc.right); x++) {
                        
                        // Calc the % of whole taken by this pixel fragment

                        DoubleRect drcPixel = new DoubleRect(x, y, x + 1.0, y + 1.0);
                        drcPixel.Intersect(drc);
                        double nAreaPixel = drcPixel.Width * drcPixel.Height;

                        // Get pixel

                        if (bmMask.GetPixel(x, y) == Color.FromArgb(255, 0, 255))
                            nAreaBackground += nAreaPixel;
                    }
                }
            }

            // If background is above a threshold, this pixel will be background,
            // otherwise foreground.

            bool fBackground;
            if (nAreaBackground / nAreaTotal >= nAreaBackgroundThreshold) {
                fBackground = true;
                nAreaTotal = nAreaBackground;
            } else {
                fBackground = false;
                nAreaTotal -= nAreaBackground;
            }
            
            double r = 0;
            double g = 0;
            double b = 0;
            
            for (int y = (int)Math.Floor(drc.top); y < (int)Math.Ceiling(drc.bottom); y++) {
                for (int x = (int)Math.Floor(drc.left); x < (int)Math.Ceiling(drc.right); x++) {
                    // Foreground / background?

                    Color clr = bm.GetPixel(x, y);
                    if (bmMask != null) {
                        if (fBackground) {
                            if (bmMask.GetPixel(x, y) != Color.FromArgb(255, 0, 255))
                                continue;
                        } else {
                            if (bmMask.GetPixel(x, y) == Color.FromArgb(255, 0, 255))
                                continue;
                        }
                    }

                    // Calc the % of whole taken by this pixel fragment

                    DoubleRect drcPixel = new DoubleRect(x, y, x + 1.0, y + 1.0);
                    drcPixel.Intersect(drc);
                    double nAreaPixel = drcPixel.Width * drcPixel.Height;
                    double nPercentPixel = nAreaPixel / nAreaTotal;

                    // Add in the color components

                    r += clr.R * nPercentPixel;
                    g += clr.G * nPercentPixel;
                    b += clr.B * nPercentPixel;
                }
            }

            // Tweak luminance & saturation

            Color clrT = Color.FromArgb((int)r, (int)g, (int)b);
            double nLuminance = clrT.GetBrightness() * (fBackground ? nLuminanceMultBackground : nLuminanceMultForeground);
            if (nLuminance > 1.0)
                nLuminance = 1.0;
            double nSaturation = clrT.GetSaturation() * (fBackground ? nSaturationMultBackground : nSaturationMultForeground);
            if (nSaturation > 1.0)
                nSaturation = 1.0;
            MyHSLtoRGB(clrT.GetHue(), nSaturation, nLuminance);
            return Color.FromArgb((int)(rT * 255.0), (int)(gT * 255.0), (int)(bT * 255.0));
        }
		
		static double rT, gT, bT;

        /*
         * given h,s,l on [0..1],
         * return r,g,b on [0..1]
         */
        unsafe static void
            HSL_to_RGB(double h, double sl, double l) {
            double v;

            v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
            if (v <= 0) {
                rT = gT = bT = 0.0;
            } else {
                double m;
                double sv;
                int sextant;
                double fract, vsf, mid1, mid2;

                m = l + l - v;
                sv = (v - m ) / v;
                h *= 6.0;
                sextant = (int)h;    
                fract = h - sextant;
                vsf = v * sv * fract;
                mid1 = m + vsf;
                mid2 = v - vsf;
                switch (sextant) {
                case 0: rT = v; gT = mid1; bT = m; break;
                case 1: rT = mid2; gT = v; bT = m; break;
                case 2: rT = m; gT = v; bT = mid1; break;
                case 3: rT = m; gT = mid2; bT = v; break;
                case 4: rT = mid1; gT = m; bT = v; break;
                case 5: rT = v; gT = m; bT = mid2; break;
                }
            }
        }

        unsafe static void MyHSLtoRGB(double h, double s, double l) {
            // From Graphics Gems. Convert Foley's 0..360 to 0..1

            HSL_to_RGB(h / 360.0, s, l);
        }
    }

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
