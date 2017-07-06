#ifndef __SDLANIMSPRITE_H__
#define __SDLANIMSPRITE_H__

#include "base/criticalsection.h"
#include "base/messagehandler.h"
#include "inc/basictypes.h"
#include "game/ht.h"

namespace wi {

class SdlAnimSprite : public AnimSprite, base::MessageHandler {
public:
    SdlAnimSprite(SpriteManager *psprm);
    ~SdlAnimSprite();

    // AnimationSprite
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
    bool CreateSurface(UnitGob *pgob);

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
    DibBitmap *pbm_;
    SpriteManager *psprm_;
    float nScaleStart_, nScaleEnd_;
    long msAnimateStart_;
    dword cmsAnimate_;
    dword cmsAnimationRate_;
    bool fAutoDestroy_;
};

} // namespace wi

#endif // __SDLANIMSPRITE_H__
