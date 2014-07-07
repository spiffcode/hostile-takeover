// Bitmap Cruncher. Reads in windows bitmap, outputs ht bitmap
//
// Inputs:
// - Bitmap in .bmp format
// - Palette in Jasc format
// - Transparent information
// - HT output depth
//
// Outputs:
// - HT bitmap in appropriate depth

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

#define BigWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))

byte gargbPal[256][3];
int gcPalEntries;
int BMP_RGBToColorIndex(int r, int g, int b, byte palette[][3], int paletteSize);

void Usage()
{
	printf("\n");
	printf("Usage: bcrunch <-s> <-tcorner | -tvalue value> [-p palette] [-d depth] [-o output] input\n");
	printf("-s: show image in top left corner of screen\n");
	printf("-tcorner: use top, left corner of image for transparent color.\n");
	printf("-tvalue: specify transparent pixel value.\n");
	printf("-trgb: specify transparent rgb value.\n");
	printf("-p: specify paintshop pro compatible palette to match output against.\n");
	printf("-d: specify output bit depth\n");
	printf("-o: specify ouput filename\n");
	printf("\n");
	printf("Note if no transparency option specified, -trgb 255 0 255 assumed.\n");
	printf("\n");
	exit(1);
}

// Read in jasc format palette file (paintshop pro)

bool ReadPalette(char *pszFn)
{
	FILE *pf = fopen(pszFn, "r");

	if (pf == NULL)
		return false;

	// Scan for "JASC-PAL"

	char szT[128];
	int c;
	c = fscanf(pf, "%s\n", szT);
	if (c != 1)
		return false;
	if (strcmp(szT, "JASC-PAL") != 0)
		return false;

	// Scan for version info (we support "0100")

	c = fscanf(pf, "%s\n", szT);
	if (c != 1)
		return false;
	if (strcmp(szT, "0100") != 0)
		return false;

	// Scan for size of palette

	c = fscanf(pf, "%d\n", &gcPalEntries);
	if (c != 1)
		return false;
	switch (gcPalEntries) {
	case 2:
	case 4:
	case 16:
	case 256:
		break;

	default:
		return false;
	}

	// Read in the palette info

	for (int n = 0; n < gcPalEntries; n++) {
		int r;
		int g;
		int b;
		c = fscanf(pf, "%d %d %d\n", &r, &g, &b);
		if (c != 3)
			return false;
		gargbPal[n][0] = (byte)r;
		gargbPal[n][1] = (byte)g;
		gargbPal[n][2] = (byte)b;
	}
	fclose(pf);
	return true;
}

dword GetPixelValue(BITMAPINFO *pbmi, byte *pb, int x, int y)
{
	int cxImage = pbmi->bmiHeader.biWidth;
	int cyImage = abs(pbmi->bmiHeader.biHeight);

	if (pbmi->bmiHeader.biHeight >= 0)
		y = (pbmi->bmiHeader.biHeight - 1) - y;

	byte *pbT;
	int cbRow;
	byte n;

	switch (pbmi->bmiHeader.biBitCount) {
	case 1:
		cbRow = (cxImage / 8 + 3) & ~3;
		pbT = pb + y * cbRow + x / 8;
		n = (*pbT >> (7 - (x & 7))) & 1;
		return (dword)n;

	case 2:
		cbRow = (cxImage / 4 + 3) & ~3;
		pbT = pb + y * cbRow + x / 4;
		n = (*pbT >> (7 - (x & 3) * 2)) & 3;
		return (dword)n;

	case 4:
		cbRow = (cxImage / 2 + 3) & ~3;
		pbT = pb + y * cbRow + x / 2;
		n = (*pbT >> (7 - (x & 1) * 4)) & 0xf;
		return (dword)n;

	case 8:
		cbRow = (cxImage + 3) & ~3;
		pbT = pb + y * cbRow + x;
		n = *pbT;
		return (dword)n;

	case 16:
		cbRow = (cxImage * 2 + 3) & ~3;
		pbT = pb + y * cbRow + x * 2;
		return (dword)(*(word *)pbT);

	case 24:
		cbRow = (cxImage * 3 + 3) & ~3;
		pbT = pb + y * cbRow + x * 3;
		return (dword)((pbT[2] << 16) | (pbT[1] << 8) | (pbT[0]));

	default:
		return 0;
	}
}

void ConvertPixel2RGB(BITMAPINFO *pbmi, dword dw, byte rgb[3])
{
	RGBQUAD *prgbq = pbmi->bmiColors;
	rgb[0] = 0;
	rgb[1] = 0;
	rgb[2] = 0;

	switch (pbmi->bmiHeader.biBitCount) {
	case 1:
	case 2:
	case 4:
	case 8:
		rgb[0] = prgbq[dw].rgbRed;
		rgb[1] = prgbq[dw].rgbGreen;
		rgb[2] = prgbq[dw].rgbBlue;
		break;

	case 16:
		{
			// Must be BI_RGB or BI_BITFIELDS

			if (!(pbmi->bmiHeader.biCompression & (BI_RGB | BI_BITFIELDS))) {
				rgb[0] = 0;
				rgb[1] = 0;
				rgb[2] = 0;
				break;
			}

			// BI_RGB which is 5,5,5 in this order

			word aw[3];
			aw[0] = 0x7c00;
			aw[1] = 0x3e0;
			aw[2] = 0x1f;

			// Most 16bpp images are BI_BITFIELDS

			if (pbmi->bmiHeader.biCompression & BI_BITFIELDS) {
				aw[0] = *(word *)&prgbq[0];
				aw[1] = *(word *)&prgbq[1];
				aw[2] = *(word *)&prgbq[2];
			}

			for (int n = 0; n < 3; n++) {
				word w = aw[n];
				int c = 0;
				while ((w & 1) == 0) {
					w >>= 1;
					c++;
				}
				word wT = (((word)dw & aw[n]) >> c) & w;
				rgb[n] = (byte)((float)wT / (float)w * 255.0f + 0.5f);
			}
		}
		break;

	case 24:
		{
			byte *pbT = (byte *)&dw;
			rgb[0] = pbT[2];
			rgb[1] = pbT[1];
			rgb[2] = pbT[0];
		}
		break;
	}
}

dword ConvertRGB2Pixel(BITMAPINFO *pbmi, byte rgb[3])
{
	switch (pbmi->bmiHeader.biBitCount) {
	case 1:
	case 2:
	case 4:
	case 8:
		{
			byte rgbPal[256][3];
			for (int n = 0; n < (int)pbmi->bmiHeader.biClrUsed; n++) {
				rgbPal[n][0] = pbmi->bmiColors[n].rgbRed;
				rgbPal[n][1] = pbmi->bmiColors[n].rgbGreen;
				rgbPal[n][2] = pbmi->bmiColors[n].rgbBlue;
			}
			return (dword)BMP_RGBToColorIndex(rgb[0], rgb[1], rgb[2], rgbPal, pbmi->bmiHeader.biClrUsed);
		}

	case 16:
		{
			// r5g6b5

			word w = 0;
			w |= ((word)(((float)rgb[0] / 255.0f * 32.0f) + 0.5f) & 0x1f) << 11;
			w |= ((word)(((float)rgb[1] / 255.0f * 64.0f) + 0.5f) & 0x3f) << 5;
			w |= ((word)(((float)rgb[2] / 255.0f * 32.0f) + 0.5f) & 0x1f) << 0;
			return (dword)w;
		}
		break;

	case 24:
		{
			dword dw = 0;
			byte *pbT = (byte *)&dw;
			pbT[2] = rgb[0];
			pbT[1] = rgb[1];
			pbT[0] = rgb[2];
			return dw;
		}
	}
	return 0;
}

BITMAPINFO *ReadBitmap(char *pszFilename, byte **ppb)
{
	// Open the alleged bitmap file.

	FILE *pfil = fopen(pszFilename, "rb");

	// NOTE: Contrary to what the documentation says, fopen() returns NULL
	// when it can't find the file, not -1.

	if (pfil == (FILE *)-1 || pfil == NULL) {
		printf("Couldn't open the bitmap file!\n");
		return NULL;
	}
	
	// Get the bitmap header.

	BITMAPFILEHEADER bfh;
	if (fread(&bfh, sizeof(bfh), 1, pfil) != 1) {
		printf("Couldn't read the bitmap file header!\n");
		fclose(pfil);
		return NULL;
	}

	// Is it really a bitmap?

	if (bfh.bfType != ('B' | ((word)'M' << 8))) {
		printf("Not a valid Windows bitmap file!\n");
		fclose(pfil);
		return NULL;
	}

	// Read the whole header, color table, and bitmap bits into memory.

	long lPosBmi = ftell(pfil);
	fseek(pfil, 0, SEEK_END);
	long lPosEnd = ftell(pfil);
	fseek(pfil, lPosBmi, SEEK_SET);

	BITMAPINFO *pbmiSrc = (BITMAPINFO *)new char[lPosEnd - lPosBmi];
	if (pbmiSrc == NULL) {
		printf("Couldn't allocate bitmap read buffer!\n");
		fclose(pfil);
		return NULL;
	}

	if (fread(pbmiSrc, lPosEnd - lPosBmi, 1, pfil) != 1) {
		printf("Couldn't read bitmap header, colors, bits!\n");
		delete pbmiSrc;
		fclose(pfil);
		return NULL;
	}

	fclose(pfil);

	*ppb = ((byte *)pbmiSrc) + bfh.bfOffBits - sizeof(BITMAPFILEHEADER);
	return pbmiSrc;
}

// From pilrc. Easy to write from scratch, but I knew this already existed

int BMP_RGBToColorIndex(int r, 
                    int g, 
                    int b,
                    byte palette[][3],
                    int paletteSize)
{
  int index, lowValue, i, *diffArray;

  // generate the color "differences" for all colors in the palette
  diffArray = (int *)malloc(paletteSize * sizeof(int));
  for (i=0; i < paletteSize; i++) {
    diffArray[i] = ((palette[i][0]-r)*(palette[i][0]-r)) +
                   ((palette[i][1]-g)*(palette[i][1]-g)) +
                   ((palette[i][2]-b)*(palette[i][2]-b));
  }

  // find the palette index that has the smallest color "difference"
  index    = 0;
  lowValue = diffArray[0];
  for (i=1; i<paletteSize; i++) {
	if (diffArray[i] < lowValue) {
	  lowValue = diffArray[i];
	  index    = i;
	}
  }

  // clean up
  free(diffArray);

  return index;
}

// Something simple for the moment

struct Bitmap { // bm
	word cx;
	word cy;
	word wTrans;
};

byte *PrepareImage8bpp(BITMAPINFO *pbmi, byte *pbBits, dword dwTransparent, dword *pcb)
{
	// Find an unused color index that we can specify as the transparent color index

	int x, y;

	int cxImage = pbmi->bmiHeader.biWidth;
	int cyImage = abs(pbmi->bmiHeader.biHeight);
	int acUsed[256];
	memset(acUsed, 0, sizeof(acUsed));
	for (x = 0; x < cxImage; x++) {
		for (y = 0; y < cyImage; y++) {
			dword dw = GetPixelValue(pbmi, pbBits, x, y);
			if (dw == dwTransparent)
				continue;
			byte rgb[3];
			ConvertPixel2RGB(pbmi, dw, rgb);
			byte b = BMP_RGBToColorIndex(rgb[0], rgb[1], rgb[2], gargbPal, gcPalEntries);
			acUsed[b]++;
		}
	}
	
	// First see if the transparent color we asked for mapped to an index that isn't used.
	// Otherwise look for an unused color index

	byte rgbT[3];
	ConvertPixel2RGB(pbmi, dwTransparent, rgbT);
	int nT = BMP_RGBToColorIndex(rgbT[0], rgbT[1], rgbT[2], gargbPal, gcPalEntries);
	int nUnused = -1;
	if (acUsed[nT] == 0) {
		nUnused = nT;
	} else {
		for (int n = 0; n < 256; n++) {
			if (acUsed[n] == 0) {
				nUnused = n;
				break;
			}
		}
	}

	// If we couldn't find an unused color index we have a mapping error

	if (nUnused == -1) {
		printf("All colors in the output palette are in used by this image therefore\n");
		printf("no transparent color can be specified.\n");
		exit(1);
	}

	// Init our new image format

	byte bTrans = (byte)nUnused;
	word cbRow = (cxImage + 1) & ~1;
	word cbImage = cbRow * cyImage;
	*pcb = sizeof(Bitmap) + cbImage;
	byte *pb = new byte[*pcb];
	memset(pb, bTrans, *pcb);
	Bitmap *pbm = (Bitmap *)pb;
	pbm->cx = BigWord(cxImage);
	pbm->cy = BigWord(cyImage);
	pbm->wTrans = bTrans | (bTrans << 8);

	// Palette match and stuff new image

	byte *pbT = (byte *)(pbm + 1);
	for (x = 0; x < cxImage; x++) {
		for (y = 0; y < cyImage; y++) {
			dword dw = GetPixelValue(pbmi, pbBits, x, y);
			byte b;
			if (dw == dwTransparent) {
				b = bTrans;
			} else {
				byte rgb[3];
				ConvertPixel2RGB(pbmi, dw, rgb);
				b = BMP_RGBToColorIndex(rgb[0], rgb[1], rgb[2], gargbPal, gcPalEntries);
			}
			pbT[y * cbRow + x] = b;
		}
	}

	return pb;
}

byte *PrepareImage8bppGrayscale(BITMAPINFO *pbmi, byte *pbBits, dword dwTransparent, dword *pcb)
{
	int cxImage = pbmi->bmiHeader.biWidth;
	int cyImage = abs(pbmi->bmiHeader.biHeight);
	word cbRow = (cxImage + 1) & ~1;
	word cbImage = cbRow * cyImage;

	*pcb = sizeof(Bitmap) + cbImage;
	byte *pb = new byte[*pcb];
	memset(pb, 0, *pcb);
	Bitmap *pbm = (Bitmap *)pb;
	pbm->cx = BigWord(cxImage);
	pbm->cy = BigWord(cyImage);
	pbm->wTrans = 0xffff;

	// Palette match and stuff new image

	byte *pbT = (byte *)(pbm + 1);
	for (int x = 0; x < cxImage; x++) {
		for (int y = 0; y < cyImage; y++) {
			dword dw = GetPixelValue(pbmi, pbBits, x, y);
			byte b = 0xff;
			if (dw != dwTransparent) {
				byte rgb[3];
				ConvertPixel2RGB(pbmi, dw, rgb);
				b = BMP_RGBToColorIndex(rgb[0], rgb[1], rgb[2], gargbPal, gcPalEntries);
			}
			pbT[y * cbRow + x] = b;
		}
	}

	return pb;
}

int main(int cArgs, char **ppszArgs)
{
	if (cArgs == 1)
		Usage();

	// Skip the filename

	cArgs--;
	ppszArgs++;

	// Process parameters

	bool fShowImage = false;
	bool fCornerTransparent = false;
	bool fTransparentRGB = false;
	byte rgbTrans[3];
	dword dwTransparent = 0xff000000;
	byte *pbBits;
	BITMAPINFO *pbmi;
	int cBppOutput = 0;
	char szFnOutput[MAX_PATH];
	char szFnInput[MAX_PATH];

	szFnOutput[0] = 0;
	while (cArgs > 0) {
		// Grab the next parameter

		char *psz = *ppszArgs++;
		cArgs--;

		// -p %s (palette filename)

		if (strcmp(psz, "-p") == 0) {
			if (cArgs < 1) {
				printf("Error parsing -p.\n");
				Usage();
			}
			char *pszFn = *ppszArgs++;
			cArgs--;
			if (!ReadPalette(pszFn)) {
				printf("Error reading palette %s.\n", pszFn);
				Usage();
			}
			continue;
		}

		// -?

		if (strcmp(psz, "-?") == 0)
			Usage();

		// -tcorner (transparent pixel at 0,0)

		if (strcmp(psz, "-tcorner") == 0) {
			fCornerTransparent = true;
			continue;
		}

		// -tvalue %x (transparent pixel value in hex before conversion)

		if (strcmp(psz, "-tvalue") == 0) {
			if (cArgs < 1) {
				printf("Error parsing -tvalue.\n");
				Usage();
			}
			psz = *ppszArgs++;
			cArgs--;
			int c = sscanf(psz, "%x", &dwTransparent);
			if (c != 1) {
				printf("Error parsing -tvalue.\n");
				Usage();
			}
			continue;
		}

		// -trgb %d %d %d (transparent rgb)

		if (strcmp(psz, "-trgb") == 0) {
			if (cArgs < 3) {
				printf("Error parsing -trgb.\n");
				Usage();
			}
			rgbTrans[0] = (byte)atoi(*ppszArgs++);
			cArgs--;
			rgbTrans[1] = (byte)atoi(*ppszArgs++);
			cArgs--;
			rgbTrans[2] = (byte)atoi(*ppszArgs++);
			cArgs--;
			fTransparentRGB = true;
			continue;
		}

		// -d %d (output depth, 2, 4, 8, 16 supported)

		if (strcmp(psz, "-d") == 0) {
			if (cArgs < 1) {
				printf("Error parsing -d.\n");
				Usage();
			}
			psz = *ppszArgs++;
			cArgs--;
			int c = sscanf(psz, "%d", &cBppOutput);
			if (c != 1) {
				printf("Error parsing -d.\n");
				Usage();
			}
			switch (cBppOutput) {
			case 2:
			case 4:
			case 8:
			case 16:
				break;

			default:
				printf("Legal depths are 2, 4, 8 or 16.\n");
				Usage();
			}
			continue;
		}

		// -s (show image in top left)

		if (strcmp(psz, "-s") == 0) {
			fShowImage = true;
			continue;
		}

		// -o %s (output filename)

		if (strcmp(psz, "-o") == 0) {
			if (cArgs < 1) {
				printf("Error parsing -o.\n");
				Usage();
			}
			psz = *ppszArgs++;
			cArgs--;
			int c = sscanf(psz, "%s", szFnOutput);
			if (c != 1) {
				printf("Error parsing -o.\n");
				Usage();
			}
			continue;
		}

		// None of the above, it must be the input filename

		int c = sscanf(psz, "%s", szFnInput);
		if (c != 1) {
			printf("Error opening input filename %s.\n", psz);
			Usage();
		}

		// Load in and convert to rgb

		pbmi = ReadBitmap(szFnInput, &pbBits);
		if (pbmi == NULL) {
			printf("Error opening input filename %s.\n", psz);
			Usage();
		}

		if (fCornerTransparent) {
			// -tcorner specified. Get the transparent pixel value from the top left corner.

			dwTransparent = GetPixelValue(pbmi, pbBits, 0, 0);
			ConvertPixel2RGB(pbmi, dwTransparent, rgbTrans);
		} else if (fTransparentRGB) {
			// -trgb specified. Get the transparent color from this rgb
	
			dwTransparent = ConvertRGB2Pixel(pbmi, rgbTrans);
		} else {
			// Perhaps no transparent color was specified. If so,
			// assume 255 0 255

			if (dwTransparent == 0xff000000) {
				rgbTrans[0] = 255;
				rgbTrans[1] = 0;
				rgbTrans[2] = 255;
				dwTransparent = ConvertRGB2Pixel(pbmi, rgbTrans);
			} else {
				// -tvalue specified. Get the rgb for this transparent pixel value

				ConvertPixel2RGB(pbmi, dwTransparent, rgbTrans);
			}
		}

		// If the input image is paletted, ensure the incoming palette has
		// the transparent color in it rather than it being a fuzzy match
		// which can lead to mapping errors.

		switch (pbmi->bmiHeader.biBitCount) {
		case 1:
		case 2:
		case 4:
		case 8:
			{
				RGBQUAD *prgbq = &pbmi->bmiColors[dwTransparent];
				if (prgbq->rgbRed != rgbTrans[0] || prgbq->rgbGreen != rgbTrans[1] || prgbq->rgbBlue != rgbTrans[2]) {
					printf("%s warning: transparent color specified is not in the\n", szFnInput);
					printf("image's palette. This could lead to transparent color mapping problems.\n");
				}
			}
		}
	}

	// Show the image if asked

	int cxImage = pbmi->bmiHeader.biWidth;
	int cyImage = abs(pbmi->bmiHeader.biHeight);
	if (fShowImage) {
		HDC hdc = GetDC(NULL);
		for (int x = 0; x < cxImage; x++) {
			for (int y = 0; y < cyImage; y++) {
				dword dw = GetPixelValue(pbmi, pbBits, x, y);
				byte rgb[3];
				ConvertPixel2RGB(pbmi, dw, rgb);
				if (dw != dwTransparent) {
					SetPixel(hdc, x, y, RGB(rgb[0], rgb[1], rgb[2]));
				}
			}
		}
		ReleaseDC(NULL, hdc);
	}

	dword cb;
	byte *pbNew = NULL;
	switch (cBppOutput) {
	case 1:
	case 2:
	case 4:
	case 16:
	default:
		printf("Output depth %d not supported.\n", cBppOutput);
		return 1;

	case 8:
		if (gcPalEntries <= 16) {
			pbNew = PrepareImage8bppGrayscale(pbmi, pbBits, dwTransparent, &cb);
		} else {
			pbNew = PrepareImage8bpp(pbmi, pbBits, dwTransparent, &cb);
		}
		break;
	}
	if (pbNew == NULL) {
		printf("Error processing bitmap file.\n");
		Usage();
	}

	// Save it out

	FILE *pf = fopen(szFnOutput, "wb");
	if (pf == NULL) {
		printf("Could not create output file %s.\n");
		return 1;
	}
	if (fwrite(pbNew, cb, 1, pf) != 1) {
		fclose(pf);
		DeleteFile(szFnOutput);
		printf("Could not create output file %s.\n");
		Usage();
	}
	fclose(pf);
	delete pbNew;

	byte rgb[3];
	ConvertPixel2RGB(pbmi, dwTransparent, rgb);
	printf("%s(%d) -> %s(%d), %dx%d, trans:%d,%d,%d, %d bytes\n", szFnInput, pbmi->bmiHeader.biBitCount, szFnOutput, cBppOutput, cxImage, cyImage, rgb[0], rgb[1], rgb[2], cb);
	delete pbmi;
	return 0;
}