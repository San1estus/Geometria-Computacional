#include "geo.h"

using namespace std;

vector<point> randomPoints(int n){
    vector<point>points;
    for(int i = 0; i < n; i++){
        double xrand = 100 * (double)rand()/(RAND_MAX+1);
        double yrand = 100* (double)rand()/(RAND_MAX+1);

        points.push_back({xrand, yrand});
    }

    return points;
}

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

vector<point> jarvisMarch(vector<point> &p){
    int n = p.size();
    if(n<=3) return p;
    vector<point> CH;
    int l = 0;
    for(int i = 1; i < n; i++){
        if(p[i].x < p[l].x || (p[i].x == p[l].x && p[i].y < p[l].y)) l = i;
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

int main(void){
    srand(time(NULL));
    int n;
    cin >> n;
    vector<point> p = randomPoints(n);

    p.push_back(p[0]);

    vector<point>CHp = jarvisMarch(p);
    for(int i = 0; i < CHp.size(); i++){
        cout << CHp[i].x << ' ' <<CHp[i].y <<'\n';
    }
    
    cout << '\n' << isConvex(CHp);
    return 0;
}