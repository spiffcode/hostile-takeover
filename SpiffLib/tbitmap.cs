using System;
using System.IO;
using System.Collections;
using System.Collections.Specialized;
using System.Drawing;
using System.Drawing.Imaging;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace SpiffLib {
	class TBitmap {
		enum Align { None, Even, Odd, Both };
		enum ColorType { Unknown, Shadow, Side, Transparent, Data };
		enum Op {
			EvenData1, EvenData2, EvenData3, EvenData4, EvenData5, EvenData6, EvenData7, EvenData8, EvenData9,
			EvenData10, EvenData11, EvenData12, EvenData13, EvenData14, EvenData15, EvenData16, EvenData17,
			EvenData18, EvenData19, EvenData20, EvenData21, EvenData22, EvenData23, EvenData24, EvenData25,
			EvenData26, EvenData27, EvenData28, EvenData29, EvenData30, EvenData31, EvenData32, EvenData33,
			EvenData34, EvenData35, EvenData36, EvenData37, EvenData38, EvenData39, EvenData40, EvenData41,
			EvenData42, EvenData43, EvenData44, EvenData45, EvenData46, EvenData47, EvenData48,

			OddData1, OddData2, OddData3, OddData4, OddData5, OddData6, OddData7, OddData8, OddData9,
			OddData10, OddData11, OddData12, OddData13, OddData14, OddData15, OddData16, OddData17,
			OddData18, OddData19, OddData20, OddData21, OddData22, OddData23, OddData24, OddData25,
			OddData26, OddData27, OddData28, OddData29, OddData30, OddData31, OddData32, OddData33,
			OddData34, OddData35, OddData36, OddData37, OddData38, OddData39, OddData40, OddData41,
			OddData42, OddData43, OddData44, OddData45, OddData46, OddData47, OddData48,

			EvenSide1, EvenSide2, EvenSide3, EvenSide4, EvenSide5, EvenSide6, EvenSide7, EvenSide8, EvenSide9,
			EvenSide10, EvenSide11, EvenSide12, EvenSide13, EvenSide14, EvenSide15, EvenSide16,

			OddSide1, OddSide2, OddSide3, OddSide4, OddSide5, OddSide6, OddSide7, OddSide8, OddSide9,
			OddSide10, OddSide11, OddSide12, OddSide13, OddSide14, OddSide15, OddSide16,

			Shadow1, Shadow2, Shadow3, Shadow4, Shadow5, Shadow6, Shadow7, Shadow8, Shadow9,
			Shadow10, Shadow11, Shadow12, Shadow13, Shadow14, Shadow15, Shadow16, Shadow17,
			Shadow18, Shadow19, Shadow20, Shadow21, Shadow22, Shadow23, Shadow24,

			Transparent1, Transparent2, Transparent3, Transparent4, Transparent5, Transparent6, Transparent7, Transparent8, Transparent9,
			Transparent10, Transparent11, Transparent12, Transparent13, Transparent14, Transparent15, Transparent16, Transparent17,
			Transparent18, Transparent19, Transparent20, Transparent21, Transparent22, Transparent23, Transparent24, Transparent25,
			Transparent26, Transparent27, Transparent28, Transparent29, Transparent30, Transparent31, Transparent32,

			NextScan0, NextScan1, NextScan2, NextScan3, NextScan4, NextScan5, NextScan6, NextScan7, NextScan8, NextScan9,
			NextScan10, NextScan11, NextScan12, NextScan13, NextScan14, NextScan15, NextScan16, NextScan17, NextScan18,
			NextScan19, NextScan20, NextScan21, NextScan22, NextScan23, NextScan24, NextScan25, NextScan26, NextScan27,
			NextScan28, NextScan29, NextScan30, NextScan31, NextScan32, NextScan33, NextScan34, NextScan35, NextScan36,
			NextScan37, NextScan38, NextScan39, NextScan40, NextScan41, NextScan42, NextScan43, NextScan44, NextScan45,
			NextScan46, NextScan47, NextScan48,

			Align,
			End
		};
		int m_cx;
		int m_cy;
		int m_yBaseline;
		ScanData[] m_asdEven;
		Palette m_pal;
		ArrayList m_alsFrames = new ArrayList();
		static Color s_clrShadow = Color.FromArgb(156, 212, 248);
		static Color s_clrTransparent = Color.FromArgb(255, 0, 255);
		static Color s_clrSideIndex0 = Color.FromArgb(0, 116, 232);
		static Color s_clrSideIndex1 = Color.FromArgb(0, 96, 196);
		static Color s_clrSideIndex2 = Color.FromArgb(0, 64, 120);
		static Color s_clrSideIndex3 = Color.FromArgb(0, 48, 92);
		static Color s_clrSideIndex4 = Color.FromArgb(0, 32, 64);
		static int s_iclrShadow = -1;
		static int s_iclrTransparent = -2;
		static int s_iclrNotAssigned = -3;
		static int s_iclrSideFirst = -8;
		static int s_iclrSideLast = -4;
		static int s_cpDataMax = 48;
		static int s_cpSideMax = 16;
		static int s_cpShadowMax = 24;
		static int s_cpTransparentMax = 32;
		static int s_cpNextScanMax = 48;

		public TBitmap(string strFile, Palette pal) {
			m_pal = pal;
			Init(new Bitmap(strFile));
		}

		public TBitmap(Bitmap bm, Palette pal) {
			m_pal = pal;
			Init(bm);
		}

		public TBitmap(Palette pal) {
			m_pal = pal;
		}

		void Init(Bitmap bm) {
			m_cx = bm.Width;
			m_cy = bm.Height;
			m_yBaseline = Misc.FindBaseline(bm);
			int[][] aaiclrScans = GetScans(bm);
			m_asdEven = CompileScanData(Align.Even, aaiclrScans);
		}

		ScanData[] CompileScanData(Align alignMaster, int[][] aaiclrScans) {
			ArrayList alsSd = new ArrayList();
			Align alignStart = Align.Even;
			for (int y = 0; y < aaiclrScans.GetLength(0); y++) {
				if (alsSd.Count != 0)
					alignStart = ((ScanData)alsSd[alsSd.Count - 1]).GetNextDataAlignment(alignStart);
				alsSd.Add(new ScanData(alignMaster, alignStart, aaiclrScans[y]));
			}
			return (ScanData[])alsSd.ToArray(typeof(ScanData));
		}

		unsafe int[][] GetScans(Bitmap bm) {
			// Special colors
			int[][] aaiclrScans = new int[m_cy][];

			// Lock down bits for speed
			Rectangle rc = new Rectangle(0, 0, m_cx, m_cy);
			BitmapData bmd = bm.LockBits(rc, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
			byte *pbBase = (byte *)bmd.Scan0.ToPointer();
			for (int y = 0; y < m_cy; y++) {
				int[] aiclrScan = new int[m_cx];
				for (int x = 0; x < m_cx; x++) {
					// Get color
					byte *pb = pbBase + y * bmd.Stride + x * 3;
					Color clr = Color.FromArgb(pb[2], pb[1], pb[0]);

					if (clr == s_clrTransparent) {
						aiclrScan[x] = s_iclrTransparent;
						continue;
					}

					if (clr == s_clrShadow) {
						aiclrScan[x] = s_iclrShadow;
						continue;
					}

					if (clr == s_clrSideIndex0) {
						aiclrScan[x] = s_iclrSideFirst + 0;
						continue;
					}

					if (clr == s_clrSideIndex1) {
						aiclrScan[x] = s_iclrSideFirst + 1;
						continue;
					}

					if (clr == s_clrSideIndex2) {
						aiclrScan[x] = s_iclrSideFirst + 2;
						continue;
					}

					if (clr == s_clrSideIndex3) {
						aiclrScan[x] = s_iclrSideFirst + 3;
						continue;
					}

					if (clr == s_clrSideIndex4) {
						aiclrScan[x] = s_iclrSideFirst + 4;
						continue;
					}

					aiclrScan[x] = m_pal.FindClosestEntry(clr);
				}
				aaiclrScans[y] = aiclrScan;
			}
			bm.UnlockBits(bmd);
			return aaiclrScans;
		}

		class ScanData {
			ArrayList m_alsIclr = new ArrayList();
			ArrayList m_alsSideCodes = new ArrayList();
			ArrayList m_alsOps = new ArrayList();

			struct Run {
				public int cp;
				public ColorType ct;
				public Align align;
				public int[] aiclr;
			}

			public ScanData(Align alignMaster, Align alignData, int[] aiclrScan) {
				CompileRuns(alignData, GetRuns(alignMaster, aiclrScan));
			}

			public Align GetNextDataAlignment(Align alignStart) {
				if (alignStart == Align.Even) {
					return (m_alsIclr.Count & 1) != 0 ? Align.Odd : Align.Even;
				} else {
					return (m_alsIclr.Count & 1) == 0 ? Align.Odd : Align.Even;
				}
			}

			public int[] GetColorData() {
				return (int[])m_alsIclr.ToArray(typeof(int));
			}

			public int[] GetSideCodes() {
				return (int[])m_alsSideCodes.ToArray(typeof(int));
			}

			public Op[] GetOps() {
				return (Op[])m_alsOps.ToArray(typeof(Op));
			}

			void AddSideCodes(Align align, int[] aiclr) {
				m_alsSideCodes.AddRange(EncodeSideColors(align, aiclr));
			}

			ColorType GetColorType(int iclr) {
				if (iclr == s_iclrTransparent)
					return ColorType.Transparent;

				if (iclr == s_iclrShadow)
					return ColorType.Shadow;

				if (iclr >= s_iclrSideFirst && iclr <= s_iclrSideLast)
					return ColorType.Side;

				return ColorType.Data;
			}

			Run[] GetRuns(Align alignScan, int[] aiclrScan) {
				// First calc and remember the runs
				ArrayList alsCtRuns = new ArrayList();
				ArrayList alsCounts = new ArrayList();
				ColorType ctRun = ColorType.Unknown;
				int iiclrFirst = 0;
				for (int iiclr = 0; iiclr < aiclrScan.Length; iiclr++) {
					// Classify color
					ColorType ctCurrent = GetColorType(aiclrScan[iiclr]);

					// Continue if we're in a run
					if (ctRun == ctCurrent)
						continue;

					// Add the run type and the count of pixels this is
					if (ctRun != ColorType.Unknown) {
						alsCtRuns.Add(ctRun);
						alsCounts.Add(iiclr - iiclrFirst);
					}

					// Start a run with this color type
					iiclrFirst = iiclr;
					ctRun = ctCurrent;
				}
				// Add the last run
				alsCtRuns.Add(ctRun);
				alsCounts.Add(aiclrScan.Length - iiclrFirst);

				// Now all the scan is classified in terms of runs of color types. Now create runs.
				ArrayList alsRun = new ArrayList();
				int iiclrRun = 0;
				for (int i = 0; i < alsCtRuns.Count; i++) {
					AddRun(alsRun, alignScan, iiclrRun, aiclrScan, (int)alsCounts[i], (ColorType)alsCtRuns[i]);
					iiclrRun += (int)alsCounts[i];
				}

				// All done
				return (Run[])alsRun.ToArray(typeof(Run));
			}

			void AddRun(ArrayList alsRun, Align alignScan, int iiclrRun, int[] aiclrScan, int cpRun, ColorType ctRun) {
				// Figure out the alignment of this run at the dst if drawn
				Align alignDst;
				if (alignScan == Align.Even) {
					alignDst = (iiclrRun & 1) != 0 ? Align.Odd : Align.Even;
				} else {
					alignDst = (iiclrRun & 1) == 0 ? Align.Odd : Align.Even;
				}

				// Copy run data, figure required alignment
				int[] aiclrRun = null;
				Align alignRun = Align.None;
				switch (ctRun) {
				case ColorType.Data:
				case ColorType.Side:
					alignRun = alignDst;
					aiclrRun = new int[cpRun];
					for (int iiclr = iiclrRun; iiclr < iiclrRun + cpRun; iiclr++)
						aiclrRun[iiclr - iiclrRun] = aiclrScan[iiclr];
					break;

				case ColorType.Shadow:
					break;

				case ColorType.Transparent:
					break;

				default:
					Debug.Assert(false);
					break;
				}

				// Add this run
				Run run;
				run.ct = ctRun;
				run.cp = cpRun;
				run.align = alignRun;
				run.aiclr = aiclrRun;
				alsRun.Add(run);
			}

			int[] EncodeSideColors(Align align, int[] aiclr) {
				int[] asc = new int[(aiclr.Length + 1) / 2];
				for (int isc = 0; isc < asc.Length; isc++) {
					int iclr1 = aiclr[isc * 2];
					int iclr2 = s_iclrSideFirst;
					if (isc * 2 + 1 < aiclr.Length)
						iclr2 = aiclr[isc * 2 + 1];
					asc[isc] = (iclr1 - s_iclrSideFirst) * 2 * 16 + (iclr2 - s_iclrSideFirst) * 2;
				}
				return asc;
			}

			Run[] ChopRuns(Run[] arun) {
				// Chop the runs into compatible pieces. Most runs won't get chopped, only runs whose
				// length exceeds the op handlers' abilities. Chop wisely, especially runs with
				// alignment and / or optimal size issues.

				ArrayList alsRuns = new ArrayList();
				foreach (Run run in arun) {
					int cp = run.cp;
					Align align = run.align;
					Align alignNext = align;
					int[] aiclr;
					switch (run.ct) {
					case ColorType.Data:
						aiclr = (int[])run.aiclr.Clone();
						while (aiclr.Length != 0) {
							// Figure out the run length to use if this run is being chopped.
							// Best bet is to ensure dword moves across the chop
							int cpT = aiclr.Length;
							if (cpT > s_cpDataMax) {
								if (align == Align.Even) {
									cpT = s_cpDataMax;
								} else {
									cpT = ((s_cpDataMax - 1) & ~3) + 1;
									alignNext = Align.Even;
								}
							}

							// Add the run
							Run runT = run;
							runT.cp = cpT;
							runT.align = align;
							runT.aiclr = new int[cpT];
							for (int iiclr = 0; iiclr < cpT; iiclr++)
								runT.aiclr[iiclr] = aiclr[iiclr];
							alsRuns.Add(runT);

							// Adjust the colors left
							int ciclrLeft = aiclr.Length - cpT;
							int[] aiclrT = new int[ciclrLeft];
							for (int iiclr = 0; iiclr < ciclrLeft; iiclr++)
								aiclrT[iiclr] = aiclr[iiclr + cpT];
							aiclr = aiclrT;
							align = alignNext;
						}
						break;

					case ColorType.Side:
						aiclr = (int[])run.aiclr.Clone();
						while (aiclr.Length != 0) {
							// Figure out the run length to use if this run is being chopped.
							// Best bet is to ensure dword moves across the chop
							int cpT = aiclr.Length;
							if (cpT > s_cpSideMax) {
								if (align == Align.Even) {
									cpT = s_cpSideMax;
								} else {
									cpT = ((s_cpSideMax - 1) & ~3) + 1;
									alignNext = Align.Even;
								}
							}

							// Add the run
							Run runT = run;
							runT.cp = cpT;
							runT.align = align;
							runT.aiclr = new int[cpT];
							for (int iiclr = 0; iiclr < cpT; iiclr++)
								runT.aiclr[iiclr] = aiclr[iiclr];
							alsRuns.Add(runT);

							// Adjust the colors left
							int ciclrLeft = aiclr.Length - cpT;
							int[] aiclrT = new int[ciclrLeft];
							for (int iiclr = 0; iiclr < ciclrLeft; iiclr++)
								aiclrT[iiclr] = aiclr[iiclr + cpT];
							aiclr = aiclrT;
							align = alignNext;
						}
						break;

					case ColorType.Shadow:
						while (cp != 0) {
							int cpT = cp > s_cpShadowMax ? s_cpShadowMax : cp;
							Run runT = run;
							runT.cp = cpT;
							alsRuns.Add(runT);
							cp -= cpT;
						}
						break;

					case ColorType.Transparent:
						while (cp != 0) {
							int cpT = cp > s_cpTransparentMax ? s_cpTransparentMax : cp;
							Run runT = run;
							runT.cp = cpT;
							alsRuns.Add(runT);
							cp -= cpT;
						}
						break;
					}
				}
				return (Run[])alsRuns.ToArray(typeof(Run));
			}

			void CompileRuns(Align alignStart, Run[] arun) {
				// First chop the run into compatible pieces.
				arun = ChopRuns(arun);

				// Now go through all runs in the scan and encode into ops and data
				foreach (Run run in arun) {
					switch (run.ct) {
					case ColorType.Data:
						Align align = GetNextDataAlignment(alignStart);
						if (run.align != align && run.cp != 1) {
							m_alsOps.Add(Op.Align);
							m_alsIclr.Add(s_iclrNotAssigned);
						}
						if (run.align == Align.Even) {
							m_alsOps.Add((Op)(run.cp + (int)Op.EvenData1 - 1));
						} else {
							m_alsOps.Add((Op)(run.cp + (int)Op.OddData1 - 1));
						}
						m_alsIclr.AddRange(run.aiclr);
						break;

					case ColorType.Side:
						if (run.align == Align.Even) {
							m_alsOps.Add((Op)(run.cp + (int)Op.EvenSide1 - 1));
						} else {
							m_alsOps.Add((Op)(run.cp + (int)Op.OddSide1 - 1));
						}
						AddSideCodes(run.align, run.aiclr);
						break;

					case ColorType.Shadow:
						m_alsOps.Add((Op)(run.cp + (int)Op.Shadow1 - 1));
						break;

					case ColorType.Transparent:
						m_alsOps.Add((Op)(run.cp + (int)Op.Transparent1 - 1));
						break;

					default:
						Debug.Assert(false);
						break;
					}
				}
			}
		}

		ArrayList SerializeOps(ScanData[] asd) {
			// Serialize ops. Remove trailing and beginning transparency between scans
			// and replace with NextScan ops.
			ArrayList alsOps = new ArrayList();
			ArrayList alsT = new ArrayList();
			int cpSkip = -1;
			foreach (ScanData sd in asd) {
				alsT.Clear();
				alsT.AddRange(sd.GetOps());

				// If cpSkip != -1 then we've removed some trailing transparency
				// from the last scan. Search the start of this new scan.
				if (cpSkip != -1) {
					int iop = 0;
					for (; iop < alsT.Count; iop++) {
						// Add up the transparency
						Op op = (Op)alsT[iop];
						if (op >= Op.Transparent1 && op <= Op.Transparent32) {
							cpSkip += op - Op.Transparent1 + 1;
							continue;
						}
						break;
					}

					// Hit a non-transparent op or end of list. Remove found transparent ops
					alsT.RemoveRange(0, iop);

					// Max is...
					int cpMax = m_cx > s_cpNextScanMax ? s_cpNextScanMax : m_cx;

					// If there is too much transparency to endcode in one
					// NextScan op, cut into pieces.
					int cpExtra = cpSkip - cpMax;
					if (cpExtra > 0) {
						cpSkip -= cpExtra;
						while (cpExtra != 0) {
							int cpT = cpExtra <= s_cpTransparentMax ? cpExtra : s_cpTransparentMax;
							alsT.Insert(0, Op.Transparent1 + cpT - 1);
							cpExtra -= cpT;
						}
					}

					// Insert NextScan op
					alsT.Insert(0, Op.NextScan0 + cpSkip);
				}

				// Now remove trailing transparency if there is any.
				cpSkip = 0;
				int iopTrailing = -1;
				for (int iop = 0; iop < alsT.Count; iop++) {
					Op op = (Op)alsT[iop];
					if (op >= Op.Transparent1 && op <= Op.Transparent32) {
						if (iopTrailing == -1)
							iopTrailing = iop;
						cpSkip += op - Op.Transparent1 + 1;
						continue;
					} else {
						iopTrailing = -1;
						cpSkip = 0;
					}
				}

				// Remove the trailing transparency
				if (iopTrailing != -1) {
					// Remove this transparency
					alsT.RemoveRange(iopTrailing, alsT.Count - iopTrailing);

					// If we've skipped more than the largest EndScan, insert some
					// transparency.
					int cpExtra = cpSkip - s_cpNextScanMax;
					if (cpExtra > 0) {
						cpSkip -= cpExtra;
						while (cpExtra != 0) {
							int cpT = cpExtra <= s_cpTransparentMax ? cpExtra : s_cpTransparentMax;
							alsT.Add(Op.Transparent1 + cpT - 1);
							cpExtra -= cpT;
						}
					}
				}

				// alsT is ready to add to the ops list
				alsOps.AddRange(alsT);
			}

			// Add End op
			alsOps.Add(Op.End);
			return alsOps;
		}

		byte[] SerializeScanData()
		{
			// Combine all ScanData data
			ScanData[] asd = m_asdEven;
			ArrayList alsSideCodes = new ArrayList();
			ArrayList alsIclr = new ArrayList();
			foreach (ScanData sd in asd) {
				alsSideCodes.AddRange(sd.GetSideCodes());
				alsIclr.AddRange(sd.GetColorData());
			}

			// Make ops. Add side codes to op stream to unify stream.
			ArrayList alsOpsT = SerializeOps(asd);
			ArrayList alsOps = new ArrayList();
			foreach (Op op in alsOpsT) {
				alsOps.Add(op);
				if (op >= Op.EvenSide1 && op <= Op.EvenSide16) {
					int csc = (((op - Op.EvenSide1) + 1) + 1) / 2;
					for (int isc = 0; isc < csc; isc++)
						alsOps.Add(alsSideCodes[isc]);
					alsSideCodes.RemoveRange(0, csc);
				}
				if (op >= Op.OddSide1 && op <= Op.OddSide16) {
					int csc = (((op - Op.OddSide1) + 1) + 1) / 2;
					for (int isc = 0; isc < csc; isc++)
						alsOps.Add(alsSideCodes[isc]);
					alsSideCodes.RemoveRange(0, csc);
				}
			}

			// Remember count of bytes needed for aligned data. Add one for both alignements.
			// This is needed during compiling. Make it an even count.
			int cbaiclrUnpacked = ((alsIclr.Count + 1) + 1) & ~1;

			// Create the packed, unaligned color data
			ArrayList alsIclrPacked = new ArrayList();
			foreach (int iclr in alsIclr) {
				if (iclr != s_iclrNotAssigned)
					alsIclrPacked.Add(iclr);
			}

			// Serialize:
			//	public ushort ibaiclr;
			//  public ushort cbaiclrUnpacked;
			//	public byte[] aop;
			//  public byte[] aiclr;

			// Write placeholders
			BinaryWriter bwtr = new BinaryWriter(new MemoryStream());
			bwtr.Write((ushort)0);
			bwtr.Write((ushort)0);

			// Data
			foreach (Op op in alsOps)
				bwtr.Write((byte)op);
			int ibaiclr = (ushort)bwtr.BaseStream.Position;
			foreach (int iclr in alsIclrPacked)
				bwtr.Write((byte)iclr);

			// Fix up pointers
			int cb = (int)bwtr.BaseStream.Length;
			bwtr.BaseStream.Seek(0, SeekOrigin.Begin);
			bwtr.Write((ushort)Misc.SwapUShort((ushort)ibaiclr));
			bwtr.Write((ushort)Misc.SwapUShort((ushort)cbaiclrUnpacked));

			// Return buffer
			byte[] ab = new byte[cb];
			bwtr.BaseStream.Seek(0, SeekOrigin.Begin);
			bwtr.BaseStream.Read(ab, 0, cb);
			bwtr.Close();
			return ab;
		}

		static byte[] Serialize(TBitmap[] atbm) {
			//struct TBitmapHeader:
			//  ctbm
			//  TBitmapEntry[] atbme;
			//		word cx;
			//		word cy;
			//      word yBaseline;
			//		word ibsd;
			//		word cbsd;
			//  ScanData[] asd;

			// Write header info
			BinaryWriter bwtr = new BinaryWriter(new MemoryStream());
			bwtr.Write(Misc.SwapUShort((ushort)atbm.Length));

			// Serialize TBitmapEntry's
			ArrayList alsSdBytes = new ArrayList();
			int ibCurrent = 2 + 10 * atbm.Length;
			for (int itbm = 0; itbm < atbm.Length; itbm++) {
				bwtr.Write(Misc.SwapUShort((ushort)atbm[itbm].m_cx));
				bwtr.Write(Misc.SwapUShort((ushort)atbm[itbm].m_cy));
				bwtr.Write(Misc.SwapUShort((ushort)atbm[itbm].m_yBaseline));
				bwtr.Write(Misc.SwapUShort((ushort)ibCurrent));
				byte[] ab = atbm[itbm].SerializeScanData();
				alsSdBytes.AddRange(ab);
				bwtr.Write(Misc.SwapUShort((ushort)ab.Length));
				ibCurrent += ab.Length;
				Debug.Assert(ibCurrent < ushort.MaxValue);
			}

			// Write sd bytes
			bwtr.Write((byte[])alsSdBytes.ToArray(typeof(byte)));

			// Done
			byte[] abT = new Byte[bwtr.BaseStream.Length];
			bwtr.BaseStream.Seek(0, SeekOrigin.Begin);
			bwtr.BaseStream.Read(abT, 0, abT.Length);
			bwtr.Close();
			return abT;
		}

		static public void Save(Bitmap[] abm, Palette pal, string strFile) {
			// Open file for writing
			BinaryWriter bwtr = new BinaryWriter(new FileStream(strFile, FileMode.Create, FileAccess.Write));

			// Convert into tbm's
			TBitmap[] atbm = new TBitmap[abm.Length];
			for (int ibm = 0; ibm < abm.Length; ibm++)
				atbm[ibm] = new TBitmap(abm[ibm], pal);

			// Serialize the whole thing
			bwtr.Write(Serialize(atbm));

			// All done
			bwtr.Close();
		}

		static public void SaveFont(string strFileBitmap, Palette pal, string strFileAscii, string strFileSave) {
			// Get the character order
			TextReader tr = new StreamReader(strFileAscii);
			string strAscii = tr.ReadLine();
			tr.Close();

			// Get the character count
			int cch = strAscii.Length;

			// Load the image, lose scaling factor
			Bitmap bmFile = new Bitmap(strFileBitmap);
			Bitmap bm = Misc.NormalizeBitmap(bmFile);
			bmFile.Dispose();

// Turn this on to see the character -> glyph mapping as it happens (help for
// finding 'font bugs'). Set a breakpoint below on frm.Dispose().

#if SHOWFONT
			Form frm = new Form();
			frm.Height = 1000;
			frm.Show();
			Graphics gT = frm.CreateGraphics();
			gT.InterpolationMode = InterpolationMode.NearestNeighbor;
			int yDst = 0;
			int xDst = 0;
#endif
			// Scan the bitmap for widths
			int xLast = 0;
			int ich = 0;
			byte[] acxChar = new byte[256];
			for (int x = 0; x < bm.Width; x++) {
				if (bm.GetPixel(x, 0) != Color.FromArgb(255, 0, 255)) {
					Debug.Assert(ich < cch);
					acxChar[strAscii[ich]] = (byte)(x - xLast);
#if SHOWFONT
					gT.DrawString(strAscii[ich].ToString(), frm.Font, new SolidBrush(frm.ForeColor), new PointF(xDst, yDst));
					Rectangle rcDst = new Rectangle(xDst + 20, yDst + 2, (x - xLast), bm.Height);
					Rectangle rcSrc = new Rectangle(xLast, 1, x - xLast, bm.Height);
					gT.DrawImage(bm, rcDst, rcSrc, GraphicsUnit.Pixel);
					yDst += Math.Max(bm.Height, frm.Font.Height);
					if (yDst > frm.ClientRectangle.Height) {
						xDst += 50;
						yDst = 0;
					}

					gT.Flush();
					Application.DoEvents();
#endif
					ich++;
					xLast = x;
				}
			}
#if SHOWFONT
			gT.Dispose();
			frm.Dispose();
#endif

			if (ich != cch) {
				MessageBox.Show(String.Format("Expecting {0} characters but found {2}{1}.",
						cch, ich, ich < cch ? "only " : ""), "bcr2 - Font Compilation Error");
				Debug.Assert(ich == cch - 1);
			}
			int cy = bm.Height - 1;

			// Save serialization
			ArrayList alsSdEven = new ArrayList();
			int xT = 0;
			int ichDefault = -1;
			for (ich = 0; ich < cch; ich++) {
				// ? is the default "no glyph" character
				if (strAscii[ich] == '?')
					ichDefault = ich;

				// Get subimage
				int cx = acxChar[strAscii[ich]];
				Rectangle rcT = new Rectangle(xT, 1, cx, cy);
				xT += cx;
				Bitmap bmT = new Bitmap(cx, cy, PixelFormat.Format24bppRgb);
				Graphics g = Graphics.FromImage(bmT);
				g.DrawImage(bm, 0, 0, rcT, GraphicsUnit.Pixel);
				g.Dispose();

				// Compile scan data
				TBitmap tbm = new TBitmap(bmT, pal);
				bmT.Dispose();

				// Save scan data serialization
				alsSdEven.Add(tbm.SerializeScanData());
			}

			//FontHeader {
			//	word cy;
			//	byte acxChar[256];
			//	word mpchibsdEven[256];
			//	ScanData asd[1];
			//};

			// First serialize scan data

			ArrayList alsIbsdEven = new ArrayList();
			ArrayList alsSd = new ArrayList();
			foreach (byte[] absd in alsSdEven) {
				if ((alsSd.Count & 1) != 0)
					alsSd.Add((byte)0);
				alsIbsdEven.Add(alsSd.Count);
				alsSd.AddRange(absd);
			}

			// Write out to file
			BinaryWriter bwtr = new BinaryWriter(new FileStream(strFileSave, FileMode.Create, FileAccess.Write));

			// Height
			bwtr.Write(Misc.SwapUShort((ushort)cy));

			// Ascii ordered char widths in bytes. First init 0's to width of '?'

			if (ichDefault != -1) {
				for (ich = 0; ich < acxChar.Length; ich++) {
					if (acxChar[ich] == 0) {
						acxChar[ich] = acxChar[strAscii[ichDefault]];
					}
				}
			}
			bwtr.Write(acxChar);

			// Ascii ordered offsets to even scan data (even)
			// Fill unused entries to entry for '?'
			int[] aibsdEven = new int[256];
			for (int ibsd = 0; ibsd < aibsdEven.Length; ibsd++)
				aibsdEven[ibsd] = -1;
			for (int i = 0; i < cch; i++)
				aibsdEven[strAscii[i]] = (int)alsIbsdEven[i];
			if (ichDefault != -1) {
				for (int ibsd = 0; ibsd < aibsdEven.Length; ibsd++) {
					if (aibsdEven[ibsd] == -1) {
						aibsdEven[ibsd] = (int)alsIbsdEven[ichDefault];
					}
				}
			}

			// Write it out
			int cbHeader = 2 + 256 + 512;
			for (int i = 0; i < 256; i++)
				bwtr.Write(Misc.SwapUShort((ushort)(cbHeader + aibsdEven[i])));

			// Now save scan data
			bwtr.Write((byte[])alsSd.ToArray(typeof(byte)));

			// Done
			bwtr.Close();
		}
	}
}
