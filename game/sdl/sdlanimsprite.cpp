#include "base/tick.h"
#include "base/thread.h"
#include "game/sdl/sdlanimsprite.h"
#include "game/sdl/sysmessages.h"

namespace wi {

SdlAnimSprite::SdlAnimSprite(SpriteManager *psprm) {
    hash_ = 0;
    cx_ = 0;
    cy_ = 0;
    xOrigin_ = 0;
    yOrigin_ = 0;
    texture_ = NULL;
    surface_ = NULL;
    nScale_ = 1.0f;
    x_ = 0;
    y_ = 0;
    fVisible_ = true;
    psprm_ = psprm;
}

SdlAnimSprite::~SdlAnimSprite() {
    psprm_->Remove(this);

    crit_.Enter();
    SDL_DestroyTexture(texture_);
    texture_ = NULL;
    SDL_FreeSurface(surface_);
    surface_ = NULL;
    crit_.Leave();
}

void SdlAnimSprite::CaptureFrame(UnitGob *pgob) {
    // Surface already created?

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

    // Create surface

    bool fSuccess = CreateSurface(pgob);
    crit_.Leave();
    if (!fSuccess) {
        return;
    }

    // Add this sprite
    psprm_->Add(this);
}

bool SdlAnimSprite::CreateSurface(UnitGob *pgob) {
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

    // Subvert the TBitmap shdowing to our purpose.
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
        delete[] pb8;
        return false;
    }

    // Convert to 32bpp. Sdl will rotate at draw time.
    byte *pbT = pb8;
    dword *pdwT = pdw32;
    int cpT = cp;
    while (cpT-- != 0) {
        *pdwT++ = mp8bpp32bpp_[*pbT++];
    }
    delete[] pb8;

    // Create the appropriate masks

    dword rmask = 0xff000000;
    dword gmask = 0x00ff0000;
    dword bmask = 0x0000ff00;
    dword amask = 0x000000ff;

    surface_ = SDL_CreateRGBSurfaceFrom(pdw32, cx_, cy_, 32, (cx_ * sizeof(dword)), bmask, gmask, rmask, amask);

    if (surface_ == NULL) {
        delete[] pdw32;
        return false;
    }
    
    return true;
}

void SdlAnimSprite::Draw(void *pv, Size *psiz) {
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

    SDL_Rect rc = {
        x_ + rcBounds.left,
        y_ + rcBounds.top,
        rcBounds.Width() ,
        rcBounds.Height()
    };

    if (surface_ != NULL) {
        // Create a texture from the surface
        texture_ = SDL_CreateTextureFromSurface((SDL_Renderer *)pv, surface_);

        // Render the texture
        SDL_RenderCopy((SDL_Renderer *)pv, texture_, NULL, &rc);
    }

    crit_.Leave();
}

void SdlAnimSprite::GetBounds(Rect *prc) {
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

void SdlAnimSprite::SetScale(float nScale) {
    if (nScale_ == nScale) {
        return;
    }
    nScale_ = nScale;
    psprm_->Update(this);
}

void SdlAnimSprite::SetPosition(int x, int y) {
    if (x_ == x && y_ == y) {
        return;
    }
    x_ = x;
    y_ = y;
    psprm_->Update(this);
}

void SdlAnimSprite::Show(bool fShow) {
    if (fVisible_ == fShow) {
        return;
    }
    fVisible_ = fShow;
    psprm_->Update(this);
}

void SdlAnimSprite::SetScaleAnimation(float nScaleStart, float nScaleEnd,
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

void SdlAnimSprite::OnMessage(base::Message *pmsg) {
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
