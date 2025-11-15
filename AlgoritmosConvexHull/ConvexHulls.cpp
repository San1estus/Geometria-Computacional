#include "../geo.hpp"
#include <bits/stdc++.h>
#define sz(a) (int)a.size()
using namespace std;

// Función para generar puntos aleatorios
vector<Point> randomPoints(int n){
    vector<Point>points;
    for(int i = 0; i < n; i++){
        double xrand = 100 * (double)rand()/(RAND_MAX+1);
        double yrand = 100 * (double)rand()/(RAND_MAX+1);

        points.push_back({xrand, yrand});
    }

    return points;
}

inline bool isConvex(vector<Point> &p){
    int n = sz(p);
    if (n < 3) return false; 
    if (n == 3) return true; 
    int dir = 0;
    
    for(int i = 0; i < n; i++){
        if (orientation(p[i],p[(i+1) % n],p[(i+2) % n]) != 0) {
            if (dir == 0) dir = (ccw(p[i],p[(i+1) % n],p[(i+2) % n]) > 0 ? 1 : -1);
            else if ((ccw(p[i],p[(i+1) % n],p[(i+2) % n]) > 0 ? 1 : -1) != dir) return false;
            else dir = ccw(p[i],p[(i+1) % n],p[(i+2) % n]);
        }
    }
    return true;
}

vector<Point> jarvisMarch(vector<Point> &p){
    int n = sz(p);
    if(n < 3){cout << "Es un(a) " << (n%2 ? "punto" : "linea") << '\n'; return p;}
    if(n<=3) return p; // Si n = 3 es un triangulo.
    vector<Point> CH;
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
                if(orientation(p[pivot], p[i], p[k]) == LEFT){
                    k=i;
                }

                // Maneja el caso colineal
                else if(!orientation(p[pivot], p[i], p[k]) != RIGHT&& orientation(p[pivot], p[i], p[k]) !=LEFT && dist(p[pivot], p[i]) > dist(p[pivot], p[k])){
                k = i;
            }
            }
        }
        pivot = k;
    }while(pivot!=l);

    return CH;
}

vector<Point> grahamScan(vector<Point> &p){
    int n = sz(p);
    if(n < 3){cout << "Es un(a) " << (n%2 ? "punto" : "linea") << '\n'; return p;}
    if(n<=3) return p; // Si n = 3 es un triangulo.
    int l = 0;
    
    for(int i = 1; i < n; i++){
        if(p[i] < p[l]) l = i;
    }

    swap(p[0], p[l]);
    
    sort(++p.begin(), p.end(),[&](Point a, Point b){
        if (ccw(p[0], a, b)) return true;
        if (ccw(p[0], b, a)) return false;
        
        // Esto maneja el caso colineal, prefiriendo añadir mas puntos.
        return dist(p[0], a) < dist(p[0], b);
    });
    
    vector<Point> CH({p[0],p[1]});
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

vector<Point> monotoneChain(vector<Point> &p){
    int n = sz(p);
    if(n < 3){cout << "Es un(a) " << (n%2 ? "punto" : "linea") << '\n'; return p;}
    if(n==3) return p; // Si n = 3 es un triangulo.
    int k = 0;
    vector<Point> CH(2*n);

    // Hace el hull inferior, compara hacia arriba
    sort(p.begin(), p.end());
    for(int i = 0; i < n; i++){
        while((k >= 2) && !ccw(CH[k-2], CH[k-1], p[i])) --k;
        CH[k++] = p[i];
    }

    // Hace el hull superior, compara hacia abajo
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
    vector<Point> p = randomPoints(n);
    
    // Descomentar para imprimir todos los puntos
    //print(p);

    vector<Point>CHp = jarvisMarch(p);
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