//
//  oglview2.h
//  wi
//
//  Created by Scott Ludwig on 5/22/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#ifndef __OGLVIEW2_H__
#define __OGLVIEW2_H__

#include "../ht.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>
#import <CoreGraphics/CGContext.h>
#include "ogldib.h"
#import "wiview.h"

@interface OglView2 : WiView {
    EAGLContext *context_;
    GLuint tex_;
    GLuint framebufferTex_;
    GLuint renderbuffer_;
    GLuint framebufferRender_;
    GLfloat coords_[8];
    GLfloat verts_[12];
    wi::OglDib *pbm_;
}
- (id)initWithFrame:(struct CGRect)rect;
- (void)initLayer;
- (void)initGraphics;
- (void)cleanupGraphics;
- (void)dealloc;
- (void)frameStart;
- (void)frameComplete;
- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation width:(int)cx height:(int)cy;
- (void)setPalette:(wi::Palette *)ppal;
@end

/* EAGL and GL functions calling wrappers that log on error */
#ifdef DEBUG
#define CHECK_GL_ERROR() ({ GLenum __error = glGetError(); if(__error) NSLog(@"OpenGL error 0x%04X in %s", __error, __FUNCTION__); (__error ? NO : YES); })
#else
#define CHECK_GL_ERROR()
#endif

#endif // __OGLVIEW2_H__