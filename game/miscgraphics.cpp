// This gets included in the windows build
// Also gets included by ARMlet code

#define SwapDword(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | (((x)&0xFF000000)>>24))
#define SwapWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))

void Shadow(byte *pbDst, int cbRowDst, int cx, int cy, byte *aclrMap)
{
#if defined(__CPU_68K) && !defined(DEV_BUILD)
	return;
#else

	byte *pbRow = pbDst;
	while (cy-- != 0) {
		int cxT = cx;
		byte *pbRowT = pbRow;
		while (cxT-- != 0) {
//			*pbRowT++ = aclrMap[*pbRowT];
            // BUGBUG: if we post-increment pbRowT as above the CodeWarrior ARM
            // compiler screws it up such that the RHS pbT is one byte ahead of
            // the LHS pbT, the end result being that the shadowed area is
            // shifted one pixel to the left!

			*pbRowT = aclrMap[*pbRowT];
			pbRowT++;
		}
		pbRow += cbRowDst;
	}
#endif
}

void Fill(byte *pbDst, int cx, int cy, int cbStride, byte bFill)
{
#if defined(__CPU_68K) && !defined(DEV_BUILD)
	return;
#else

	int cbReturn = cbStride - cx;
	while (cy-- != 0) {
		int cxT = cx;
		while (cxT-- != 0)
			*pbDst++ = bFill;
		pbDst += cbReturn;
	}
#endif
}

void LeftToRightBlt(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy)
{
#if defined(__CPU_68K) && !defined(DEV_BUILD)
	return;
#else

	int cwSrcReturn = (cxSrcStride - cx) / 2;
	int cwDstReturn = (cxDstStride - cx) / 2;
	word *pwSrc = (word *)pbSrc;
	word *pwDst = (word *)pbDst;
	while (cy-- != 0) {
		int cw = cx / 2;
		while (cw-- != 0)
			*pwDst++ = *pwSrc++;
		pwSrc += cwSrcReturn;
		pwDst += cwDstReturn;
	}
#endif
}

void RightToLeftBlt(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy)
{
#if defined(__CPU_68K) && !defined(DEV_BUILD)
	return;
#else

	if (cy <= 0)
		return;
	int cwSrcReturn = (cxSrcStride - cx) / 2;
	int cwDstReturn = (cxDstStride - cx) / 2;
	word *pwSrc = (word *)(pbSrc + (long)(cy - 1) * cxSrcStride + cx - 2);
	word *pwDst = (word *)(pbDst + (long)(cy - 1) * cxDstStride + cx - 2);
	while (cy-- != 0) {
		int cw = cx / 2;
		while (cw-- != 0)
			*pwDst-- = *pwSrc--;
		pwSrc -= cwSrcReturn;
		pwDst -= cwDstReturn;
	}
#endif
}

void DrawTileMap(byte **ppbMap, int ctx, int cty, byte *pbDst, int cbDstStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside, int cxTile, int cyTile)
{
#if defined(__CPU_68K) && !defined(DEV_BUILD)
	return;
#else

	int cxRightDst = cxLeftTile + ctxInside * cxTile + cxRightTile;
	int cyBottomDst = cyTopTile + ctyInside * cyTile + cyBottomTile;

	int ctxT = ctxInside + ((cxLeftTile != 0) ? 1 : 0) + ((cxRightTile != 0) ? 1 : 0);
	int ctyT = ctyInside + ((cyTopTile != 0) ? 1 : 0) + ((cyBottomTile != 0) ? 1 : 0);

	int xOffsetTile = cxLeftTile == 0 ? cxLeftTile : cxTile - cxLeftTile;
	int yOffsetTile = cyTopTile == 0 ? cyTopTile : cyTile - cyTopTile;

	byte **ppbMapT = ppbMap;
	int yTile = -yOffsetTile;
	for (int ty = 0; ty < ctyT; ty++, yTile += cyTile) {
		int xTile = -xOffsetTile;
		word *pwDst = (word *)(pbDst + (long)((yTile < 0 ? 0 : yTile) * cbDstStride));
		for (int tx = 0; tx < ctxT; tx++, xTile += cxTile) {
			int xLeft = xTile >= 0 ? xTile : 0;
			int xRight = xTile + cxTile;
			if (xRight > cxRightDst)
				xRight = cxRightDst;

			if (*ppbMapT == NULL) {
				ppbMapT++;
				pwDst += (xRight - xLeft) / 2;
				continue;
			}

			int yTop = yTile >= 0 ? yTile : 0;
			int yBottom = yTile + cyTile;
			if (yBottom > cyBottomDst)
				yBottom = cyBottomDst;

			int xSrc = xLeft - xTile;
			int ySrc = yTop - yTile;
			int cxSrc = xRight - xLeft;
			int cySrc = yBottom - yTop;
			// Swap if data is 68K but this code is armlet (v1.0 format)

#if defined(PIL) && !defined(PNO)
			dword dw = (dword)*ppbMapT;
			word *pwSrcT = (word *)SwapDword(dw);
#else
			word *pwSrcT = (word *)*ppbMapT;
#endif
			word *pwDstT = pwDst;

			if (xSrc == 0 && ySrc == 0 && cxSrc == 16 && cySrc == 16) {
				word cwSkip = (cbDstStride - 16) / 2;
				for (int n = 0; n < 16; n++, pwDstT += cwSkip) {
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
				}
			} else if (xSrc == 0 && ySrc == 0 && cxSrc == 24 && cySrc == 24) {
				word cwSkip = (cbDstStride - 24) / 2;
				for (int n = 0; n < 24; n++, pwDstT += cwSkip) {
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
				}
			} else if (xSrc == 0 && ySrc == 0 && cxSrc == 32 && cySrc == 32) {
				word cwSkip = (cbDstStride - 32) / 2;
				for (int n = 0; n < 32; n++, pwDstT += cwSkip) {
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
					*pwDstT++ = *pwSrcT++;
				}
			} else {
				int cwDstReturn = (cbDstStride - cxSrc) / 2;
				int cwSrcReturn = (cxTile - cxSrc) / 2;
				word *pwSrcT2 = (word *)(((byte *)pwSrcT) + ySrc * cxTile + xSrc);
				word *pwDstT2 = pwDstT;
				for (int y = 0; y < cySrc; y++) {
					for (int x = 0; x < cxSrc / 2; x++)
						*pwDstT2++ = *pwSrcT2++;
					pwSrcT2 += cwSrcReturn;
					pwDstT2 += cwDstReturn;
				}
			}
			ppbMapT++;
			pwDst += cxSrc / 2;
		}
		ppbMapT += ctx - ctxT;
	}
#endif
}

inline word GetSideColorOffset(int b0, int b1, int b2, int b3)
{
	return (word)((b0 * 125 + b1 * 25 + b2 * 5 + b3 * 1) * 4);
}

word Compile8(byte *pbCompileBuffer, ScanData *psd, bool fOdd)
{
	// Make space for header

	byte *pb = pbCompileBuffer;
	pb += sizeof(word); // ibasc;
	pb += sizeof(word); // ibaiclr;

	// Copy over ops. Translate even / odd to asked alignment
	// Filter out sc's (will go to another stream).

	byte *pop = (byte *)(psd + 1);
	byte *pbT = pb;
	if (fOdd)
		*pbT++ = kopAlign;
	while (true) {
		int op = (int)(word)*pop++;
		if (op >= kopEvenData1 && op <= kopEvenData48) {
			if (fOdd)
				op = op - kopEvenData1 + kopOddData1;
			*pbT++ = op;
			continue;
		}
		if (op >= kopOddData1 && op <= kopOddData48) {
			if (fOdd)
				op = op - kopOddData1 + kopEvenData1;
			*pbT++ = op;
			continue;
		}
		if (op >= kopEvenSide1 && op <= kopEvenSide16) {
			int cb = op - kopEvenSide1 + 1;
			pop += (cb + 1) / 2;
			if (fOdd)
				op = op - kopEvenSide1 + kopOddSide1;
			*pbT++ = op;
			continue;
		}
		if (op >= kopOddSide1 && op <= kopOddSide16) {
			int cb = op - kopOddSide1 + 1;
			pop += (cb + 1) / 2;
			if (fOdd)
				op = op - kopOddSide1 + kopEvenSide1;
			*pbT++ = op;
			continue;
		}
		*pbT++ = op;
		if (op == kopEnd)
			break;
	}
	if (((byte)(pword)pbT) & 1)
		pbT++;
	int cbOps = pbT - pb;

	// Transcode side codes into alignment sensitive mapping table offsets

	word *pwT = (word *)pbT;
	pop = (byte *)(psd + 1);
	while (true) {
		int op = (int)(word)*pop++;
		int cb;
		bool fOddOp;
		if (op >= kopEvenSide1 && op <= kopEvenSide16) {
			cb = op - kopEvenSide1 + 1;
			fOddOp = fOdd;
		} else if (op >= kopOddSide1 && op <= kopOddSide16) {
			cb = op - kopOddSide1 + 1;
			fOddOp = !fOdd;
		} else if (op == kopEnd) {
			break;
		} else {
			continue;
		}

		int cbT = cb;
		byte abSideIndex[512];
		byte *pbSideIndex = abSideIndex;
		while (true) {
			byte sc = *pop++;
			*pbSideIndex++ = ((sc >> 5) & 0x7);
			cbT--;
			if (cbT == 0)
				break;
			*pbSideIndex++ = ((sc >> 1) & 0x7);
			cbT--;
			if (cbT == 0)
				break;
		}
		
		pbSideIndex = abSideIndex;
		cbT = cb;
		if (fOddOp) {
			byte b1 = *pbSideIndex++;
			*pwT++ = GetSideColorOffset(b1, 0, 0, 0);
			cbT--;
		}
		while (cbT != 0) {
			byte b1, b2, b3, b4;
			switch (cbT) {
			case 1:
				b1 = *pbSideIndex++;
				*pwT++ = GetSideColorOffset(b1, 0, 0, 0);
				cbT--;
				break;

			case 2:
			case 3:
				b1 = *pbSideIndex++;
				b2 = *pbSideIndex++;
				*pwT++ = GetSideColorOffset(b1, b2, 0, 0);
				cbT -= 2;
				break;

			case 4:
			default:
				b1 = *pbSideIndex++;
				b2 = *pbSideIndex++;
				b3 = *pbSideIndex++;
				b4 = *pbSideIndex++;
				*pwT++ = GetSideColorOffset(b1, b2, b3, b4);
				cbT -= 4;
				break;
			}
		}
	}
	pbT = (byte *)pwT;
	int cbScs = pbT - (pb + cbOps);

	// Now copy data and align as we go

	pop = pbCompileBuffer + 4;
	byte *pbSrc = ((byte *)psd) + SwapWord(psd->ibaiclr);
	while (true) {
		int op = (int)(word)*pop++;
		if (op == kopEnd)
			break;
		if (op == kopAlign) {
			pbT++;
			continue;
		}
		if (op >= kopEvenData1 && op <= kopEvenData48) {
			int cb = op - kopEvenData1 + 1;
			byte *pbDstT = pbT;
			byte *pbSrcT = pbSrc;
			while (pbDstT < &pbT[cb])
				*pbDstT++ = *pbSrcT++;
			pbT += cb;
			pbSrc += cb;
		}
		if (op >= kopOddData1 && op <= kopOddData48) {
			int cb = op - kopOddData1 + 1;
			byte *pbDstT = pbT;
			byte *pbSrcT = pbSrc;
			while (pbDstT < &pbT[cb])
				*pbDstT++ = *pbSrcT++;
			pbT += cb;
			pbSrc += cb;
		}
	}

	// Stuff in correct byte offsets

	pwT = (word *)pbCompileBuffer;

	// ibasc
	*pwT++ = 4 + cbOps;

	// ibaiclr
	*pwT = 4 + cbOps + cbScs;

	return pbT - pbCompileBuffer;
}

void DrawDispatch(byte *pb, byte *pbSrc, byte *pbDst, int cbReturn, dword *mpscaiclr, byte *mpiclriclrShadow)
{
#if defined(__CPU_68K) && !defined(DEV_BUILD)
	return;
#else

	word *pw = (word *)pb;
	word *psc = (word *)(pb + pw[0]);
	if (pbSrc == NULL)
		pbSrc = pb + pw[1];
	byte *pop = pb + sizeof(word) + sizeof(word);

	while (true) {
		int op = (int)(word)*pop++;
		if (op == kopAlign) {
			pbSrc++;
			continue;
		}
		if (op >= kopEvenData1 && op <= kopEvenData48) {
			int cb = op - kopEvenData1 + 1;
			for (; cb >= 2; cb -= 2) {
				*((word *)pbDst) = *((word *)pbSrc);
				pbDst += 2;
				pbSrc += 2;
			}
			if (cb != 0)
				*pbDst++ = *pbSrc++;
			continue;
		}
		if (op >= kopOddData1 && op <= kopOddData48) {
			int cb = op - kopOddData1 + 1;
			*pbDst++ = *pbSrc++;
			cb--;
			for (; cb >= 2; cb -= 2) {
				*((word *)pbDst) = *((word *)pbSrc);
				pbDst += 2;
				pbSrc += 2;
			}
			if (cb != 0)
				*pbDst++ = *pbSrc++;
			continue;
		}
		if (op >= kopEvenSide1 && op <= kopEvenSide16) {
			int cb = op - kopEvenSide1 + 1;
			for (; cb >= 4; cb -= 4) {
				word *pwSrc = (word *)(((byte *)mpscaiclr) + (*psc));
				*((word *)pbDst) = *pwSrc++;
				pbDst += 2;
				*((word *)pbDst) = *pwSrc++;
				pbDst += 2;
				psc++;
			}
			for (; cb >= 2; cb -= 2) {
				*((word *)pbDst) = *((word *)(((byte *)mpscaiclr) + (*psc)));
				psc++;
				pbDst += 2;
			}
			if (cb != 0) {
				*pbDst++ = *((byte *)(((byte *)mpscaiclr) + (*psc)));
				psc++;
			}
			continue;
		}
		if (op >= kopOddSide1 && op <= kopOddSide16) {
			int cb = op - kopOddSide1 + 1;
			*pbDst++ = *((byte *)(((byte *)mpscaiclr) + (*psc)));
			psc++;
			cb--;
			for (; cb >= 4; cb -= 4) {
				word *pwSrc = (word *)(((byte *)mpscaiclr) + (*psc));
				*((word *)pbDst) = *pwSrc++;
				pbDst += 2;
				*((word *)pbDst) = *pwSrc++;
				pbDst += 2;
				psc++;
			}
			for (; cb >= 2; cb -= 2) {
				*((word *)pbDst) = *((word *)(((byte *)mpscaiclr) + (*psc)));
				psc++;
				pbDst += 2;
			}
			if (cb != 0) {
				*pbDst++ = *((byte *)(((byte *)mpscaiclr) + (*psc)));
				psc++;
			}
			continue;
		}
		if (op >= kopShadow1 && op <= kopShadow24) {
			int cb = op - kopShadow1 + 1;
			while (cb-- != 0) {
				*pbDst = mpiclriclrShadow[*pbDst];
				pbDst++;
			}
			continue;
		}
		if (op >= kopTransparent1 && op <= kopTransparent32) {
			int cb = op - kopTransparent1 + 1;
			pbDst += cb;
			continue;
		}
		if (op >= kopNextScan0 && op <= kopNextScan48) {
			int cb = op - kopNextScan0;
			pbDst += cb + cbReturn;
			continue;
		}
		if (op == kopEnd)
			return;
	}
#endif
}
