#ifndef __SDLSELECTIONSPRITE_H__
#define __SDLSELECTIONSPRITE_H__

#include "base/criticalsection.h"
#include "inc/basictypes.h"
#include "game/ht.h"
#include "game/dragrect.h"
#include "game/sprite.h"

namespace wi {

class SdlSelectionSprite : public SelectionSprite {
public:
    SdlSelectionSprite(SpriteManager *psprm);
    ~SdlSelectionSprite();

    // SelectionSprite
    virtual void SetDragRect(const DragRect& drc);
    virtual const DragRect& GetDragRect();

    // Sprite
    virtual SpriteManager *GetManager() { return psprm_; }
    virtual void SetScale(float scale) {}
    virtual void SetPosition(int x, int y) {}
    virtual void Show(bool fShow);
    virtual bool IsVisible() { return fVisible_; }
    virtual void GetBounds(Rect *prc) {}

    // PlatformSprite
    virtual void Draw(void *pv, Size *psiz);

private:
    base::CriticalSection crit_;
    SpriteManager *psprm_;
    DragRect drc_;
    bool fVisible_;
};

} // namespace wi

#endif // __SDLSELECTIONSPRITE_H__
