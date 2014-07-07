#import <QuartzCore/QuartzCore.h>
#import "game/iphone/cgview.h"
#import "game/iphone/iphone.h"
#include "base/log.h"

@implementation CgView

- (id)initWithFrame:(struct CGRect)rect
{
    self = [super initWithFrame: rect];
    if (self != nil) {
        [self initGraphics];
        image_ = NULL;
        arcInvalid_ = NULL;
        crcInvalid_ = 0;
        crcInvalidMax_ = 0;
    }
    return self;
}

- (void)dealloc
{
	CGContextRelease(bmCtx_);
    if (image_ != NULL) {
        CGImageRelease(image_);
    }
    delete arcInvalid_;
    [super dealloc];
}

- (void)setFormMgrs:(wi::FormMgr *)pfrmmSimUI input:(wi::FormMgr *)pfrmmInput
{
    // Now allocate the max # of invalid rects that may result

    wi::Size sizMap0;
    pfrmmSimUI->GetUpdateMap()->GetMapSize(&sizMap0);
    wi::Size sizMap1;
    pfrmmInput->GetUpdateMap()->GetMapSize(&sizMap1);

    int crcMax = (sizMap0.cx * sizMap0.cy + sizMap1.cx * sizMap1.cy) / 2;
    delete arcInvalid_;
    arcInvalid_ = new CGRect[crcMax];
    crcInvalid_ = 0;
    crcInvalidMax_ = crcMax;
}

- (void)initGraphics
{
    // The OS erases invalid areas black whether clearsContext is YES or NO.

    [[self layer] setOpaque: YES];
    self.clearsContextBeforeDrawing = NO;

    int w = rect_.size.width;
    int h = rect_.size.height;

    int allocSize = 4 * w * h;

    pb_ = (unsigned char *)malloc(allocSize);
    memset(pb_, 0, allocSize);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    bmCtx_ = CGBitmapContextCreate(pb_, w, h, 8, w * 4, colorSpace,
            kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrderDefault);
    CGColorSpaceRelease(colorSpace);
    
    tmFlip_ = CGAffineTransformMake(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, h);
}

- (void)drawRect:(CGRect)frame
{
    // OS 4.0 introduces contentScaleFactor. If this is set to anything other
    // than 1.0, the OS performs a very slow blt. OS 4.0 defaults it to 2.0
    // on iPhone 4. Set it back to 1.0 if the selector is present.

    if ([self respondsToSelector:@selector(setContentScaleFactor:)]) {
        [self setContentScaleFactor: 1.0];
    }

    base::CritScope cs(pcrit_);

    if (image_ == NULL) {
        return;
    }
	CGRect rc = [self bounds];
	CGContextRef ctx = UIGraphicsGetCurrentContext();    
    CGContextSaveGState(ctx);
    CGContextConcatCTM(ctx, tmFlip_);
	CGContextDrawImage(ctx, rc, image_);
    [self drawSprites: ctx];
    CGContextRestoreGState(ctx);
}

- (void)frameStart
{
    pcrit_->Enter();
    pbm_->ResetChanged();
}

- (void)invalidate
{
    // Runs on main thread

    base::CritScope cs(pcrit_);
    for (int i = 0; i < crcInvalid_; i++) {
        [self setNeedsDisplayInRect:arcInvalid_[i]];
    }
    crcInvalid_ = 0;
}

//#define INVALID_RECTS

- (void)frameComplete:(int)cfrmm maps:(wi::UpdateMap **)apupd
        rects:(wi::Rect *)arc scrolled:(bool)fScrolled
{
    if (!havePalette_) {
        pcrit_->Leave();
        return;
    }

    // Sprites may have changes, or front dib may have changed, or neither.
    bool fImageChanged = false;
    if (pbm_->HasChanged()) {
        if (image_ != NULL) {
            CGImageRelease(image_);
            image_ = NULL;
        }
        image_ = CGBitmapContextCreateImage(bmCtx_);
        fImageChanged = true;
    }

    bool fSpriteChanged = false;
    if (fSpriteDirty_) {
        fSpriteDirty_ = false;
        fSpriteChanged = true;
    }

    if (!fSpriteChanged && !fImageChanged) {
        pcrit_->Leave();
        return;
    }

    if (fScrolled || fSpriteChanged) {
#ifdef INVALID_RECTS
        arcInvalid_[0] = rect_;
        crcInvalid_ = 1;
        [self performSelectorOnMainThread:@selector(invalidate)
                withObject:nil waitUntilDone: NO];
#else
        [self performSelectorOnMainThread:@selector(setNeedsDisplay)
                withObject:nil waitUntilDone: NO];
#endif
        pcrit_->Leave();
        return;
    }

    if (fImageChanged) {
#ifdef INVALID_RECTS
        // Collect the invalid rects, and send them over to the main
        // thread

        CGRect *prc = &arcInvalid_[crcInvalid_];
        CGRect *prcMax = &arcInvalid_[crcInvalidMax_];
        int cy = (int)rect_.size.width;

        for (int i = 0; i < cfrmm; i++) {
            wi::UpdateMap *pupd = apupd[i];
            int yTop = arc[i].top;

            bool fFirst = true;
            wi::Rect rc;
            while (prc < prcMax && pupd->EnumUpdateRects(fFirst, NULL, &rc)) {
                fFirst = false;
                prc->origin.x = cy - (rc.bottom + yTop);
                prc->origin.y = rc.left;
                prc->size.width = rc.Height();
                prc->size.height = rc.Width();
                prc++;
            }
        }
        crcInvalid_ = prc - arcInvalid_;
        [self performSelectorOnMainThread:@selector(invalidate)
                withObject:nil waitUntilDone: NO];
#else
        [self performSelectorOnMainThread:@selector(setNeedsDisplay)
                withObject:nil waitUntilDone: NO];
#endif
        pcrit_->Leave();
        return;
    }
}

- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation width:(int)cx height:(int)cy
{
    wi::SurfaceProperties props;
    [self getSurfaceProperties:&props];
    pbm_ = wi::CreateCgDib(pb_, &props, cx, cy, nDegreeOrientation);
    return pbm_;
}

- (void)setPalette:(wi::Palette *)ppal
{
    if (pbm_ != NULL) {
        // 8->8888 mapping table. 3x faster than 555 format even though it is
        // converted to 565 for display.
        dword mp8bpp32bpp[256];
        for (int n = 0; n < BigWord(ppal->cEntries); n++) {
            byte *pb = (byte *)&mp8bpp32bpp[n];
            *pb++ = 0;
            *pb++ = ppal->argb[n][0];
            *pb++ = ppal->argb[n][1];
            *pb++ = ppal->argb[n][2];
        }  
        pbm_->SetPalette(mp8bpp32bpp, sizeof(mp8bpp32bpp));            

        havePalette_ = true;
    }
}

- (void)getSurfaceProperties:(wi::SurfaceProperties *)pprops
{
    pprops->cxWidth = (int)rect_.size.width;
    pprops->cyHeight = (int)rect_.size.height;
    pprops->cbxPitch = 4;
    pprops->cbyPitch = (int)rect_.size.width * 4;
    pprops->ffFormat = wi::kfDirect888;    
}
@end
