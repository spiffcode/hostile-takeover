#ifndef __SDLSPRITEMGR_H__
#define __SDLSPRITEMGR_H__

#include "game/sprite.h"
#include "game/sdl/sdlselectionsprite.h"

namespace wi {

class SdlSpriteManager : public SpriteManager {
public:
    SdlSpriteManager() {}

    virtual void SetClipRects(wi::Rect *prc1, wi::Rect *prc2) {
        LOG() << "SdlSpriteManager::SetClipRects not implemented yet";
        return;
    }
    virtual wi::AnimSprite *CreateAnimSprite() {
        LOG() << "SdlSpriteManager::CreateAnimSprite not implemented yet";
        return NULL;
    }
    virtual wi::SelectionSprite *CreateSelectionSprite() {
        return new SdlSelectionSprite(this);
    }
    virtual void Add(wi::Sprite *pspr) {
        LOG() << "SdlSpriteManager::Add not implemented yet";
        return;
    }
    virtual void Remove(wi::Sprite *pspr) {
        LOG() << "SdlSpriteManager::Remove not implemented yet";
        return;
    }
    virtual void Update(wi::Sprite *pspr) {
        LOG() << "SdlSpriteManager::Update not implemented yet";
        return;
    }
};

} // namespace wi

#endif // __SDLSPRITEMGR_H__
