using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections;

namespace SpiffLib
{
	/// <summary>
	/// The Misc class encapsulates various static helper methods that lack a more
	/// appropriate home.
	/// </summary>
	public class Misc
	{
		class Preprocessor {
			Hashtable m_htbl = new Hashtable();

			public Preprocessor(Stream stmInclude) {
				// Parse stmInclude and make a hash table

				Regex rexI = new Regex(@"^\#define\s*(?<name>\w+)\s*(?<value>[\-0-9]+).*$");

				stmInclude.Seek(0, SeekOrigin.Begin);
				TextReader tr = new StreamReader(stmInclude);
				while (true) {
					string strT = tr.ReadLine();
					if (strT == null)
						break;

					// Remove white space

					strT.Trim();
					if (strT.Length == 0)
						continue;

					// Comment?

					if (strT.StartsWith("//"))
						continue;

					// Match against regex

					Match mat = rexI.Match(strT);
					string strName = mat.Groups["name"].Value;
					int nValue = Int32.Parse(mat.Groups["value"].Value);
					if (strName.Length != 0)
						m_htbl.Add(strName, nValue);
				}
			}

			public MemoryStream Preprocess(Stream stmContent) {
				// Go through stmContent and replace matches with values

				stmContent.Seek(0, SeekOrigin.Begin);
				TextReader tr = new StreamReader(stmContent);
				MemoryStream stm = new MemoryStream();
				TextWriter tw = new StreamWriter(stm);

				while (true) {
					string strT = tr.ReadLine();
					if (strT == null)
						break;

					// Parse for alpha numeric sequences

					string strResult = Regex.Replace(strT, @"\w+", new MatchEvaluator(ReplaceText));
					tw.WriteLine(strResult);
				}

				tw.Flush();
				return stm;
			}

			string ReplaceText(Match mat) {
				string str = mat.ToString();
				if (!m_htbl.ContainsKey(str))
					return str;
				return ((int)m_htbl[str]).ToString();
			}
		}
		
		public static MemoryStream PreprocessStream(Stream stmContent, Stream stmInclude) {
			Preprocessor pp = new Preprocessor(stmInclude);
			return pp.Preprocess(stmContent);
		}

		public static byte[] GetByteArrayFromString(string str) {
			byte[] ab = new byte[str.Length + 1];
			for (int n = 0; n < str.Length; n++)
				ab[n] = (byte)str[n];
			ab[str.Length] = 0;
			return ab;
		}

		/// <summary>
		/// Takes RGB values as floats in the range from 0 to 1 and returns
		/// HSL values also in the range from 0 to 1.
		///
		/// Thank you Ken Fishkin of Pixar Inc and Graphics Gems
		/// </summary>
		/// <param name="r"></param>
		/// <param name="g"></param>
		/// <param name="b"></param>
		/// <param name="h"></param>
		/// <param name="s"></param>
		/// <param name="l"></param>
		public static unsafe void RgbToHsl(float r, float g, float b, float *h, float *s, float *l) {
			float v;
			float m;
			float vm;
			float r2, g2, b2;

			v = Math.Max(r,g);
			v = Math.Max(v,b);
			m = Math.Min(r,g);
			m = Math.Min(m,b);

			*h = 0;
			*s = 0;
			if ((*l = (m + v) / 2.0f) <= 0.0f) 
				return;
			if ((*s = vm = v - m) > 0.0f) {
				*s /= (*l <= 0.5f) ? (v + m ) : (2.0f - v - m) ;
			} else
				return;

			r2 = (v - r) / vm;
			g2 = (v - g) / vm;
			b2 = (v - b) / vm;

			if (r == v)
				*h = (g == m ? 5.0f + b2 : 1.0f - g2);
			else if (g == v)
				*h = (b == m ? 1.0f + r2 : 3.0f - b2);
			else
				*h = (r == m ? 3.0f + g2 : 5.0f - r2);

			*h /= 6;
		}

		/// <summary>
		/// Takes HSL values as floats in the range from 0 to 1 and returns
		/// RGB values also in the range from 0 to 1.
		///
		/// Thank you Ken Fishkin of Pixar Inc and Graphics Gems
		/// </summary>
		/// <param name="h"></param>
		/// <param name="s"></param>
		/// <param name="l"></param>
		/// <param name="r"></param>
		/// <param name="g"></param>
		/// <param name="b"></param>
		public static unsafe void HslToRgb(float h, float s, float l, float *r, float *g, float *b) {
			float v;

			v = (l <= 0.5f) ? (l * (1.0f + s)) : (l + s - l * s);
			if (v <= 0) {
				*r = *g = *b = 0.0f;
			} else {
				float m;
				float sv;
				int sextant;
				float fract, vsf, mid1, mid2;

				m = l + l - v;
				sv = (v - m ) / v;
				h *= 6.0f;
				sextant = (int)h;	
				fract = h - sextant;
				vsf = v * sv * fract;
				mid1 = m + vsf;
				mid2 = v - vsf;
				switch (sextant) {
				case 0: *r = v; *g = mid1; *b = m; break;
				case 1: *r = mid2; *g = v; *b = m; break;
				case 2: *r = m; *g = v; *b = mid1; break;
				case 3: *r = m; *g = mid2; *b = v; break;
				case 4: *r = mid1; *g = m; *b = v; break;
				case 5: *r = v; *g = m; *b = mid2; break;
				}
			}
		}

		/// <summary>
		/// All this routine does is create a duplicate of the passed-in bitmap. Its utility
		/// comes from the fact that it doesn't duplicate the properties (e.g., horizontal 
		/// and vertical resolution) that are attached to bitmaps after they've been read
		/// from files. These properties can interfere with the expected operation of various
		/// Bitmap/Graphics methods (e.g., DragImage will inexplicably scale the bitmap when
		/// drawn from based on its intrinsic resolution properties). We would prefer to just
		/// clear out these properties or force them to be ignored but all such attempts to
		/// date have been rebuffed.
		/// </summary>
		/// <param name="bmSrc">The Bitmap to be 'normalized'</param>
		/// <returns>A property-free Bitmap</returns>
		public static unsafe Bitmap NormalizeBitmap(Bitmap bmSrc) {
			Bitmap bmDst = new Bitmap(bmSrc.Width, bmSrc.Height, bmSrc.PixelFormat);

			Rectangle rc = new Rectangle(0, 0, bmSrc.Width, bmSrc.Height);
			BitmapData bmdSrc = bmSrc.LockBits(rc, ImageLockMode.ReadOnly, bmSrc.PixelFormat);
			BitmapData bmdDst = bmDst.LockBits(rc, ImageLockMode.WriteOnly, bmSrc.PixelFormat);

			int cul = bmdSrc.Height * bmdSrc.Stride / sizeof(ulong);
			ulong *pulSrc = (ulong *)bmdSrc.Scan0;
			ulong *pulDst = (ulong *)bmdDst.Scan0;
			while (cul-- > 0) {
				*pulDst++ = *pulSrc++;
			}
			
			bmSrc.UnlockBits(bmdSrc);
			bmDst.UnlockBits(bmdDst);
			
			return bmDst;
		}

		static Color s_clrShadow = Color.FromArgb(156, 212, 248);
		static Color s_clrTransparent = Color.FromArgb(255, 0, 255);

		/// <summary>
		/// Scan from the bottom up. First scanline containing non-shadow, non-transparent color
		/// is the baseline.
		/// </summary>
		/// <param name="bm"></param>
		/// <returns>A scanline offset from the top of the image</returns>
		public static unsafe int FindBaseline(Bitmap bm) {
			Rectangle rc = new Rectangle(0, 0, bm.Width, bm.Height);
			BitmapData bmd = bm.LockBits(rc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
			
			byte *pbBase = (byte *)bmd.Scan0;
			for (int y = bm.Height - 1; y >= 0; y--) {
				byte *pb = pbBase + y * bmd.Stride;
				for (int x = 0; x < bm.Width; x++, pb += 3) {
					Color clr = Color.FromArgb(pb[2], pb[1], pb[0]);
					if (clr != s_clrTransparent && clr != s_clrShadow) {
						bm.UnlockBits(bmd);
						return y;
					}
				}
			}
			
			bm.UnlockBits(bmd);
			return 0;
		}

		/// <summary>
		/// The SwapUShort method swaps the high and low bytes of the passed ushort
		/// value. It's good for converting between big and little-endian ushort values.
		/// </summary>
		/// <param name="us">The ushort value whose bytes to swap</param>
		/// <returns>The byte-swapped ushort</returns>
		public static ushort SwapUShort(ushort us) {
			int n = (int)us;
			return (ushort)(((n >> 8) & 0x00ff) | ((n << 8) & 0xff00));
		}

		/// <summary>
		/// The SwapUInt method swaps the endian order of a 4 byte type.
		/// </summary>
		/// <param name="ui">The uint value whose bytes to swap</param>
		/// <returns>The byte-swapped uint</returns>
		public static uint SwapUInt(uint ui) {
			return ((((ui)&0xFF)<<24) | (((ui)&0xFF00)<<8) | (((ui)&0xFF0000)>>8) | (((ui)&0xFF000000)>>24));
		}

		/// <summary>
		/// This cool method returns a new bitmap which has a copy of the passed-in bitmap
		/// surrounded by a colored outline.
		/// </summary>
		/// <param name="bm">Bitmap to outline</param>
		/// <param name="cp">width/height of outline in pixels</param>
		/// <param name="clrEdge">color of the outline</param>
		/// <returns>Bitmap with traced edges</returns>
		public static Bitmap TraceEdges(Bitmap bm, int cp, Color clrEdge) {
			// Create a mask of sorts
			Color clrTransparent = Color.FromArgb(255, 0, 255);
			Bitmap bmEdge = new Bitmap(bm.Width, bm.Height);
			Graphics gEdge = Graphics.FromImage(bmEdge);
			gEdge.Clear(clrTransparent);
			gEdge.DrawImage(bm, 0, 0);
			for (int x = 0; x < bmEdge.Width; x++) {
				for (int y = 0; y < bmEdge.Height; y++) {
					Color clrT = bmEdge.GetPixel(x, y);
					if (clrT != clrTransparent) {
						bmEdge.SetPixel(x, y, clrEdge);
					}
				}
			}
			bmEdge.MakeTransparent(clrTransparent);

			// Draw the mask aligned to create edges
			Bitmap bmT = new Bitmap(bm.Width + cp * 2, bm.Height + cp * 2);
			Graphics gT = Graphics.FromImage(bmT);
			gT.Clear(clrTransparent);
			gT.DrawImage(bmEdge, 0, 0);
			gT.DrawImage(bmEdge, cp, 0);
			gT.DrawImage(bmEdge, cp * 2, 0);
			gT.DrawImage(bmEdge, cp * 2, cp);
			gT.DrawImage(bmEdge, cp * 2, cp * 2);
			gT.DrawImage(bmEdge, cp, cp * 2);
			gT.DrawImage(bmEdge, 0, cp * 2);
			gT.DrawImage(bmEdge, 0, cp);

			// Draw the actual image in the middle; we're done
			gT.DrawImage(bm, cp, cp);
			bmT.MakeTransparent(clrTransparent);

			// Force cleanup
			gEdge.Dispose();
			gT.Dispose();
			return bmT;
		}

		/// <summary>
		/// The GetPropertyItemValue method takes a <see cref="PropertyItem"/> instance (usually
		/// acquired via Image.PropertyItems[]) and returns its value as a (boxed)
		/// CLR type.
		/// </summary>
		/// <example>
		/// <code>
		/// string strCopyright = "no copyright specified";
		/// Bitmap bm = new Bitmap("pict.jpg");
		/// foreach (PropertyItem prpi in bm.PropertyItems) {
		///     if ((PropertyTag)prpi.Id == PropertyTag.Copyright)
		///	        strCopyright = Misc.GetPropertyItemValue(prpi);
		///	}
		///	bm.Dispose();
		///	</code>
		/// </example>
		/// <param name="prpi">The <see cref="PropertyItem"/> containing the desired Value</param>
		/// <returns>The value, usually a boxed primitive type</returns>
		/// <remarks>If the <see cref="PropertyTagType"/> of the item is unknown the original 
		/// Value byte array is returned.
		/// </remarks>
		/// <seealso cref="PropertyItem"/>
		/// <seealso cref="Bitmap"/>
		static public object GetPropertyItemValue(PropertyItem prpi) {
			switch ((PropertyTagType)prpi.Type) {
			case PropertyTagType.ASCII: {
				// UNDONE: a cleaner way to do this?
				StreamReader stmr = new StreamReader(new MemoryStream(prpi.Value), Encoding.ASCII);
				return stmr.ReadToEnd();
			}

			case PropertyTagType.Byte:
				return prpi.Value[0];

			case PropertyTagType.Long:
				return BitConverter.ToUInt32(prpi.Value, 0);

			case PropertyTagType.Rational: {
				UInt32 n1 = BitConverter.ToUInt32(prpi.Value, 0);
				UInt32 n2 = BitConverter.ToUInt32(prpi.Value, 4);
				return (float)n1 / (float)n2;
			}

			case PropertyTagType.Short:
				return BitConverter.ToUInt16(prpi.Value, 0);

			case PropertyTagType.SLONG:
				return BitConverter.ToInt32(prpi.Value, 0);

			case PropertyTagType.SRational: {
				Int32 n1 = BitConverter.ToInt32(prpi.Value, 0);
				Int32 n2 = BitConverter.ToInt32(prpi.Value, 4);
				return  (float)n1 / (float)n2;
			}
			}

			return prpi.Value;
		}
	}

	/// <summary>
	/// This enumeration should be supplied as part of the FCL and used by the PropertyInfo
	/// class but it isn't. I dug its values from a GDI+ headerfile, GdiPlusImaging.h
	/// </summary>
	[Serializable]
	public enum PropertyTagType : short {
		/// <summary>
		/// 
		/// </summary>
		Byte = 1,
		/// <summary>
		/// 
		/// </summary>
		ASCII = 2,
		/// <summary>
		/// 
		/// </summary>
		Short = 3,
		/// <summary>
		/// 
		/// </summary>
		Long = 4,
		/// <summary>
		/// 
		/// </summary>
		Rational = 5,
		/// <summary>
		/// 
		/// </summary>
		Undefined = 7,
		/// <summary>
		/// 
		/// </summary>
		SLONG = 9,
		/// <summary>
		/// 
		/// </summary>
		SRational = 10
	}

	/// <summary>
	/// This enumeration should be supplied as part of the FCL and used by the PropertyInfo
	/// class but it isn't. I dug its values from a GDI+ headerfile, GdiPlusImaging.h
	/// </summary>
	[Serializable]
	public enum PropertyTag : int {
		/// <summary>
		/// 
		/// </summary>
		ExifIFD = 0x8769,
		/// <summary>
		/// 
		/// </summary>
		GpsIFD = 0x8825,

		/// <summary>
		/// 
		/// </summary>
		NewSubfileType = 0x00FE,
		/// <summary>
		/// 
		/// </summary>
		SubfileType = 0x00FF,
		/// <summary>
		/// 
		/// </summary>
		ImageWidth = 0x0100,
		/// <summary>
		/// 
		/// </summary>
		ImageHeight = 0x0101,
		/// <summary>
		/// 
		/// </summary>
		BitsPerSample = 0x0102,
		/// <summary>
		/// 
		/// </summary>
		Compression = 0x0103,
		/// <summary>
		/// 
		/// </summary>
		PhotometricInterp = 0x0106,
		/// <summary>
		/// 
		/// </summary>
		ThreshHolding = 0x0107,
		/// <summary>
		/// 
		/// </summary>
		CellWidth = 0x0108,
		/// <summary>
		/// 
		/// </summary>
		CellHeight = 0x0109,
		/// <summary>
		/// 
		/// </summary>
		FillOrder = 0x010A,
		/// <summary>
		/// 
		/// </summary>
		DocumentName = 0x010D,
		/// <summary>
		/// 
		/// </summary>
		ImageDescription = 0x010E,
		/// <summary>
		/// 
		/// </summary>
		EquipMake = 0x010F,
		/// <summary>
		/// 
		/// </summary>
		EquipModel = 0x0110,
		/// <summary>
		/// 
		/// </summary>
		StripOffsets = 0x0111,
		/// <summary>
		/// 
		/// </summary>
		Orientation = 0x0112,
		/// <summary>
		/// 
		/// </summary>
		SamplesPerPixel = 0x0115,
		/// <summary>
		/// 
		/// </summary>
		RowsPerStrip = 0x0116,
		/// <summary>
		/// 
		/// </summary>
		StripBytesCount = 0x0117,
		/// <summary>
		/// 
		/// </summary>
		MinSampleValue = 0x0118,
		/// <summary>
		/// 
		/// </summary>
		MaxSampleValue = 0x0119,
		/// <summary>
		/// Image resolution in width direction
		/// </summary>
		XResolution = 0x011A,
		/// <summary>
		/// Image resolution in height direction
		/// </summary>
		YResolution = 0x011B,
		/// <summary>
		/// Image data arrangement
		/// </summary>
		PlanarConfig = 0x011C,
		/// <summary>
		/// 
		/// </summary>
		PageName = 0x011D,
		/// <summary>
		/// 
		/// </summary>
		XPosition = 0x011E,
		/// <summary>
		/// 
		/// </summary>
		YPosition = 0x011F,
		/// <summary>
		/// 
		/// </summary>
		FreeOffset = 0x0120,
		/// <summary>
		/// 
		/// </summary>
		FreeByteCounts = 0x0121,
		/// <summary>
		/// 
		/// </summary>
		GrayResponseUnit = 0x0122,
		/// <summary>
		/// 
		/// </summary>
		GrayResponseCurve = 0x0123,
		/// <summary>
		/// 
		/// </summary>
		T4Option = 0x0124,
		/// <summary>
		/// 
		/// </summary>
		T6Option = 0x0125,
		/// <summary>
		/// Unit of X and Y resolution
		/// </summary>
		ResolutionUnit = 0x0128,
		/// <summary>
		/// 
		/// </summary>
		PageNumber = 0x0129,
		/// <summary>
		/// 
		/// </summary>
		TransferFuncition = 0x012D,
		/// <summary>
		/// 
		/// </summary>
		SoftwareUsed = 0x0131,
		/// <summary>
		/// 
		/// </summary>
		DateTime = 0x0132,
		/// <summary>
		/// 
		/// </summary>
		Artist = 0x013B,
		/// <summary>
		/// 
		/// </summary>
		HostComputer = 0x013C,
		/// <summary>
		/// 
		/// </summary>
		Predictor = 0x013D,
		/// <summary>
		/// 
		/// </summary>
		WhitePoint = 0x013E,
		/// <summary>
		/// 
		/// </summary>
		PrimaryChromaticities = 0x013F,
		/// <summary>
		/// 
		/// </summary>
		ColorMap = 0x0140,
		/// <summary>
		/// 
		/// </summary>
		HalftoneHints = 0x0141,
		/// <summary>
		/// 
		/// </summary>
		TileWidth = 0x0142,
		/// <summary>
		/// 
		/// </summary>
		TileLength = 0x0143,
		/// <summary>
		/// 
		/// </summary>
		TileOffset = 0x0144,
		/// <summary>
		/// 
		/// </summary>
		TileByteCounts = 0x0145,
		/// <summary>
		/// 
		/// </summary>
		InkSet = 0x014C,
		/// <summary>
		/// 
		/// </summary>
		InkNames = 0x014D,
		/// <summary>
		/// 
		/// </summary>
		NumberOfInks = 0x014E,
		/// <summary>
		/// 
		/// </summary>
		DotRange = 0x0150,
		/// <summary>
		/// 
		/// </summary>
		TargetPrinter = 0x0151,
		/// <summary>
		/// 
		/// </summary>
		ExtraSamples = 0x0152,
		/// <summary>
		/// 
		/// </summary>
		SampleFormat = 0x0153,
		/// <summary>
		/// 
		/// </summary>
		SMinSampleValue = 0x0154,
		/// <summary>
		/// 
		/// </summary>
		SMaxSampleValue = 0x0155,
		/// <summary>
		/// 
		/// </summary>
		TransferRange = 0x0156,

		/// <summary>
		/// 
		/// </summary>
		JPEGProc = 0x0200,
		/// <summary>
		/// 
		/// </summary>
		JPEGInterFormat = 0x0201,
		/// <summary>
		/// 
		/// </summary>
		JPEGInterLength = 0x0202,
		/// <summary>
		/// 
		/// </summary>
		JPEGRestartInterval = 0x0203,
		/// <summary>
		/// 
		/// </summary>
		JPEGLosslessPredictors = 0x0205,
		/// <summary>
		/// 
		/// </summary>
		JPEGPointTransforms = 0x0206,
		/// <summary>
		/// 
		/// </summary>
		JPEGQTables = 0x0207,
		/// <summary>
		/// 
		/// </summary>
		JPEGDCTables = 0x0208,
		/// <summary>
		/// 
		/// </summary>
		JPEGACTables = 0x0209,

		/// <summary>
		/// 
		/// </summary>
		YCbCrCoefficients = 0x0211,
		/// <summary>
		/// 
		/// </summary>
		YCbCrSubsampling = 0x0212,
		/// <summary>
		/// 
		/// </summary>
		YCbCrPositioning = 0x0213,
		/// <summary>
		/// 
		/// </summary>
		REFBlackWhite = 0x0214,

		/// <summary>
		/// This TAG is defined by ICC for embedded ICC in TIFF
		/// </summary>
		ICCProfile = 0x8773,
		/// <summary>
		/// This TAG is defined by ICC for embedded ICC in TIFF
		/// </summary>
		Gamma = 0x0301,
		/// <summary>
		/// This TAG is defined by ICC for embedded ICC in TIFF
		/// </summary>
		ICCProfileDescriptor = 0x0302,
		/// <summary>
		/// This TAG is defined by ICC for embedded ICC in TIFF
		/// </summary>
		SRGBRenderingIntent = 0x0303,

		/// <summary>
		/// 
		/// </summary>
		ImageTitle = 0x0320,
		/// <summary>
		/// 
		/// </summary>
		Copyright = 0x8298,

		// Extra TAGs (Like Adobe Image Information tags etc.)

		/// <summary>
		/// 
		/// </summary>
		ResolutionXUnit = 0x5001,
		/// <summary>
		/// 
		/// </summary>
		ResolutionYUnit = 0x5002,
		/// <summary>
		/// 
		/// </summary>
		ResolutionXLengthUnit = 0x5003,
		/// <summary>
		/// 
		/// </summary>
		ResolutionYLengthUnit = 0x5004,
		/// <summary>
		/// 
		/// </summary>
		PrintFlags = 0x5005,
		/// <summary>
		/// 
		/// </summary>
		PrintFlagsVersion = 0x5006,
		/// <summary>
		/// 
		/// </summary>
		PrintFlagsCrop = 0x5007,
		/// <summary>
		/// 
		/// </summary>
		PrintFlagsBleedWidth = 0x5008,
		/// <summary>
		/// 
		/// </summary>
		PrintFlagsBleedWidthScale = 0x5009,
		/// <summary>
		/// 
		/// </summary>
		HalftoneLPI = 0x500A,
		/// <summary>
		/// 
		/// </summary>
		HalftoneLPIUnit = 0x500B,
		/// <summary>
		/// 
		/// </summary>
		HalftoneDegree = 0x500C,
		/// <summary>
		/// 
		/// </summary>
		HalftoneShape = 0x500D,
		/// <summary>
		/// 
		/// </summary>
		HalftoneMisc = 0x500E,
		/// <summary>
		/// 
		/// </summary>
		HalftoneScreen = 0x500F,
		/// <summary>
		/// 
		/// </summary>
		JPEGQuality = 0x5010,
		/// <summary>
		/// 
		/// </summary>
		GridSize = 0x5011,
		/// <summary>
		/// 1 = JPEG, 0 = RAW RGB
		/// </summary>
		ThumbnailFormat = 0x5012, 
		/// <summary>
		/// 
		/// </summary>
		ThumbnailWidth = 0x5013,
		/// <summary>
		/// 
		/// </summary>
		ThumbnailHeight = 0x5014,
		/// <summary>
		/// 
		/// </summary>
		ThumbnailColorDepth = 0x5015,
		/// <summary>
		/// 
		/// </summary>
		ThumbnailPlanes = 0x5016,
		/// <summary>
		/// 
		/// </summary>
		ThumbnailRawBytes = 0x5017,
		/// <summary>
		/// 
		/// </summary>
		ThumbnailSize = 0x5018,
		/// <summary>
		/// 
		/// </summary>
		ThumbnailCompressedSize = 0x5019,
		/// <summary>
		/// 
		/// </summary>
		ColorTransferFunction = 0x501A,
		/// <summary>
		/// RAW thumbnail bits in JPEG format or RGB format depends on PropertyTagThumbnailFormat
		/// </summary>
		ThumbnailData = 0x501B,

		// Thumbnail related TAGs

		/// <summary>
		/// Thumbnail width
		/// </summary>
		ThumbnailImageWidth = 0x5020,
		/// <summary>
		/// Thumbnail height
		/// </summary>
		ThumbnailImageHeight = 0x5021,
		/// <summary>
		/// Number of bits per component
		/// </summary>
		ThumbnailBitsPerSample = 0x5022,
		/// <summary>
		/// Compression Scheme
		/// </summary>
		ThumbnailCompression = 0x5023,
		/// <summary>
		/// Pixel composition
		/// </summary>
		ThumbnailPhotometricInterp = 0x5024,
		/// <summary>
		/// Image Tile
		/// </summary>
		ThumbnailImageDescription = 0x5025,
		/// <summary>
		/// Manufacturer of Image
		/// </summary>
		ThumbnailEquipMake = 0x5026,

		// Input equipment

		/// <summary>
		/// Model of Image input equipment
		/// </summary>
		ThumbnailEquipModel = 0x5027,
		/// <summary>
		/// Image data location
		/// </summary>
		ThumbnailStripOffsets = 0x5028,
		/// <summary>
		/// Orientation of image
		/// </summary>
		ThumbnailOrientation = 0x5029,
		/// <summary>
		/// Number of components
		/// </summary>
		ThumbnailSamplesPerPixel = 0x502A,
		/// <summary>
		/// Number of rows per strip
		/// </summary>
		ThumbnailRowsPerStrip = 0x502B,
		/// <summary>
		/// Bytes per compressed strip
		/// </summary>
		ThumbnailStripBytesCount = 0x502C,
		/// <summary>
		/// Resolution in width direction
		/// </summary>
		ThumbnailResolutionX = 0x502D,
		/// <summary>
		///  Resolution in height direction
		/// </summary>
		ThumbnailResolutionY = 0x502E,
		/// <summary>
		/// Image data arrangement
		/// </summary>
		ThumbnailPlanarConfig = 0x502F,
		/// <summary>
		/// Unit of X and Y resolution
		/// </summary>
		ThumbnailResolutionUnit = 0x5030,
		/// <summary>
		/// Transfer function
		/// </summary>
		ThumbnailTransferFunction = 0x5031,
		/// <summary>
		/// Software used
		/// </summary>
		ThumbnailSoftwareUsed = 0x5032,
		/// <summary>
		/// File change date and time
		/// </summary>
		ThumbnailDateTime = 0x5033,
		/// <summary>
		/// Person who created the image
		/// </summary>
		ThumbnailArtist = 0x5034,
		/// <summary>
		/// White point chromaticity
		/// </summary>
		ThumbnailWhitePoint = 0x5035,
		/// <summary>
		/// Chromaticities of primaries
		/// </summary>
		ThumbnailPrimaryChromaticities = 0x5036,
		/// <summary>
		/// Color space transformaion coefficients
		/// </summary>
		ThumbnailYCbCrCoefficients = 0x5037,
		/// <summary>
		/// Subsampling ratio of Y to C
		/// </summary>
		ThumbnailYCbCrSubsampling = 0x5038,
		/// <summary>
		/// Y and C position
		/// </summary>
		ThumbnailYCbCrPositioning = 0x5039,
		/// <summary>
		/// Pair of black and white reference values
		/// </summary>
		ThumbnailRefBlackWhite = 0x503A,
		/// <summary>
		/// Copyright holder
		/// </summary>
		ThumbnailCopyRight = 0x503B,

		/// <summary>
		/// 
		/// </summary>
		LuminanceTable = 0x5090,
		/// <summary>
		/// 
		/// </summary>
		ChrominanceTable = 0x5091,

		/// <summary>
		/// 
		/// </summary>
		FrameDelay = 0x5100,
		/// <summary>
		/// 
		/// </summary>
		LoopCount = 0x5101,

		/// <summary>
		/// Unit specifier for pixel/unit
		/// </summary>
		PixelUnit = 0x5110,
		/// <summary>
		/// Pixels per unit in X
		/// </summary>
		PixelPerUnitX = 0x5111,
		/// <summary>
		/// Pixels per unit in Y
		/// </summary>
		PixelPerUnitY = 0x5112,
		/// <summary>
		/// Palette histogram
		/// </summary>
		PaletteHistogram = 0x5113,

		// EXIF specific tag

		/// <summary>
		/// 
		/// </summary>
		ExifExposureTime = 0x829A,
		/// <summary>
		/// 
		/// </summary>
		ExifFNumber = 0x829D,

		/// <summary>
		/// 
		/// </summary>
		ExifExposureProg = 0x8822,
		/// <summary>
		/// 
		/// </summary>
		ExifSpectralSense = 0x8824,
		/// <summary>
		/// 
		/// </summary>
		ExifISOSpeed = 0x8827,
		/// <summary>
		/// 
		/// </summary>
		ExifOECF = 0x8828,

		/// <summary>
		/// 
		/// </summary>
		ExifVer = 0x9000,
		/// <summary>
		/// Date and time of original
		/// </summary>
		ExifDTOrig = 0x9003,
		/// <summary>
		/// Date and time of digital data generation
		/// </summary>
		ExifDTDigitized = 0x9004,

		/// <summary>
		/// 
		/// </summary>
		ExifCompConfig = 0x9101,
		/// <summary>
		/// 
		/// </summary>
		ExifCompBPP = 0x9102,

		/// <summary>
		/// 
		/// </summary>
		ExifShutterSpeed = 0x9201,
		/// <summary>
		/// 
		/// </summary>
		ExifAperture = 0x9202,
		/// <summary>
		/// 
		/// </summary>
		ExifBrightness = 0x9203,
		/// <summary>
		/// 
		/// </summary>
		ExifExposureBias = 0x9204,
		/// <summary>
		/// 
		/// </summary>
		ExifMaxAperture = 0x9205,
		/// <summary>
		/// 
		/// </summary>
		ExifSubjectDist = 0x9206,
		/// <summary>
		/// 
		/// </summary>
		ExifMeteringMode = 0x9207,
		/// <summary>
		/// 
		/// </summary>
		ExifLightSource = 0x9208,
		/// <summary>
		/// 
		/// </summary>
		ExifFlash = 0x9209,
		/// <summary>
		/// 
		/// </summary>
		ExifFocalLength = 0x920A,
		/// <summary>
		/// 
		/// </summary>
		ExifMakerNote = 0x927C,
		/// <summary>
		/// 
		/// </summary>
		ExifUserComment = 0x9286,
		/// <summary>
		/// Date and Time subseconds
		/// </summary>
		ExifDTSubsec = 0x9290,
		/// <summary>
		/// Date and Time original subseconds
		/// </summary>
		ExifDTOrigSS = 0x9291,
		/// <summary>
		/// Date and TIme digitized subseconds
		/// </summary>
		ExifDTDigSS = 0x9292,

		/// <summary>
		/// 
		/// </summary>
		ExifFPXVer = 0xA000,
		/// <summary>
		/// 
		/// </summary>
		ExifColorSpace = 0xA001,
		/// <summary>
		/// 
		/// </summary>
		ExifPixXDim = 0xA002,
		/// <summary>
		/// 
		/// </summary>
		ExifPixYDim = 0xA003,
		/// <summary>
		/// related sound file
		/// </summary>
		ExifRelatedWav = 0xA004,
		/// <summary>
		/// 
		/// </summary>
		ExifInterop = 0xA005,
		/// <summary>
		/// 
		/// </summary>
		ExifFlashEnergy = 0xA20B,
		/// <summary>
		/// Spatial Frequency Response
		/// </summary>
		ExifSpatialFR = 0xA20C,
		/// <summary>
		/// Focal Plane X Resolution
		/// </summary>
		ExifFocalXRes = 0xA20E,
		/// <summary>
		/// Focal Plane Y Resolution
		/// </summary>
		ExifFocalYRes = 0xA20F,
		/// <summary>
		/// Focal Plane Resolution Unit
		/// </summary>
		ExifFocalResUnit = 0xA210,
		/// <summary>
		/// 
		/// </summary>
		ExifSubjectLoc = 0xA214,
		/// <summary>
		/// 
		/// </summary>
		ExifExposureIndex = 0xA215,
		/// <summary>
		/// 
		/// </summary>
		ExifSensingMethod = 0xA217,
		/// <summary>
		/// 
		/// </summary>
		ExifFileSource = 0xA300,
		/// <summary>
		/// 
		/// </summary>
		ExifSceneType = 0xA301,
		/// <summary>
		/// 
		/// </summary>
		ExifCfaPattern = 0xA302,

		/// <summary>
		/// 
		/// </summary>
		GpsVer = 0x0000,
		/// <summary>
		/// 
		/// </summary>
		GpsLatitudeRef = 0x0001,
		/// <summary>
		/// 
		/// </summary>
		/// <summary>
		/// 
		/// </summary>
		GpsLatitude = 0x0002,
		/// <summary>
		/// 
		/// </summary>
		GpsLongitudeRef = 0x0003,
		/// <summary>
		/// 
		/// </summary>
		GpsLongitude = 0x0004,
		/// <summary>
		/// 
		/// </summary>
		GpsAltitudeRef = 0x0005,
		/// <summary>
		/// 
		/// </summary>
		GpsAltitude = 0x0006,
		/// <summary>
		/// 
		/// </summary>
		GpsGpsTime = 0x0007,
		/// <summary>
		/// 
		/// </summary>
		GpsGpsSatellites = 0x0008,
		/// <summary>
		/// 
		/// </summary>
		GpsGpsStatus = 0x0009,
		/// <summary>
		/// 
		/// </summary>
		GpsGpsMeasureMode = 0x000A,
		/// <summary>
		/// Measurement precision
		/// </summary>
		GpsGpsDop = 0x000B,
		/// <summary>
		/// 
		/// </summary>
		GpsSpeedRef = 0x000C,
		/// <summary>
		/// 
		/// </summary>
		GpsSpeed = 0x000D,
		/// <summary>
		/// 
		/// </summary>
		GpsTrackRef = 0x000E,
		/// <summary>
		/// 
		/// </summary>
		GpsTrack = 0x000F,
		/// <summary>
		/// 
		/// </summary>
		GpsImgDirRef = 0x0010,
		/// <summary>
		/// 
		/// </summary>
		GpsImgDir = 0x0011,
		/// <summary>
		/// 
		/// </summary>
		GpsMapDatum = 0x0012,
		/// <summary>
		/// 
		/// </summary>
		GpsDestLatRef = 0x0013,
		/// <summary>
		/// 
		/// </summary>
		GpsDestLat = 0x0014,
		/// <summary>
		/// 
		/// </summary>
		GpsDestLongRef = 0x0015,
		/// <summary>
		/// 
		/// </summary>
		GpsDestLong = 0x0016,
		/// <summary>
		/// 
		/// </summary>
		GpsDestBearRef = 0x0017,
		/// <summary>
		/// 
		/// </summary>
		GpsDestBear = 0x0018,
		/// <summary>
		/// 
		/// </summary>
		GpsDestDistRef = 0x0019,
		/// <summary>
		/// 
		/// </summary>
		GpsDestDist = 0x001A,
	}
}
