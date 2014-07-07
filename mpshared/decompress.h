#ifndef __DECOMPRESS_H__
#define __DECOMPRESS_H__

#include "inc/basictypes.h"

namespace wi {

struct CompressionHeader // coh
{
	word fCompressed;
	word cbUncompressed;
	word cbCompressed;
};
#define kcbCompressionHeader 6 // sizeof returns 8 on some platforms

extern "C" word DecompressChunk(byte **ppbCompressed, byte *pbDecompressed,
        byte *pbCacheEnd, word cbMax);
extern "C" void DecompressToMemory(byte *pbCompressed, byte *pbOut,
        CompressionHeader *pcoh);

} // namespace wi

#endif // __DECOMPRESS_H__
