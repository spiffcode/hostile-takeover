//
//  wiview.mm
//  wi
//
//  Created by Scott Ludwig on 4/21/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "game/iphone/wiview.h"
#import "game/iphone/spritemgradapter.h"
#import "game/iphone/iphoneanimsprite.h"
#import "game/iphone/selectionsprite.h"
#import "game/iphone/webviewcontroller.h"
#include "game/iphone/input.h"
#include "mpshared/misc.h"

@implementation WiView

- (id)initWithFrame:(struct CGRect)rect
{
    self = [super initWithFrame: rect];
    if (self == nil) {
        return nil;
    }

    rect_ = CGRectMake(0, 0, rect.size.height, rect.size.width);

    [self setMultipleTouchEnabled: YES];
    atouch_[0] = NULL;
    atouch_[1] = NULL;
    havePalette_ = false;
    pb_ = NULL;
    memset(&rcClip1_, 0, sizeof(rcClip1_));
    memset(&rcClip2_, 0, sizeof(rcClip2_));
    pcrit_ = new base::CriticalSection();
    psprm_ = new wi::SpriteManagerAdapter(self);
    cpspr_ = 0;
    fSpriteDirty_ = false;
    game_thread_ = NULL;

    return self;
}

- (void)dealloc
{
    if (pb_ != NULL) {
        free(pb_);
    }
    delete psprm_;
    delete pcrit_;
    [super dealloc];
}

- (void)setGameThread:(base::Thread *)thread
{
    game_thread_ = thread;
}

- (void)checkTracking:(UIEvent *)event
{
    // This is just in case - if we lose an up, this detects it and resets
    // things. So far touchesCancelled appears to be working as advertised.

    int cTouchesTracking = 0;
    for (int i = 0; i < 2; i++) {
        if (atouch_[i] != NULL) {
            cTouchesTracking++;
        }
    }
    int cTouchesFound = 0;
    NSSet *allTouches = [event allTouches];
    for (UITouch *touch in allTouches) {
        for (int i = 0; i < 2; i++) {
            if (touch == atouch_[i]) {
                cTouchesFound++;
            }
        }
    }

    // If it's whacked, post an up and set the slot to NULL.

    if (cTouchesFound != cTouchesTracking) {
        for (int i = 0; i < 2; i++) {
            if (atouch_[i] != NULL) {
                atouch_[i] = NULL;
                base::Message msg;
                msg.id = (i == 0) ? kidmMouseUp : kidmMouseUp2;
                msg.x = aptLast_[i].x;
                msg.y = aptLast_[i].y;
                msg.ff = 0;
                msg.ms = wi::HostGetMillisecondCount();
                if (game_thread_ != NULL) {
                    game_thread_->Post(&msg);
                }
            }
        }
    }
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    // Sometimes we won't get an up. Handle this case.

    [self checkTracking:event];

    // Keep track of touches. According to the documentation, the touch *
    // pointer value is constant throughout the life of the touch, so it
    // can be used as sort of an id.


    // Only process touches for this view. WebView and ChatView are
    // handled as children, and events in their toolbar area somehow
    // get sent here. However this parent only wants touches for its
    // own view, not for the whole window.

    for (UITouch *touch in touches) {
        if (touch.view != self) {
            continue;
        }
        for (int i = 0; i < 2; i++) {
            if (atouch_[i] == NULL) {
                atouch_[i] = touch;
                CGPoint point = [touch locationInView: nil];
                base::Message msg;
                msg.id = (i == 0) ? kidmMouseDown : kidmMouseDown2;
                msg.x = (int)point.x;
                msg.y = (int)point.y;
                msg.ff = 0;
                msg.ms = wi::HostGetMillisecondCount();
                if (game_thread_ != NULL) {
                    game_thread_->Post(&msg);
                }
                aptLast_[i].x = (int)point.x;
                aptLast_[i].y = (int)point.y;
                break;
            }
        }
    }
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    for (UITouch *touch in touches) {
        if (touch.view != self) {
            continue;
        }
        for (int i = 0; i < 2; i++) {
            if (atouch_[i] == touch) {
                CGPoint point = [touch locationInView: nil];
                base::Message msg;
                msg.id = (i == 0) ? kidmMouseMove : kidmMouseMove2;
                msg.x = (int)point.x;
                msg.y = (int)point.y;
                msg.ff = 0;
                msg.ms = wi::HostGetMillisecondCount();
                if (game_thread_ != NULL) {
                    game_thread_->Post(&msg,
                            (i == 0) ? kidmMouseMove2 : kidmMouseMove);
                }
                aptLast_[i].x = (int)point.x;
                aptLast_[i].y = (int)point.y;
                break;
            }
        }
    }
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    for (UITouch *touch in touches) {
        if (touch.view != self) {
            continue;
        }
        for (int i = 0; i < 2; i++) {
            if (atouch_[i] == touch) {
                atouch_[i] = NULL;
                CGPoint point = [touch locationInView: nil];
                base::Message msg;
                msg.id = (i == 0) ? kidmMouseUp : kidmMouseUp2;
                msg.x = (int)point.x;
                msg.y = (int)point.y;
                msg.ff = 0;
                msg.ms = wi::HostGetMillisecondCount();
                if (game_thread_ != NULL) {
                    game_thread_->Post(&msg);
                }
                aptLast_[i].x = (int)point.x;
                aptLast_[i].y = (int)point.y;
                break;
            }
        }
    }
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
    for (int i = 0; i < 2; i++) {
        if (atouch_[i] != NULL) {
            atouch_[i] = NULL;
            base::Message msg;
            msg.id = (i == 0) ? kidmMouseUp : kidmMouseUp2;
            msg.x = aptLast_[i].x;
            msg.y = aptLast_[i].y;
            msg.ff = 0;
            msg.ms = wi::HostGetMillisecondCount();
            if (game_thread_ != NULL) {
                game_thread_->Post(&msg);
            }
        }
    }
}

- (void)getSurfaceProperties:(wi::SurfaceProperties *)pprops
{
    pprops->cxWidth = (int)rect_.size.width;
    pprops->cyHeight = (int)rect_.size.height;
    pprops->cbxPitch = 2;
    pprops->cbyPitch = (int)rect_.size.width * 2;
    pprops->ffFormat = wi::kfDirect565;    
}

- (wi::SpriteManager *)getSpriteManager
{
    return psprm_;
}

- (void)setClipRects:(wi::Rect *)prc1 prc2:(wi::Rect *)prc2
{
    rcClip1_ = *prc1;
    rcClip2_ = *prc2;
}

- (wi::AnimSprite *)createAnimSprite
{
    return new wi::IPhoneAnimSprite(psprm_);
}

- (wi::SelectionSprite *)createSelectionSprite
{
    return new wi::IPhoneSelectionSprite(psprm_);
}

- (void)addSprite:(wi::Sprite *)pspr
{
    base::CritScope cs(pcrit_);

    // If already added, just recreate the layer

    bool fFound = false;
    for (int i = 0; i < cpspr_; i++) {
        if (pspr == apspr_[i]) {
            fFound = true;
        }
    }
    if (!fFound && cpspr_ < ARRAYSIZE(apspr_) - 1) {
        apspr_[cpspr_] = pspr;
        cpspr_++;
    }

    fSpriteDirty_ = true;
}

- (void)removeSprite:(wi::Sprite *)pspr
{
    base::CritScope cs(pcrit_);

    bool fFound = false;
    for (int i = 0; i < cpspr_; i++) {
        if (pspr == apspr_[i]) {
            cpspr_--;
            if (i < ARRAYSIZE(apspr_) - 1) {
                memmove(&apspr_[i], &apspr_[i + 1],
                        (ARRAYSIZE(apspr_) - 1 - i) * ELEMENTSIZE(apspr_));
            }
            fFound = true;
            break;
        }
    }

    fSpriteDirty_ = true;
}

- (void)updateSprite:(wi::Sprite *)pspr
{
    fSpriteDirty_ = true;
}

- (void)drawSprites:(CGContextRef)ctx
{
    base::CritScope cs(pcrit_);

    if (cpspr_ == 0) {
        return;
    }

    // Save context before setting clip rects
    CGContextSaveGState(ctx);

    // First clip to the playfield up to just before the left edge of the
    // minimap, then the area above the minimap.

#if 0
    CGRect arcClip[2];
    arcClip[0].origin.x = rcClip1_.left;
    arcClip[0].origin.y = (int)rect_.size.width - rcClip1_.top -
            rcClip1_.Height();
    arcClip[0].size.width = rcClip1_.Width();
    arcClip[0].size.height = rcClip1_.Height();
    arcClip[1].origin.x = rcClip2_.left;
    arcClip[1].origin.y = (int)rect_.size.width - rcClip2_.top -
            rcClip2_.Height();
    arcClip[1].size.width = rcClip2_.Width();
    arcClip[1].size.height = rcClip2_.Height();
#else
    CGRect arcClip[2];
    arcClip[0].origin.x = rcClip1_.left;
    arcClip[0].origin.y = rect_.size.height - rcClip1_.bottom;
    arcClip[0].size.width = rcClip1_.Width();
    arcClip[0].size.height = rcClip1_.Height();
    arcClip[1].origin.x = rcClip2_.left;
    arcClip[1].origin.y = rect_.size.height - rcClip2_.bottom;
    arcClip[1].size.width = rcClip2_.Width();
    arcClip[1].size.height = rcClip2_.Height();
#endif
    CGContextClipToRects(ctx, arcClip, 2);

    // Draw sprites
    wi::Size siz;
    siz.cx = (int)rect_.size.width;
    siz.cy = (int)rect_.size.height;
    for (int i = 0; i < cpspr_; i++) {
        apspr_[i]->Draw(ctx, &siz);
    }

    // Restore context
    CGContextRestoreGState(ctx);
}

- (void)setFormMgrs:(wi::FormMgr *)pfrmmSimUI input:(wi::FormMgr *)pfrmmInput
{
}

- (void)resetScrollOffset
{
}
@end
