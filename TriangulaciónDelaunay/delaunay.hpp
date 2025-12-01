#ifndef DELAUNAY
#define DELAUNAY
#include "../geo.hpp"
#include <algorithm>
#include <vector>
#include <memory>
#include <utility>
#define INVALID numeric_limits<size_t>::max()
using namespace std;


// Calcula el cuadrado del radio de la circunferencia circunscrita de un triangulo
double circumRadius(const Point p, const Point q, const Point r){
	Point n, l;
	double a, b, c;
	n = q-p;
	l = r-p;

	a = dot(n, n);
	b = dot(l, l);
	c = cross(n, l);

	double x = (l.y * a - n.y * b) * 0.5 / c;
	double y = (n.x * b - l.x * a) * 0.5 / c;

	if(a != 0 && b != 0 && c!=0){
		return x*x + y*y;
	}
	return numeric_limits<double>::max();
}

Point circumCenter(const Point p, const Point q, const Point r){
	Point n, l;
	double a, b, c;
	n = q-p;
	l = r-p;

	a = dot(n, n);
	b = dot(l, l);
	c = cross(n, l);

	double x = p.x + (l.y * a - n.y * b) * 0.5 / c;
	double y = p.y + (n.x * b - l.x * a) * 0.5 / c;

	return Point(x, y);
}

bool inCircle(const Point a, const Point b, const Point c, const Point p){
	Point d = a-p;
	Point e = b-p;
	Point f = c-p;

	double ap = dot(d,d);
	double bp = dot(e,e);
	double cp = dot(f,f);

	double det = d.x*(e.y*cp-bp*f.y) - d.y*(e.x*cp-bp*f.x) + ap*(e.x*f.y-e.y*f.x);

	return det < 0.0;
}

unsigned int findP0(const vector<Point> &points){
	Point p0(0, numeric_limits<double>::min());
	unsigned int idx = -1;

	for(int i = 0; i < sz(points); i++){
		if(p0.y <= points[i].y){
			p0.y = points[i].y;
			idx = i;
		}
	}
	
	return idx;
}

struct DelaunayPoint{
	size_t i;
	Point p;
	size_t t;
	size_t prev;
	size_t next;
	bool removed;
};

// Comparador por distancia
struct compare{
	vector<Point> const& coords;
	Point center;

	bool operator()(size_t i, size_t j){
		double d1 = dist(coords[i], center);
		double d2 = dist(coords[j], center);
		double diffDist = d1-d2;
		Point diffCoords = coords[i] - coords[j];

		// Si el punto i esta mas cerca se regresa true
		if(diffDist != 0.0){
			return diffDist < 0.0;
			// Se desempata por coordenda x, por cercania de coordenada x
		} else if (diffCoords.x != 0.0){
			return diffCoords.x < 0.0;
			// Por ultimo por coordendada y, por cercania de coordenada y
		} else{
			return diffCoords.y < 0.0;
		}
	}
};

// Funcion monotona creciente respecto al angulo real, pero sin usar funciones trigonometricas.
double pseudoAngle(Point p){
	double aux = p.x/(std::abs(p.x)+std::abs(p.y));
	return (p.y > 0.0 ? 3.0 - aux : 1.0 + aux) / 4.0;
}


class Delaunay{
	public:
		vector<Point> const& points;
		vector<size_t> triangles;
		vector<size_t> halfEdges;
		vector<size_t> hullPrev;
		vector<size_t> hullNext;
		vector<size_t> hullTri;
		size_t hullStart;

		Delaunay(vector<Point> const& input);

		double get_hull_area();

	private:
		vector<size_t> hullHash;
		Point circumCenterPoint;
		size_t hashSize;
		vector<size_t> illegalEdgesStack;

		size_t legalize(size_t a);
		size_t hashKey(Point p) const;
		size_t addTriangle(size_t i0, size_t i1, size_t i2, size_t a, size_t b, size_t c);
		void link(size_t a, size_t b);
};

size_t fast_mod(const size_t i, const size_t j){
	return i >= j ? i % j : i; 
}

Delaunay::Delaunay(vector<Point> const& input) : points(input){
	size_t n = points.size();

	Point maxPoint(numeric_limits<double>::min(), numeric_limits<double>::min()), minPoint(numeric_limits<double>::max(),numeric_limits<double>::max());
	
	vector<size_t> ids;
	ids.reserve(n);

	for(size_t i = 0; i < n; i++){
		double x = points[i].x;
		double y = points[i].y;

		if(x < minPoint.x) minPoint.x = x;
		if(y < minPoint.y) minPoint.y = y;
		if(x > maxPoint.x) maxPoint.x = x;
		if(y > maxPoint.y) maxPoint.y = y;

		ids.push_back(i);
	}

	Point centroid((minPoint+maxPoint)/2);
	double minDist = numeric_limits<double>::max();

	size_t i0 = INVALID;
	size_t i1 = INVALID;
	size_t i2 = INVALID;

	// Punto inicial, punto mas cercano al centroide
	for(size_t i = 0; i < n; i++){
		double d = dist(centroid, points[i]);
		if(d < minDist){
			i0 = i;
			minDist = d;
		}
	}
	
	Point p0 = points[i0];

	minDist = numeric_limits<double>::max();

	// Punto mas cercano al inicial
	for(size_t i =0; i < n; i++){
		if(i == i0) continue;
		double d = dist(p0, points[i]);
		if(d < minDist){
			i1 = i;
			minDist = d;
		}
	}
	
	Point p1 = points[i1];
	double minRadius = numeric_limits<double>::max();
	// Punto que forma el circuncirculo mas pequeño con p0 y p1
	for(size_t i = 0; i < n; i++){
		if(i == i0 || i == i1) continue;
		double r = circumRadius(p0, p1, points[i]);

		if(r < minRadius){
			i2 = i;
			minRadius = r;
		}
	}

	Point p2 = points[i2];

	if(orientation(p0, p1, p2) == LEFT){
		swap(i1, i2);
		swap(p1, p2);
	}

	Point circumCenterPoint = circumCenter(p0, p1, p2);

	sort(ids.begin(), ids.end(), compare{points, circumCenterPoint});

	hashSize = static_cast<size_t>(llround(ceil(sqrt(n))));

	hullHash.resize(hashSize);

	fill(hullHash.begin(), hullHash.end(), INVALID);

	hullPrev.resize(n);
	hullNext.resize(n);
	hullTri.resize(n);

	hullStart = i0;

	size_t hullSize = 3;

	hullNext[i0] = hullPrev[i2] = i1;
	hullNext[i1] = hullPrev[i0] = i2;
	hullNext[i2] = hullPrev[i1] = i0;

	hullTri[i0] = 0;
	hullTri[i1] = 1;
	hullTri[i2] = 2;

	hullHash[hashKey(p0)] = i0;
	hullHash[hashKey(p1)] = i1;
	hullHash[hashKey(p2)] = i2;

	size_t maxTriangles = n < 3 ? 1 : 2*n-5;
	triangles.reserve(maxTriangles * 3);
	halfEdges.reserve(maxTriangles * 3);
	addTriangle(i0, i1, i2, INVALID, INVALID, INVALID);

	Point temp(numeric_limits<double>::quiet_NaN(),numeric_limits<double>::quiet_NaN());

	for(size_t k = 0; k < n; k++){
		size_t i = ids[k];
		Point p = points[i];

		// Saltar puntos cercanos
		if(k > 0 && equalPoints(p, temp)) continue;
		temp = p;

		// Saltar el triangulo inicial
		if(equalPoints(p, p0) || equalPoints(p, p1) || equalPoints(p, p2)) continue;
		size_t start = 0;

		size_t key = hashKey(p);
		for(size_t j = 0; j < hashSize; j++){
			start = hullHash[fast_mod(key + j, hashSize)];
			if(start != INVALID && start != hullNext[start]) break;
		}

		start = hullPrev[start];
		size_t e = start;
		size_t q;

		while(q = hullNext[e], (orientation(p, points[e], points[q]) != LEFT)){
			e = q;
			if(e == start){
				e = INVALID;
				break;
			}
		}

		if(e == INVALID) continue;

		size_t t = addTriangle(e, i,hullNext[e], INVALID, INVALID, hullTri[e]);
		hullTri[i] = legalize(t+2);
		hullTri[e] = t;
		hullSize++;

		size_t next = hullNext[e];
		
		while (q = hullNext[next], orientation(p, points[next], points[q]) == LEFT){
			t = addTriangle(next, i, q, hullTri[i], INVALID, hullTri[next]);
			hullTri[i] = legalize(t+2);
			hullNext[next] = next;
			hullSize--;
			next = q;
		}

		if(e == start){
			while(q = hullPrev[e], orientation(p, points[q], points[e]) == LEFT){
				t = addTriangle(q, i, e, INVALID, hullTri[e], hullTri[q]);
				legalize(t+2);
				hullTri[q] = t;
				hullNext[e] = e;
				hullSize--;
				e = q;
			}
		}

		hullPrev[i] = e;
		hullStart = e;
		hullPrev[next] = i;
		hullNext[e] = i;
		hullNext[i] = next;

		hullHash[hashKey(p)] = i;
		hullHash[hashKey(points[e])] = e;
	}
}

double Delaunay::get_hull_area(){
	vector<double> hullArea;
	size_t e = hullStart;
	do {
		hullArea.push_back((points[e].x-points[hullPrev[e]].x) * (points[e].y + points[hullPrev[e]].y));
		e = hullNext[e]; 
	} while(e != hullStart);
	return sum(hullArea);
}

size_t Delaunay::legalize(size_t a){
	size_t i = 0;
	size_t ar = 0;
	illegalEdgesStack.clear();
	while(true){
		size_t b = halfEdges[a];

		size_t a0 = 3 * (a/3);
		ar = a0 + (a+2) %3;

		if(b == INVALID){
			if(i > 0){
				i--;
				a = illegalEdgesStack[i];
				continue;
			} else{
				break;
			}
		}

		size_t b0 = 3 * (b/3);
		size_t al = a0 + (a+1)%3;
		size_t bl = b0 + (b+2)%3;

		size_t p0 = triangles[ar];
		size_t pr = triangles[a];
		size_t pl = triangles[al];
		size_t p1 = triangles[bl];

		bool illegal = inCircle(points[p0], points[pr], points[pl], points[p1]);

		if(illegal){
			triangles[a] = p1;
			triangles[b] = p0;

			auto hbl = halfEdges[bl];

			if(hbl == INVALID){
				size_t e = hullStart;
				do{
					if(hullTri[e] == bl){
						hullTri[e] = a;
						break;
					}
					e = hullNext[e];
				} while(e != hullStart);
			}

			link(a, hbl);
			link(b, halfEdges[ar]);
			link(ar, bl);
			size_t br = b0 + (b+1)%3;

			if(i < illegalEdgesStack.size()){
				illegalEdgesStack[i] = br;
			} else {
				illegalEdgesStack.push_back(br);
			}
			i++;
		} else{
			if(i > 0){
				i--;
				a = illegalEdgesStack[i];
				continue;
			} else{
				break;
			}
		}
	}

	return ar;
}

size_t Delaunay::hashKey(Point p) const{
	const Point aux = p - circumCenterPoint;
    
    double angle = pseudoAngle(aux); 
    
    size_t key = static_cast<size_t>(std::llround(std::floor(angle * static_cast<double>(hashSize))));
    
    return fast_mod(key, hashSize);
}

size_t Delaunay::addTriangle(size_t i0, size_t i1, size_t i2, size_t a, size_t b, size_t c){
	size_t t = triangles.size();
	triangles.push_back(i0);
	triangles.push_back(i1);
	triangles.push_back(i2);

	link(t, a);
	link(t+1, b);
	link(t+2,c);

	return t;
}

void Delaunay::link(const size_t a, const size_t b){
	size_t s = halfEdges.size();
	if(a == s){
		halfEdges.push_back(b);
	} else if(a < s){
		halfEdges[a] = b;
	} else{
		throw runtime_error("Union invalida");
	}

	if(b != INVALID){
		size_t s2 = halfEdges.size();
		if(b == s2){
			halfEdges.push_back(a);
		} else if (b < s2){
			halfEdges[b] = a;
		} else{
		throw runtime_error("Union invalida");
		}
	}
}
#endif
