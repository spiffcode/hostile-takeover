//
//  oglview2.m
//  wi
//
//  Created by Scott Ludwig on 5/22/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "oglview2.h"

@implementation OglView2

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (id)initWithFrame:(struct CGRect)rect
{
    self = [super initWithFrame: rect];
    if (self == nil)
        return nil;
    
    rect_ = rect;
    pb_ = NULL;
    tex_ = 0;
    framebufferTex_ = 0;
    renderbuffer_ = 0;
    framebufferRender_ = 0;
    context_ = NULL;
    
    [self initLayer];
    return self;
}

- (void)dealloc {
	[super dealloc];
    [self cleanupGraphics];
}


- (void)initLayer
{
    // The basic approach is to update an ogl texture from back buffer +
    // update map, then texture map that into a render buffer, then show
    // that in this view.
    //
    // The contentious part is updating the ogl texture. It is regularly a
    // very slow process, however when a texture is set up to be a rendering
    // destination, updating the texture with glTexSubImage2D is much faster,
    // because the texture bits are stored more efficiently for updating this
    // way.
    //
    // Two framebuffer objects are created. One with the texture in the color
    // buffer attach point, one with the destination renderbuffer in the color
    // buffer attach point. For each frame, the texture is updated, then
    // texture mapped into the render buffer, then the render buffer is shown
    // on screen.
    
    // Clear before drawing set to NO is a little faster. The game doesn't
    // use a drawing context (not sure if this matters for CAEAGLLayer).
    self.clearsContextBeforeDrawing = NO;
        
    // Setting the view as opaque means the OS doesn't need to working
    // about compositing layers underneath.
    
    CAEAGLLayer *layer = (CAEAGLLayer *)self.layer;
    layer.opaque = YES;
    
    // No retained backing, and 565 format
    layer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithBool:FALSE],
            kEAGLDrawablePropertyRetainedBacking,
            kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat,
            (char *)NULL];

    // Create and initialize an EAGL Context
    context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    [EAGLContext setCurrentContext:context_];
    
    // Now create the render buffer, use the CAEAGLLayer for storage, and
    // attach it to another framebuffer.
    glGenRenderbuffersOES(1, &renderbuffer_);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, renderbuffer_);
	[context_ renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];    
    
    // Bind the renderbuffer to a framebuffer object's color
    // buffer attach point. This will be the rendering destination.
    glGenFramebuffersOES(1, &framebufferRender_);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebufferRender_);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, renderbuffer_);    
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, renderbuffer_);
    
    // Create a texture and framebuffer. Attach texture to framebuffer.
    // This is done so that the texture is internally stored in a format
    // that makes glTexSubImage2D() fast (otherwise it is really slow).
    
    // Texture must be power of 2 dimensions
    int cxTex = (int)rect_.size.width;
    if ((cxTex & (cxTex - 1)) != 0) {
        cxTex <<= 1;
        while ((cxTex & (cxTex - 1)) != 0)
            cxTex &= cxTex - 1;
    }        
    int cyTex = (int)rect_.size.height;
    if ((cyTex & (cyTex - 1)) != 0) {
        cyTex <<= 1;
        while ((cyTex & (cyTex - 1)) != 0)
            cyTex &= cyTex - 1;
    }
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &tex_);    
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cxTex, cyTex, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    glGenFramebuffersOES(1, &framebufferTex_);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebufferTex_);
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, tex_, 0);
    
    // Bind framebufferRender_ and tex_ here, since this is what
    // draw code needs.
    glBindTexture(GL_TEXTURE_2D, tex_);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebufferRender_);    
    
    // Perform the rest of misc gl initialization.    
    glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, rect_.size.width, rect_.size.height);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glOrthof(0.0f, rect_.size.width, rect_.size.height, 0.0f, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Texture map parameters
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // Pre-initialize so this isn't done during drawing
    coords_[0] = 0.0f;
    coords_[1] = 0.0f;
    
    coords_[2] = rect_.size.width / cxTex;
    coords_[3] = 0.0f;
    
    coords_[4] = 0.0f;
    coords_[5] = rect_.size.height / cyTex;
    
    coords_[6] = rect_.size.width / cxTex;
    coords_[7] = rect_.size.height / cyTex;

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, coords_);    
    
    verts_[0] = 0.0f;
    verts_[1] = rect_.size.height;
    verts_[2] = 1.0f;
    
    verts_[3] = rect_.size.width;
    verts_[4] = rect_.size.height;
    verts_[5] = 1.0f;
    
    verts_[6] = 0.0f;
    verts_[7] = 0.0f;
    verts_[8] = 1.0f;
    
    verts_[9] = rect_.size.width;
    verts_[10] = 0.0f;
    verts_[11] = 1.0f;

    glEnableClientState(GL_VERTEX_ARRAY);    
    glVertexPointer(3, GL_FLOAT, 0, verts_);

    // Clear the color buffer    
    glClear(GL_COLOR_BUFFER_BIT);
        
    // Don't have palette yet
    havePalette_ = false;    
}

- (void)initGraphics {
    // NOTE: initGraphics is called from the game thread
    [EAGLContext setCurrentContext:context_];
}

- (void)cleanupGraphics {
    // NOTE: called on game thread
    
    // Detach texture from framebufferTex_, unbind and delete both
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebufferTex_);            
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, 0, GL_TEXTURE_2D, 0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &tex_);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
    glDeleteFramebuffersOES(1, &framebufferTex_);
    
    // Detach renderbuffer from framebufferRender_, unbind and delete both
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebufferRender_);        
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, 0);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);        
    glDeleteRenderbuffersOES(1, &framebufferRender_);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);    
    glDeleteFramebuffersOES(1, &framebufferRender_);

    [EAGLContext setCurrentContext:NULL];
    [context_ release];
    
    if (pb_ != NULL) {
        free(pb_);
        pb_ = NULL;
    }
    
    tex_ = 0;
    framebufferTex_ = 0;
    renderbuffer_ = 0;
    framebufferRender_ = 0;
    context_ = NULL;
}

- (void)frameStart {
    pbm_->ResetChanged();
}

- (void)frameComplete {
    if (!havePalette_)
        return;
    if (!pbm_->HasChanged())
        return;
    
    // Draw texture.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    [context_ presentRenderbuffer:renderbuffer_];
}

- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation width:(int)cx height:(int)cy {
    // This is called from the game thread. initGraphics here because the
    // game thread will be doing the rendering.

    [self initGraphics];

    // Allocate the memory OglDib uses
    int cb = 2 * cx * cy;
    pb_ = (unsigned char *)malloc(cb);
    memset(pb_, 0, cb);
    wi::SurfaceProperties props;
    [self getSurfaceProperties:&props];
    pbm_ = wi::CreateOglDib(pb_, &props, cx, cy, nDegreeOrientation);
    return pbm_;
}

- (void)setPalette:(wi::Palette *)ppal {
    if (pbm_ != NULL) {
        // 8->565 mapping table. OpenGL ES understands 565 format.
        word mp8bpp16bpp[256];                
        for (int n = 0; n < BigWord(ppal->cEntries); n++) {
            mp8bpp16bpp[n] = ((ppal->argb[n][0] << 8) & 0xf800) | ((ppal->argb[n][1] << 3) & 0x07e0) | ((ppal->argb[n][2] >> 3) & 0x001f);
        }
        pbm_->SetPalette(mp8bpp16bpp, sizeof(mp8bpp16bpp));
        havePalette_ = true;
    }
}
@end
