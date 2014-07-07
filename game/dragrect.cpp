#include "game/dragrect.h"
#include "game/ht.h"

namespace wi {

void DragRect::Init(const DPoint& pt0, const DPoint& pt1, const DPoint& pt2) {
    pt0_ = pt0;
    v01_ = Vec2d(pt0, pt1);
    v02_ = Vec2d(pt0, pt2);
}

void DragRect::TrackPoints01(const DPoint& pt0, const DPoint& pt1) {
    // width variable, height constant, theta variable
    double dy = v01_.flip90().project(v02_);
    pt0_ = pt0;
    v01_ = Vec2d(pt0, pt1);
    v02_ = Vec2d(pt0, v01_.flip90().unit().scale(dy).add(pt1));
}

void DragRect::TrackPoints02(const DPoint& pt0, const DPoint& pt2) {
    // width variable, height variable, theta constant
    double fdy = v02_.project(v01_) / v02_.mag();
    double fdx = Vec2d(v02_.unit().scale(v02_.mag() * fdy).add(pt0_),
            v01_.add(pt0_)).mag() / v02_.mag();
    v02_ = Vec2d(pt0, pt2);
    DPoint ptT = v02_.unit().scale(v02_.mag() * fdy).add(pt0);
    DPoint pt1 = v02_.flip270().unit().scale(v02_.mag() * fdx).add(ptT);
    v01_ = Vec2d(pt0, pt1);
    pt0_ = pt0;
}

void DragRect::TrackPoints03(const DPoint& pt0, const DPoint& pt3) {
    // width constant, height variable, theta variable
    Vec2d v03(pt0, pt3);
    DPoint pt1 = v03.flip270().unit().scale(v01_.mag()).add(pt0);
    pt0_ = pt0;
    v01_ = Vec2d(pt0, pt1);
    v02_ = Vec2d(pt0, v03.add(pt1));
}

void DragRect::TrackPoints12(const DPoint& pt1, const DPoint& pt2) {
    // width constant, height variable, theta variable
    Vec2d v12(pt1, pt2);
    DPoint pt0 = v12.flip90().unit().scale(v01_.mag()).add(pt1);
    pt0_ = pt0;
    v01_ = Vec2d(pt0, pt1);
    v02_ = Vec2d(pt0, pt2);
}

void DragRect::TrackPoints13(const DPoint& pt1, const DPoint& pt3) {
    // width variable, height variable, theta constant
    double fdy = v02_.project(v01_) / v02_.mag();
    double fdx = Vec2d(v02_.unit().scale(v02_.mag() * fdy).add(pt0_),
            v01_.add(pt0_)).mag() / v02_.mag();
    Vec2d v13(pt1, pt3);
    DPoint ptT = v13.unit().scale(v13.mag() * fdy).add(pt1);
    DPoint pt0 = v13.flip90().unit().scale(v13.mag() * fdx).add(ptT);
    pt0_ = pt0;
    v01_ = Vec2d(pt0, pt1);
    v02_ = Vec2d(pt0, v01_.add(pt3));
}

void DragRect::TrackPoints23(const DPoint& pt2, const DPoint& pt3) {
    // width variable, height constant, theta variable
    double dy = v01_.flip90().project(v02_);
    Vec2d v32(pt3, pt2);
    pt0_ = v32.flip270().unit().scale(dy).add(pt3);
    v01_ = Vec2d(pt0_, v32.add(pt0_));
    v02_ = Vec2d(pt0_, pt2);
}

void DragRect::TrackPoint0(const DPoint& pt0) {
    // pt2 fixed, v01 direction locked
    v02_ = Vec2d(pt0, v02_.add(pt0_));
    v01_ = v01_.unit().scale(v01_.project(v02_));
    pt0_ = pt0;
}

void DragRect::TrackPoint1(const DPoint& pt1) {
    // pt3 fixed, v01 direction locked
    DPoint pt3 = v01_.flip180().add(v02_.add(pt0_));
    v01_ = v01_.unit().scale(-v01_.project(Vec2d(pt1, pt3)));
    pt0_ = v01_.flip180().add(pt1);
    v02_ = Vec2d(pt0_, v01_.add(pt3));
}

void DragRect::TrackPoint2(const DPoint& pt2) {
    // pt0 fixed, v01 direction locked
    v02_ = Vec2d(pt0_, pt2);
    v01_ = v01_.unit().scale(v01_.project(v02_));
}

void DragRect::TrackPoint3(const DPoint& pt3) {
    // pt1 fixed, v01 direction locked
    DPoint pt1 = v01_.add(pt0_);
    Vec2d v13(pt1, pt3);
    v01_ = v01_.unit().scale(-v01_.project(v13));
    pt0_ = v01_.flip180().add(pt1);
    v02_ = Vec2d(pt0_, v01_.add(pt3));
}

void DragRect::GetPoints(DPoint *apt) const {
    apt[0] = pt0_;
    apt[1] = v01_.add(pt0_);
    apt[2] = v02_.add(pt0_);
    apt[3] = v01_.flip180().add(apt[2]);
}

dword DragRect::HitTest(const DPoint& pt, Vec2d *pvOffset) const {
    DPoint apt[4];
    GetPoints(apt);

    int iBest = -1;
    double magBest = MAXFLOAT;
    for (int i = 0; i < ARRAYSIZE(apt); i++) {
        double mag = Vec2d(pt, apt[i]).mag();
        if (mag < magBest) {
            magBest = mag;
            iBest = i;
        }
    }

    if (pvOffset != NULL) {
        *pvOffset = Vec2d(pt, apt[iBest]);
    }

    // Use a mask in case this will reflect edge and corner hittesting
    // in the future.
    return 1 << iBest;
}

void DragRect::GetBoundingRect(Rect *prc) const {
    prc->left = 9999;
    prc->right = -1;
    prc->top = 9999;
    prc->bottom = -1;

    DPoint apt[4];
    GetPoints(apt);

    for (int i = 0; i < ARRAYSIZE(apt); i++) {
        if (apt[i].x < prc->left) {
            prc->left = apt[i].x;
        }
        if (apt[i].x > prc->right) {
            prc->right = apt[i].x;
        }
        if (apt[i].y < prc->top) {
            prc->top = apt[i].y;
        }
        if (apt[i].y > prc->bottom) {
            prc->bottom = apt[i].y;
        }
    }
}

bool DragRect::PtIn(const DPoint& pt) const {
    // Look at the z portion of the cross product on each side. The result
    // should be all negative or positive to be inside.

    DPoint apt[4];
    GetPoints(apt);

    double d0 = (pt.x - apt[0].x) * (apt[1].y - apt[0].y) -
            (pt.y - apt[0].y) * (apt[1].x - apt[0].x);
    double d1 = (pt.x - apt[1].x) * (apt[2].y - apt[1].y) -
            (pt.y - apt[1].y) * (apt[2].x - apt[1].x);
    double d2 = (pt.x - apt[2].x) * (apt[3].y - apt[2].y) -
            (pt.y - apt[2].y) * (apt[3].x - apt[2].x);
    double d3 = (pt.x - apt[3].x) * (apt[0].y - apt[3].y) -
            (pt.y - apt[3].y) * (apt[0].x - apt[3].x);

    if (d0 < 0.0 && d1 < 0.0 && d2 < 0.0 && d3 < 0.0) {
        return true;
    }
    if (d0 >= 0.0 && d1 >= 0.0 && d2 >= 0.0 && d3 >= 0.0) {
        return true;
    }

    return false;
}

void DragRect::ScrollExpand(dword maskA, dword maskB, double dx, double dy) {
    // This only applies when one finger is down. Imagine the map being
    // scrolled by dx, dy, and the scroll rect expanding. Basically, the
    // fixed point of the rect scrolls in the dx, dy direction.

    if (maskA != 0 && maskB != 0) {
        return;
    }

    // Simply "scroll" the fixed point

    DPoint pt;
    DPoint apt[4];
    GetPoints(apt);
    switch (maskA | maskB) {
    case 1:
        pt = apt[2];
        pt.x += dx;
        pt.y += dy;
        TrackPoint2(pt);
        break;
    case 2:
        pt = apt[3];
        pt.x += dx;
        pt.y += dy;
        TrackPoint3(pt);
        break;
    case 4:
        pt = apt[0];
        pt.x += dx;
        pt.y += dy;
        TrackPoint0(pt);
        break;
    case 8:
        pt = apt[1];
        pt.x += dx;
        pt.y += dy;
        TrackPoint1(pt);
        break;
    }
} 

void DragRect::TrackPoints(dword maskA, const DPoint& ptA,
        dword maskB, const DPoint& ptB) {
    if (maskA == 0) {
        switch (maskB) {
        case 1:
            TrackPoint0(ptB);
            break;
        case 2:
            TrackPoint1(ptB);
            break;
        case 4:
            TrackPoint2(ptB);
            break;
        case 8:
            TrackPoint3(ptB);
            break;
        default:
            break;
        }
    } else if (maskA == 1) {
        switch (maskB) {
        case 0:
            TrackPoint0(ptA);
            break;
        case 1:
            TrackPoint0(ptA);
            break;
        case 2:
            TrackPoints01(ptA, ptB);
            break;
        case 4:
            TrackPoints02(ptA, ptB);
            break;
        case 8:
            TrackPoints03(ptA, ptB);
            break;
        default:
            break;
        }
    } else if (maskA == 2) {
        switch (maskB) {
        case 0:
            TrackPoint1(ptA);
            break;
        case 1:
            TrackPoints01(ptB, ptA);
            break;
        case 2:
            TrackPoint2(ptA);
            break;
        case 4:
            TrackPoints12(ptA, ptB);
            break;
        case 8:
            TrackPoints13(ptA, ptB);
            break;
        default:
            break;
        }
    } else if (maskA == 4) {
        switch (maskB) {
        case 0:
            TrackPoint2(ptA);
            break;
        case 1:
            TrackPoints02(ptB, ptA);
            break;
        case 2:
            TrackPoints12(ptB, ptA);
            break;
        case 4:
            TrackPoint2(ptA);
            break;
        case 8:
            TrackPoints23(ptA, ptB);
            break;
        default:
            break;
        }
    } else if (maskA == 8) {
        switch (maskB) {
        case 0:
            TrackPoint3(ptA);
            break;
        case 1:
            TrackPoints03(ptB, ptA);
            break;
        case 2:
            TrackPoints13(ptB, ptA);
            break;
        case 4:
            TrackPoints23(ptB, ptA);
            break;
        case 8:
            TrackPoint3(ptA);
            break;
        default:
            break;
        }
    }
}

} // namespace wi
