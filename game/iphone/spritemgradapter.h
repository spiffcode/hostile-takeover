#ifndef __SPRITEMGRADAPTER_H__
#define __SPRITEMGRADAPTER_H__

#include "game/sprite.h"

// C++ SpriteManager wrapper

namespace wi {

class SpriteManagerAdapter : public SpriteManager {
public:
    SpriteManagerAdapter(WiView *view) : view_(view) { }

    virtual void SetClipRects(wi::Rect *prc1, wi::Rect *prc2) {
        return [view_ setClipRects:prc1 prc2:prc2];
    }
    virtual wi::AnimSprite *CreateAnimSprite() {
        return [view_ createAnimSprite];
    }
    virtual wi::SelectionSprite *CreateSelectionSprite() {
        return [view_ createSelectionSprite];
    }
    virtual void Add(wi::Sprite *pspr) {
        return [view_ addSprite:pspr];
    }
    virtual void Remove(wi::Sprite *pspr) {
        return [view_ removeSprite:pspr];
    }
    virtual void Update(wi::Sprite *pspr) {
        return [view_ updateSprite:pspr];
    }

private:
    WiView *view_;
};

} // namespace wi

#endif // __SPRITEMGRADAPTER_H__
