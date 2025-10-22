/*Header con todas las funciones basicas de Geometria*/

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

// Se usa para funciones que se usan de manera frecuente y son pequeñas para optimización
point operator-(point p){
    return point(-p.x,-p.y);
}

point operator-(point p, point q){
    return point(p.x-q.x,p.y-q.y);
}

point operator+(point p, point q){
    return point(p.x+q.x,p.y+q.y);
}

point operator*(point p, double esc){
    return point(p.x*esc,p.y*esc);
}

point operator*(double esc, point p){
    return point(p.x*esc,p.y*esc);
}

point operator/(point p, double esc){
    esc = 1.0/esc;
    return p*esc;
}

bool operator<(point p, point q){
    return (p.x < q.x || (fabs((p.x - q.x)) < EPS && p.y < q.y));
}
double cross(point p, point q){
    return p.x*q.y-q.x*p.y;
}

double dot(point p, point q){
    return p.x*q.x+p.y*q.y;
}

double sqnorm(point p){
    return dot(p,p);
}

double dist(point p, point q){
    return sqrt(sqnorm(p-q));
}

int orientation(point p, point q, point r){ 
    double val = cross(p-r, q-r);
    if(fabs(val) < EPS) return COLLINEAL;
    return (val > 0 ? LEFT : RIGHT);
}

bool inBounds(point p, point q, point r){
    return (r.x <= max(p.x, q.x) && r.x >= min(p.x, q.x) && r.y <= max(p.y, q.y) && r.y >= min(p.y, q.y));
}


bool intersect(point p, point q, point r, point l){
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
 
// p es el punto del que se calcula el angulo
double angle(point p, point q, point r){
    point qp = q-p, rp = r-p;

    double cos_angle = dot(qp, rp) / ((double)sqrt(sqnorm(qp)) * (double)sqrt(sqnorm(rp)));

    if (cos_angle > 1.0) cos_angle = 1.0;
    if (cos_angle < -1.0) cos_angle = -1.0;
    return acos(cos_angle);
}



int insidePolygon(point p, const vector<point>& points){
    int n = sz(points);
    if (n < 3) return OUT;

    for(int i = 0; i < n; i++){
        point p1 = points[i];
        point p2 = points[(i + 1) % n];
        
        if(orientation(p1, p2, p) == COLLINEAL && inBounds(p1, p2, p)){
            return ON;
        }
    }

    double sum = 0.0;
    for(int i = 0; i < n; i++){
        point p1 = points[i];
        point p2 = points[(i + 1) % n];

        double ang = angle(p, p1, p2); 

        if(orientation(p, p1, p2) == LEFT){ 
            sum += ang; 
        } else {
            sum -= ang;
        }
    }

    double TWO_PI = 2.0 * acos(-1.0);
    double epsilon = 1e-9; 
    
    if (fabs(fabs(sum) - TWO_PI) < epsilon) {
        return IN;
    }
    
    return OUT; 
}

void print(const vector<point> &p){
    int n = (int)p.size();
    for(int i = 0; i < n; i++){
        cout << i+1 << ": " <<  p[i].x << ' ' << p[i].y << '\n';
    }
    cout << "\n\n";
}
