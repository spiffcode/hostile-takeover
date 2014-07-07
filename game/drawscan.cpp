#include "game/ht.h"
#include "game/ops.h"

namespace wi {

int YClipToScan(int yClip, int cx, byte*& pop, byte*& pbSrc, word*& psc) {
    if (yClip == 0) {
        return 0;
    }
    int offset = 0;
    int y = 0;
    while (true) {
		int op = (int)(word)*pop++;
		if (op == kopAlign) {
			pbSrc++;
			continue;
		}
		if (op >= kopEvenData1 && op <= kopEvenData48) {
			int cb = op - kopEvenData1 + 1;
            offset += cb;
            pbSrc += cb;
			continue;
		}
		if (op >= kopOddData1 && op <= kopOddData48) {
			int cb = op - kopOddData1 + 1;
            offset += cb;
            pbSrc += cb;
			continue;
		}
		if (op >= kopEvenSide1 && op <= kopEvenSide16) {
			int cb = op - kopEvenSide1 + 1;
            offset += cb;
            psc += (cb >> 2);
            psc += ((cb & 2) >> 1);
            psc += (cb & 1);
			continue;
		}
		if (op >= kopOddSide1 && op <= kopOddSide16) {
			int cb = op - kopOddSide1 + 1;
            offset += cb;
            psc++;
            cb--;
            psc += (cb >> 2);
            psc += ((cb & 2) >> 1);
            psc += (cb & 1);
			continue;
        }
		if (op >= kopShadow1 && op <= kopShadow24) {
			offset += (op - kopShadow1 + 1);
			continue;
		}
		if (op >= kopTransparent1 && op <= kopTransparent32) {
			offset += (op - kopTransparent1 + 1);
			continue;
		}
		if (op >= kopNextScan0 && op <= kopNextScan48) {
			offset += ((op - kopNextScan0) - cx);
            y++;
            if (y == yClip) {
                break;
            }
			continue;
		}
		if (op == kopEnd) {
			break;
        }
	}
    return offset;
}

int DrawScan(byte *pbDst, int cx, byte*& pbSrc, byte*& pop, word*& psc,
        dword *mpscaiclr, byte *mpiclriclrShadow) {

    byte *pbDstStart = pbDst;
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
			return &pbDst[op - kopNextScan0] - pbDstStart;
		}
		if (op == kopEnd)
			return 0;
	}
}

} // namespace wi
