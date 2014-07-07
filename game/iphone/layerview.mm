#import <QuartzCore/QuartzCore.h>
#import "game/iphone/layerview.h"
#import "game/iphone/iphone.h"
#include "base/log.h"

@implementation LayerView

- (id)initWithFrame:(struct CGRect)rect
{
    self = [super initWithFrame: rect];
    if (self == nil) {
        return nil;
    }

    //self.backgroundColor = [UIColor clearColor];
    [[self layer] setOpaque: YES];
    self.clearsContextBeforeDrawing = NO;
    [[self.superview layer] setOpaque: YES];
    self.superview.clearsContextBeforeDrawing = NO;
    rect_ = rect;

    // This produces a coordinate system that is like a landscape sector 0NO;
    // (origin bottom left). This will make the OS rotate blts for us.

    tm_ = CGAffineTransformMake(1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
            rect.size.height);
    tm_ = CGAffineTransformRotate(tm_, 4.7123889749);
    tm_ = CGAffineTransformTranslate(tm_, -rect_.size.height, 0);
    initialized_ = false;

    crcInvalid_ = 0;
    arcInvalid_ = NULL;

    return self;
}

- (void)dealloc
{
    if (bmCtx_ != NULL) {
        CGContextRelease(bmCtx_);
    }
    delete arcInvalid_;
    [super dealloc];
}

- (void)setFormMgrs:(wi::FormMgr *)pfrmmSimUI input:(wi::FormMgr *)pfrmmInput
{
    wi::Size sizMap0;
    pfrmmSimUI->GetUpdateMap()->GetMapSize(&sizMap0);
    wi::Size sizDib;
    pfrmmSimUI->GetDib()->GetSize(&sizDib);
    wi::Rect rc;
    rc.Set(0, 0, sizDib.cx, sizDib.cy);
    pbm_->InitLayerMap(0, &rc, &sizMap0);
    wi::Size sizMap1;
    pfrmmInput->GetUpdateMap()->GetMapSize(&sizMap1);
    rc.top = sizDib.cy;
    pfrmmInput->GetDib()->GetSize(&sizDib);
    rc.bottom = rc.top + sizDib.cy;
    pbm_->InitLayerMap(1, &rc, &sizMap1);

    // Now allocate the max # of invalid rects that may result
   
    int crcMax = (sizMap0.cx * sizMap0.cy + sizMap1.cx * sizMap1.cy) / 2;
    arcInvalid_ = new CGRect[crcMax];
    initialized_ = true;
}

- (void)drawRect:(CGRect)frame
{
    if (!initialized_) {
        return;
    }
	CGContextRef ctx = UIGraphicsGetCurrentContext();    
    base::CritScope cs(pcrit_);
    CGContextSaveGState(ctx);
    CGContextConcatCTM(ctx, tm_);

    // iPhone expands invalid areas in some mysterious cases,
    // so it is not possible to rely solely on game invalid rects.
    // This bounding rect is the best thing iPhone provides - however
    // the ctx is clipped to the rects the game provided (plus the
    // mysterious extra).
    // Also: there appears to be no way to stop iPhone from erasing
    // the invalid areas before drawRect gets control. Setting
    // the clear property to NO does nothing.

    CGRect rcInvalid= CGRectMake(frame.origin.y, frame.origin.x,
            frame.size.height, frame.size.width);
    pbm_->Draw(ctx, rcInvalid);
//    [self drawSprites: ctx];
    CGContextRestoreGState(ctx);
}

- (void)frameStart
{
    // Hold the critical section around game drawing. This way,
    // the graphics, and the invalidation of the view can be updated
    // atomically.

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

- (void)frameComplete:(int)cfrmm maps:(wi::UpdateMap **)apupd
        rects:(wi::Rect *)arc scrolled:(bool)fScrolled
{

    if (!havePalette_ || !pbm_->HasChanged()) {
        pcrit_->Leave();
        return;
    }

    // Everything needs to redraw if scrolling

    if (fScrolled) {
        pbm_->MarkUpdate(); 
        arcInvalid_[0] = rect_;
        crcInvalid_ = 1;
        pcrit_->Leave();
        [self performSelectorOnMainThread:@selector(invalidate)
                withObject:nil waitUntilDone: NO];
        return;
    }

    // Collect the invalid rects, and send them over to the main
    // thread

    CGRect *prc = arcInvalid_;
    crcInvalid_ = 0;
    int cy = (int)rect_.size.width;

    for (int i = 0; i < cfrmm; i++) {
        wi::UpdateMap *pupd = apupd[i];
        int yTop = arc[i].top;

        bool fFirst = true;
        wi::Rect rc;
        while (pupd->EnumUpdateRects(fFirst, NULL, &rc)) {
            fFirst = false;
            prc->origin.x = cy - (rc.bottom + yTop);
            prc->origin.y = rc.left;
            prc->size.width = rc.Height();
            prc->size.height = rc.Width();
            prc++;
        }
    }
    crcInvalid_ = prc - arcInvalid_;
    pcrit_->Leave();

    [self performSelectorOnMainThread:@selector(invalidate)
            withObject:nil waitUntilDone: NO];
}

- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation width:(int)cx height:(int)cy
{
    pbm_ = wi::CreateLayerDib(cx, cy);
    return pbm_;
}

- (void)resetScrollOffset
{
    pbm_->ResetScrollOffset();
}

- (void)setPalette:(wi::Palette *)ppal
{
    if (pbm_ != NULL) {
        // 8->8888 mapping table. 3x faster than 555 format.
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
