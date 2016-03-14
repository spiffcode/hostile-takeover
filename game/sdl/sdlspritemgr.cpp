#include "game/ht.h"
#include "game/sdl/sdlspritemgr.h"
#include "SDL.h"

namespace wi {

SdlSpriteManager::SdlSpriteManager() {
    pcrit_ = new base::CriticalSection();
    cpspr_ = 0;
    fSpriteDirty_ = false;
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
        apspr_[i]->Draw(renderer, &siz);
    }
}


} // namespace wi
