using System;
using System.IO;
using System.Collections;
using System.Collections.Specialized;
using System.Drawing;
using System.Drawing.Imaging;
using System.Diagnostics;

namespace SpiffLib {
	class TBitmapSR {
		enum Op {
			EvenData1, // 0
			EvenData1_Inc, // 1
			EvenData2, // 2
			EvenData2_Inc, // 3
			EvenData3, // 4
			EvenData3_Inc, // 5
			EvenData4, // 6
			EvenData4_Inc, // 7
			EvenData5, // 8
			EvenData5_Inc, // 9
			EvenData6, // 10
			EvenData6_Inc, // 11
			EvenData7, // 12
			EvenData7_Inc, // 13
			EvenData8, // 14
			EvenData8_Inc, // 15
			EvenData9, // 16
			EvenData9_Inc, // 17
			EvenData10, // 18
			EvenData10_Inc, // 19
			EvenData11, // 20
			EvenData11_Inc, // 21
			EvenData12, // 22
			EvenData12_Inc, // 23
			EvenData13, // 24
			EvenData13_Inc, // 25
			EvenData14, // 26
			EvenData14_Inc, // 27
			EvenData15, // 28
			EvenData15_Inc, // 29
			EvenData16, // 30
			EvenData16_Inc, // 31
			EvenData17, // 32
			EvenData17_Inc, // 33
			EvenData18, // 34
			EvenData18_Inc, // 35
			EvenData19, // 36
			EvenData19_Inc, // 37
			EvenData20, // 38
			EvenData20_Inc, // 39
			EvenData21, // 40
			EvenData21_Inc, // 41
			EvenData22, // 42
			EvenData22_Inc, // 43
			EvenData23, // 44
			EvenData23_Inc, // 45
			EvenData24, // 46
			EvenData24_Inc, // 47
			EvenData25, // 48
			EvenData25_Inc, // 49
			EvenData26, // 50
			EvenData26_Inc, // 51
			EvenData27, // 52
			EvenData27_Inc, // 53
			EvenData28, // 54
			EvenData28_Inc, // 55
			EvenData29, // 56
			EvenData29_Inc, // 57
			EvenData30, // 58
			EvenData30_Inc, // 59
			EvenData31, // 60
			EvenData31_Inc, // 61
			EvenData32, // 62
			EvenData32_Inc, // 63
			OddData1, // 64
			OddData1_Inc, // 65
			OddData2, // 66
			OddData2_Inc, // 67
			OddData3, // 68
			OddData3_Inc, // 69
			OddData4, // 70
			OddData4_Inc, // 71
			OddData5, // 72
			OddData5_Inc, // 73
			OddData6, // 74
			OddData6_Inc, // 75
			OddData7, // 76
			OddData7_Inc, // 77
			OddData8, // 78
			OddData8_Inc, // 79
			OddData9, // 80
			OddData9_Inc, // 81
			OddData10, // 82
			OddData10_Inc, // 83
			OddData11, // 84
			OddData11_Inc, // 85
			OddData12, // 86
			OddData12_Inc, // 87
			OddData13, // 88
			OddData13_Inc, // 89
			OddData14, // 90
			OddData14_Inc, // 91
			OddData15, // 92
			OddData15_Inc, // 93
			OddData16, // 94
			OddData16_Inc, // 95
			OddData17, // 96
			OddData17_Inc, // 97
			OddData18, // 98
			OddData18_Inc, // 99
			OddData19, // 100
			OddData19_Inc, // 101
			OddData20, // 102
			OddData20_Inc, // 103
			OddData21, // 104
			OddData21_Inc, // 105
			OddData22, // 106
			OddData22_Inc, // 107
			OddData23, // 108
			OddData23_Inc, // 109
			OddData24, // 110
			OddData24_Inc, // 111
			OddData25, // 112
			OddData25_Inc, // 113
			OddData26, // 114
			OddData26_Inc, // 115
			OddData27, // 116
			OddData27_Inc, // 117
			OddData28, // 118
			OddData28_Inc, // 119
			OddData29, // 120
			OddData29_Inc, // 121
			OddData30, // 122
			OddData30_Inc, // 123
			OddData31, // 124
			OddData31_Inc, // 125
			OddData32, // 126
			OddData32_Inc, // 127
			Side1, // 128
			Side2, // 129
			Side3, // 130
			Side4, // 131
			Side5, // 132
			Side6, // 133
			Side7, // 134
			Side8, // 135
			Side9, // 136
			Side10, // 137
			Side11, // 138
			Side12, // 139
			Side13, // 140
			Side14, // 141
			Side15, // 142
			Side16, // 143
			Side17, // 144
			Side18, // 145
			Side19, // 146
			Side20, // 147
			Side21, // 148
			Side22, // 149
			Side23, // 150
			Side24, // 151
			Side25, // 152
			Side26, // 153
			Side27, // 154
			Side28, // 155
			Side29, // 156
			Side30, // 157
			Side31, // 158
			Side32, // 159
			Shadow1, // 160
			Shadow2, // 161
			Shadow3, // 162
			Shadow4, // 163
			Shadow5, // 164
			Shadow6, // 165
			Shadow7, // 166
			Shadow8, // 167
			Shadow9, // 168
			Shadow10, // 169
			Shadow11, // 170
			Shadow12, // 171
			Shadow13, // 172
			Shadow14, // 173
			Shadow15, // 174
			Shadow16, // 175
			Shadow17, // 176
			Shadow18, // 177
			Shadow19, // 178
			Shadow20, // 179
			Shadow21, // 180
			Shadow22, // 181
			Shadow23, // 182
			Shadow24, // 183
			Shadow25, // 184
			Shadow26, // 185
			Shadow27, // 186
			Shadow28, // 187
			Shadow29, // 188
			Shadow30, // 189
			Shadow31, // 190
			Shadow32, // 191
			EvenDataLB, // 192
			EvenDataLB_Inc, // 193
			EvenDataLW, // 194
			EvenDataLW_Inc, // 195
			EvenDataLWB, // 196
			EvenDataLWB_Inc, // 197
			EvenDataL, // 198
			EvenDataL_Inc, // 199
			OddDataLB, // 200
			OddDataLB_Inc, // 201
			OddDataLW, // 202
			OddDataLW_Inc, // 203
			OddDataLWB, // 204
			OddDataLWB_Inc, // 205
			OddDataL, // 206
			OddDataL_Inc, // 207
			SideN, // 208
			ShadowN, // 209
			TransparentN, // 210
			EndScan, // 211
			Error, // 212
			EvenDataStart = 0,
			EvenDataEnd = 63,
			OddDataStart = 64,
			OddDataEnd = 127,
			SideStart = 128,
			SideEnd = 159,
			ShadowStart = 160,
			ShadowEnd = 191,
			EvenDataNStart = 192,
			EvenDataNEnd = 199,
			OddDataNStart = 200,
			OddDataNEnd = 207
		};

		enum OpType { None, Data, Side, Shadow, Transparent };

		struct Frame {
			public ArrayList alsArunsEven;
			public ArrayList alsArunsOdd;
		}

		struct Run {
			public int cp;
			public ColorType ct;
			public Align align;
			public int[] aiclr;
			public bool fNeedSrcAlign;
		}

		struct RunArgs {
			public Op op;
			public int cpSrc;
			public int cpArgs;
			public int cpDst;
		}

		class CompileResults {
			public ArrayList alsIclr;
			public ArrayList alsRa;
			public int[,] aaiiclrEven;
			public int[,] aairaEven;
			public int[,] aaiiclrOdd;
			public int[,] aairaOdd;
		}

		int m_cx;
		int m_cy;
		Palette m_pal;
		ArrayList m_alsFrames = new ArrayList();
		CompileResults m_crLast;
		static Color m_clrShadow = Color.FromArgb(156, 212, 248);
		static Color m_clrTransparent = Color.FromArgb(255, 0, 255);
		static int s_iclrShadow = -1;
		static int s_iclrTransparent = -2;
		static int s_iclrNotAssigned = -3;
		static int s_iclrSideFirst = 16;
		static int s_iclrSideLast = 20;
		static int s_cpSideOpFixedMax = 32;
		static int s_cpShadowOpFixedMax = 32;
		static int s_cpDataOpFixedMax = 32;
		static Op[] s_mpcpTrailingOpDataEven = { Op.EvenDataL, Op.EvenDataLB, Op.EvenDataLW, Op.EvenDataLWB };
		static Op[] s_mpcpTrailingOpDataOdd = { Op.OddDataL, Op.OddDataLB, Op.OddDataLW, Op.OddDataLWB };

		public TBitmapSR(Palette pal) {
			m_cx = -1;
			m_cy = -1;
			m_pal = pal;
		}

		public void Dispose() {
		}

		public void AddFrame(Bitmap bm) {
			if (m_cx == -1) {
				m_cx = bm.Width;
				m_cy = bm.Height;
			}
			if (m_cx != bm.Width || m_cy != bm.Height)
				throw new Exception("Bitmap different width and height!");

			// Get color converted scanlines for this bitmap
			int[][] aaiclrScans = GetScans(bm);

			// Add ScanRecords for each scan
			Frame frame = new Frame();
			frame.alsArunsEven = new ArrayList();
			frame.alsArunsOdd = new ArrayList();
			for (int y = 0; y < m_cy; y++) {
				frame.alsArunsEven.Add(GetRuns(Align.Even, aaiclrScans[y]));
				frame.alsArunsOdd.Add(GetRuns(Align.Odd, aaiclrScans[y]));
			}

			// Add this frame
			m_alsFrames.Add(frame);
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

					// Transparent and shadow don't exist in the output palette
					// so they have special values.
					if (clr == m_clrTransparent) {
						aiclrScan[x] = s_iclrTransparent;
						continue;
					}
					if (clr == m_clrShadow) {
						aiclrScan[x] = s_iclrShadow;
						continue;
					}

					// Find closest entry in this palette
					aiclrScan[x] = m_pal.FindClosestEntry(clr);
				}
				aaiclrScans[y] = aiclrScan;
			}
			bm.UnlockBits(bmd);
			return aaiclrScans;
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

				// We've encountered a color of a different type.
				// If we're leaving a side run or if we're entering a data run, then check if side colors are
				// close ahead. If they are then enter a side run or stay in the existing side run for
				// efficiency's sake.
				if (ctCurrent == ColorType.Data) {
					for (int iiclrT = iiclr + 1; iiclrT < iiclr + 6; iiclrT++) {
						// If we've run out of pixels, continue the side run if that's what we're in
						if (iiclrT >= aiclrScan.Length) {
							if (ctRun == ColorType.Side)
								ctCurrent = ColorType.Side;
							break;
						}

						// If we encountered a side color, don't switch
						ColorType ctT = GetColorType(aiclrScan[iiclrT]);
						if (ctT == ColorType.Side) {
							ctCurrent = ColorType.Side;
							break;
						}

						// If we've hit something other than data, it's cheaper to stay with the side run than
						// switch to a small data run.
						if (ctT != ColorType.Data) {
							if (ctRun == ColorType.Side)
								ctCurrent = ColorType.Side;
							break;
						}
					}
					if (ctRun == ctCurrent)
						continue;
				}

				// Add the run type and the count of pixels this is
				if (ctRun != ColorType.Unknown) {
					alsCtRuns.Add(ctRun);
					alsCounts.Add(iiclr - iiclrFirst);
					Debug.Assert((int)alsCounts[alsCounts.Count - 1] <= 255);
				}

				// Start a run with this color type
				iiclrFirst = iiclr;
				ctRun = ctCurrent;
			}
			// Add the last run
			alsCtRuns.Add(ctRun);
			alsCounts.Add(aiclrScan.Length - iiclrFirst);
			Debug.Assert((int)alsCounts[alsCounts.Count - 1] <= 255);

			// Now all the scan is classified in terms of runs of color types. Now create runs.
			ArrayList alsRun = new ArrayList();
			int iiclrRun = 0;
			for (int i = 0; i < alsCtRuns.Count; i++) {
				if (i == alsCtRuns.Count - 1 && (ColorType)alsCtRuns[i] == ColorType.Transparent)
					break;
				AddRun(alsRun, alignScan, iiclrRun, aiclrScan, (int)alsCounts[i], (ColorType)alsCtRuns[i]);
				iiclrRun += (int)alsCounts[i];
			}

			// Add terminator
			Run run;
			run.ct = ColorType.EndScan;
			run.cp = 0;
			run.align = Align.None;
			run.aiclr = null;
			run.fNeedSrcAlign = false;
			alsRun.Add(run);

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
				// Always aligned; needed to ensure word / dword copies
				alignRun = alignDst;
				goto CopyRun;

			case ColorType.Side:
CopyRun:
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
			run.fNeedSrcAlign = false;
			alsRun.Add(run);
		}

		struct RunDataInstance {
			public Run[] arun;
			public int[] aiclr;
			public Align align;
		}

		RunDataInstance[] PermuteRunData(Run[] arun, int cMax) {
			ArrayList alsRdInst = new ArrayList();
			Permute2(Align.Even, 0, arun, new int[0], alsRdInst, cMax);
			Permute2(Align.Odd, 0, arun, new int[0], alsRdInst, cMax);
			return (RunDataInstance[])alsRdInst.ToArray(typeof(RunDataInstance));
		}

		void Permute2(Align align, int irun, Run[] arun, int[] aiclr, ArrayList alsRdInst, int cMax) {
			if (irun >= arun.Length) {
				RunDataInstance rdinst;
				rdinst.arun = (Run[])arun.Clone();
				rdinst.aiclr = (int[])aiclr.Clone();
				rdinst.align = align;
				alsRdInst.Add(rdinst);
				return;
			}
		
			// Determine arguments
			RunArgs ra = GetRunArg(arun[irun]);
			int nArg = GetCountArg(ra);
			if (arun[irun].align != Align.None) {
				// Forced alignment
				ArrayList alsIclr = new ArrayList();
				alsIclr.AddRange(aiclr);
				Align alignDst;
				if (align == Align.Even) {
					alignDst = (aiclr.Length & 1) != 0 ? Align.Odd : Align.Even;
				} else {
					alignDst = (aiclr.Length & 1) == 0 ? Align.Odd : Align.Even;
				}
				arun[irun].fNeedSrcAlign = false;
				if (nArg != -1) {
					if (alignDst == arun[irun].align) {
						arun[irun].fNeedSrcAlign = true;
						alsIclr.Add(s_iclrNotAssigned);
					}
					alsIclr.Add(nArg);
				} else {
					if (alignDst != arun[irun].align) {
						arun[irun].fNeedSrcAlign = true;
						alsIclr.Add(s_iclrNotAssigned);
					}
				}
				alsIclr.AddRange(arun[irun].aiclr);
				Permute2(align, irun + 1, arun, (int[])alsIclr.ToArray(typeof(int)), alsRdInst, cMax);
				return;
			} else {
				// No adjust
				ArrayList alsIclr = new ArrayList();
				alsIclr.AddRange(aiclr);
				if (nArg != -1)
					alsIclr.Add(nArg);
				if (arun[irun].aiclr != null)
					alsIclr.AddRange(arun[irun].aiclr);
				arun[irun].fNeedSrcAlign = false;
				Permute2(align, irun + 1, arun, (int[])alsIclr.ToArray(typeof(int)), alsRdInst, cMax);
				if (cMax != -1 && alsRdInst.Count >= cMax)
					return;

#if false
//This saves a modest amount of space however currently we're not supporting _Incs of all ops.
				// With adjust
				alsIclr = new ArrayList();
				alsIclr.AddRange(aiclr);
				alsIclr.Add(s_iclrNotAssigned);
				if (fCountArg)
					alsIclr.Add(arun[irun].cp - 1);
				arun[irun].fNeedSrcAlign = true;
				if (arun[irun].aiclr != null)
					alsIclr.AddRange(arun[irun].aiclr);
				Permute2(align, irun + 1, arun, (int[])alsIclr.ToArray(typeof(int)), alsRdInst, cMax);
#endif
			}
		}

		int CompileImageData(ArrayList alsIclr, Run[] arun, bool fSmallest) {
			// Get all permutations possible of run data
			RunDataInstance[] ardinst = PermuteRunData(arun, fSmallest ? -1 : 1);

			// Match the best permutation with existing data
			int iiclrStart;
			int ird = AddBestRunDataInstance(alsIclr, ardinst, out iiclrStart);
			RunDataInstance rdinst = ardinst[ird];

			// Patch the passed arun with the matched run
			for (int irun = 0; irun < arun.Length; irun++)
				arun[irun] = rdinst.arun[irun];

			// Return where the data starts
//temp
//			Debug.Assert(iiclrStart == MatchImageData(rdinst.align, rdinst.aiclr, alsIclr));
			return iiclrStart;
		}

		int AddBestRunDataInstance(ArrayList alsIclr, RunDataInstance[] ardinst, out int iiclrStart) {
			// Match against all permutations
			int irdLowest = 0;
			int iiclrLowest = alsIclr.Count;
			for (int ird = 0; ird < ardinst.Length; ird++) {
				RunDataInstance rdinst = ardinst[ird];
				int iiclrMatch = MatchImageData(rdinst.align, rdinst.aiclr, alsIclr);
				if (iiclrMatch < iiclrLowest) {
					iiclrLowest = iiclrMatch;
					irdLowest = ird;
				}
			}

			// Contained or partial match
			if (iiclrLowest < alsIclr.Count) {
				// Patch s_iclrNotAssigned in dst since we can reuse them
				RunDataInstance rdinst = ardinst[irdLowest];
				int iiclr = 0;
				for (; iiclr < rdinst.aiclr.Length && iiclr + iiclrLowest < alsIclr.Count; iiclr++) {
					if ((int)alsIclr[iiclrLowest + iiclr] == s_iclrNotAssigned)
						alsIclr[iiclrLowest + iiclr] = rdinst.aiclr[iiclr];
					Debug.Assert(rdinst.aiclr[iiclr] == s_iclrNotAssigned || rdinst.aiclr[iiclr] == (int)alsIclr[iiclrLowest + iiclr]);
				}

				// Add the rest if needed
				for (; iiclr < rdinst.aiclr.Length; iiclr++) {
					Debug.Assert(iiclrLowest + iiclr == alsIclr.Count);
					alsIclr.Add(rdinst.aiclr[iiclr]);
				}
				iiclrStart = iiclrLowest;
				return irdLowest;
			}

			// No best match; add the smallest
			int irdSmallest = 0;
			int cbSmallest = 0x7fff;
			Align alignDst = (alsIclr.Count & 1) != 0 ? Align.Odd : Align.Even;
			for (int ird = 0; ird < ardinst.Length; ird++) {
				RunDataInstance rdinst = ardinst[ird];
				int cbT = rdinst.aiclr.Length;

//temp
#if true
				if (alignDst != rdinst.align)
					continue;
#endif

				if (alignDst != rdinst.align)
					cbT++;
				if (cbT < cbSmallest) {
					irdSmallest = ird;
					cbSmallest = cbT;
				}
			}

			// Add the data
			if (alignDst != ardinst[irdSmallest].align)
				alsIclr.Add(s_iclrNotAssigned);
			int iiclrDst = alsIclr.Count;
			alsIclr.AddRange(ardinst[irdSmallest].aiclr);
			iiclrStart = iiclrDst;
			return irdSmallest;
		}

		int MatchImageData(Align align, int[] aiclr, ArrayList alsIclr) {
//temp
#if false
			// Find a full or partial (off the end) match
			bool fMatch = false;
			for (int iiclrDst = 0; iiclrDst < alsIclr.Count; iiclrDst++) {
				// Proper alignment?
				Align alignDst = (iiclrDst & 1) != 0 ? Align.Odd : Align.Even;
				if (align != alignDst)
					continue;
				
				// Match
				fMatch = true;
				for (int iiclrSrc = 0; iiclrSrc < aiclr.Length; iiclrSrc++) {
					// Partial match?
					int iiclrDstT = iiclrDst + iiclrSrc;
					if (iiclrDstT >= alsIclr.Count)
						return iiclrDst;

					// Continue matching
					if (aiclr[iiclrSrc] == s_iclrNotAssigned)
						continue;
					if ((int)alsIclr[iiclrDstT] == s_iclrNotAssigned)
						continue;
					if (aiclr[iiclrSrc] == (int)alsIclr[iiclrDstT])
						continue;
					fMatch = false;
					break;
				}
				if (fMatch)
					return iiclrDst;
			}

#endif
			// No match
			return alsIclr.Count;
		}

		int GetCountArg(RunArgs ra) {
			switch (ra.op) {
			case Op.EvenDataLB:
			case Op.EvenDataLB_Inc:
				Debug.Assert(((ra.cpDst - 1) & 3) == 0);
				return (63 - (ra.cpDst - 1) / 4) * 2;

			case Op.EvenDataLW:
			case Op.EvenDataLW_Inc:
				Debug.Assert(((ra.cpDst - 2) & 3) == 0);
				return (63 - (ra.cpDst - 2) / 4) * 2;

			case Op.EvenDataLWB:
			case Op.EvenDataLWB_Inc:
				Debug.Assert(((ra.cpDst - 3) & 3) == 0);
				return (63 - (ra.cpDst - 3) / 4) * 2;

			case Op.EvenDataL:
			case Op.EvenDataL_Inc:
				Debug.Assert(((ra.cpDst - 0) & 3) == 0);
				return (64 - (ra.cpDst - 0) / 4) * 2;

			case Op.OddDataLB:
			case Op.OddDataLB_Inc:
				Debug.Assert(((-1 + ra.cpDst - 1) & 3) == 0);
				return (63 - (-1 + ra.cpDst - 1) / 4) * 2;

			case Op.OddDataLW:
			case Op.OddDataLW_Inc:
				Debug.Assert(((-1 + ra.cpDst - 2) & 3) == 0);
				return (63 - (-1 + ra.cpDst - 2) / 4) * 2;

			case Op.OddDataLWB:
			case Op.OddDataLWB_Inc:
				Debug.Assert(((-1 + ra.cpDst - 3) & 3) == 0);
				return (63 - (-1 + ra.cpDst - 3) / 4) * 2;

			case Op.OddDataL:
			case Op.OddDataL_Inc:
				Debug.Assert(((-1 + ra.cpDst - 0) & 3) == 0);
				return (63 - (-1 + ra.cpDst - 0) / 4) * 2;

			case Op.SideN:
			case Op.ShadowN:
				Debug.Assert(ra.cpDst < 64);
				return (64 - ra.cpDst) * 6 / 2;

			case Op.TransparentN:
				return ra.cpDst;
			}
			return -1;
		}

		RunArgs GetRunArg(Run run) {
			// Figure out the appropriate op for this run
			Op op = Op.Error;
			switch (run.ct) {
			case ColorType.Transparent:
				op = Op.TransparentN;
				Debug.Assert(run.aiclr == null);
				break;

			case ColorType.Side:
				if (run.cp <= s_cpSideOpFixedMax) {
					op = Op.SideStart + run.cp - 1;
				} else {
					op = Op.SideN;
				}
				Debug.Assert(run.aiclr != null);
				Debug.Assert(run.aiclr.Length == run.cp);
				break;

			case ColorType.Shadow:
				if (run.cp <= s_cpShadowOpFixedMax) {
					op = Op.ShadowStart + run.cp - 1;
				} else {
					op = Op.ShadowN;
				}
				Debug.Assert(run.aiclr == null);
				break;

			case ColorType.Data:
				if (run.cp <= s_cpDataOpFixedMax) {
					if (run.align == Align.Even) {
						op = (Op)((int)Op.EvenDataStart + (run.cp - 1) * 2 + (run.fNeedSrcAlign ? 1 : 0));
					} else if (run.align == Align.Odd) {
						op = (Op)((int)Op.OddDataStart + (run.cp - 1) * 2 + (run.fNeedSrcAlign ? 1 : 0));
					} else {
						Debug.Assert(false);
					}
				} else {
					if (run.align == Align.Even) {
						op = s_mpcpTrailingOpDataEven[run.aiclr.Length % 4];
					} else {
						op = s_mpcpTrailingOpDataOdd[(run.aiclr.Length - 1)% 4];
					}
					op += (run.fNeedSrcAlign ? 1 : 0);
				}
				Debug.Assert(run.aiclr != null);
				Debug.Assert(run.aiclr.Length == run.cp);
				break;

			case ColorType.EndScan:
				op = Op.EndScan;
				Debug.Assert(run.aiclr == null);
				break;

			default:
				op = Op.Error;
				Debug.Assert(false);
				break;
			}

			// Make RunArgs
			RunArgs ra;
			ra.op = op;
			ra.cpDst = run.cp;
			ra.cpSrc = 0;
			ra.cpArgs = 0;
			if (run.aiclr != null)
				ra.cpSrc += run.aiclr.Length;
			if (run.fNeedSrcAlign) {
				ra.cpArgs++;
				ra.cpSrc++;
			}
			if (GetCountArg(ra) != -1) {
				ra.cpSrc++;
				ra.cpArgs++;
			}
			return ra;
		}

		RunArgs[] MakeRunArgs(Run[] arun) {
			ArrayList alsRa = new ArrayList();
			foreach (Run run in arun)
				alsRa.Add(GetRunArg(run));
			return (RunArgs[])alsRa.ToArray(typeof(RunArgs));
		}

		int CompileRunArgs(ArrayList alsRa, Run[] arun) {
			RunArgs[] ara = MakeRunArgs(arun);

//temp
#if false
			// Incorporate the run args into the list. Match against existing args if possible
			bool fMatch = false;
			int iraDst = 0;
			for (; iraDst < alsRa.Count; iraDst++) {
				fMatch = true;
				for (int iraSrc = 0; iraSrc < ara.Length; iraSrc++) {
					int iraDstT = iraSrc + iraDst;
					if (iraDstT >= alsRa.Count) {
						for (; iraSrc < ara.Length; iraSrc++)
							alsRa.Add(ara[iraSrc]);
						fMatch = true;
						break;
					}
					RunArgs raSrc = ara[iraSrc];
					RunArgs raDst = (RunArgs)alsRa[iraDstT];
					if (raSrc.cpDst != raDst.cpDst || raSrc.op != raDst.op) {
						fMatch = false;
						break;
					}
				}
				if (fMatch)
					break;
			}
			if (!fMatch)
				alsRa.AddRange(ara);

			return iraDst;
#else
			int iraDst = alsRa.Count;
			alsRa.AddRange(ara);
			return iraDst;
#endif
		}

		public void Compile(bool fSmallest) {
			CompileResults cr = new CompileResults();
			cr.alsIclr = new ArrayList();
			cr.alsRa = new ArrayList();
			cr.aaiiclrEven = new int[m_alsFrames.Count, m_cy];
			cr.aairaEven = new int[m_alsFrames.Count, m_cy];
			cr.aaiiclrOdd = new int[m_alsFrames.Count, m_cy];
			cr.aairaOdd = new int[m_alsFrames.Count, m_cy];
			for (int iFrame = 0; iFrame < m_alsFrames.Count; iFrame++) {
				Frame frame = (Frame)m_alsFrames[iFrame];
				for (int y = 0; y < m_cy; y++) {
					cr.aaiiclrEven[iFrame, y] = CompileImageData(cr.alsIclr, (Run[])frame.alsArunsEven[y], fSmallest);
					cr.aairaEven[iFrame, y] = CompileRunArgs(cr.alsRa, (Run[])frame.alsArunsEven[y]);
				}
				for (int y = 0; y < m_cy; y++) {
					cr.aaiiclrOdd[iFrame, y] = CompileImageData(cr.alsIclr, (Run[])frame.alsArunsOdd[y], fSmallest);
					cr.aairaOdd[iFrame, y] = CompileRunArgs(cr.alsRa, (Run[])frame.alsArunsOdd[y]);
				}
			}
			m_crLast = cr;
		}

		byte[] Serialize() {

			//  word type;
			//struct TBitmapSRHeader {
			//	word cx;
			//	word cy;
			//	word cra;
			//	word ibra;
			//	word cFrames;
			//	Frame aframe[1];
			//};

			if (m_crLast == null)
				throw new Exception("Compile image first!");
			CompileResults cr = m_crLast;
			int ibSrStart = 2 + 2 + 2 + 2 + 2 + 4 * m_alsFrames.Count;
			int cbSr = m_alsFrames.Count * (m_cy * 4 + m_cy * 4);
			int ibRaStart = ibSrStart + cbSr;
			int cbRa = cr.alsRa.Count * 4;
			int ibDataStart = ibRaStart + cbRa;

			// Write header info
			BinaryWriter bwtr = new BinaryWriter(new MemoryStream());
			bwtr.Write(Misc.SwapUShort((ushort)TbmType.SkipRun)); // not counted in offsets
			bwtr.Write(Misc.SwapUShort((ushort)m_cx));
			bwtr.Write(Misc.SwapUShort((ushort)m_cy));
			bwtr.Write(Misc.SwapUShort((ushort)cr.alsRa.Count));
			bwtr.Write(Misc.SwapUShort((ushort)ibRaStart));
			bwtr.Write(Misc.SwapUShort((ushort)m_alsFrames.Count));

			// Write isrEven, isrOdd for each frame
			int isr = 0;
			foreach (Frame frame in m_alsFrames) {
				bwtr.Write(Misc.SwapUShort((ushort)(isr + ibSrStart)));
				isr += frame.alsArunsEven.Count * 4;
				bwtr.Write(Misc.SwapUShort((ushort)(isr + ibSrStart)));
				isr += frame.alsArunsOdd.Count * 4;
			}

			// Write sr's (ScanRecords) for each frame
			for (int iFrame = 0; iFrame < m_alsFrames.Count; iFrame++) {
				Frame frame = (Frame)m_alsFrames[iFrame];
				for (int y = 0; y < m_cy; y++) {
					long pos = bwtr.BaseStream.Position - 2;
					bwtr.Write(Misc.SwapUShort((ushort)(cr.aaiiclrEven[iFrame, y] + ibDataStart - pos)));
					bwtr.Write(Misc.SwapUShort((ushort)(cr.aairaEven[iFrame, y] * 4)));
				}
				for (int y = 0; y < m_cy; y++) {
					long pos = bwtr.BaseStream.Position - 2;
					bwtr.Write(Misc.SwapUShort((ushort)(cr.aaiiclrOdd[iFrame, y] + ibDataStart - pos)));
					bwtr.Write(Misc.SwapUShort((ushort)(cr.aairaOdd[iFrame, y] * 4)));
				}
			}

			// Write Ras
			foreach (RunArgs ra in cr.alsRa) {
				bwtr.Write((byte)ra.op);
				bwtr.Write((byte)ra.cpSrc);
				bwtr.Write((byte)ra.cpArgs);
				bwtr.Write((byte)ra.cpDst);
			}

			// Write iclrData
			foreach (int iclr in cr.alsIclr)
				bwtr.Write((byte)iclr);

			// Done
			byte[] ab = new Byte[bwtr.BaseStream.Length];
			bwtr.BaseStream.Seek(0, SeekOrigin.Begin);
			bwtr.BaseStream.Read(ab, 0, ab.Length);
			bwtr.Close();
			return ab;
		}

#if false
		// From runs version
		public Bitmap CreateBitmap(int iFrame, bool fEven, bool fAsRuns) {
			Bitmap bm = new Bitmap(m_cx, m_cy);
			Graphics gMem = Graphics.FromImage(bm);
			gMem.Clear(Color.FromArgb(0, 255, 255));

			Frame frame = (Frame)m_alsFrames[iFrame];
			ArrayList alsArun = fEven ? frame.alsArunsEven : frame.alsArunsOdd;

			int y = 0;
			int nColor = 0;
			foreach(Run[] arun in alsArun) {
				int x = 0;
				foreach (Run run in arun) {
					// Next run
					if (run.ct == ColorType.EndScan) {
						y++;
						break;
					}

					// Transparency
					if (run.ct == ColorType.Transparent) {
						for (int xT = x; xT < x + run.cp; xT++)
							bm.SetPixel(xT, y, Color.FromArgb(0, 255, 255));
						x += run.cp;
						continue;
					}

					// Colorize
					if (fAsRuns) {
						nColor ^= 1;
						Color clr = nColor != 0 ? Color.FromArgb(255, 0, 0) : Color.FromArgb(0, 0, 255);
						for (int xT = x; xT < x + run.cp; xT++)
							bm.SetPixel(xT, y, clr);
						x += run.cp;
					} else {
						switch (run.ct) {
						case ColorType.Data:
						case ColorType.Side:
							for (int xT = x; xT < x + run.cp; xT++)
								bm.SetPixel(xT, y, m_pal[run.aiclr[xT - x]]);
							x += run.cp;
							break;

						case ColorType.Shadow:
							for (int xT = x; xT < x + run.cp; xT++)
								bm.SetPixel(xT, y, Color.FromArgb(255, 255, 0)); // Color.FromArgb(156, 212, 248));
							x += run.cp;
							break;
						}
					}
				}
			}

			gMem.Dispose();
			return bm;
		}
#else
		// From compiled version
		public Bitmap CreateBitmap(int iFrame, bool fEven, bool fAsRuns) {
			if (m_crLast == null)
				throw new Exception("Compile image first!");
			CompileResults cr = m_crLast;

			Bitmap bm = new Bitmap(m_cx, m_cy);
			Graphics gMem = Graphics.FromImage(bm);
			gMem.Clear(Color.FromArgb(0, 255, 255));

			Frame frame = (Frame)m_alsFrames[iFrame];
			int[,] aaiiclr = fEven ? cr.aaiiclrEven : cr.aaiiclrOdd;
			int[,] aaira = fEven ? cr.aairaEven : cr.aairaOdd;

			int xOrigin = fEven ? 0 : 1;
			int nColor = 0;
			for (int y = 0; y < m_cy; y++) {
				int x = 0;
				int iiclr = aaiiclr[iFrame, y];
				int irun = aaira[iFrame, y];
				while (true) {
					RunArgs ra = (RunArgs)cr.alsRa[irun++];
					if (ra.op == Op.EndScan)
						break;

					// Colorize
					if (fAsRuns) {
						nColor ^= 1;
						Color clr = nColor != 0 ? Color.FromArgb(255, 0, 0) : Color.FromArgb(0, 0, 255);
						for (int xT = x; xT < x + ra.cpDst; xT++)
							bm.SetPixel(xT, y, clr);
						x += ra.cpDst;
					} else {
						// Transparency
						if (ra.op == Op.TransparentN) {
							Debug.Assert((int)cr.alsIclr[iiclr++] == ra.cpDst);
							for (int xT = x; xT < x + ra.cpDst; xT++)
								bm.SetPixel(xT, y, Color.FromArgb(0, 255, 255));
							x += ra.cpDst;
							continue;
						}
						if (ra.op >= Op.EvenDataStart && ra.op <= Op.EvenDataEnd) {
							Debug.Assert(((int)ra.op - (int)Op.EvenDataStart) / 2 + 1 == ra.cpDst);
							if (((int)ra.op & 1) != 0)
								iiclr++;
							Debug.Assert(((x + xOrigin) & 1) == 0 && (iiclr & 1) == 0);
							for (int xT = x; xT < x + ra.cpDst; xT++)
								bm.SetPixel(xT, y, m_pal[(int)cr.alsIclr[iiclr++]]);
							x += ra.cpDst;
							continue;
						}
						if (ra.op >= Op.OddDataStart && ra.op <= Op.OddDataEnd) {
							Debug.Assert(((int)ra.op - (int)Op.OddDataStart) / 2 + 1 == ra.cpDst);
							if (((int)ra.op & 1) != 0)
								iiclr++;
							Debug.Assert(((x + xOrigin) & 1) != 0 && (iiclr & 1) != 0);
							for (int xT = x; xT < x + ra.cpDst; xT++)
								bm.SetPixel(xT, y, m_pal[(int)cr.alsIclr[iiclr++]]);
							x += ra.cpDst;
							continue;
						}
						if (ra.op >= Op.EvenDataNStart && ra.op <= Op.OddDataNEnd) {
							if (((int)ra.op & 1) != 0)
								iiclr++;
							iiclr++;
							Debug.Assert(((x + xOrigin) & 1) == (iiclr & 1));
							for (int xT = x; xT < x + ra.cpDst; xT++)
								bm.SetPixel(xT, y, m_pal[(int)cr.alsIclr[iiclr++]]);
							x += ra.cpDst;
							continue;
						}
						if (ra.op >= Op.SideStart && ra.op <= Op.SideEnd) {
							Debug.Assert((int)ra.op - (int)Op.Side1 + 1 == ra.cpDst);
							for (int xT = x; xT < x + ra.cpDst; xT++)
								bm.SetPixel(xT, y, m_pal[(int)cr.alsIclr[iiclr++]]);
							x += ra.cpDst;
							continue;
						}
						if (ra.op == Op.SideN) {
							Debug.Assert((int)cr.alsIclr[iiclr++] == ra.cpDst);
							for (int xT = x; xT < x + ra.cpDst; xT++)
								bm.SetPixel(xT, y, m_pal[(int)cr.alsIclr[iiclr++]]);
							x += ra.cpDst;
							continue;
						}
						if (ra.op >= Op.ShadowStart && ra.op <= Op.ShadowEnd) {
							Debug.Assert((int)ra.op - (int)Op.Shadow1 + 1 == ra.cpDst);
							for (int xT = x; xT < x + ra.cpDst; xT++)
								bm.SetPixel(xT, y, Color.FromArgb(255, 255, 0));
							x += ra.cpDst;
							continue;
						}
						if (ra.op == Op.ShadowN) {
							for (int xT = x; xT < x + ra.cpDst; xT++)
								bm.SetPixel(xT, y, Color.FromArgb(255, 255, 0));
							x += ra.cpDst;
							continue;
						}
						Debug.Assert(false);
					}
				}
			}

			gMem.Dispose();
			return bm;
		}
#endif

		public void PrintStats() {
			if (m_crLast == null)
				throw new Exception("Compile image first!");
			CompileResults cr = m_crLast;

			int cRuns = 0;
			int cSkips = 0;
			int cRunsInScan = 0;
			int cScans = 0;
			foreach (Frame frame in m_alsFrames) {
				foreach (Run[] arun in frame.alsArunsEven) {
					cScans++;
					foreach (Run run in arun) {
						cRunsInScan++;
						cRuns++;
						if (run.ct == ColorType.Transparent)
							cSkips++;
					}
				}
				foreach (Run[] arun in frame.alsArunsOdd) {
					cScans++;
					foreach (Run run in arun) {
						cRunsInScan++;
						cRuns++;
						if (run.ct == ColorType.Transparent)
							cSkips++;
					}
				}
			}

			Console.WriteLine("Runs: " + cRuns);			
			Console.WriteLine("Skips: " + cSkips);
			Console.WriteLine("Avg runs per image: " + (float)cRuns / 2.0 / (float)m_alsFrames.Count);
			Console.WriteLine("Avg runs per scan: " + (float)cRunsInScan / (float)cScans);

			Console.WriteLine("RunArgs size: " + cr.alsRa.Count * 4);
			Console.WriteLine("RunData size: " + cr.alsIclr.Count);
			Console.WriteLine("SR size: " + m_cy * 4 * m_alsFrames.Count * 2);
		}

		public void Save(string strFile) {
			if (m_crLast == null)
				throw new Exception("Compile image first!");
			BinaryWriter bwtr = new BinaryWriter(new FileStream(strFile, FileMode.Create, FileAccess.Write));
			bwtr.Write(Serialize());
			bwtr.Close();
		}

		public static void EmitOpEnum() {
			TextWriter tw = Console.Out;
			tw.WriteLine("enum Op {");
			int iop = 0;
			int iopEvenDataStart = iop;
			for (int cp = 1; cp <= s_cpDataOpFixedMax; cp++) {
				tw.WriteLine("\tEvenData" + cp + ", // " + iop++);
				tw.WriteLine("\tEvenData" + cp + "_Inc, // " + iop++);
			}
			int iopEvenDataEnd = iop - 1;
			int iopOddDataStart = iop;
			for (int cp = 1; cp <= s_cpDataOpFixedMax; cp++) {
				tw.WriteLine("\tOddData" + cp + ", // " + iop++);
				tw.WriteLine("\tOddData" + cp + "_Inc, // " + iop++);
			}
			int iopOddDataEnd = iop - 1;
			int iopSideStart = iop;
			for (int cp = 1; cp <= s_cpSideOpFixedMax; cp++) {
				tw.WriteLine("\tSide" + cp + ", // " + iop++);
				//tw.WriteLine("\tSide" + cp + "_Inc, // " + iop++);
			}
			int iopSideEnd = iop - 1;
			int iopShadowStart = iop;
			for (int cp = 1; cp <= s_cpShadowOpFixedMax; cp++) {
				tw.WriteLine("\tShadow" + cp + ", // " + iop++);
				//tw.WriteLine("\tShadow" + cp + "_Inc, // " + iop++);
			}
			int iopShadowEnd = iop - 1;

			tw.WriteLine("\tEvenDataLB, // " + iop++);
			tw.WriteLine("\tEvenDataLB_Inc, // " + iop++);
			tw.WriteLine("\tEvenDataLW, // " + iop++);
			tw.WriteLine("\tEvenDataLW_Inc, // " + iop++);
			tw.WriteLine("\tEvenDataLWB, // " + iop++);
			tw.WriteLine("\tEvenDataLWB_Inc, // " + iop++);
			tw.WriteLine("\tEvenDataL, // " + iop++);
			tw.WriteLine("\tEvenDataL_Inc, // " + iop++);
			tw.WriteLine("\tOddDataLB, // " + iop++);
			tw.WriteLine("\tOddDataLB_Inc, // " + iop++);
			tw.WriteLine("\tOddDataLW, // " + iop++);
			tw.WriteLine("\tOddDataLW_Inc, // " + iop++);
			tw.WriteLine("\tOddDataLWB, // " + iop++);
			tw.WriteLine("\tOddDataLWB_Inc, // " + iop++);
			tw.WriteLine("\tOddDataL, // " + iop++);
			tw.WriteLine("\tOddDataL_Inc, // " + iop++);
			tw.WriteLine("\tSideN, // " + iop++);
			//tw.WriteLine("\tSideN_Inc, // " + iop++);
			tw.WriteLine("\tShadowN, // " + iop++);
			//tw.WriteLine("\tShadowN_Inc, // " + iop++);
			tw.WriteLine("\tTransparentN, // " + iop++);
			//tw.WriteLine("\tTransparentN_Inc, // " + iop++);
			tw.WriteLine("\tEndScan, // " + iop++);
			tw.WriteLine("\tError, // " + iop++);

			tw.WriteLine("\tEvenDataStart = " + iopEvenDataStart + ",");
			tw.WriteLine("\tEvenDataEnd = " + iopEvenDataEnd + ",");
			tw.WriteLine("\tOddDataStart = " + iopOddDataStart + ",");
			tw.WriteLine("\tOddDataEnd = " + iopOddDataEnd + ",");
			tw.WriteLine("\tSideStart = " + iopSideStart + ",");
			tw.WriteLine("\tSideEnd = " + iopSideEnd + ",");
			tw.WriteLine("\tShadowStart = " + iopShadowStart + ",");
			tw.WriteLine("\tShadowEnd = " + iopShadowEnd);

			tw.WriteLine("};");
			tw.Close();
		}

		public static void EmitHandlers() {
			//TextWriter tw = new StreamWriter(new MemoryStream());
			TextWriter tw = Console.Out;
			StringCollection strc = new StringCollection();

			//tw.WriteLine(".text");
			tw.WriteLine("");

			// Fixed data handlers
			for (int cp = 1; cp <= s_cpDataOpFixedMax; cp++)
				EmitDataHandler(tw, cp, Align.Even, false, strc);
			for (int cp = 1; cp <= s_cpDataOpFixedMax; cp++)
				EmitDataHandler(tw, cp, Align.Odd, false, strc);

			// Clipped fixed data handlers
			for (int cp = 1; cp <= s_cpDataOpFixedMax; cp++)
				EmitDataHandler(tw, cp, Align.Even, true, strc);
			for (int cp = 1; cp <= s_cpDataOpFixedMax; cp++)
				EmitDataHandler(tw, cp, Align.Odd, true, strc);

			// Fixed side handlers
			for (int cp = 1; cp <= s_cpSideOpFixedMax; cp++)
				EmitSideHandler(tw, cp, strc);
			// Fixed shadow handlers
			for (int cp = 1; cp <= s_cpShadowOpFixedMax; cp++)
				EmitShadowHandler(tw, cp, strc);

			// Non-fixed data handlers
			EmitDataNHandler(tw, "EvenDataLB", Align.Even, 63 * 4 + 1, strc);
			EmitDataNHandler(tw, "EvenDataLW", Align.Even, 63 * 4 + 2, strc);
			EmitDataNHandler(tw, "EvenDataLWB", Align.Even, 63 * 4 + 2 + 1, strc);
			EmitDataNHandler(tw, "EvenDataL", Align.Even, 64 * 4, strc);
			EmitDataNHandler(tw, "OddDataLB", Align.Odd, 1 + 63 * 4 + 1, strc);
			EmitDataNHandler(tw, "OddDataLW", Align.Odd, 1 + 63 * 4 + 2, strc);
			EmitDataNHandler(tw, "OddDataLWB", Align.Odd, 1 + 63 * 4 + 2 + 1, strc);
			EmitDataNHandler(tw, "OddDataL", Align.Odd, 1 + 63 * 4, strc);

			// Table
			strc.Add("SideN");
			strc.Add("ShadowN");
			strc.Add("TransparentN");
			strc.Add("EndScan");
			tw.WriteLine(".data");
			tw.WriteLine(".even");
			tw.WriteLine(".globl gapfnRunOps");
			tw.WriteLine("gapfnRunOps:");
			for (int i = 0; i < strc.Count; i++) {
				string str = ".long " + strc[i];
				str = str.PadRight(40) + "| " + i + " - " + ((Op)i).ToString();
				tw.WriteLine(str);
			}
			tw.WriteLine("");

			// Clip tables
			EmitLeftClipTables(tw);
			EmitRightClipTables(tw);

			tw.Close();
		}

		static OpType GetOpType(Op op) {
			if (op >= Op.EvenDataStart && op <= Op.EvenDataEnd)
				return OpType.Data;
			if (op >= Op.OddDataStart && op <= Op.OddDataEnd)
				return OpType.Data;
			if (op >= Op.EvenDataNStart && op <= Op.EvenDataNEnd)
				return OpType.Data;
			if (op >= Op.OddDataNStart && op <= Op.OddDataNEnd)
				return OpType.Data;
			if (op >= Op.SideStart && op <= Op.SideEnd)
				return OpType.Side;
			if (op == Op.SideN)
				return OpType.Side;
			if (op >= Op.ShadowStart && op <= Op.ShadowEnd)
				return OpType.Shadow;
			if (op == Op.ShadowN)
				return OpType.Shadow;
			if (op == Op.TransparentN)
				return OpType.Transparent;
			Debug.Assert(false);
			return OpType.None;
		}

		static Align GetOpRightAlignment(Op op) {
			// If it's not data, we don't know the alignment
			if (GetOpType(op) != OpType.Data)
				return Align.None;

			if (op >= Op.EvenDataStart && op <= Op.EvenDataEnd) {
				int cp = (int)(op - (int)Op.EvenDataStart) / 2 + 1;
				return (cp & 1) != 0 ? Align.Odd : Align.Even;
			}

			if (op >= Op.OddDataStart && op <= Op.OddDataEnd) {
				int cp = (int)(op - (int)Op.OddDataStart) / 2 + 1;
				return (cp & 1) != 0 ? Align.Even : Align.Odd;
			}				

			switch (op) {
			case Op.EvenDataLB:
			case Op.EvenDataLB_Inc:
				return Align.Odd;

			case Op.EvenDataLW:
			case Op.EvenDataLW_Inc:
				return Align.Even;

			case Op.EvenDataLWB:
			case Op.EvenDataLWB_Inc:
				return Align.Odd;

			case Op.EvenDataL:
			case Op.EvenDataL_Inc:
				return Align.Even;

			case Op.OddDataLB:
			case Op.OddDataLB_Inc:
				return Align.Odd;

			case Op.OddDataLW:
			case Op.OddDataLW_Inc:
				return Align.Even;

			case Op.OddDataLWB:
			case Op.OddDataLWB_Inc:
				return Align.Odd;

			case Op.OddDataL:
			case Op.OddDataL_Inc:
				return Align.Even;
			}

			Debug.Assert(false);
			return Align.None;
		}

		static Align GetOpLeftAlignment(Op op) {
			// If it's not data, we don't know the alignment
			if (GetOpType(op) != OpType.Data)
				return Align.None;

			if (op >= Op.EvenDataStart && op <= Op.EvenDataEnd)
				return Align.Even;

			if (op >= Op.OddDataStart && op <= Op.OddDataEnd)
				return Align.Odd;

			switch (op) {
			case Op.EvenDataLB:
			case Op.EvenDataLB_Inc:
				return Align.Even;

			case Op.EvenDataLW:
			case Op.EvenDataLW_Inc:
				return Align.Even;

			case Op.EvenDataLWB:
			case Op.EvenDataLWB_Inc:
				return Align.Even;

			case Op.EvenDataL:
			case Op.EvenDataL_Inc:
				return Align.Even;

			case Op.OddDataLB:
			case Op.OddDataLB_Inc:
				return Align.Odd;

			case Op.OddDataLW:
			case Op.OddDataLW_Inc:
				return Align.Odd;

			case Op.OddDataLWB:
			case Op.OddDataLWB_Inc:
				return Align.Odd;

			case Op.OddDataL:
			case Op.OddDataL_Inc:
				return Align.Odd;
			}

			Debug.Assert(false);
			return Align.None;
		}

		static void EmitLeftClipTables(TextWriter tw) {
			// Emit gmpopapfnLeftClip
			tw.WriteLine(".globl gmpopapfnLeftClip");
			tw.WriteLine("gmpopapfnLeftClip:");
			for (Op op = Op.EvenData1; op < Op.EndScan; op++) {
				string strT = null;
				switch (GetOpType(op)) {
				case OpType.Data:
					if (GetOpRightAlignment(op) == Align.Even) {
						strT = ".long gmpcpRightpfnREEData";
					} else {
						strT = ".long gmpcpRightpfnREOData";
					}
					break;

				case OpType.Side:
					strT = ".long gmpcppfnSide";
					break;

				case OpType.Shadow:
					strT = ".long gmpcppfnShadow";
					break;

				case OpType.Transparent:
					strT = ".long gmpcppfnTransparent";
					break;

				default:
					Debug.Assert(false);
					break;
				}
				if (strT != null)
					tw.WriteLine(strT.PadRight(40) + "| " + (int)op + " - " + op.ToString());
			}
			tw.WriteLine("");

			// gmpcpRightpfnREEData (REE == Right Edge Even)
			tw.WriteLine("gmpcpRightpfnREEData:");
			for (int cpRight = 0; cpRight < 256; cpRight++) {
				if (cpRight == 0) {
					tw.WriteLine(".long 0");
					continue;
				}
				Align alignLeftEdge = (cpRight & 1) != 0 ? Align.Odd : Align.Even;
				switch (alignLeftEdge) {
				case Align.Even:
					if (cpRight <= s_cpDataOpFixedMax) {
						tw.WriteLine(".long EvenData" + cpRight + "_Clip");
					} else {
						tw.WriteLine(".long EvenDataN_Clip");
					}
					break;

				case Align.Odd:
					if (cpRight <= s_cpDataOpFixedMax) {
						tw.WriteLine(".long OddData" + cpRight + "_Clip");
					} else {
						tw.WriteLine(".long OddDataN_Clip");
					}
					break;
				}
			}
			tw.WriteLine("");

			// gmpcpRightpfnREOData (REO == Right Edge Odd)
			tw.WriteLine("gmpcpRightpfnREOData:");
			for (int cpRight = 0; cpRight < 256; cpRight++) {
				if (cpRight == 0) {
					tw.WriteLine(".long 0");
					continue;
				}
				Align alignLeftEdge = (cpRight & 1) != 0 ? Align.Even : Align.Odd;
				switch (alignLeftEdge) {
				case Align.Even:
					if (cpRight <= s_cpDataOpFixedMax) {
						tw.WriteLine(".long EvenData" + cpRight + "_Clip");
					} else {
						tw.WriteLine(".long EvenDataN_Clip");
					}
					break;

				case Align.Odd:
					if (cpRight <= s_cpDataOpFixedMax) {
						tw.WriteLine(".long OddData" + cpRight + "_Clip");
					} else {
						tw.WriteLine(".long OddDataN_Clip");
					}
					break;
				}
			}
			tw.WriteLine("");

			// gmpcppfnSide
			tw.WriteLine("gmpcppfnSide:");
			for (int cpRight = 0; cpRight < 256; cpRight++) {
				if (cpRight == 0) {
					tw.WriteLine(".long 0");
					continue;
				}
				if (cpRight <= s_cpSideOpFixedMax) {
					tw.WriteLine(".long Side" + cpRight + "_Clip");
				} else {
					tw.WriteLine(".long SideN_Clip");
				}
			}
			tw.WriteLine("");

			// gmpcppfnShadow
			tw.WriteLine("gmpcppfnShadow:");
			for (int cpRight = 0; cpRight < 256; cpRight++) {
				if (cpRight == 0) {
					tw.WriteLine(".long 0");
					continue;
				}
				if (cpRight <= s_cpShadowOpFixedMax) {
					tw.WriteLine(".long Shadow" + cpRight);
				} else {
					tw.WriteLine(".long ShadowN_Clip");
				}
			}
			tw.WriteLine("");

			//gmpcppfnTransparent
			tw.WriteLine("gmpcppfnTransparent:");
			for (int cpRight = 0; cpRight < 256; cpRight++) {
				if (cpRight == 0) {
					tw.WriteLine(".long 0");
					continue;
				}
				tw.WriteLine(".long TransparentN_Clip");
			}
			tw.WriteLine("");
		}

		static void EmitRightClipTables(TextWriter tw) {
			// Emit gmpopapfnRightClip
			tw.WriteLine(".globl gmpopapfnRightClip");
			tw.WriteLine("gmpopapfnRightClip:");
			for (Op op = Op.EvenData1; op < Op.EndScan; op++) {
				string strT = null;
				switch (GetOpType(op)) {
				case OpType.Data:
					if (GetOpLeftAlignment(op) == Align.Even) {
						strT = ".long gmpcpLeftpfnLEEData";
					} else {
						strT = ".long gmpcpLeftpfnLEOData";
					}
					break;

				case OpType.Side:
					strT = ".long gmpcppfnSide";
					break;

				case OpType.Shadow:
					strT = ".long gmpcppfnShadow";
					break;

				case OpType.Transparent:
					strT = ".long gmpcppfnTransparent";
					break;

				default:
					Debug.Assert(false);
					break;
				}
				if (strT != null)
					tw.WriteLine(strT.PadRight(40) + "| " + (int)op + " - " + op.ToString());
			}
			tw.WriteLine("");

			// gmpcpLeftpfnLEEData (LEE == Left Edge Even)
			tw.WriteLine("gmpcpLeftpfnLEEData:");
			for (int cpLeft = 0; cpLeft < 256; cpLeft++) {
				if (cpLeft == 0) {
					tw.WriteLine(".long 0");
					continue;
				}
				if (cpLeft <= s_cpDataOpFixedMax) {
					tw.WriteLine(".long EvenData" + cpLeft + "_Clip");
				} else {
					tw.WriteLine(".long EvenDataN_Clip");
				}
			}
			tw.WriteLine("");

			// gmpcpLeftpfnLEOData (LEO == Left Edge Odd)
			tw.WriteLine("gmpcpLeftpfnLEOData:");
			for (int cpLeft = 0; cpLeft < 256; cpLeft++) {
				if (cpLeft == 0) {
					tw.WriteLine(".long 0");
					continue;
				}
				if (cpLeft <= s_cpDataOpFixedMax) {
					tw.WriteLine(".long OddData" + cpLeft + "_Clip");
				} else {
					tw.WriteLine(".long OddDataN_Clip");
				}
			}
			tw.WriteLine("");
		}

		static void EmitDataNHandler(TextWriter tw, string strLabel, Align align, int cp, StringCollection strc) {
			strc.Add(strLabel);
			strc.Add(strLabel + "_Inc");
			tw.WriteLine(strLabel + "_Inc:");
			tw.WriteLine("\taddq.w #1,%a0");
			tw.WriteLine(strLabel + ":");
			tw.WriteLine("\tmove.b (%a0)+,%d0");
			if (align == Align.Odd) {
				tw.WriteLine("\tmove.b (%a0)+,(%a1)+");
				cp--;
				align = Align.Even;
			}
			tw.WriteLine("\tjmp 2(%pc,%d0.w)");
			EmitAlignedDataCopy(tw, cp, align);
			tw.WriteLine("\tmove.l (%a2)+,%a3");
			tw.WriteLine("\tjmp (%a3)");
			tw.WriteLine("");
		}

		static void EmitShadowHandler(TextWriter tw, int cp, StringCollection strc) {
			string strLabel = "Shadow" + cp;
			strc.Add(strLabel);
			tw.WriteLine(strLabel + ":");
			while (cp-- != 0) {
				tw.WriteLine("\tmove.b (%a1),%d0");
				tw.WriteLine("\tmove.b (%a5,%d0.w),(%a1)+");
			}
			tw.WriteLine("\tmove.l (%a2)+,%a3");
			tw.WriteLine("\tjmp (%a3)");
			tw.WriteLine("");
		}

		static void EmitSideHandler(TextWriter tw, int cp, StringCollection strc) {
			string strLabel = "Side" + cp;
			strc.Add(strLabel);

			tw.WriteLine(strLabel + "_Clip:");
			tw.WriteLine("\tadd.w %d2,%a0");
			tw.WriteLine(strLabel + ":");
			while (cp-- != 0) {
				tw.WriteLine("\tmove.b (%a0)+,%d0");
				tw.WriteLine("\tmove.b (%a4,%d0.w),(%a1)+");
			}
			tw.WriteLine("\tmove.l (%a2)+,%a3");
			tw.WriteLine("\tjmp (%a3)");
			tw.WriteLine("");
		}

		static void EmitDataHandler(TextWriter tw, int cp, Align align, bool fClip, StringCollection strc) {
			string strLabel = (align == Align.Even ? "EvenData" : "OddData") + cp;
			if (!fClip) {
				string strLabelInc = strLabel + "_Inc";
				strc.Add(strLabel);
				strc.Add(strLabelInc);
				tw.WriteLine(strLabelInc + ":");
				tw.WriteLine("\taddq.w #1,%a0");
				tw.WriteLine(strLabel + ":");
			} else {
				tw.WriteLine(strLabel + "_Clip:");
				tw.WriteLine("\tadd.w %d2,%a0");
			}
			EmitAlignedDataCopy(tw, cp, align);
			tw.WriteLine("\tmove.l (%a2)+,%a3");
			tw.WriteLine("\tjmp (%a3)");
			tw.WriteLine("");
		}

		static void EmitAlignedDataCopy(TextWriter tw, int cp, Align align) {
			Align alignWrite = align;
			while (cp != 0) {
				if (alignWrite == Align.Odd) {
					tw.WriteLine("\tmove.b (%a0)+,(%a1)+");
					cp--;
					alignWrite = Align.Even;
					continue;
				}
				switch (cp) {
				case 1:
					tw.WriteLine("\tmove.b (%a0)+,(%a1)+");
					alignWrite = Align.Odd;
					cp -= 1;
					Debug.Assert(cp == 0);
					break;

				case 2:
					tw.WriteLine("\tmove.w (%a0)+,(%a1)+");
					cp -= 2;
					break;

				case 3:
					tw.WriteLine("\tmove.w (%a0)+,(%a1)+");
					tw.WriteLine("\tmove.b (%a0)+,(%a1)+");
					alignWrite = Align.Odd;
					cp -= 3;
					Debug.Assert(cp == 0);
					break;

				case 4:
					tw.WriteLine("\tmove.l (%a0)+,(%a1)+");
					cp -= 4;
					break;

				default:
					tw.WriteLine("\tmove.l (%a0)+,(%a1)+");
					cp -= 4;
					break;
				}
			}
		}

		// Emit code to draw the image. This is for data collection purposes at the moment

		public void EmitCode(string str, Align align) {
			TextWriter tw = Console.Out;
			tw.WriteLine(".section code4,\"x\"");
			tw.WriteLine(".even");
			tw.WriteLine(".globl " + str);
			tw.WriteLine(str + ":");
			tw.WriteLine("\tlea " + str + "_data(%pc),%a0");
			tw.WriteLine("\tmove.w #" + (m_cy - 1) + ",%d1");

			Frame frame = (Frame)m_alsFrames[0];
			ArrayList aalsRuns;
			if (align == Align.Even) {
				aalsRuns = frame.alsArunsEven;
			} else {
				aalsRuns = frame.alsArunsOdd;
			}

			int xEmitLast = 0;
			int yEmitLast = 0;
			ArrayList alsIclr = new ArrayList();
			for (int y = 0; y < m_cy; y++) {
				int x = 0;
				Run[] arun = (Run[])aalsRuns[y];
				foreach (Run run in arun) {
					if (run.ct == ColorType.EndScan)
						break;
					Align alignDst;
					if (align == Align.Even) {
						alignDst = (x & 1) != 0 ? Align.Odd : Align.Even;
					} else {
						alignDst = (x & 1) == 0 ? Align.Odd : Align.Even;
					}
					Align alignSrc = (alsIclr.Count & 1) != 0 ? Align.Odd : Align.Even;
					int cp;

					// First destination stepping
					switch (run.ct) {
					case ColorType.Data:
					case ColorType.Shadow:
					case ColorType.Side:
						if (y - yEmitLast > 1) {
							int cyStep = y - yEmitLast;
							if (cyStep <= 8) {
								tw.WriteLine("\tsubq.w #" + cyStep + ",%d1");
							} else {
								tw.WriteLine("\tsub.w #" + cyStep + ",%d1");
							}
							tw.WriteLine("\tbpl.b 1f");
							tw.WriteLine("\trts");
							tw.WriteLine("1:");
							int cbStep = ((y - yEmitLast) - 1) * 160 + (160 - xEmitLast) + x;
							tw.WriteLine("\tadd.w #" + cbStep + ",%a1");
						} else if (y - yEmitLast == 1) {
							tw.WriteLine("\tdbra %d1,1f");
							tw.WriteLine("\trts");
							tw.WriteLine("1:");
							int cbStep = ((y - yEmitLast) - 1) * 160 + (160 - xEmitLast) + x;
							tw.WriteLine("\tadd.w #" + cbStep + ",%a1");
						} else if (y == yEmitLast) {
							int cbStep = x - xEmitLast;
							if (cbStep <= 8) {
								if (cbStep > 0)
									tw.WriteLine("\taddq.w #" + cbStep + ",%a1");
							} else {
								tw.WriteLine("\tadd.w #" + cbStep + ",%a1");
							}
						}
						break;
					}

					// Now handle run handling code
					switch (run.ct) {
					case ColorType.Data:
						if (alignSrc != alignDst) {
							alsIclr.Add(s_iclrNotAssigned);
							tw.WriteLine("\taddq.w #1,%a0");
							alignSrc = alignDst;
						}
						alsIclr.AddRange(run.aiclr);
						EmitAlignedDataCopy(tw, run.cp, alignDst);
						x += run.cp;
						xEmitLast = x;
						yEmitLast = y;
						break;

					case ColorType.Shadow:
						cp = run.cp;
						while (cp-- != 0) {
							tw.WriteLine("\tmove.b (%a1),%d0");
							tw.WriteLine("\tmove.b 0.b(%a5,%d0.w),(%a1)+");
						}
						x += run.cp;
						xEmitLast = x;
						yEmitLast = y;
						break;

					case ColorType.Side:
						alsIclr.AddRange(run.aiclr);
						cp = run.cp;
						while (cp-- != 0) {
							tw.WriteLine("\tmove.b (%a0)+,%d0");
							tw.WriteLine("\tmove.b 0.b(%a4,%d0.w),(%a1)+");
						}
						x += run.cp;
						xEmitLast = x;
						yEmitLast = y;
						break;

					case ColorType.Transparent:
						x += run.cp;
						break;
					}
				}
			}
			tw.WriteLine("\trts");
			tw.WriteLine("");

			// Emit data:
			tw.WriteLine(".even");
			tw.WriteLine(str + "_data:");
			foreach (int iclr in alsIclr)
				tw.WriteLine(".byte " + iclr);
			tw.Close();
		}

		public int FrameCount {
			get {
				return m_alsFrames.Count;
			}
		}
	}
}
