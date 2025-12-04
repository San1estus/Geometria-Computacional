#include "delaunay.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

vector<Point3> readInput(const string& filename){
	vector<Point3> points;
	ifstream file(filename);
	if(!file.is_open()){
		throw runtime_error("No se encontro el archivo.");
	}

	string line;
	while(getline(file, line)){
		stringstream ss(line);
		double x, y, z;
		if(!(ss >> x >> y >> z)){
			continue;
		}
		points.push_back({x,y,z});
	}
	return points;
}


int main(){
	vector<Point3> points = readInput("input.txt");

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