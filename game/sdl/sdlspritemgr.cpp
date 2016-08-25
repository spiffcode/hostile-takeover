#include "game/ht.h"
#include "game/sdl/sdlspritemgr.h"
#include "SDL.h"

namespace wi {

SdlSpriteManager::SdlSpriteManager() {
    pcrit_ = new base::CriticalSection();
    cpspr_ = 0;
    fSpriteDirty_ = false;
    memset(&rcClip1_, 0, sizeof(rcClip1_));
    memset(&rcClip2_, 0, sizeof(rcClip2_));
}

SdlSpriteManager::~SdlSpriteManager() {

}

void SdlSpriteManager::Add(wi::Sprite *pspr) {
    base::CritScope cs(pcrit_);

    // If already added, just recreate the layer

    bool fFound = false;
    for (int i = 0; i < cpspr_; i++) {
        if (pspr == apspr_[i]) {
            fFound = true;
        }
    }
    if (!fFound && cpspr_ < ARRAYSIZE(apspr_) - 1) {
        apspr_[cpspr_] = pspr;
        cpspr_++;
    }

    fSpriteDirty_ = true;
}

void SdlSpriteManager::Remove(wi::Sprite *pspr) {
    base::CritScope cs(pcrit_);

    bool fFound = false;
    for (int i = 0; i < cpspr_; i++) {
        if (pspr == apspr_[i]) {
            cpspr_--;
            if (i < ARRAYSIZE(apspr_) - 1) {
                memmove(&apspr_[i], &apspr_[i + 1],
                        (ARRAYSIZE(apspr_) - 1 - i) * ELEMENTSIZE(apspr_));
            }
            fFound = true;
            break;
        }
    }

    fSpriteDirty_ = true;
}

void SdlSpriteManager::Update(wi::Sprite *pspr) {
    fSpriteDirty_ = true;
}

void SdlSpriteManager::DrawSprites(SDL_Renderer *renderer, Size siz) {
    base::CritScope cs(pcrit_);

    if (cpspr_ == 0) {
        return;
    }

    for (int i = 0; i < cpspr_; i++) {
        // Draw clipped to rcClip1
        SDL_RenderSetClipRect(renderer, &rcClip1_);
        apspr_[i]->Draw(renderer, &siz);

        // Draw clipped to rcClip2
        SDL_RenderSetClipRect(renderer, &rcClip2_);
        apspr_[i]->Draw(renderer, &siz);
    }

    SDL_RenderSetClipRect(renderer, NULL);
}

void SdlSpriteManager::SetClipRects(wi::Rect *prc1, wi::Rect *prc2) {
    rcClip1_.x = prc1->left;
    rcClip1_.y = prc1->top;
    rcClip1_.w = prc1->Width();
    rcClip1_.h = prc1->Height();

    rcClip2_.x = prc2->left;
    rcClip2_.y = prc2->top;
    rcClip2_.w = prc2->Width();
    rcClip2_.h = prc2->Height();
}


} // namespace wi
