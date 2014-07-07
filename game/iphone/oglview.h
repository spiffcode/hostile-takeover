// iphone beta 5 sdk made this implementation obsolete
#if 0

//
//  oglview.h
//  wi
//
//  Created by Scott Ludwig on 4/19/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#ifndef __OGLVIEW_H__
#define __OGLVIEW_H__

#include "../ht.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>
#import <CoreGraphics/CGContext.h>
#import "wiview.h"

namespace wi {
class OglDib;
}

@interface OglView : WiView {
    EAGLContext context_;
    EAGLSurface	surface_;
    EAGLSurface surfacePBuffer_;
    GLuint texName_;
    wi::OglDib *pbm_;
}
- (id)initWithFrame:(struct CGRect)rect;
- (void)initSurfaceAndContext;
- (void)initGraphics;
- (void)dealloc;
- (void)frameStart;
- (void)frameComplete;
- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation width:(int)cx height:(int)cy;
- (void)setPalette:(wi::Palette *)ppal;
@end

/* EAGL and GL functions calling wrappers that log on error */
#ifdef DEBUG
#define CALL_EAGL_FUNCTION(__FUNC__, ...) ({ EAGLError __error = __FUNC__( __VA_ARGS__ ); if(__error != kEAGLErrorSuccess) NSLog(@"%s() called from %s returned error %i", #__FUNC__, __FUNCTION__, __error); (__error ? NO : YES); })
#define CHECK_GL_ERROR() ({ GLenum __error = glGetError(); if(__error) NSLog(@"OpenGL error 0x%04X in %s", __error, __FUNCTION__); (__error ? NO : YES); })
#else
#define CALL_EAGL_FUNCTION(__FUNC__, ...) ({ __FUNC__( __VA_ARGS__ ); })
#define CHECK_GL_ERROR()
#endif

#endif // __OGLVIEW_H__

#endif // 0