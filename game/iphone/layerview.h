#ifndef __LAYERVIEW_H__
#define __LAYERVIEW_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>
#import <CoreGraphics/CGContext.h>
#include "game/ht.h"
#import "game/iphone/wiview.h"
#include "game/iphone/layerdib.h"
#include <vector>

@interface LayerView : WiView {
    CGContextRef bmCtx_;
    wi::LayerDib *pbm_;
    CGAffineTransform tm_;
    bool initialized_;
    CGRect *arcInvalid_;
    int crcInvalid_;
}
- (id)initWithFrame:(CGRect)rect;
- (void)dealloc;
- (void)frameStart;
- (void)frameComplete:(int)cfrmm maps:(wi::UpdateMap **)apupd
        rects:(wi::Rect *)arc scrolled:(bool)fScrolled;
- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation
        width:(int)cx height:(int)cy;
- (void)setPalette:(wi::Palette *)ppal;
- (void)setFormMgrs:(wi::FormMgr *)pfrmmSimUI input:(wi::FormMgr *)pfrmmInput;
- (void)resetScrollOffset;
@end

#endif // __LAYERVIEW_H__
