#include "mpshared/decompress.h"
#include "mpshared/misc.h"
#include "inc/rip.h"
#include <string.h>

namespace wi {

word DecompressChunk(byte **ppbCompressed, byte *pbDecompressed,
        byte *pbCacheEnd, word cbMax)
{
	byte *pbSrc = *ppbCompressed;
	byte *pbDst = pbDecompressed;

    bool fChunk = true;
    if (pbCacheEnd == NULL) {
        pbCacheEnd = pbDecompressed;
        fChunk = false;
    }

	bool fDone = false;
	while (!fDone) {
        if (fChunk) {
            // If we're almost to the max size, break out (max copy size is 129)

            if (pbDst - pbDecompressed > cbMax - (8 * 129))
                break;
        }

		// Grab the next flag byte

		byte bT = *pbSrc++;
		for (int ibit = 7; ibit >= 0; ibit--) {
			// Literal or code?

			if ((bT & (1 << ibit)) != 0) {
				*pbDst++ = *pbSrc++;
				continue;
			}

			// Not a literal. Get code.

			word code = ((word)(*pbSrc++)) << 8;
			code |= *pbSrc++;

			// Extended code? Check for count == 0

			word offBackwards = (code & 0x1fff);
			word cb;
			if (code & 0xe000) {
				// Count != 0, so not an extended code

				cb = ((code & 0xe000) >> 13) + 1;
			} else {
				// Extended code. End?

				if (offBackwards == 0) {
					fDone = true;
					break;
				}

				// Extended match

				offBackwards = (code << 1) | (*pbSrc >> 7);
				cb = (*pbSrc++ & 0x7f) + 2;
			}

			// Copy this chunk into the output

			int offStart = (pbDst - pbDecompressed) - offBackwards;
			if (offStart >= 0) {
				// Block is in local memory
				memcpy(pbDst, pbDecompressed + offStart, cb);
				pbDst += cb;
			} else {
				// Block is in cache memory or half in cache, half local

				if ((int)cb + offStart <= 0) {
					// Block is totally in cache memory

					memcpy(pbDst, pbCacheEnd + offStart, cb);
					pbDst += cb;
				} else {
					// Block is partly in both

					memcpy(pbDst, pbCacheEnd + offStart, -offStart);
					pbDst += -offStart;
					memcpy(pbDst, pbDecompressed, (int)cb + offStart);
					pbDst += (int)cb + offStart;
				}
			}
		}
	}

	// Pass back the current point in the compressed source and the # bytes decompressed

	*ppbCompressed = pbSrc;
	return pbDst - pbDecompressed;
}

void DecompressToMemory(byte *pbCompressed, byte *pbOut,
        CompressionHeader *pcoh)
{
	byte *pbCompressedT = pbCompressed;
    DecompressChunk(&pbCompressedT, pbOut, NULL, 0);
}

} // namespace wi
