#include "game/sdl/sdlselectionsprite.h"

#define GRABBIE_SIZE 10
#define GRABBIE_SIZE_HALF GRABBIE_SIZE/2

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

    DPoint apt[4];
    drc_.GetPoints(apt);

    #define renderer (SDL_Renderer *)pv
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < ARRAYSIZE(apt); i++) {
        int next = i + 1 == ARRAYSIZE(apt) ? 0 : i + 1;

        // x/y are swapped
        Point pt1 = {apt[i].y, apt[i].x};
        Point pt2 = {apt[next].y, apt[next].x};

        // draw the selection sprite line
        SDL_RenderDrawLine(renderer, pt1.x, pt1.y, pt2.x, pt2.y);

        // create and draw a grabbie rect
        SDL_Rect rect = {pt1.x - GRABBIE_SIZE_HALF, pt1.y - GRABBIE_SIZE_HALF,
            GRABBIE_SIZE, GRABBIE_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }

    crit_.Leave();
}

} // namespace wi
