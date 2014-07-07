#ifndef __OGLDIB_H__
#define __OGLDIB_H__

#include "../ht.h"
#include "cgdib.h"

namespace wi {
    
class OglDib : public CgDib
{
public:
    virtual void Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst);
    
    bool HasChanged() { return changed_; }
    void ResetChanged() { changed_ = false; }
    
private:
    bool changed_;
};
OglDib *CreateOglDib(byte *pb, const SurfaceProperties *pprops, int cx,
        int cy, int nDegreeOrientation);
    
} // namespace wi
    
#endif // __OGLDIB_H__
