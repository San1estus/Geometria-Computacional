#include "delaunay.hpp"
#include <iostream>

int main(){
	vector<Point> points;
	points.push_back({-1, 1});
	points.push_back({1, 1});
	points.push_back({1, -1});
	points.push_back({-1, -1});

	Delaunay d(points);
	cout << d.triangles.size() << '\n';
	for(size_t i =0; i < d.triangles.size(); i+=3){
		cout << "Puntos del triangulo " << i/3 << ":\n";
		cout << d.points[d.triangles[i]].x << ' ' << d.points[d.triangles[i]].y << '\n';
		cout << d.points[d.triangles[i+1]].x << ' ' << d.points[d.triangles[i+1]].y << '\n';
		cout << d.points[d.triangles[i+2]].x << ' ' << d.points[d.triangles[i+2]].y << '\n';
	}

	return 0;
}