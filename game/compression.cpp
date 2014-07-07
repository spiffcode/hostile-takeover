#include "ht.h"
#include "mpshared/decompress.h"

namespace wi {

void DecompressToCache(byte *pbCompressed, CacheHandle hc, CompressionHeader *pcoh)
{
	// Writing to the cache requires use of gcam.Write()

	byte *pbCache = (byte *)gcam.GetPtr(hc);
	byte *pbCompressedT = pbCompressed;
	word cbDecompressed = 0;
	word cb;
	while ((cb = DecompressChunk(&pbCompressedT, gpbScratch, pbCache + cbDecompressed, gcbScratch / 2)) != 0) {
		gcam.Write(hc, cbDecompressed, gpbScratch, cb);
		cbDecompressed += cb;
		if (cbDecompressed == BigWord(pcoh->cbUncompressed))
			break;
		Assert(cbDecompressed <= BigWord(pcoh->cbUncompressed));
		Assert(pbCompressedT <= pbCompressed + BigWord(pcoh->cbCompressed));
	}
}

} // namespace wi
