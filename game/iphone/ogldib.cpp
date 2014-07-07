#include "ogldib.h"
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>


namespace wi {
    
OglDib *CreateOglDib(byte *pb, const SurfaceProperties *pprops, int cx,
        int cy, int nDegreeOrientation)
{
	OglDib *pbm = new OglDib;
	if (pbm == NULL)
		return NULL;
	if (!pbm->Init(pb, pprops, cx, cy, nDegreeOrientation)) {
		delete pbm;
		return NULL;
	}
	return pbm;
}

void OglDib::Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst)
{
    // The overall approach is to blt to a holding buffer, and then upload
    // this via glTexSubImage2D to the texture. This api assumes the texture
    // it is being sent is sequential. It assumes lines start on a given
    // byte boundary (default 4 byte, which is what this code assumes).
    // So, 8 bit data -> converted to 16 bit data in holding buffer ->
    // uploaded to OpenGL ES.
    
    // Get src dib dimensions
    
    Size sizSrc;
    pbmSrc->GetSize(&sizSrc);
    
    // Clip to source rect
    // NOTE: Not exactly good form to change the caller's rect.
    
    if (prcSrc->left < 0)
        prcSrc->left = 0;
    if (prcSrc->top < 0)
        prcSrc->top = 0;
    if (prcSrc->right > sizSrc.cx)
        prcSrc->right = sizSrc.cx;
    if (prcSrc->bottom > sizSrc.cy)
        prcSrc->bottom = sizSrc.cy;
    
    // Clip to dest
    
    if (xDst < 0) {
        prcSrc->left -= xDst;
        xDst = 0;
    }
    if (yDst < 0) {
        prcSrc->top -= yDst;
        yDst = 0;
    }
    int xRightDst = xDst + prcSrc->Width();
    if (xRightDst > m_siz.cx)
        prcSrc->right -= xRightDst - m_siz.cx;
    int yBottomDst = yDst + prcSrc->Height();
    if (yBottomDst > m_siz.cy)
        prcSrc->bottom -= yBottomDst - m_siz.cy;
    
    // Anything to blt?
    
    if (prcSrc->IsEmpty())
        return;
    
    if (palette_ == NULL)
        return;
    
    // Remember that bits have changed
    
    changed_ = true;
    
    // Simulator does portrait mode for now, due to how the simulator
    // handles rotation.
#if defined(SIMULATOR)
    // Fill the dest in backwards because gl texture coordinates have 0,0
    // in the lower left.
    
    word *pwDst = (word *)m_pb;
    int cpDstStride = ((prcSrc->Width() + 1) & ~1) - prcSrc->Width();
    
    byte *pbSrc = pbmSrc->GetBits() + (prcSrc->bottom - 1) * sizSrc.cx + prcSrc->left;
    int cpSrcStride = -prcSrc->Width() - sizSrc.cx;
    word *mp8bpp16bpp = (word *)palette_;
    
    for (int y = prcSrc->top; y < prcSrc->bottom; y++) {
        for (int x = prcSrc->left; x < prcSrc->right; x++) {
            *pwDst++ = mp8bpp16bpp[*pbSrc++];
        }
        pbSrc += cpSrcStride;
        pwDst += cpDstStride;
    }
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, xDst, m_siz.cy - yDst - prcSrc->Height(),
                    prcSrc->Width(), prcSrc->Height(),
                    GL_RGB, GL_UNSIGNED_SHORT_5_6_5, m_pb);
#endif
    
    // Real device does landscape mode because it is better, with the
    // same orientation the youtube player uses, which is button on the
    // right.
#if !defined(SIMULATOR)
    // Fill the dest in backwards because gl texture coordinates have 0,0
    // in the lower left.
    
    word *pwDst = (word *)m_pb;
    int cpDstStride = ((prcSrc->Height() + 1) & ~1) - prcSrc->Height();
    
    byte *pbSrc = pbmSrc->GetBits() + (prcSrc->bottom - 1) * sizSrc.cx + prcSrc->right - 1;
    int cpSrcStride = prcSrc->Height() * sizSrc.cx - 1;
    word *palette = (word *)palette_;
    
    for (int x = prcSrc->left; x < prcSrc->right; x++) {
        for (int y = prcSrc->top; y < prcSrc->bottom; y++) {
            *pwDst++ = palette[*pbSrc];
            pbSrc -= sizSrc.cx;
        }
        pbSrc += cpSrcStride;
        pwDst += cpDstStride;
    }
    
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    m_siz.cy - (yDst + prcSrc->Height()),
                    m_siz.cx - (xDst + prcSrc->Width()),
                    prcSrc->Height(), prcSrc->Width(),
                    GL_RGB, GL_UNSIGNED_SHORT_5_6_5, m_pb);
#endif
}
    
} // namespace wi    
