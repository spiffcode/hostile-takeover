// iphone beta 5 sdk made this implementation obsolete
#if 0

//
//  oglview.m
//  wi
//
//  Created by Scott Ludwig on 4/19/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "oglview.h"

@implementation OglView

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (id)initWithFrame:(struct CGRect)rect
{
    self = [super initWithFrame: rect];
    if (self != nil) {
        [self initSurfaceAndContext];
    }
    return self;
}

- (void)dealloc
{
	[super dealloc];
}

- (void)initSurfaceAndContext {
    // These properties make things faster
    
    self.opaque = TRUE;
    self.clearsContextBeforeDrawing = NO;
    
    // Create the window surface, the context, make current.
    // For an unknown reason, these need to be called on the main thread.
    
    CAEAGLLayer *layer = (CAEAGLLayer *)[self layer];
    CALL_EAGL_FUNCTION(eaglCreateWindowSurface, kEAGLPixelFormat_RGB565_D0, [layer nativeWindow], &surface_);
    CALL_EAGL_FUNCTION(eaglCreateContext, NULL, &context_);
}

- (void)initGraphics {
    CALL_EAGL_FUNCTION(eaglMakeCurrent, surface_, context_);
    
    // Initialize the viewport and projection, modelview matrices
    
    glDisable(GL_DEPTH_TEST);
    CHECK_GL_ERROR();
	glViewport(0, 0, rect_.size.width, rect_.size.height);
    CHECK_GL_ERROR();
    glMatrixMode(GL_PROJECTION);
    CHECK_GL_ERROR();
	glLoadIdentity();
    CHECK_GL_ERROR();
    glOrthof(0.0f, rect_.size.width, rect_.size.height, 0.0f, -1.0, 1.0);
    CHECK_GL_ERROR();
    glMatrixMode(GL_MODELVIEW);
    CHECK_GL_ERROR();
    glLoadIdentity();
    CHECK_GL_ERROR();
    
    // Create a texture bound to a pbuffer. This is done because glTexSubImage2D
    // is much faster changing this kind of texture than a regular texture.
    // OpenGL ES reformats regular textures for optimal texture mapping speed,
    // assuming they don't change much. OpenGL ES does not reformat pbuffer
    // textures, since it assumes pbuffers change frequently.

    glEnable(GL_TEXTURE_2D);
    CHECK_GL_ERROR();
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    CHECK_GL_ERROR();
    glGenTextures(1, &texName_);
    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, texName_);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();
    
    // Need texture to be power of 2 dimensions
    
    int cx = (int)rect_.size.width;
    if ((cx & (cx - 1)) != 0) {
        cx <<= 1;
        while ((cx & (cx - 1)) != 0)
            cx &= cx - 1;
    }        
    int cy = (int)rect_.size.height;
    if ((cy & (cy - 1)) != 0) {
        cy <<= 1;
        while ((cy & (cy - 1)) != 0)
            cy &= cy - 1;
    }
    
    // The simulator doesn't support PBuffers (yet). Use a regular texture since it's fast
    // enough on the simulator.
#if !defined(SIMULATOR)
    CALL_EAGL_FUNCTION(eaglCreatePBufferSurface, kEAGLPixelFormat_RGB565_D0, cx, cy, &surfacePBuffer_);
    CALL_EAGL_FUNCTION(eaglTexImagePBuffer, surfacePBuffer_);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cx, cy, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    CHECK_GL_ERROR();
#endif

    // The simulator doesn't support GL_TEXTURE_CROP_RECT_OES.
    // Use regular texture mapping instead.
#if !defined(SIMULATOR)
    // Set crop rectangle in texels (l, b, w, h)
    int rcCrop[4] = { 0, 0, (int)rect_.size.width, (int)rect_.size.height };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rcCrop);    
    CHECK_GL_ERROR();
#else
    // On the simulator, regular texture mapping is required
    glEnableClientState(GL_VERTEX_ARRAY);
    CHECK_GL_ERROR();                       
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    CHECK_GL_ERROR();   
#endif
    
    // Clear the color buffer    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR();

    // Allocate the memory OglDib uses
    int w = rect_.size.width;
    int h = rect_.size.height;
    int cb = 2 * w * h;
    pb_ = (unsigned char *)malloc(cb);
    memset(pb_, 0, cb);
    
    // Don't have palette yet
    havePalette_ = false;
}

- (void)frameStart {
    pbm_->ResetChanged();
}

- (void)frameComplete {
    if (!havePalette_)
        return;
    if (!pbm_->HasChanged())
        return;
    
#if !defined(SIMULATOR)
    // Draw texture and show on screen
    glDrawTexiOES(0, 0, 1, (int)rect_.size.width, (int)rect_.size.height);
    CHECK_GL_ERROR();
    CALL_EAGL_FUNCTION(eaglSwapBuffers, surface_);
#else
    // Draw texture.
    GLfloat coords[] = {
        0.0f, 0.0f,
        320.0f / 512.0f, 0.0f,
        0.0f, 480.0f / 512.0f,
        320.0f / 512.0f, 480.0f / 512.0f
    };
    
    GLfloat verts[] = {
        0.0f, 480.0f, 1.0f,
        320.0f, 480.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        320.0f, 0.0f, 1.0f
    };

    glVertexPointer(3, GL_FLOAT, 0, verts);
    CHECK_GL_ERROR();                       
    glTexCoordPointer(2, GL_FLOAT, 0, coords);
    CHECK_GL_ERROR();                       
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL_ERROR();
    CALL_EAGL_FUNCTION(eaglSwapBuffers, surface_);    
#endif
}

- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation width:(int)cx height:(int)cy {
    // This is called from the game thread. initGraphics here because the
    // game thread will be doing the rendering.

    [self initGraphics];
    
    pbm_ = wi::CreateOglDib(pb_, cx, cy, nDegreeOrientation);
    return pbm_;
}

- (void)setPalette:(wi::Palette *)ppal {
    if (pbm_ != NULL) {
        // 8->565 mapping table. OpenGL ES understands 565 format.
        word mp8bpp16bpp[256];                
        for (int n = 0; n < BigWord(ppal->cEntries); n++) {
            mp8bpp16bpp[n] = ((ppal->argb[n][0] << 8) & 0xf800) | ((ppal->argb[n][1] << 3) & 0x07e0) | ((ppal->argb[n][2] >> 3) & 0x001f);
        }
        pbm_->SetPalette(mp8bpp16bpp);
        havePalette_ = true;
    }
}
@end

#endif // 0