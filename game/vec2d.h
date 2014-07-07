#ifndef __VEC2D_H__
#define __VEC2D_H__

#include <math.h>

namespace wi {

struct DPoint {
    double x;
    double y;
};

class Vec2d {
public:
    Vec2d() {
        dx = 0;
        dy = 0;
    }

    Vec2d(const DPoint& pt0, const DPoint& pt1) {
        dx = pt1.x - pt0.x;
        dy = pt1.y - pt0.y;
    }

    Vec2d(double dx, double dy) {
        this->dx = dx;
        this->dy = dy;
    }

    Vec2d(const Vec2d& v) {
        dx = v.dx;
        dy = v.dy;
    }

    Vec2d unit() const {
        double magT = mag();
        return Vec2d(dx / magT, dy / magT);
    }

    double mag() const {
        return sqrt(dx * dx + dy * dy);
    }

    Vec2d scale(double amount) const {
        return Vec2d(dx * amount, dy * amount);
    }

    DPoint add(const DPoint& pt) const {
        DPoint ptT;
        ptT.x = pt.x + dx;
        ptT.y = pt.y + dy;
        return ptT;
    }

    double dot(const Vec2d& v) const {
        return dx * v.dx + dy * v.dy;
    }
    
    double project(const Vec2d& v) const {
        return unit().dot(v);
    }

    Vec2d flip90() const {
        return Vec2d(-dy, dx);
    }

    Vec2d flip180() const {
        return Vec2d(-dx, -dy);
    }

    Vec2d flip270() const {
        return Vec2d(dy, -dx);
    }

    double dx;
    double dy;
};

} // namespace wi

#endif // __VEC2D_H__
