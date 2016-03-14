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

    DPoint apt[4];
    drc_.GetPoints(apt);

    float density = gpdisp->Density();

    #define renderer (SDL_Renderer *)pv
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < ARRAYSIZE(apt); i++) {
        int next = i + 1 == ARRAYSIZE(apt) ? 0 : i + 1;

        // x/y points for point a/b of the selection sprite line
        int x1 = roundf(apt[i].y * density);
        int y1 = roundf(apt[i].x * density);
        int x2 = roundf(apt[next].y * density);
        int y2 = roundf(apt[next].x * density);

        // draw the selection sprite line
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

        int gs = 20;        // grabbie size
        int gsh = gs / 2;   // grabbie size half

        // create and draw a grabbie rect
        SDL_Rect rect = {x1 - gsh, y1 - gsh , gs, gs};
        SDL_RenderFillRect(renderer, &rect);
    }

    crit_.Leave();
}

} // namespace wi
