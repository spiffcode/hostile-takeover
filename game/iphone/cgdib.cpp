#include "cgdib.h"

namespace wi {
    
CgDib *CreateCgDib(byte *pb, const SurfaceProperties *pprops, int cx, int cy,
        int nDegreeOrientation)
{
    CgDib *pbm = new CgDib;
    if (pbm == NULL)
        return NULL;
    if (!pbm->Init(pb, pprops, cx, cy, nDegreeOrientation)) {
        delete pbm;
        return NULL;
    }
    return pbm;
}

bool CgDib::Init(byte *pb, const SurfaceProperties *pprops, int cx, int cy,
        int nDegreeOrientation)
{    
    m_props = *pprops;
    switch (nDegreeOrientation) {
    case 0:
        // Standard portrait, button on bottom
        
        m_cbOffOrigin = 0;
        m_cbxPitch = m_props.cbxPitch;
        m_cbyPitch = m_props.cbyPitch;
        break;
        
    case 90:
        // button on left
        
        m_cbOffOrigin = m_props.cbyPitch * (m_props.cyHeight - 1);
        m_cbxPitch = -m_props.cbyPitch;
        m_cbyPitch = m_props.cbxPitch;
        break;
        
    case 180:
        // button on top
        
        m_cbOffOrigin = m_props.cbyPitch * (m_props.cyHeight - 1) + m_props.cbxPitch * (m_props.cxWidth - 1);
        m_cbxPitch = -m_props.cbxPitch;
        m_cbyPitch = -m_props.cbyPitch;
        break;
        
    case 270:
        // button on right
        
        m_cbOffOrigin = m_props.cbxPitch * (m_props.cxWidth - 1);
        m_cbxPitch = m_props.cbyPitch;
        m_cbyPitch = -m_props.cbxPitch;
        break;
        
    default:
        return false;
    }
    
    m_nDegreeOrientation = nDegreeOrientation;
    m_pb = pb;
    m_cbRow = m_cbxPitch * cx;
    m_siz.cx = cx;
    m_siz.cy = cy;

    //temp
    //m_wf |= kfDibWantScrolls;
    
    return true;
}

void CgDib::SetPalette(void *palette, int cb)
{
    if (palette_ != NULL)
        free(palette_);
    palette_ = malloc(cb);
    memcpy((byte *)palette_, (byte *)palette, cb);
}

void CgDib::Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst)
{
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
    
    int xRight = _min(prcSrc->right, m_siz.cx);
    int yBottom = _min(prcSrc->bottom, m_siz.cy);
    byte *pbSrc = pbmSrc->GetBits() + (prcSrc->top * m_siz.cx) + prcSrc->left;
    int cbDstInset = xDst * m_cbxPitch;
    int cbSrcStride = m_siz.cx - prcSrc->Width();
    
    byte *pbScreen = m_pb;
    if (pbScreen == NULL || palette_ == NULL)
        return;
    
    changed_ = true;
    
    pbScreen += m_cbOffOrigin;
    
#ifdef k555
    word *pwDst = (word*)(pbScreen + (yDst * m_cbyPitch));
    word *palette = (word *)palette_;
    
    for (int y = prcSrc->top; y < yBottom; y++) {
        word *pwDstT = (word*)(((byte*)pwDst) + cbDstInset);
        for (int x = prcSrc->left; x < xRight; x++) {
            *pwDstT = palette[*pbSrc++];
            pwDstT = (word*)(((byte*)pwDstT) + m_cbxPitch);
        }
        pbSrc += cbSrcStride;
        pwDst = (word*)(((byte*)pwDst) + m_cbyPitch);
    }
#else
    dword *pdwDst = (dword *)(pbScreen + (yDst * m_cbyPitch));
    dword *palette = (dword *)palette_;
    
    for (int y = prcSrc->top; y < yBottom; y++) {
        dword *pdwDstT = (dword *)(((byte *)pdwDst) + cbDstInset);
        for (int x = prcSrc->left; x < xRight; x++) {
            *pdwDstT = palette[*pbSrc++];
            pdwDstT = (dword *)(((byte *)pdwDstT) + m_cbxPitch);
        }
        pbSrc += cbSrcStride;
        pdwDst = (dword *)(((byte *)pdwDst) + m_cbyPitch);
    }
#endif
}

#define TopToBottomBlt LeftToRightBlt
#define FastestBlt LeftToRightBlt
#define BottomToTopBlt RightToLeftBlt

void CgDib::Scroll(Rect *prcSrc, int xDst, int yDst)
{
    // Rotate the coordinates as appropriate and call FastIntraBlt()
    
	int xDstT, yDstT;
	Rect rcT;
	switch (m_nDegreeOrientation) {
    case 0:
        rcT = *prcSrc;
        xDstT = xDst;
        yDstT = yDst;
        break;
       
#if 0 
// these two need a little help if they are ever used.
    case 90:
        rcT.left = prcSrc->top;
        rcT.top = m_siz.cy - (prcSrc->left + prcSrc->Width());
        rcT.right = rcT.left + prcSrc->Height();
        rcT.bottom = rcT.top + prcSrc->Width();
        xDstT = yDst;
        yDstT = m_siz.cy - (xDst + prcSrc->Width());
        break;
        
    case 180:
        rcT.left = m_siz.cx - (prcSrc->left + prcSrc->Width());
        rcT.top = m_siz.cy - (prcSrc->top + prcSrc->Height());
        rcT.right = rcT.left + prcSrc->Width();
        rcT.bottom = rcT.top + prcSrc->Height();
        xDstT = m_siz.cx - (xDst + prcSrc->Width());
        yDstT = m_siz.cy - (yDst + prcSrc->Height());
        break;
#endif
        
    case 270:
        rcT.left = m_siz.cy - prcSrc->bottom;
        rcT.top = prcSrc->left;
        rcT.right = rcT.left + prcSrc->Height();
        rcT.bottom = rcT.top + prcSrc->Width();
        xDstT = m_siz.cy - (yDst + prcSrc->Height());
        yDstT = xDst;
        break;
	}
    
    FastIntraBlt(&rcT, xDstT, yDstT);
}

void CgDib::FastIntraBlt(Rect *prcSrc, int xDst, int yDst)
{
    // This blt expects parameters relative to how the memory is physically
    // arranged.
    
	// Clip to source rect
    
	if (prcSrc->left < 0)
		prcSrc->left = 0;
	if (prcSrc->top < 0)
		prcSrc->top = 0;
	if (prcSrc->right > m_props.cxWidth)
		prcSrc->right = m_props.cxWidth;
	if (prcSrc->bottom > m_props.cyHeight)
		prcSrc->bottom = m_props.cyHeight;
    
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
	if (xRightDst > m_props.cxWidth)
		prcSrc->right -= xRightDst - m_props.cxWidth;
	int yBottomDst = yDst + prcSrc->Height();
	if (yBottomDst > m_props.cyHeight)
		prcSrc->bottom -= yBottomDst - m_props.cyHeight;
    
	// Anything to blt?
    
	if (prcSrc->IsEmpty())
		return;
    
    // Calc addresses
    
    int cbRow = m_props.cxWidth * 4;
    dword *pdwSrc = (dword *)(m_pb + (long)prcSrc->top * cbRow) + prcSrc->left;
    dword *pdwDst = (dword *)(m_pb + (long)yDst * cbRow) + xDst;
    
    // If same y, ...
    
    if (yDst == prcSrc->top) {
        // Overlap?
        
        int cxInterval = prcSrc->right - xDst;
        if (cxInterval > 0 && cxInterval < prcSrc->Width() * 2) {
            // Overlap. If bltting to the left, copy from left to right
            
            if (xDst < prcSrc->left) {
                LeftToRightBlt(pdwSrc, pdwDst, prcSrc->Width(), prcSrc->Height());
            } else {
                RightToLeftBlt(pdwSrc, pdwDst, prcSrc->Width(), prcSrc->Height());
            }
        } else {
            // No overlap. Do the fastest blt
            
            FastestBlt(pdwSrc, pdwDst, prcSrc->Width(), prcSrc->Height());
        }
    } else {
        int cyInterval = prcSrc->bottom - yDst;
        if (cyInterval > 0 && cyInterval < prcSrc->Height() * 2) {
            // Overlap. If bltting upwards, copy from top to bottom
            
            if (yDst < prcSrc->top) {
                TopToBottomBlt(pdwSrc, pdwDst, prcSrc->Width(), prcSrc->Height());
            } else {
                BottomToTopBlt(pdwSrc, pdwDst, prcSrc->Width(), prcSrc->Height());
            }
        } else {
            // No overlap. Do the fastest blt
            
            FastestBlt(pdwSrc, pdwDst, prcSrc->Width(), prcSrc->Height());
        }
    }
}

void CgDib::LeftToRightBlt(dword *pdwSrc, dword *pdwDst, int cx, int cy)
{
    int cdwReturn = m_props.cxWidth - cx;
    while (cy-- > 0) {
        int cdw = cx;
        while (cdw-- > 0)
            *pdwDst++ = *pdwSrc++;
        pdwSrc += cdwReturn;
        pdwDst += cdwReturn;
    }
}

void CgDib::RightToLeftBlt(dword *pdwSrc, dword *pdwDst, int cx, int cy)
{
    int cdwReturn = m_props.cxWidth - cx;
    pdwSrc += (cy - 1) * m_props.cxWidth + cx - 1;
    pdwDst += (cy - 1) * m_props.cxWidth + cx - 1;
    while (cy-- > 0) {
        int cdw = cx;
        while (cdw-- > 0)
            *pdwDst-- = *pdwSrc--;
        pdwSrc -= cdwReturn;
        pdwDst -= cdwReturn;
    }
}    

} // namespace wi
