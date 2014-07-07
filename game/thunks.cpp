#include "ht.h"

#ifndef __CPU_68K
#include "game/ops.h"
#endif

namespace wi {

#ifndef __CPU_68K
#include "miscgraphics.cpp"
#endif

void FillShadowThunk(byte *pbDst, int cbRowDst, int cx, int cy, byte *aclrMap)
{
#ifndef __CPU_68K
	Shadow(pbDst, cbRowDst, cx, cy, aclrMap);
#else
	if (gfArmPresent) {
		FillShadowArm(pbDst, cbRowDst, cx, cy, aclrMap);
	} else {
		FillShadow68K(pbDst, cbRowDst, cx, cy, aclrMap);
	}
#endif
}

void FillThunk(byte *pbDst, int cx, int cy, int cbStride, byte bFill)
{
#ifndef __CPU_68K
	Fill(pbDst, cx, cy, cbStride, bFill);
#else
	if (gfArmPresent) {
		FillArm(pbDst, cx, cy, cbStride, bFill);
	} else {
		Fill68K(pbDst, cx, cy, cbStride, bFill);
	}
#endif
}

void LeftToRightBltThunk(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy)
{
#ifndef __CPU_68K
	LeftToRightBlt(pbSrc, cxSrcStride, pbDst, cxDstStride, cx, cy);
#else
	if (gfArmPresent) {
		LeftToRightBltArm(pbSrc, cxSrcStride, pbDst, cxDstStride, cx, cy);
	} else {
		LeftToRightBlt68K(pbSrc, cxSrcStride, pbDst, cxDstStride, cx, cy);
	}
#endif
}

void RightToLeftBltThunk(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy)
{
#ifndef __CPU_68K
	RightToLeftBlt(pbSrc, cxSrcStride, pbDst, cxDstStride, cx, cy);
#else
	if (gfArmPresent) {
		RightToLeftBltArm(pbSrc, cxSrcStride, pbDst, cxDstStride, cx, cy);
	} else {
		RightToLeftBlt68K(pbSrc, cxSrcStride, pbDst, cxDstStride, cx, cy);
	}
#endif
}

void DrawTileMapThunk(byte **ppbMap, int ctx, int cty, byte *pbDst, int cbDstStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside, int cxTile, int cyTile)
{
#ifndef __CPU_68K
 	DrawTileMap(ppbMap, ctx, cty, pbDst, cbDstStride, cxLeftTile, cyTopTile, cxRightTile, cyBottomTile, ctxInside, ctyInside, cxTile, cyTile);
#else
	if (gfArmPresent) {
		DrawTileMapArm(ppbMap, ctx, cty, pbDst, cbDstStride, cxLeftTile, cyTopTile, cxRightTile, cyBottomTile, ctxInside, ctyInside, cxTile, cyTile);
	} else {
		switch (cxTile) {
		case 16:
			DrawTileMap816(ppbMap, ctx, cty, pbDst, cbDstStride, cxLeftTile, cyTopTile, cxRightTile, cyBottomTile, ctxInside, ctyInside);
			return;

		case 24:
			DrawTileMap824(ppbMap, ctx, cty, pbDst, cbDstStride, cxLeftTile, cyTopTile, cxRightTile, cyBottomTile, ctxInside, ctyInside);
			return;
		}
	}
#endif
}

word Compile8Thunk(byte *pb, ScanData *psd, bool fOdd)
{
#ifndef __CPU_68K
	return Compile8(pb, psd, fOdd);
#else
	if (gfArmPresent) {
		return Compile8Arm(pb, psd, fOdd);
	} else {
		return Compile868K(pb, psd, fOdd);
	}
#endif
}

void DrawDispatchThunk(byte *pb, byte *pbSrc, byte *pbDst, int cbReturn, dword *mpscaiclrSide, byte *mpiclriclrShadow)
{
#ifndef __CPU_68K
	DrawDispatch(pb, pbSrc, pbDst, cbReturn, mpscaiclrSide, mpiclriclrShadow);
#else
	if (gfArmPresent) {
		DrawDispatchArm(pb, pbSrc, pbDst, cbReturn, mpscaiclrSide, mpiclriclrShadow);
	} else {
		DrawDispatch68K(pb, pbSrc, pbDst, cbReturn, mpscaiclrSide, mpiclriclrShadow);
	}
#endif
}

} // namespace wi
