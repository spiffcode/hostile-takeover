#include "game/iphone/layermap.h"
#include <CoreGraphics/CGContext.h>
#include <CoreGraphics/CGBitmapContext.h>
#include <CoreGraphics/CGLayer.h>
#include <Foundation/Foundation.h>

namespace wi {

LayerMap::LayerMap() {
    apltile_ = NULL;
    ctx_ = 0;
    cty_ = 0;
    xOffset_ = 0;
    yOffset_ = 0;
    memset(&rc_, 0, sizeof(rc_));
    pdwTile_ = NULL;
    bmCtx_ = NULL;
}

LayerMap::~LayerMap() {
    if (apltile_ != NULL) {
        int cltiles = ctx_ * cty_;
        for (int i = 0; i < cltiles; i++) {
            delete apltile_[i];
        }
        delete apltile_;
    }
    delete pdwTile_;
    if (bmCtx_ != NULL) {
        CGContextRelease(bmCtx_);
    }
}

bool LayerMap::Init(const Rect *prc, const Size *psizMap) {
    rc_ = *prc;
    ctx_ = psizMap->cx;
    cty_ = psizMap->cy;

    // Alloc the BitmapContext
    pdwTile_ = new dword[gcxTile * gcyTile];
    if (pdwTile_ == NULL) {
        return false;
    }
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    bmCtx_ = CGBitmapContextCreate(pdwTile_, gcxTile, gcyTile,
            8, gcxTile * 4, colorSpace,
            kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrderDefault);
    CGColorSpaceRelease(colorSpace);
    if (bmCtx_ == NULL) {
        return false;
    }

    // Alloc the layertile map
    apltile_ = new LayerTile *[ctx_ * cty_];
    if (apltile_ == NULL) {
        return false;
    }
    memset(apltile_, 0, sizeof(apltile_[0]) * ctx_ * cty_);

    // Allocate layer objects
    LayerTile **ppltile = apltile_;
    for (int ty = 0; ty < cty_; ty++) {
        for (int tx = 0; tx < ctx_; tx++) {
            LayerTile *pltile = new LayerTile(bmCtx_);
            if (pltile == NULL) {
                return false;
            }
            *ppltile++ = pltile;
        }
    }
    return true;
}

bool LayerMap::UpdateTiles(DibBitmap *pbmSrc, UpdateMap *pupd, void *palette) {
    Size sizMap;
    pupd->GetMapSize(&sizMap);
    if (sizMap.cx != ctx_ || sizMap.cy != cty_) {
        return false;
    }

    // Update layer tiles from pbmSrc.

	bool *pfInvalid = pupd->GetInvalidMap();
    LayerTile **ppltile = apltile_;

    bool changed = false;
    for (int ty = 0; ty < cty_; ty++) {
        for (int tx = 0; tx < ctx_; tx++, pfInvalid++, ppltile++) {
            if (!(*pfInvalid)) {
                continue;
            }
            Rect rc;
            rc.left = tx * gcxTile - xOffset_;
            rc.top = ty * gcyTile - yOffset_;
            rc.right = rc.left + gcxTile;
            rc.bottom = rc.top + gcyTile;
            UpdateTile(*ppltile, pbmSrc, &rc, palette);
            changed = true;
        }
    }
    return changed;
}

void LayerMap::UpdateTile(LayerTile *pltile, DibBitmap *pbmSrc, Rect *prcSrc,
        void *palette) {

    // Clip prcSrc to the dib
    int xDst = 0;
    if (prcSrc->left < 0) {
        xDst = -prcSrc->left;
        prcSrc->left = 0;
    }
    int yDst = 0;
    if (prcSrc->top < 0) {
        yDst = -prcSrc->top;
        prcSrc->top = 0;
    }
    Size sizDib;
    pbmSrc->GetSize(&sizDib);
    if (prcSrc->right > sizDib.cx) {
        prcSrc->right = sizDib.cx;
    }
    if (prcSrc->bottom > sizDib.cy) {
        prcSrc->bottom = sizDib.cy;
    }
    if (prcSrc->IsEmpty()) {
        return;
    }

    // This code assumes even alignment
    if ((prcSrc->left & 1) || (prcSrc->right & 1)) {
        return;
    }

    // Grab the tile, and convert to 32bpp
    byte *pbSrc = pbmSrc->GetBits() + prcSrc->top * sizDib.cx + prcSrc->left;
    int cbSrcReturn = sizDib.cx - prcSrc->Width();
    dword *pdwDst = pdwTile_ + yDst * gcxTile + xDst;
    int cdwDstReturn = gcxTile - prcSrc->Width();

    for (int y = prcSrc->top; y < prcSrc->bottom; y++) {
        for (int x = prcSrc->left; x < prcSrc->right; x += 2) {
            word w = *((word *)pbSrc);
            pbSrc += 2;
            *pdwDst++ = ((dword *)palette)[w & 0xff];
            *pdwDst++ = ((dword *)palette)[(w >> 8) & 0xff];
        }
        pbSrc += cbSrcReturn;
        pdwDst += cdwDstReturn;
    }

    // Update the layer. Note could clip the dest as a little speedup.

    CGImageRef image = CGBitmapContextCreateImage(bmCtx_);
    pltile->UpdateImage(image);
    CGImageRelease(image);
}

void LayerMap::Draw(CGContextRef ctx, int cy, const CGRect& rcInvalid) {
    // Set clipping rect

    CGContextSaveGState(ctx);
    CGRect rcClip;
    rcClip.origin.y = cy - rc_.top - rc_.Height();
    rcClip.origin.x = rc_.left;
    rcClip.size.width = rc_.Width();
    rcClip.size.height = rc_.Height();
    CGContextClipToRect(ctx, rcClip);

    // Draw layers

    LayerTile **apltile = apltile_;
    int y = rc_.top - yOffset_;
    for (int ty = 0; ty < cty_; ty++, y += gcyTile) {
        int x = rc_.left - xOffset_;
        for (int tx = 0; tx < ctx_; tx++, x += gcxTile, apltile++) {
            LayerTile *pltile = *apltile;
            CGRect rc;
            rc.origin.x = x;
            rc.origin.y = cy - y - gcyTile;
            rc.size.width = gcxTile;
            rc.size.height = gcyTile;
#if 0
            if (!pltile->IsUpdated()) {
                continue;
            }
#else
            // Unfortunately, this must be done because iPhone extends
            // the invalid region by itself, so the game can't track it
            // reliably.
            if (!CGRectIntersectsRect(rcInvalid, rc)) {
                continue;
            }
#endif
            pltile->Draw(ctx, rc);
        }
    }

    // Restore clipping rect
    CGContextRestoreGState(ctx);
}

void LayerMap::Scroll(int dx, int dy) {
    if (dx == 0 && dy == 0) {
        return;
    }

	// Calc new sub-tile offsets

	int xOffsetNew, yOffsetNew;
	if (dx <= 0) {
		xOffsetNew = PcFracFromUpc(xOffset_ - dx);
	} else {
		xOffsetNew = PcFracFromUpc(gcxTile -
                PcFracFromUpc(gcxTile - xOffset_ + dx));
	}
	if (dy <= 0) {
		yOffsetNew = PcFracFromUpc(yOffset_ - dy);
	} else {
		yOffsetNew = PcFracFromUpc(gcyTile -
                PcFracFromUpc(gcyTile - yOffset_ + dy));
	}

	// Figure out the number of whole tiles to scroll

	int dtx;
	if (dx <= 0) {
		dtx = -TcFromPc(xOffset_ - dx);
	} else {
		dtx = TcFromPc((gcxTile - 1) - xOffset_ + dx);
	}

	int dty;
	if (dy <= 0) {
		dty = -TcFromPc(yOffset_ - dy);
	} else {
		dty = TcFromPc((gcyTile - 1) - yOffset_ + dy);
	}

    // Update new offset

    xOffset_ = xOffsetNew;
    yOffset_ = yOffsetNew;
    if (dtx == 0 && dty == 0) {
        MarkUpdateAll();
        return;
    }

    // Actually need to scroll the tiles. Figure out the scrolling rect

    TRect trcSrc;
    trcSrc.Set(0, 0, ctx_, cty_);
    int txDst = dtx;
    int tyDst = dty;

    // Clip

    if (trcSrc.left < 0) {
        trcSrc.left = 0;
    }
    if (trcSrc.top < 0) {
        trcSrc.top = 0;
    }
    if (trcSrc.right > ctx_) {
        trcSrc.right = ctx_;
    }
    if (trcSrc.bottom > cty_) {
        trcSrc.bottom = cty_;
    }

    // Figure out dst

    if (txDst < 0) {
        trcSrc.left -= txDst;
        txDst = 0;
    }
    if (tyDst < 0) {
        trcSrc.top -= tyDst;
        tyDst = 0;
    }

    // Clip right and bottom edges

    int txRightDst = txDst + trcSrc.Width();
    if (txRightDst > ctx_) {
        trcSrc.right -= txRightDst - ctx_;
    }
    int tyBottomDst = tyDst + trcSrc.Height();
    if (tyBottomDst > cty_) {
        trcSrc.bottom -= tyBottomDst - cty_;
    }

    // Anything left to copy? If not, all done

    if (trcSrc.IsEmpty()) {
        MarkUpdateAll();
        return;
    }

    // Scroll the tiles in a non-destructive direction

    if (trcSrc.top == tyDst) {
        if (txDst < trcSrc.left) {
            LRTBExchange(&trcSrc, txDst, tyDst);
        } else {
            RLBTExchange(&trcSrc, txDst, tyDst);
        }
    } else {
        if (tyDst < trcSrc.top) {
            LRTBExchange(&trcSrc, txDst, tyDst);
        } else {
            RLBTExchange(&trcSrc, txDst, tyDst);
        }
    }
}

void LayerMap::ResetScrollOffset() {
    xOffset_ = 0;
    yOffset_ = 0;
    MarkUpdateAll();
}

void LayerMap::MarkUpdateAll() {
    for (int ty = 0; ty < cty_; ty++) {
        for (int tx = 0; tx < ctx_; tx++) {
            apltile_[ty * ctx_ + tx]->MarkUpdated();
        }
    }
}

void LayerMap::LRTBExchange(TRect *ptrc, int txDst, int tyDst) {
    LayerTile **ppltileSrc = &apltile_[ptrc->top * ctx_ + ptrc->left];
    LayerTile **ppltileDst = &apltile_[tyDst * ctx_ + txDst];
    int ctxReturn = ctx_ - ptrc->Width();

    int ctyT = ptrc->Height();
    while (ctyT-- != 0) {
    int ctxT = ptrc->Width();
        while (ctxT-- != 0) {
            LayerTile *pltileT = *ppltileDst;
            *ppltileDst++ = *ppltileSrc;
            (*ppltileSrc)->MarkUpdated();
            *ppltileSrc++ = pltileT;
        }
        ppltileDst += ctxReturn;
        ppltileSrc += ctxReturn;
    }
}

void LayerMap::RLBTExchange(TRect *ptrc, int txDst, int tyDst) {
    LayerTile **ppltileSrc = &apltile_[(ptrc->bottom - 1) * ctx_ +
            ptrc->right - 1];
    LayerTile **ppltileDst = &apltile_[(tyDst + ptrc->Height() - 1) * ctx_ +
            txDst + ptrc->Width() - 1];
    int ctxReturn = ctx_ - ptrc->Width();

    int ctyT = ptrc->Height();
    while (ctyT-- != 0) {
        int ctxT = ptrc->Width();
        while (ctxT-- != 0) {
            LayerTile *pltileT = *ppltileDst;
            *ppltileDst-- = *ppltileSrc;
            (*ppltileSrc)->MarkUpdated();
            *ppltileSrc-- = pltileT;
        }
        ppltileDst -= ctxReturn;
        ppltileSrc -= ctxReturn;
    }
}

} // namespace wi
