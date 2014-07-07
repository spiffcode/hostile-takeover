#ifndef __IPHONESELECTIONSPRITE_H__
#define __IPHONESELECTIONSPRITE_H__

#include "base/criticalsection.h"
#include "inc/basictypes.h"
#include "game/ht.h"
#include "game/dragrect.h"
#include "game/sprite.h"
#import <CoreGraphics/CGContext.h>
#import <QuartzCore/CALayer.h>

namespace wi {

class IPhoneSelectionSprite : public SelectionSprite {
public:
    IPhoneSelectionSprite(SpriteManager *psprm);
    ~IPhoneSelectionSprite();

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

#endif // __IPHONESELECTIONSPRITE_H__
