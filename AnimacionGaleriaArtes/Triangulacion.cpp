#include "DCEL.hpp"

using namespace std;

int main(void){
    vector<Point> points;

    points.push_back({1.0f, 0.0f});
    points.push_back({0.5f, 2.0f});
    points.push_back({0.0f, 0.0f});
    points.push_back({0.5f, -2.0f});
    points.push_back({1.0f, 0.0f});

    DoubleConnectedEdgeList DCEL1(points);
    DCEL1.addHalfEdges(*DCEL1.vertices.begin(), *next(DCEL1.vertices.begin(), 2));
    DCEL1.addHalfEdges(*next(DCEL1.vertices.begin(), 1), *next(DCEL1.vertices.begin(), 3));
    DCEL1.printDCEL();
    return 0;
}