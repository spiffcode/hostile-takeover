//
//  wiview.h
//  wi
//
//  Created by Scott Ludwig on 4/21/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>
#include "game/ht.h"
#include "game/sprite.h"
#include "base/criticalsection.h"
#include "base/thread.h"

namespace wi {
class SpriteManagerAdapter;
}

@interface WiView : UIView {
    UITouch *atouch_[2];
    CGRect rect_;
    byte *pb_;
    bool havePalette_;
    wi::Rect rcClip1_;
    wi::Rect rcClip2_;
    wi::SpriteManagerAdapter *psprm_;
    wi::Sprite *apspr_[16];
    int cpspr_;
    base::CriticalSection *pcrit_;
    base::Thread *game_thread_;
    bool fSpriteDirty_;
    wi::Point aptLast_[2];
}
- (id)initWithFrame:(CGRect)rect;
- (void)dealloc;
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)getSurfaceProperties:(wi::SurfaceProperties *)pprops;
- (wi::SpriteManager *)getSpriteManager;
- (void)setClipRects:(wi::Rect *)prc1 prc2:(wi::Rect *)prc2;
- (wi::AnimSprite *)createAnimSprite;
- (wi::SelectionSprite *)createSelectionSprite;
- (void)addSprite:(wi::Sprite *)pspr;
- (void)removeSprite:(wi::Sprite *)pspr;
- (void)updateSprite:(wi::Sprite *)pspr;
- (void)drawSprites:(CGContextRef)ctx;
- (void)setFormMgrs:(wi::FormMgr *)pfrmmSimUI input:(wi::FormMgr *)pfrmmInput;
- (void)resetScrollOffset;
- (void)setGameThread:(base::Thread *)thread;
@end
