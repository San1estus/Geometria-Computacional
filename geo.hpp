/*Header con todas las funciones basicas de Geometria*/
#ifndef GEO
#define GEO
#include <cmath>
#include <vector>
#include <iostream>
using namespace std;
#define EPS 1e-9
#define sz(a) (int)a.size()

enum orientacion{
    COLLINEAL = 0,
    LEFT = 1,
    RIGHT = 2
};

enum inOrOut{
    ON = 0,
    IN = 1,
    OUT = 2
};

struct Point{
    double x, y;
    Point() : x(0), y(0) {}
    Point(double x0, double y0) : x(x0), y(y0){}

    Point& operator+=(Point o){
        x += o.x;
        y += o.y;
        return *this;
    }
    Point& operator-=(Point o){
        x -= o.x;
        y -= o.y;
        return *this;
    }
    Point& operator*=(double o){
        x *= o;
        y *= o;
        return *this;
    }
    Point& operator/=(double o){
        double div = 1.0/o;
        x /= div;
        y /= div;
        return *this;
    }

    bool operator==(Point o){
        return (fabs(x-o.x) < EPS && fabs(y-o.y) < EPS);
    }
};

// Se usa para funciones que se usan de manera frecuente y son pequeñas para optimización
Point operator-(Point p){
    return Point(-p.x,-p.y);
}

Point operator-(Point p, Point q){
    return Point(p.x-q.x,p.y-q.y);
}

Point operator+(Point p, Point q){
    return Point(p.x+q.x,p.y+q.y);
}

Point operator*(Point p, double esc){
    return Point(p.x*esc,p.y*esc);
}

Point operator*(double esc, Point p){
    return Point(p.x*esc,p.y*esc);
}

Point operator/(Point p, double esc){
    esc = 1.0/esc;
    return p*esc;
}

bool operator<(Point p, Point q){
    return (p.x < q.x || (fabs((p.x - q.x)) < EPS && p.y < q.y));
}

double cross(Point p, Point q){
    return p.x*q.y-q.x*p.y;
}

double dot(Point p, Point q){
    return p.x*q.x+p.y*q.y;
}

double sqnorm(Point p){
    return dot(p,p);
}

double dist(Point p, Point q){
    return sqrt(sqnorm(p-q));
}

int orientation(Point p, Point q, Point r){ 
    double val = cross(p-r, q-r);
    if(fabs(val) < EPS) return COLLINEAL;
    return (val > 0 ? LEFT : RIGHT);
}

bool inBounds(Point p, Point q, Point r){
    return (r.x <= max(p.x, q.x) && r.x >= min(p.x, q.x) && r.y <= max(p.y, q.y) && r.y >= min(p.y, q.y));
}

bool intersect(Point p, Point q, Point r, Point l){
    int pqr = orientation(p, q, r);
    int pql = orientation(p, q, l);
    int rlp = orientation(r, l, p);
    int rlq = orientation(r, l, q);

    if(pqr != pql && rlp != rlq) return true;

    if(pqr == COLLINEAL && inBounds(p, q, r)) return true;
    if(pql == COLLINEAL && inBounds(p, q, l)) return true;
    if(rlp == COLLINEAL && inBounds(r, l, p)) return true;
    if(rlq == COLLINEAL && inBounds(r, l, q)) return true;

    return false;
}
 
double angle(Point p, Point q, Point r){
    Point qp = q-p, rp = r-p;

    double cos_angle = dot(qp, rp) / ((double)sqrt(sqnorm(qp)) * (double)sqrt(sqnorm(rp)));

    if (cos_angle > 1.0) cos_angle = 1.0;
    if (cos_angle < -1.0) cos_angle = -1.0;
    return acos(cos_angle);
}

bool isPointInPolygon(Point p, const vector<Point>& polygon){
    int n = sz(polygon);
    
    Point far = {1e9, p.y};

    int count = 0, i = 0;
    do {
    int next = (i + 1) % n;
    
    // Checamos las intersecciones del rayo con las aristas
    if (intersect(polygon[i], polygon[next], p, far)) {

        // Si interseca verificamos si es colineal, si es colineal verificamos si esta sobre la arista del poligono o no.
        if (orientation(polygon[i], p, polygon[next]) == COLLINEAL)
            return inBounds(polygon[i], p, polygon[next]);

      count++;
    }
    i = next;
  } while (i != 0);
    return count & 1;
}

void print(const vector<Point> &p){
    int n = (int)p.size();
    for(int i = 0; i < n; i++){
        cout << i+1 << ": " <<  p[i].x << ' ' << p[i].y << '\n';
    }
    cout << "\n\n";
}
#endif