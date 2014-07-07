#include "game/iphone/layerdib.h"

namespace wi {
    
LayerDib *CreateLayerDib(int cx, int cy) {
    LayerDib *pbm = new LayerDib;
    if (pbm == NULL) {
        return NULL;
    }
    if (!pbm->Init(cx, cy)) {
        delete pbm;
        return NULL;
    }
    return pbm;
}

LayerDib::LayerDib() {
    palette_ = NULL;
}

LayerDib::~LayerDib() {
    if (palette_ != NULL) {
        free(palette_);
    }
}

bool LayerDib::Init(int cx, int cy) {
    m_pb = NULL;
    m_cbRow = 0;
    m_siz.cx = cx;
    m_siz.cy = cy;
    m_wf |= kfDibWantScrolls;
    return true;
}

bool LayerDib::InitLayerMap(int ilmap, const Rect *prc, const Size *psizMap) {
    switch (ilmap) {
    case 0:
        return lmap0_.Init(prc, psizMap);
    case 1:
        return lmap1_.Init(prc, psizMap);
    }
    return false;
}

void LayerDib::MarkUpdate() {
    lmap0_.MarkUpdateAll();
    lmap1_.MarkUpdateAll();
}
    
void LayerDib::SetPalette(void *palette, int cb) {
    if (palette_ != NULL)
        free(palette_);
    palette_ = malloc(cb);
    memcpy((byte *)palette_, (byte *)palette, cb);
}

void LayerDib::BltTiles(DibBitmap *pbmSrc, UpdateMap *pupd, int yTopDst) {
    if (palette_ == NULL) {
        return;
    }

#ifdef UPDATE_ALL
    changed_ = true;
#endif

    // Which layer?
    if (yTopDst == lmap0_.GetTop()) {
        if (lmap0_.UpdateTiles(pbmSrc, pupd, palette_)) {
            changed_ = true;
        }
        return;
    }
    if (yTopDst == lmap1_.GetTop()) {
        if (lmap1_.UpdateTiles(pbmSrc, pupd, palette_)) {
            changed_ = true;
        }
        return;
    }
}

void LayerDib::Scroll(Rect *prcSrc, int xDst, int yDst) {
    int dx = xDst - prcSrc->left;
    int dy = yDst - prcSrc->top;
    if (dx != 0 || dy != 0) {
        lmap0_.Scroll(dx, dy);
        changed_ = true;
    }
}

void LayerDib::ResetScrollOffset() {
    lmap0_.ResetScrollOffset();
    changed_ = true;
}

void LayerDib::Draw(CGContextRef ctx, const CGRect& rcInvalid) {
    lmap0_.Draw(ctx, m_siz.cy, rcInvalid);
    lmap1_.Draw(ctx, m_siz.cy, rcInvalid);
}
    
} // namespace wi
