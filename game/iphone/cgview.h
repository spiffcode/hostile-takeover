#ifndef __CGVIEW_H__
#define __CGVIEW_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>
#import <CoreGraphics/CGContext.h>
#include "game/ht.h"
#import "game/iphone/wiview.h"
#include "game/iphone/cgdib.h"
#include "game/sprite.h"
#include "base/criticalsection.h"
#include <vector>

@interface CgView : WiView {
    CGContextRef bmCtx_;
    wi::CgDib *pbm_;
    CGAffineTransform tmFlip_;
    CGImageRef image_;
    CGRect *arcInvalid_;
    int crcInvalid_;
    int crcInvalidMax_;
}
- (id)initWithFrame:(CGRect)rect;
- (void)initGraphics;
- (void)dealloc;
- (void)frameStart;
- (void)frameComplete:(int)cfrmm maps:(wi::UpdateMap **)apupd
        rects:(wi::Rect *)arc scrolled:(bool)fScrolled;
- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation
        width:(int)cx height:(int)cy;
- (void)setPalette:(wi::Palette *)ppal;
@end

#endif // __CGVIEW_H__
