#ifndef __CGDIB_H__
#define __CGDIB_H__

#include "../ht.h"

namespace wi {
    
//#define k555    
    
class CgDib : public DibBitmap
{
public:
    CgDib() : palette_(NULL),changed_(false) { }
	virtual ~CgDib() { if (palette_ != NULL) free(palette_); }
    
	virtual bool Init(byte *pb, const SurfaceProperties *pprops, int cx,
            int cy, int nDegreeOrientation);
    virtual void SetPalette(void *palette, int cb);
	virtual void Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst);
    virtual void Scroll(Rect *prcSrc, int xDst, int yDst);

    bool HasChanged() { return changed_; }
    void ResetChanged() { changed_ = false; }
    
protected:
    void LeftToRightBlt(dword *pdwSrc, dword *pdwDst, int cx, int cy);
    void RightToLeftBlt(dword *pdwSrc, dword *pdwDst, int cx, int cy);
    void FastIntraBlt(Rect *prcSrc, int xDst, int yDst);
    
    bool changed_;
    void *palette_;
	long m_cbOffOrigin;
	int m_cbxPitch;
	int m_cbyPitch;
    int m_nDegreeOrientation;
    SurfaceProperties m_props;    
};
CgDib *CreateCgDib(byte *pb, const SurfaceProperties *pprops, int cx, int cy,
        int nDegreeOrientation);

} // namespace wi

#endif // __CGDIB_H__
