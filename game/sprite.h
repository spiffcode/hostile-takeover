#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "game/ht.h"

namespace wi {

class Animation;
class AnimSprite;
class SelectionSprite;
class Sprite;

class SpriteManager {
public:
    virtual void SetClipRects(Rect *prc1, Rect *prc2) = 0;
    virtual AnimSprite *CreateAnimSprite() = 0;
    virtual SelectionSprite *CreateSelectionSprite() = 0;
    virtual void Add(Sprite *pspr) = 0;
    virtual void Remove(Sprite *pspr) = 0;
    virtual void Update(Sprite *pspr) = 0;
};

class Sprite : public PlatformSprite {
public:
    virtual ~Sprite() {}

    virtual SpriteManager *GetManager() = 0;
    virtual void SetScale(float scale) = 0;
    virtual void SetPosition(int x, int y) = 0;
    virtual void Show(bool fShow) = 0;
    virtual bool IsVisible() = 0;
    virtual void GetBounds(Rect *prc) = 0;
};

class UnitGob;
class AnimSprite : public Sprite {
public:
    virtual void CaptureFrame(UnitGob *pgob) = 0;
    virtual void SetScaleAnimation(float nScaleStart, float nScaleEnd,
            dword cms, dword cmsRate, bool fAutoDestroy) = 0;
};

class DragRect;
class SelectionSprite : public Sprite {
public:
    virtual void SetDragRect(const DragRect& drc) = 0;
    virtual const DragRect& GetDragRect() = 0;
};

} // namespace wi

#endif // __SPRITE_H__
