#include "geo.h"
#include <bits/stdc++.h>
#define sz(a) (int)a.size()
using namespace std;

vector<point> randomPoints(int n){
    vector<point>points;
    for(int i = 0; i < n; i++){
        double xrand = 100 * (double)rand()/(RAND_MAX+1);
        double yrand = 100 * (double)rand()/(RAND_MAX+1);

        points.push_back({xrand, yrand});
    }

    return points;
}

inline bool isConvex(vector<point> &p){
    int n = p.size();
    p.push_back(p[0]);
    if(n <= 3) return false;  // Se duplica el primer vertice, por lo que n <= 3 es un punto o una linea.
    bool isLeft = ccw(p[0], p[1], p[2]);
    int dir = 0;
    for(int i = 1; i < n; i++){
        if (ccw(p[i],p[i+1],p[(i+2) == n ? 1 : i+2]) != 0) {
            if (dir == 0) dir = (cross > 0 ? 1 : -1);
            else if ((cross > 0 ? 1 : -1) != dir) return false;
        }
    }
    p.pop_back();
    return true;
}

vector<point> jarvisMarch(vector<point> &p){
    int n = sz(p);
    if(n<=3) return p;
    vector<point> CH;
    int l = 0;
    for(int i = 1; i < n; i++){
        if(p[i].x < p[l].x || (abs(p[i].x -p[l].x)< EPS && p[i].y < p[l].y)) l = i;
    }

    int pivot = l, k;
    do{
        CH.push_back(p[pivot]);
        k = (pivot+1)%n;
        for(int i = 1; i < n; i++){
            if(i!=pivot){
                if(ccw(p[pivot], p[i], p[k])){
                    k=i;
                }
            }
        }
        pivot = k;
    }while(pivot!=l);

    return CH;
}

vector<point> grahamScan(vector<point> &p){
    int n = p.size();
    if(n<=3) return p;
    int l = 0;
    
    for(int i = 1; i < n; i++){
        if(p[i] < p[l]) l = i;
    }

    swap(p[0], p[l]);
    sort(++p.begin(), p.end(),[&](point a, point b){
        if (ccw(p[0], a, b)) return true;
        if (ccw(p[0], b, a)) return false;
        return dist(p[0], a) < dist(p[0], b);
    });
    
    vector<point> CH({p[0],p[1]});
    int i = 2;
    while(i < n){
        int j = sz(CH)-1;
        if(ccw(CH[j-1],CH[j],p[i])){
            CH.push_back(p[i++]);
        }
        else{
            CH.pop_back();
        }
    }
    return CH;
}

vector<point> monotoneChain(vector<point> &p){
    int n = sz(p);
    vector<point> CH(2*n);
    sort(p.begin(), p.end());
    
    
    return CH;
}

int main(void){
    srand(time(NULL));
    int n;
    cin >> n;
    vector<point> p = randomPoints(n);
    
    print(p);

    vector<point>CHp = jarvisMarch(p);
    int m = sz(CHp);
    cout << isConvex(CHp) << '\n';
    print(CHp);

    CHp = grahamScan(p);
    m = sz(CHp);
    print(CHp);
    cout << isConvex(CHp);
    
    CHp = monotoneChain(p);
    m = sz(CHp);
    print(CHp);

    cout << isConvex(CHp);
    return 0;
}