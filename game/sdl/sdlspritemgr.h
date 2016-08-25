#ifndef __SDLSPRITEMGR_H__
#define __SDLSPRITEMGR_H__

#include "game/sprite.h"
#include "game/sdl/sdlselectionsprite.h"

namespace wi {

class SdlSpriteManager : public SpriteManager {
public:
    SdlSpriteManager();
    ~SdlSpriteManager();

    virtual void SetClipRects(wi::Rect *prc1, wi::Rect *prc2);
    virtual wi::AnimSprite *CreateAnimSprite() {
        LOG() << "SdlSpriteManager::CreateAnimSprite not implemented yet";
        return NULL;
    }
    virtual wi::SelectionSprite *CreateSelectionSprite() {
        return new SdlSelectionSprite(this);
    }
    virtual void Add(wi::Sprite *pspr);
    virtual void Remove(wi::Sprite *pspr);
    virtual void Update(wi::Sprite *pspr);
    virtual void DrawSprites(SDL_Renderer *renderer, Size siz);

private:
    base::CriticalSection *pcrit_;
    int cpspr_;
    wi::Sprite *apspr_[16];
    bool fSpriteDirty_;
    SDL_Rect rcClip1_;
    SDL_Rect rcClip2_;
};

} // namespace wi

#endif // __SDLSPRITEMGR_H__
