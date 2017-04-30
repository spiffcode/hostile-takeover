#include "ht.h"

namespace wi {

#if 1
void TestTBitmap(DibBitmap *pbm) secCode8;

void TestTBitmap(DibBitmap *pbm)
{
}
#else
extern "C" void BltTest68K(byte *pbSrc, byte *pbDst);
extern "C" void TBitmapTest(byte *pbDst, byte *aiclrSide, byte *aiclrShadow) secCode4;
extern "C" void FillEvenEven8(byte *pbRow, int cx, int cy, int cxDib, byte bFill);
extern "C" void CopyBits48Wide(byte *pbSrc, int cbSrcRow, byte *pbDst, int cbDstRow, int cy) secCode8;

void DoTimeTest(DibBitmap *pbm)
{
	long c = 5000;
	long lStart;
	long lEnd;
	long n;

	TBitmap *ptbmCode = (TBitmap *)CreateTBitmap("srtest.png");
	//ptbmCode->CompileTest(c);
	//ptbmCode->DrawTest(pbm, c);
	delete ptbmCode;

	byte *pbDst = new byte[48 * 32];
	byte *pbSrc = pbm->GetBits();
	lStart = HostGetTickCount();
	for (n = 0; n < c; n++)
		CopyBits48Wide(pbSrc, 160 - 48, pbDst, 0, 32);
	lEnd = HostGetTickCount();
	HostMessageBox("%ld count %ld ticks", c, lEnd - lStart);

#if 0
	ptbm = ptbmCode;
	//ptbm = CreateTBitmap("srtest.png");
	ptbm->BltTo(pbm, -24, 10);
	lStart = HostGetTickCount();
	for (n = 0; n < c; n++)
		ptbm->BltTo(pbm, -24, 10);
	lEnd = HostGetTickCount();
	HostMessageBox("%ld count %ld ticks", c, lEnd - lStart);
	delete ptbm;

	ptbm = CreateTBitmap("srtest2.png");
	lStart = HostGetTickCount();
	for (n = 0; n < c; n++)
		ptbm->BltTo(pbm, 10, 10);
	lEnd = HostGetTickCount();
	HostMessageBox("%ld count %ld ticks", c, lEnd - lStart);
	delete ptbm;

	ptbm = CreateTBitmap("sri_jog_0_0.png");
	lStart = HostGetTickCount();
	for (n = 0; n < c; n++)
		ptbm->BltTo(pbm, 10, 10);
	lEnd = HostGetTickCount();
	HostMessageBox("%ld count %ld ticks", c, lEnd - lStart);
	delete ptbm;
#endif

#if 0
	byte *pbBits = pbm->GetBits();
	lStart = HostGetTickCount();
	for (n = 0; n < c; n++)
		TBitmapTest(pbBits, gaclrSideRed, gmpiclriclrShadow);
	lEnd = HostGetTickCount();
	HostMessageBox("%ld blts %ld ticks", c, lEnd - lStart);
#endif

#if 0
	FileMap fmap;
	BitmapHeader *pbmh = (BitmapHeader *)MapFile("sri_fire_5_1.png", &fmap);
	byte *pbDst = pbm->GetBits() + 160 * 15;
	lStart = HostGetTickCount();
	for (n = 0; n < c; n++)
		BltTest68K((byte *)(pbmh + 1), pbDst);
	lEnd = HostGetTickCount();
	HostMessageBox("%ld blts %ld ticks", c, lEnd - lStart);
	UnmapFile(&fmap);
#endif
}

TBitmap *ptbmTest;
int gxTest;
int gyTest;
int dxTest;
int gcxTest;
int gcDelay;
void TestTBitmap(DibBitmap *pbm)
{
	static bool sfTested;

#if 0
	if (sfTested)
		return;
	sfTested = true;
	DoTimeTest(pbm);
	return;
#endif

#if 1
	if (ptbmTest == NULL) {
		ptbmTest = CreateTBitmap("sri_stand_4_0.png");
		if (ptbmTest == NULL)
			return;
		gxTest = 11;
		gyTest= 10;
		dxTest = 1;
		Size siz;
		ptbmTest->GetSize(&siz);
		gcxTest = siz.cx;
	}
	Trace("BltTo %d,%d", gxTest, gyTest);
	ptbmTest->BltTo(pbm, gxTest, gyTest, 1);
#if 1
	gcDelay++;
	if (gcDelay < 1) // 20)
		return;
	gcDelay = 0;
	gxTest += dxTest;
	if (dxTest == 1 && gxTest == 0) {
		gyTest--;
		if (gyTest == -14)
			gyTest = 0;
		dxTest = -1;
	}
	if (dxTest == -1 && gxTest == -7) {
		gyTest--;
		if (gyTest == -14)
			gyTest = 0;
		dxTest = 1;
	}
#endif
#endif
}

#ifdef INCL_TESTS

#if 0
//Obviously doesn't compile at the moment

//temp
	if (HostGetCurrentKeyState(keyBitPageDown)) {
		long c = 5000;
		pfogm->RevealAll();
		long lStart = HostGetTickCount();
		for (long n = 0; n < c; n++)
			ptmap->Draw(pbm, xView, yView, pfogm->GetMapPtr());
		long lEnd = HostGetTickCount();
		HostMessageBox("%ld mapdraws %ld ticks", c, lEnd - lStart);
	}

#endif
#endif
#endif

} // namespace wi
