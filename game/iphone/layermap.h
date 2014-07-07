#ifndef __LAYERMAP_H__
#define __LAYERMAP_H__

#include "game/ht.h"
#include "game/iphone/layertile.h"
#include <CoreGraphics/CGContext.h>

namespace wi {

class LayerMap {
public:
    LayerMap();
    ~LayerMap();

    bool Init(const Rect *prc, const Size *psizMap);
    bool UpdateTiles(DibBitmap *pbmSrc, UpdateMap *pupd, void *palette);
    void Draw(CGContextRef ctx, int cy, const CGRect& rcInvalid);
    void Scroll(int dx, int dy);
    void ResetScrollOffset();
    int GetTop() { return rc_.top; }
    void MarkUpdateAll();

private:
    void UpdateTile(LayerTile *pltile, DibBitmap *pbmSrc, Rect *prcSrc,
            void *palette);
    void LRTBExchange(TRect *ptrc, int txDst, int tyDst);
    void RLBTExchange(TRect *ptrc, int txDst, int tyDst);

    CGContextRef bmCtx_;
    dword *pdwTile_;
    LayerTile **apltile_;
    int ctx_, cty_;
    int xOffset_, yOffset_;
    Rect rc_;
};

} // namespace wi

#endif // __LAYERMAP_H__
