#include "game/sdl/sdlselectionsprite.h"

namespace wi {

SdlSelectionSprite::SdlSelectionSprite(SpriteManager *psprm) {
    fVisible_ = false;
    psprm_ = psprm;
    psprm_->Add(this);
}

SdlSelectionSprite::~SdlSelectionSprite() {
    psprm_->Remove(this);
}

void SdlSelectionSprite::SetDragRect(const DragRect& drc) {
    crit_.Enter();
    drc_ = drc;
    crit_.Leave();
    psprm_->Update(this);
}

const DragRect& SdlSelectionSprite::GetDragRect() {
    return drc_;
}

void SdlSelectionSprite::Show(bool fShow) {
    if (fVisible_ == fShow) {
        return;
    }
    fVisible_ = fShow;
    psprm_->Update(this);
}

void SdlSelectionSprite::Draw(void *pv, Size *psiz) {
    if (!fVisible_) {
        return;
    }

    crit_.Enter();

#if 0 // TODO(darrinm)
    DPoint apt[4];
    drc_.GetPoints(apt);

    // x/y are swapped, and y needs to be origin adjusted

    CGContextRef ctx = (CGContextRef)pv;
    CGContextSetRGBStrokeColor(ctx, 1.0, 1.0, 1.0, 1.0);
    CGContextSetLineWidth(ctx, 2.0);
    CGContextBeginPath(ctx);
    CGContextMoveToPoint(ctx, apt[0].y, psiz->cy - apt[0].x);
    for (int i = 1; i < ARRAYSIZE(apt); i++) {
        CGContextAddLineToPoint(ctx, apt[i].y, psiz->cy - apt[i].x);
    }
    CGContextAddLineToPoint(ctx, apt[0].y, psiz->cy - apt[0].x);
    CGContextStrokePath(ctx);

    // Draw circles at rect corners for "grabbies". White stroked,
    // black filled.
    CGContextSetRGBFillColor(ctx, 0.0, 0.0, 0.0, 1.0);

#define kcpRectHalf 4

    for (int i = 0; i < ARRAYSIZE(apt); i++) {
        CGRect rc = CGRectMake(apt[i].y - kcpRectHalf,
                psiz->cy - (apt[i].x + kcpRectHalf),
                kcpRectHalf * 2, kcpRectHalf * 2);
        CGContextFillEllipseInRect(ctx, rc);
        CGContextStrokeEllipseInRect(ctx, rc);
    }
#endif

    crit_.Leave();
}

} // namespace wi
