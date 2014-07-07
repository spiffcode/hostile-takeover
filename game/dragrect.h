#ifndef __DRAGRECT_H__
#define __DRAGRECT_H__

#include "inc/basictypes.h"
#include "game/vec2d.h"

namespace wi {

// pt0 is lower left, and points go ccw

class Rect;
class DragRect { // drc
public:
    void Init(const DPoint& pt0, const DPoint& pt1, const DPoint& pt2);
    void GetPoints(DPoint *apt) const;
    dword HitTest(const DPoint& pt, Vec2d *pvOffset = NULL) const;
    void TrackPoints(dword maskA, const DPoint& ptA, dword maskB,
            const DPoint& ptB);
    void GetBoundingRect(Rect *prc) const;
    bool PtIn(const DPoint& pt) const;
    void ScrollExpand(dword maskA, dword maskB, double dx, double dy);

private:
    void TrackPoint0(const DPoint& pt0);
    void TrackPoint1(const DPoint& pt1);
    void TrackPoint2(const DPoint& pt2);
    void TrackPoint3(const DPoint& pt3);
    void TrackPoints01(const DPoint& pt0, const DPoint& pt1);
    void TrackPoints02(const DPoint& pt0, const DPoint& pt2);
    void TrackPoints03(const DPoint& pt0, const DPoint& pt3);
    void TrackPoints12(const DPoint& pt1, const DPoint& pt2);
    void TrackPoints13(const DPoint& pt1, const DPoint& pt3);
    void TrackPoints23(const DPoint& pt2, const DPoint& pt3);

    DPoint pt0_;
    Vec2d v01_;
    Vec2d v02_;
};

} // namespace wi

#endif // __DRAGRECT_H__
