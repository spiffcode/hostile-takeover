#ifndef __IPHONEANIMSPRITE_H__
#define __IPHONEANIMSPRITE_H__

#include "base/criticalsection.h"
#include "base/messagehandler.h"
#include "inc/basictypes.h"
#include "game/ht.h"
#import <CoreGraphics/CGContext.h>
#import <QuartzCore/CALayer.h>

namespace wi {

class IPhoneAnimSprite : public AnimSprite, base::MessageHandler {
public:
    IPhoneAnimSprite(SpriteManager *psprm);
    ~IPhoneAnimSprite();

    // AnimationSprite
    virtual void SetPalette(Palette *ppal);
    virtual void CaptureFrame(UnitGob *pgob);
    virtual void SetScaleAnimation(float nScaleStart, float nScaleEnd,
            dword cms, dword cmsRate, bool fAutoDestroy);

    // Sprite
    virtual SpriteManager *GetManager() { return psprm_; }
    virtual void SetScale(float scale);
    virtual void SetPosition(int x, int y);
    virtual void Show(bool fShow);
    virtual bool IsVisible() { return fVisible_; }
    virtual void GetBounds(Rect *prc);

    // PlatformSprite
    virtual void Draw(void *pv, Size *psiz);

private:
    bool CreateLayer(UnitGob *pgob);

    // MessageHandler
    virtual void OnMessage(base::Message *pmsg);

    base::CriticalSection crit_;
    Rect rcUI_;
    int x_, y_;
    Rect rcBase_;
    float nScale_;
    bool fVisible_;
    dword hash_;
    int cx_, cy_;
    int xOrigin_, yOrigin_;
    CGLayer *layer_;
    SpriteManager *psprm_;
    dword mp8bpp32bpp_[256];
    float nScaleStart_, nScaleEnd_;
    long msAnimateStart_;
    dword cmsAnimate_;
    dword cmsAnimationRate_;
    bool fAutoDestroy_;
};

} // namespace wi

#endif // __IPHONEANIMSPRITE_H__
