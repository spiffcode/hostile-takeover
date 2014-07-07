#include "base/tick.h"
#include "base/thread.h"
#include "game/iphone/iphoneanimsprite.h"
#include "game/iphone/cgdib.h"
#include "game/iphone/input.h"
#include <QuartzCore/CATransaction.h>
#import <Foundation/Foundation.h>

namespace wi {

IPhoneAnimSprite::IPhoneAnimSprite(SpriteManager *psprm) {
    hash_ = 0;
    cx_ = 0;
    cy_ = 0;
    xOrigin_ = 0;
    yOrigin_ = 0;
    layer_ = NULL;
    nScale_ = 1.0f;
    x_ = 0;
    y_ = 0;
    fVisible_ = true;
    psprm_ = psprm;
}

IPhoneAnimSprite::~IPhoneAnimSprite() {
    psprm_->Remove(this);

    crit_.Enter();
    if (layer_ != NULL) {
        CGLayerRelease(layer_);
        layer_ = NULL;
    }
    crit_.Leave();
}

void IPhoneAnimSprite::CaptureFrame(UnitGob *pgob) {
    // Layer already created?

    dword hash = pgob->GetAnimationHash();
    if (hash == hash_) {
        return;
    }

    // Enter critical section since drawing occurs on the main thread

    crit_.Enter();
    hash_ = hash;

    // Remember UIBounds

    UnitConsts *puntc = GetUnitConsts(pgob->GetType());
    rcUI_.FromWorldRect(&puntc->wrcUIBounds);

    // Create layer

    bool fSuccess = CreateLayer(pgob);
    crit_.Leave();
    if (!fSuccess) {
        return;
    }

    // Add this sprite
    psprm_->Add(this);
}

bool IPhoneAnimSprite::CreateLayer(UnitGob *pgob) {
    // Get height/width
    Rect rcAni;
    pgob->GetAnimationBounds(&rcAni, false);
    cx_ = (rcAni.Width() + 1) & ~1;
    cy_ = rcAni.Height();

    // Remember the bounds of the base. This'll be used later for y
    // positioning.

    pgob->GetAnimationBounds(&rcBase_, true);

    // rcAni has negative left and top. This identifies the origin.
    xOrigin_ = rcAni.left;
    yOrigin_ = rcAni.top;

    // Alloc the 8bpp buffer.
    int cp = cx_ * cy_;
    byte *pb8 = new byte[cp];
    if (pb8 == NULL) {
        return false;
    }

    // Draw the animation into an 8 bit DibBitmap
    // The 8->32 conversion palette has been tweaked so that 255
    // will map to RGBA transparent on output.

    DibBitmap bm;
    bm.Init(pb8, cx_, cy_);
    memset(pb8, 255, cp);

    //  Subvert the TBitmap shdowing to our purpose.
    // The background is 255. Force TBitmap shadowing to turn this into 254.
    // 254 has been to RGBA color with appropriate alpha, when the 8->32bpp
    // conversion occurs.

    byte bSav = gmpiclriclrShadow[255];
    gmpiclriclrShadow[255] = 254;
    pgob->DrawAnimation(&bm, -xOrigin_, -yOrigin_);
    gmpiclriclrShadow[255] = bSav;

    // Alloc the 32bpp buffer.
    dword *pdw32 = new dword[cp];
    if (pdw32 == NULL) {
        delete pb8;
        return false;
    }

    // Convert to 32bpp. IPhone will rotate at draw time.
    byte *pbT = pb8;
    dword *pdwT = pdw32;
    int cpT = cp;
    while (cpT-- != 0) {
        *pdwT++ = mp8bpp32bpp_[*pbT++];
    }
    delete pb8;

    // Create the BitmapContext, which allows passing in of pixels
    // Create it with an alpha channel.

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef bmCtx = CGBitmapContextCreate(pdw32, cx_, cy_,
            8, cx_ * 4, colorSpace, kCGImageAlphaPremultipliedFirst |
            kCGBitmapByteOrderDefault);
    CGColorSpaceRelease(colorSpace);
    if (bmCtx == NULL) {
        delete pdw32;
        return false;
    }

    // Drawing occurs on the main thread, so the layer needs to be
    // synchronized

    if (layer_ != NULL) {
        CGLayerRelease(layer_);
        layer_ = NULL;
    }
    if (layer_ == NULL) {
        CGSize size;
        size.width = cx_;
        size.height = cy_;
        layer_ = CGLayerCreateWithContext(bmCtx, size, NULL);
    }

    if (layer_ == NULL) {
        CGContextRelease(bmCtx);
        delete pdw32;
        return false;
    }

    // Create an image from the bitmap context
    // Done with the bitmap context and buffer.

    CGImageRef image = CGBitmapContextCreateImage(bmCtx);
    CGContextRelease(bmCtx);
    delete pdw32;

    // Draw this image into the layer. The layer is only a speed up.
    // Technically, drawing could create a new image each time.

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CGContextRef ctx = CGLayerGetContext(layer_);
    CGRect rc = CGRectMake(0, 0, cx_, cy_);
    CGContextDrawImage(ctx, rc, image);
    CGImageRelease(image);
    [pool release];
    return true;
}

void IPhoneAnimSprite::Draw(void *pv, Size *psiz) {
    if (!fVisible_) {
        return;
    }

    // This gets called on the drawing thread.
    // The coordinate space pre-rotated to be the game coordinate space but at
    // sector 0 (0, 0 is at bottom left). As always, xOrigin_ and yOrigin_
    // are negative.

    crit_.Enter();

    Rect rcBounds;
    GetBounds(&rcBounds);
    CGRect rc;
    rc.origin.x = x_ + rcBounds.left;
    rc.origin.y = psiz->cy - (y_ + rcBounds.top) - rcBounds.Height();
    rc.size.width = rcBounds.Width();
    rc.size.height = rcBounds.Height();

    if (layer_ != NULL) {
        CGContextRef ctx = (CGContextRef)pv;
        CGContextDrawLayerInRect(ctx, rc, layer_);
    }

    crit_.Leave();
}

void IPhoneAnimSprite::GetBounds(Rect *prc) {
    base::CritScope cs(&crit_);

    // Scale the UI rect, centered on the non-scaled UI rect

    Rect rcUIScaled = rcUI_;
    rcUIScaled.Inflate(rcUI_.Width() * (nScale_ - 1.0f) / 2,
            rcUI_.Height() * (nScale_ - 1.0f) / 2);

    // Center the height within the scaled UI rect, so it is centered under
    // the finger. Use the base height, since that doesn't change as other
    // parts animate. Offset by the base origin so the base stays in one
    // place (even if the height changes). Finally, use the same percentage
    // offset from the UI rect to position correctly.

    int xNew = rcUIScaled.left + (xOrigin_ - rcUI_.left) * nScale_;
    int yNew = (rcUIScaled.top + rcUIScaled.bottom -
            rcBase_.Height() * nScale_) / 2 - rcBase_.top * nScale_ +
            yOrigin_ * nScale_;
    int cxNew = (float)cx_ * nScale_;
    int cyNew = (float)cy_ * nScale_;

    prc->left = xNew;
    prc->top = yNew;
    prc->right = xNew + cxNew;
    prc->bottom = yNew + cyNew;
}

void IPhoneAnimSprite::SetPalette(Palette *ppal) {
    // Set the palette mapping table
    // Note the AMXs use the first 131 colors of the palette.
    for (int n = 0; n < BigWord(ppal->cEntries); n++) {
        byte *pb = (byte *)&mp8bpp32bpp_[n];
        *pb++ = 255;
        *pb++ = ppal->argb[n][0];
        *pb++ = ppal->argb[n][1];
        *pb++ = ppal->argb[n][2];
    } 

    // Make the last color of the palete RGBA for transparent.
    // We'll fill the 8bpp image with this, so that at 8->32 conversion,
    // we get transparency.

    mp8bpp32bpp_[255] = 0;

    // Make the second to last 40% black. AMX transparent will map to this
    // with clever remapping. 40% is equivalent to tbitmap shadowmap.

    byte *pb = (byte *)&mp8bpp32bpp_[254];
    *pb++ = 102;
    *pb++ = 0;
    *pb++ = 0;
    *pb++ = 0;
}

void IPhoneAnimSprite::SetScale(float nScale) {
    if (nScale_ == nScale) {
        return;
    }
    nScale_ = nScale;
    psprm_->Update(this);
}

void IPhoneAnimSprite::SetPosition(int x, int y) {
    if (x_ == x && y_ == y) {
        return;
    }
    x_ = x;
    y_ = y;
    psprm_->Update(this);
}

void IPhoneAnimSprite::Show(bool fShow) {
    if (fVisible_ == fShow) {
        return;
    }
    fVisible_ = fShow;
    psprm_->Update(this);
}

void IPhoneAnimSprite::SetScaleAnimation(float nScaleStart, float nScaleEnd,
        dword cms, dword cmsRate, bool fAutoDestroy) {
#if 1
    SetScale(nScaleEnd);
    return;
#endif

    // If the current scale is inside the requested range, advance the
    // animation timing to that point. For example if while it is scaling
    // up there is a request to scale it down, this keeps the animation
    // stepping constant.

    if (nScale_ >= nScaleStart && nScale_ <= nScaleEnd) {
        float nPercent = (float)(nScaleEnd - nScale_) /
                (float)(nScaleEnd - nScaleStart);
        cms *= nPercent;
        nScaleStart = nScale_;
    }
    nScaleStart_ = nScaleStart;
    nScaleEnd_ = nScaleEnd;
    msAnimateStart_ = base::GetMillisecondCount();
    cmsAnimate_ = cms;
    cmsAnimationRate_ = cmsRate;
    fAutoDestroy_ = fAutoDestroy;

    thread_.Clear(this, kidmAnimationTimer);
    base::Message msg;
    msg.handler = this;
    msg.id = kidmAnimationTimer;
    thread_.PostDelayed(&msg, cmsAnimationRate_ / 10);

    SetScale(nScaleStart_);
    psprm_->Update(this);
}

void IPhoneAnimSprite::OnMessage(base::Message *pmsg) {
    if (pmsg->id == kidmAnimationTimer) {
        long msCurrent = base::GetMillisecondCount();
        float nPercent = (float)(msCurrent - msAnimateStart_) /
                (float)cmsAnimate_;

        bool fFinished = false;
        if (nPercent >= 1.0f) {
            nPercent = 1.0f;
            fFinished = true;
        }
        SetScale(nScaleStart_ + (nScaleEnd_ - nScaleStart_) * nPercent);
        psprm_->Update(this);
       
        if (fFinished) {
            if (fAutoDestroy_) {
                // Delete after the next timer, so the current frame shows

                base::Message msg;
                msg.handler = this;
                msg.id = kidmDestroyAfterAnimation;
                thread_.PostDelayed(&msg, cmsAnimationRate_ / 10);
            }
            return;
        }

        // Schedule the next timer
 
        base::Message msg;
        msg.handler = this;
        msg.id = kidmAnimationTimer;
        thread_.PostDelayed(&msg, cmsAnimationRate_ / 10);
    }

    if (pmsg->id == kidmDestroyAfterAnimation) {
        // This deletes asynchronously - very convenient

        thread_.Dispose(this);
    }
}

} // namespace wi
