#if 0

#if 0
struct Frame {
	word ibsrEven;
	word ibsrOdd;
};

struct TBitmapSRHeader {
	word cx;
	word cy;
	word cra;
	word ibra;
	word cFrames;
	Frame aframe[1];
};

class TBitmapSR : public TBitmap
{
public:
	TBitmapSR() secTBitmap;
	virtual ~TBitmapSR() secTBitmap;
	virtual bool Init(void *pv, FileMap *pfmap) secTBitmap;
	virtual void BltTo(class DibBitmap *pbmDst, int xDst, int yDst, byte *aclrMap = NULL, Rect *prcSrc = NULL) secTBitmap;
	virtual void GetSize(Size *psiz) secTBitmap;

private:
	FileMap m_fmap;
	TBitmapSRHeader *m_pbmh;
	dword *m_ppfn;
};
#endif

#include "ht.h"

struct ScanRecord {
	word officlr;
	word iraTimes4;
};

struct RunArgs {
	byte op;
	byte cpSrc;
	byte cpArgs;
	byte cpDst;
};

#ifdef PIL
extern "C" {
extern dword gapfnRunOps[];
extern dword *gmpopapfnLeftClip[];
extern dword *gmpopapfnRightClip[];
void TBltSR8NoClip(ScanRecord *psr, int cy, byte *pbDst, int cbDst, byte *aiclrSide, byte *aiclrShadow, dword *ppfn) secCode4;
void TBltSR8LeftClip(ScanRecord *psr, int xLeft, int cy, byte *pbDst, int cbDst, byte *aiclrSide, byte *aiclrShadow, dword *apfnList, RunArgs *praList, dword **mpopapfnLeftClip) secCode4;
void TBltSR8RightClip(ScanRecord *psr, int xLeft, int cy, byte *pbDst, int cbDst, byte *aiclrSide, byte *aiclrShadow, dword *apfnList, RunArgs *praList, dword **mpopapfnRightClip) secCode4;
void FillEvenEven8(byte *pbRow, int cx, int cy, int cxDib, byte bFill);
};
#endif

TBitmapSR::TBitmapSR()
{
	m_pbmh = NULL;
	m_ppfn = NULL;
}

TBitmapSR::~TBitmapSR()
{
	gmmgr.DbMemFree(m_ppfn);
	if (m_pbmh != NULL) {
		UnmapFile(&m_fmap);
		m_pbmh = NULL;
	}
}

bool TBitmapSR::Init(void *pv, FileMap *pfmap)
{
	// Init run op function pointers

	TBitmapSRHeader *pbmh = (TBitmapSRHeader *)pv;

#ifdef PIL
	int cra = BigWord(pbmh->cra);

#ifdef INCL_MEMTRACE
	Trace("tbitmapsr: %d cra's", cra);
#endif
	dword *ppfn = new dword[cra];
	if (ppfn == NULL)
		return false;
	RunArgs *praT = (RunArgs *)((byte *)pbmh + BigWord(pbmh->ibra));
	dword *ppfnT = ppfn;
	for (int ira = 0; ira < cra; ira++) {
		*ppfnT++ = gapfnRunOps[praT->op];
		praT++;
	}
	m_ppfn = (dword *)gmmgr.DbMemAlloc(cra * sizeof(dword));
	if (m_ppfn == NULL) {
		delete ppfn;
		return false;
	}
	gmmgr.DbWriteMem(m_ppfn, 0, ppfn, cra * sizeof(dword));
	delete ppfn;
#endif

	// Done

	m_pbmh = pbmh;
	m_fmap = *pfmap;
	return true;
}

#ifdef WIN
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
	DataN, // 192
	DataN_Inc, // 193
	SideN, // 194
	ShadowN, // 195
	TransparentN, // 196
	EndScan, // 197
	Error, // 198
	EvenDataStart = 0,
	EvenDataEnd = 63,
	OddDataStart = 64,
	OddDataEnd = 127,
	SideStart = 128,
	SideEnd = 159,
	ShadowStart = 160,
	ShadowEnd = 191
};

void TBltSR8NoClip(ScanRecord *psr, int cy, byte *pbDst, int cbDst, byte *aiclrSide, byte *aiclrShadow, TBitmapSRHeader *pbmh, DibBitmap *pbmDst)
{
	byte *pbDstSav = pbDst;
	for (; cy != 0; cy--, psr++) {
		byte *pbSrc = (byte *)psr + BigWord(psr->officlr);
		RunArgs *pra = (RunArgs *)((byte *)pbmh + BigWord(pbmh->ibra) + BigWord(psr->iraTimes4));

		for (; true; pra++) {
			//temp
			//gdisp.CopyToScreen(pbmDst, 0, 0);

			Op op = (Op)pra->op;
			if (op == EndScan) {
				pbDstSav += cbDst;
				pbDst = pbDstSav;
				break;
			}
			if (op <= OddDataEnd) {
				if (op & 1)
					pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = *pbSrc++;
				continue;
			}
			if (op <= SideEnd) {
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = aiclrSide[*pbSrc++];
				continue;
			}
			if (op <= ShadowEnd) {
				int cp = pra->cpDst;
				while (cp-- != 0) {
					*pbDst = aiclrShadow[*pbDst];
					pbDst++;
				}
				continue;
			}
			if (op <= DataN_Inc) {
				if (op & 1)
					pbSrc++;
				int cp = (*pbSrc++) + 1;
				while (cp-- != 0)
					*pbDst++ = *pbSrc++;
				continue;
			}
			if (op == SideN) {
				int cp = (*pbSrc++) + 1;
				while (cp-- != 0)
					*pbDst++ = aiclrSide[*pbSrc++];
				continue;
			}
			if (op == ShadowN) {
				int cp = (*pbSrc++) + 1;
				while (cp-- != 0) {
					*pbDst = aiclrShadow[*pbDst];
					pbDst++;
				}
				continue;
			}
			if (op == TransparentN) {
				pbDst += *pbSrc++;
				continue;
			}
			Assert(false);
		}
	}
}

void TBltSR8LeftClip(ScanRecord *psr, int xLeft, int cy, byte *pbDst, int cbDst, byte *aiclrSide, byte *aiclrShadow, TBitmapSRHeader *pbmh, DibBitmap *pbmDst)
{
	byte *pbDstSav = pbDst;
	for (; cy != 0; cy--, psr++) {
		byte *pbSrc = (byte *)psr + BigWord(psr->officlr);
		RunArgs *pra = (RunArgs *)((byte *)pbmh + BigWord(pbmh->ibra) + BigWord(psr->iraTimes4));
		int x = 0;
		bool fEndScan = false;
		while (x + pra->cpDst <= xLeft) {
			if (pra->op == EndScan) {
				pbDstSav += cbDst;
				pbDst = pbDstSav;
				fEndScan = true;
				break;
			}
			x += pra->cpDst;
			pbSrc += pra->cpSrc;
			pra++;
		}
		if (fEndScan)
			continue;

		// Is this run partially clipped?
		if (x != xLeft) {
			Op op = (Op)pra->op;
			int cp = x + pra->cpDst - xLeft;
			if (op <= OddDataEnd) {
				// cpSrc = N
				// cpDst = N
				// cpSrc = N + 1
				// cpDst = N
				pbSrc += xLeft - x;
				if (op & 1)
					pbSrc++;
				while (cp-- != 0)
					*pbDst++ = *pbSrc++;
			} else if (op <= SideEnd) {
				// cpSrc = N
				// cpDst = N
				pbSrc += xLeft - x;
				while (cp-- != 0)
					*pbDst++ = aiclrSide[*pbSrc++];
			} else if (op <= ShadowEnd) {
				// cpSrc = 0
				// cpDst = N
				while (cp-- != 0) {
					*pbDst = aiclrShadow[*pbDst];
					pbDst++;
				}
			} else if (op <= DataN_Inc) {
				// cpSrc = 1 + N
				// cpDst = N
				// cpSrc = 1 + N + 1
				// cpDst = N
				pbSrc += xLeft - x + 1;
				if (op & 1)
					pbSrc++;
				while (cp-- != 0)
					*pbDst++ = *pbSrc++;
			} else if (op == SideN) {
				// cpSrc = 1 + N
				// cpDst = N
				pbSrc += xLeft - x + 1;
				while (cp-- != 0)
					*pbDst++ = aiclrSide[*pbSrc++];
			} else if (op == ShadowN) {
				// cpSrc = 1
				// cpDst = N
				pbSrc += 1;
				while (cp-- != 0) {
					*pbDst = aiclrShadow[*pbDst];
					pbDst++;
				}
			} else if (op == TransparentN) {
				// cpSrc == 1
				// cpDst == N
				pbSrc += 1;
				pbDst += cp;
			}
		}

		// The rest of the full runs
		pra++;
		for (; true; pra++) {
			//temp
			//gdisp.CopyToScreen(pbmDst, 0, 0);

			Op op = (Op)pra->op;
			if (op == EndScan) {
				pbDstSav += cbDst;
				pbDst = pbDstSav;
				break;
			}
			if (op <= OddDataEnd) {
				if (op & 1)
					pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = *pbSrc++;
				continue;
			}
			if (op <= SideEnd) {
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = aiclrSide[*pbSrc++];
				continue;
			}
			if (op <= ShadowEnd) {
				int cp = pra->cpDst;
				while (cp-- != 0) {
					*pbDst = aiclrShadow[*pbDst];
					pbDst++;
				}
				continue;
			}
			if (op <= DataN_Inc) {
				pbSrc++;
				if (op & 1)
					pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = *pbSrc++;
				continue;
			}
			if (op == SideN) {
				pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = aiclrSide[*pbSrc++];
				continue;
			}
			if (op == ShadowN) {
				pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0) {
					*pbDst = aiclrShadow[*pbDst];
					pbDst++;
				}
				continue;
			}
			if (op == TransparentN) {
				pbDst += *pbSrc++;
				continue;
			}
			Assert(false);
		}
	}
}

void TBltSR8RightClip(ScanRecord *psr, int cx, int cy, byte *pbDst, int cbDst, byte *aiclrSide, byte *aiclrShadow, TBitmapSRHeader *pbmh, DibBitmap *pbmDst)
{
	byte *pbDstSav = pbDst;
	for (; cy != 0; cy--, psr++) {
		byte *pbSrc = (byte *)psr + BigWord(psr->officlr);
		RunArgs *pra = (RunArgs *)((byte *)pbmh + BigWord(pbmh->ibra) + BigWord(psr->iraTimes4));

		// Find the intersecting run
		RunArgs *praT = pra;
		int x = 0;
		while (praT->op != EndScan) {
			if (x + praT->cpDst > cx)
				break;
			x += praT->cpDst;
			praT++;
		}

		// Draw full runs
		for (; pra < praT; pra++) {
			//temp
			//gdisp.CopyToScreen(pbmDst, 0, 0);

			Op op = (Op)pra->op;
			Assert(op != EndScan);
			if (op <= OddDataEnd) {
				if (op & 1)
					pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = *pbSrc++;
				continue;
			}
			if (op <= SideEnd) {
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = aiclrSide[*pbSrc++];
				continue;
			}
			if (op <= ShadowEnd) {
				int cp = pra->cpDst;
				while (cp-- != 0) {
					*pbDst = aiclrShadow[*pbDst];
					pbDst++;
				}
				continue;
			}
			if (op <= DataN_Inc) {
				pbSrc++;
				if (op & 1)
					pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = *pbSrc++;
				continue;
			}
			if (op == SideN) {
				pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0)
					*pbDst++ = aiclrSide[*pbSrc++];
				continue;
			}
			if (op == ShadowN) {
				pbSrc++;
				int cp = pra->cpDst;
				while (cp-- != 0) {
					*pbDst = aiclrShadow[*pbDst];
					pbDst++;
				}
				continue;
			}
			if (op == TransparentN) {
				pbDst += *pbSrc++;
				continue;
			}
			Assert(false);
		}

		//temp
		//gdisp.CopyToScreen(pbmDst, 0, 0);

		// Draw clipped run
		if (x < cx) {
			Op op = (Op)pra->op;
			if (op != EndScan && op != TransparentN) {
				int cp = cx - x;
				if (op <= OddDataEnd) {
					if (op & 1)
						pbSrc++;
					while (cp-- != 0)
						*pbDst++ = *pbSrc++;
				} else if (op <= SideEnd) {
					while (cp-- != 0)
						*pbDst++ = aiclrSide[*pbSrc++];
				} else if (op <= ShadowEnd) {
					while (cp-- != 0) {
						*pbDst = aiclrShadow[*pbDst];
						pbDst++;
					}
				} else if (op <= DataN_Inc) {
					pbSrc++;
					if (op & 1)
						pbSrc++;
					while (cp-- != 0)
						*pbDst++ = *pbSrc++;
				} else if (op == SideN) {
					pbSrc++;
					while (cp-- != 0)
						*pbDst++ = aiclrSide[*pbSrc++];
				} else if (op == ShadowN) {
					pbSrc++;
					while (cp-- != 0) {
						*pbDst = aiclrShadow[*pbDst];
						pbDst++;
					}
				}
			}
		}
		pbDstSav += cbDst;
		pbDst = pbDstSav;

		//temp
		//gdisp.CopyToScreen(pbmDst, 0, 0);
	}
}
#endif

void TBitmapSR::BltTo(DibBitmap *pbmDst, int xDst, int yDst, byte *aclrMap, Rect *prcSrc)
{
	// Get dib dimensions

	Size sizT;
	pbmDst->GetSize(&sizT);

	// Source rect to blt from

	Rect rcSrc;
	if (prcSrc != NULL) {
		rcSrc = *prcSrc;
	} else {
		rcSrc.Set(0, 0, BigWord(m_pbmh->cx), BigWord(m_pbmh->cy));
	}

	// Right and bottom edge clipping

	if (sizT.cx - xDst < rcSrc.Width())
		rcSrc.right = rcSrc.left + sizT.cx - xDst;
	if (sizT.cy - yDst < rcSrc.Height())
		rcSrc.bottom = rcSrc.top + sizT.cy - yDst;

	// Before we much with xDst, get the alignment right

	Frame *pframe = &m_pbmh->aframe[0];
	word ibsr = (xDst & 1) ? BigWord(pframe->ibsrOdd) : BigWord(pframe->ibsrEven);

	// Left and top edge clipping

	if (xDst < 0) {
		rcSrc.left -= xDst;
		xDst = 0;
	}
	if (yDst < 0) {
		rcSrc.top -= yDst;
		yDst = 0;
	}

	// Anything to blt?

	if (rcSrc.IsEmpty())
		return;

#ifdef STATS_DISPLAY
	extern int gcBitmapsDrawn;
	gcBitmapsDrawn++;
#endif

	ScanRecord *psr = (ScanRecord *)((byte *)m_pbmh + ibsr + rcSrc.top * sizeof(ScanRecord));
	int cbRowDst = (sizT.cx + 1) & ~1;
	byte *pbBits = pbmDst->GetBits();

//temp
//	BitmapType *pbmpScreen = WinGetBitmap(WinGetDisplayWindow());
//	pbBits = (byte *)BmpGetBits(pbmpScreen);
//	FillEvenEven8(pbBits, 160, 160, 160, 0);
//end temp

	byte *pbDst = pbBits + yDst * cbRowDst + xDst;

	if (aclrMap == NULL)
		aclrMap = gaclrIdentity;

	if (rcSrc.left == 0) {
		if (rcSrc.Width() != BigWord(m_pbmh->cx)) {
			// Right clip

#ifdef PIL
//			TBltSR8RightClip(psr, rcSrc.Width(), rcSrc.Height(), pbDst, cbRowDst, aclrMap, gmpiclriclrShadow, m_ppfn, (RunArgs *)((byte *)m_pbmh + BigWord(m_pbmh->ibra)), gmpopapfnRightClip);
#endif
#ifdef WIN
			TBltSR8RightClip(psr, rcSrc.Width(), rcSrc.Height(), pbDst, cbRowDst, aclrMap, gmpiclriclrShadow, m_pbmh, pbmDst);
#endif
		} else {
			// No clip
#ifdef PIL
			TBltSR8NoClip(psr, rcSrc.Height(), pbDst, cbRowDst, aclrMap, gmpiclriclrShadow, m_ppfn);
#endif
#ifdef WIN
			TBltSR8NoClip(psr, rcSrc.Height(), pbDst, cbRowDst, aclrMap, gmpiclriclrShadow, m_pbmh, pbmDst);
#endif
		}
	} else {
		// left clip

#ifdef PIL
//		TBltSR8LeftClip(psr, rcSrc.left, rcSrc.Height(), pbDst, cbRowDst, aclrMap, gmpiclriclrShadow, m_ppfn, (RunArgs *)((byte *)m_pbmh + BigWord(m_pbmh->ibra)), gmpopapfnLeftClip);
#endif
#ifdef WIN
		TBltSR8LeftClip(psr, rcSrc.left, rcSrc.Height(), pbDst, cbRowDst, aclrMap, gmpiclriclrShadow, m_pbmh, pbmDst);
#endif
	}

}

void TBitmapSR::GetSize(Size *psiz)
{
	psiz->cx = BigWord(m_pbmh->cx);
	psiz->cy = BigWord(m_pbmh->cy);
}

#endif