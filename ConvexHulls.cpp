#include "geo.h"
using namespace std;

inline bool isConvex(vector<point> &p){
    int n = p.size();
    if(n <= 3) return false;  // Se duplica el primer vertice, por lo que n <= 3 es un punto o una linea.
    bool isLeft = ccw(p[0], p[1], p[2]);
    for(int i = 1; i < n; i++){
        if(ccw(p[i],p[i+1],p[(i+2) == n ? 1 : i+2], isLeft) != isLeft){
            return false;
        }
    }
    return true;
}

int main(void){
    vector<point> p;
    point a(0.3, 0.3);
    point b(1.5, 1);
    point c(0.5, 0.5);
    p.push_back(a);
    p.push_back(b);
    p.push_back(c);
    p.push_back(a);

    cout << isConvex(p) << '\n';
    return 0;
}