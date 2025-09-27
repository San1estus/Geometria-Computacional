#include "geo.h"
#include <bits/stdc++.h>
#define sz(a) (int)a.size()
using namespace std;

// Función para generar puntos aleatorios
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
    int n = sz(p);
    if (n < 3) return false; 
    if (n == 3) return true;  // Si el la cantidad de puntos es < 3 es un punto o una linea, si n = 3 es un triangulo.
    int dir = 0;
    
    for(int i = 0; i < n; i++){
        if (ccw(p[i],p[(i+1) % n],p[(i+2) % n]) != 0) {
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
        if(p[i] < p[l]) l = i;
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

                // Maneja el caso colineal
                else if(!ccw(p[pivot], p[i], p[k]) && collinear(p[pivot], p[i], p[k]) && dist(p[pivot], p[i]) > dist(p[pivot], p[k])){
                k = i;
            }
            }
        }
        pivot = k;
    }while(pivot!=l);

    return CH;
}

vector<point> grahamScan(vector<point> &p){
    int n = sz(p);
    if(n<=3) return p;
    int l = 0;
    
    for(int i = 1; i < n; i++){
        if(p[i] < p[l]) l = i;
    }

    swap(p[0], p[l]);
    
    sort(++p.begin(), p.end(),[&](point a, point b){
        if (ccw(p[0], a, b)) return true;
        if (ccw(p[0], b, a)) return false;
        
        // Esto maneja el caso colineal, prefiriendo añadir mas puntos.
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
    int k = 0;
    vector<point> CH(2*n);

    sort(p.begin(), p.end());
    for(int i = 0; i < n; i++){
        while((k >= 2) && !ccw(CH[k-2], CH[k-1], p[i])) --k;
        CH[k++] = p[i];
    }

    for(int i = n-2, t = k+1; i>=0; i--){
        while((k >= t) && !ccw(CH[k-2], CH[k-1], p[i])) --k;
        CH[k++] = p[i];
    }

    CH.resize(k);
    CH.pop_back();
    return CH;
}

int main(void){
    srand(time(NULL));
    int n;
    cout << "Indica la cantidad de puntos a generar: ";
    cin >> n;
    vector<point> p = randomPoints(n);
    
    // Descomentar para imprimir todos los puntos
    //print(p);

    vector<point>CHp = jarvisMarch(p);
    int m = sz(CHp);
    print(CHp);

    cout << isConvex(CHp) << '\n';
    CHp.clear();
    
    CHp = grahamScan(p);
    m = sz(CHp);
    print(CHp);
    cout << isConvex(CHp) << '\n';
    CHp.clear();

    CHp = monotoneChain(p);
    m = sz(CHp);
    print(CHp);

    cout << isConvex(CHp) << "\nFin";
    return 0;
}