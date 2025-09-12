#include <bits/stdc++.h>
#include <vector>
#define EPS 1e-9

struct point{
    double x, y;
    point() : x(0), y(0) {}
    point(double x0, double y0) : x(x0), y(y0){}

    point& operator+=(point o){
        x += o.x;
        y += o.y;
        return *this;
    }
    point& operator-=(point o){
        x -= o.x;
        y -= o.y;
        return *this;
    }
    point& operator*=(double o){
        x *= o;
        y *= o;
        return *this;
    }
    point& operator/=(double o){
        double div = 1.0/o;
        x /= div;
        y /= div;
        return *this;
    }

    bool operator==(point o){
        return (fabs(x-o.x) < EPS && fabs(y-o.y) < EPS);
    }
};

inline point operator-(point p){
    return point(-p.x,-p.y);
}

inline point operator-(point p, point q){
    return point(p.x-q.x,p.y-q.y);
}

inline point operator+(point p, point q){
    return point(p.x+q.x,p.y+q.y);
}

inline point operator*(point p, double esc){
    return point(p.x*esc,p.y*esc);
}

inline point operator*(double esc, point p){
    return point(p.x*esc,p.y*esc);
}

inline point operator/(point p, double esc){
    esc = 1.0/esc;
    return p*esc;
}

inline double cross(point p, point q){
    return p.x*q.y-q.x*p.y;
}

inline double dot(point p, point q){
    return p.x*q.x+p.y*q.y;
}

inline double sqnorm(point p){
    return dot(p,p);
}

inline double dist(point p, point q){
    return sqrt(sqnorm(p-q));
}

inline bool ccw(point p, point q, point r){
    return cross(p-q,p-r) > 0;
}

inline bool ccw(point p, point q, point r, bool ref){
    return ref ? (cross(p-q,p-r) > -EPS) : (cross(p-q, p-r) < EPS);
} 
