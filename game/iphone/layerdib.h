#ifndef __LAYERDIB_H__
#define __LAYERDIB_H__

#include "game/ht.h"
#include "game/iphone/layermap.h"
#include <CoreGraphics/CGContext.h>

namespace wi {
    
class LayerDib : public DibBitmap
{
public:
    LayerDib();
	~LayerDib();
    
    virtual bool Init(int cx, int cy);
    virtual void SetPalette(void *palette, int cb);
	virtual void BltTiles(DibBitmap *pbmSrc, UpdateMap *pupd, int yTopDst);
    virtual void Scroll(Rect *prcSrc, int xDst, int yDst);

    bool InitLayerMap(int ilmap, const Rect *prc, const Size *psizMap);
    void MarkUpdate();
    void ResetScrollOffset();
    void Draw(CGContextRef ctx, const CGRect& rcInvalid);
    bool HasChanged() { return changed_; }
    void ResetChanged() { changed_ = false; }
    
protected:
    bool changed_;
    void *palette_;
    LayerMap lmap0_;
    LayerMap lmap1_;
};
LayerDib *CreateLayerDib(int cx, int cy);

} // namespace wi

#endif // __LAYERDIB_H__
