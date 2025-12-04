#ifndef DELAUNAY
#define DELAUNAY

#include "../geo.hpp"
#include <algorithm>
#include <vector>
#include <memory>
#include <utility>
#include <cmath>
#include <limits>
#include <stdexcept>

#define INVALID std::numeric_limits<unsigned int>::max()

using namespace std;
struct Point3{
	double x, y, z;
	Point3(Point p) : x(p.x), y(p.y), z(0) {}
	Point3(double x0, double y0, double z0) : x(x0), y(y0), z(z0){}
};

// Calcula el cuadrado del radio de la circunferencia circunscrita
double circumRadius(const Point p, const Point q, const Point r){
  Point n = q - p;
  Point l = r - p;

  double a = dot(n, n);
  double b = dot(l, l);
  double c = cross(n, l);

  // Si c es cero, los puntos son colineales y el radio es infinito
  if(c == 0) return numeric_limits<double>::max();

  double x = (l.y * a - n.y * b) * 0.5 / c;
  double y = (n.x * b - l.x * a) * 0.5 / c;

  return x*x + y*y;
}

Point circumCenter(const Point p, const Point q, const Point r){
  Point n = q - p;
  Point l = r - p;

  double a = dot(n, n);
  double b = dot(l, l);
  double c = cross(n, l);

  // x e y son las coordenadas absolutas del circuncentro 
  double x = p.x + (l.y * a - n.y * b) * 0.5 / c;
  double y = p.y + (n.x * b - l.x * a) * 0.5 / c;

  return Point(x, y);
}

// Regresa true si el punto p está DENTRO del circuncírculo de (a, b, c).
bool inCircle(const Point a, const Point b, const Point c, const Point p){
  Point d = a - p;
  Point e = b - p;
  Point f = c - p;

  double ap = dot(d, d);
  double bp = dot(e, e);
  double cp = dot(f, f);

  double det = d.x * (e.y * cp - bp * f.y) - 
               d.y * (e.x * cp - bp * f.x) + 
               ap * (e.x * f.y - e.y * f.x);

  return det < 0.0;
}

// Comparador para ordenar los puntos por distancia al circuncentro
struct compare {
  vector<Point3> const& points;
  Point center;

  bool operator()(unsigned int i, unsigned int j){
		Point p1 = {points[i].x, points[i].z};
		Point p2 = {points[j].x, points[j].z};
    double d1 = dist(p1, center);
    double d2 = dist(p2, center);
    double diffDist = d1 - d2;

    if(std::abs(diffDist) > 1e-9){ 
      return d1 < d2;
    }
    
    Point diffCoords = p1-p2;
    if (diffCoords.x != 0.0){
      return diffCoords.x < 0.0; // Desempate por coordenada X
    } else{
      return diffCoords.y < 0.0; // Desempate por coordenada Y
    }
  }
};

// Función para calcular un pseudo-ángulo (monótona, evita trigonometría costosa)
double pseudoAngle(Point p){
  // Usamos std::abs para asegurar que funcione correctamente con doubles.
  double aux = p.x / (std::abs(p.x) + std::abs(p.y));
  return (p.y > 0.0 ? 3.0 - aux : 1.0 + aux) / 4.0;
}

unsigned int fast_mod(const unsigned int i, const unsigned int j){
  return i >= j ? i % j : i; 
}

class Delaunay {
public:
  vector<Point3> const& points;
  vector<unsigned int> triangles;
  vector<unsigned int> halfEdges;
  vector<unsigned int> hullPrev;
  vector<unsigned int> hullNext;
  vector<unsigned int> hullTri;
  unsigned int hullStart;

  Delaunay(vector<Point3> const& input);
  double getHullArea();

private:
  vector<unsigned int> hullHash;
  Point circumCenterPoint;
  unsigned int hashSize;
  vector<unsigned int> illegalEdgesStack;

  unsigned int legalize(unsigned int a);
  unsigned int hashKey(Point p) const;
  unsigned int addTriangle(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int a, unsigned int b, unsigned int c);
  void link(unsigned int a, unsigned int b);
};

Delaunay::Delaunay(vector<Point3> const& input) : points(input) {
  unsigned int n = points.size();
  if(n < 3) return;

  vector<unsigned int> ids;
  ids.reserve(n);
  Point maxPoint(numeric_limits<double>::min(), numeric_limits<double>::min());
  Point minPoint(numeric_limits<double>::max(), numeric_limits<double>::max());

  // Encontrar la Bounding Box
  for(unsigned int i = 0; i < n; i++){
    if(points[i].x < minPoint.x) minPoint.x = points[i].x;
    if(points[i].z < minPoint.y) minPoint.y = points[i].z;
    if(points[i].x > maxPoint.x) maxPoint.x = points[i].x;
    if(points[i].z > maxPoint.y) maxPoint.y = points[i].z;
    ids.push_back(i);
  }

  // Calcular el centroide de la caja
  Point centroid((minPoint + maxPoint) / 2.0);
  
  // Selección del triángulo inicial
  unsigned int i0 = INVALID;
  double minDist = numeric_limits<double>::max();
  // i0: punto más cercano al centroide
  for(unsigned int i = 0; i < n; i++){
		Point p = {points[i].x, points[i].z};
    double d = dist(centroid, p);
    if(d < minDist){
      i0 = i;
      minDist = d;
    }
  }

  Point p0 = {points[i0].x, points[i0].y};

  unsigned int i1 = INVALID;
  minDist = numeric_limits<double>::max();
  // i1: punto más cercano a i0
  for(unsigned int i = 0; i < n; i++){
    if(i == i0) continue;
		Point p = {points[i].x, points[i].z};
    double d = dist(p0, p);
    if(d < minDist){
      i1 = i;
      minDist = d;
    }
  }
  Point p1 = {points[i1].x, points[i1].y};

  unsigned int i2 = INVALID;
  double minRadius = numeric_limits<double>::max();
  // i2 es el punto que forma el circuncírculo más pequeño con p0 y p1
  for(unsigned int i = 0; i < n; i++){
    if(i == i0 || i == i1) continue;
		Point p = {points[i].x, points[i].z};
    double r = circumRadius(p0, p1, p);
    if(r < minRadius){
      i2 = i;
      minRadius = r;
    }
  }
  Point p2 = {points[i2].x, points[i2].y};

  // Asegurar que el triángulo inicial (i0, i1, i2) esté en orden horario (CW)
  if(orientation(p0, p1, p2) == LEFT){
    swap(i1, i2);
    swap(p1, p2);
  }

  // Almacenar el circuncentro para el hashing y la ordenación
  circumCenterPoint = circumCenter(p0, p1, p2);

  // Ordenar los puntos por distancia al circuncentro
  sort(ids.begin(), ids.end(), compare{points, circumCenterPoint});

  // Inicializar estructuras del Convex Hull
  hashSize = static_cast<unsigned int>(llround(ceil(sqrt(n))));
  hullHash.resize(hashSize);
  fill(hullHash.begin(), hullHash.end(), INVALID);

  hullPrev.resize(n);
  hullNext.resize(n);
  hullTri.resize(n);

  hullStart = i0;
  
  // Establecer el triángulo inicial como la envolvente convexa inicial
  hullNext[i0] = hullPrev[i2] = i1;
  hullNext[i1] = hullPrev[i0] = i2;
  hullNext[i2] = hullPrev[i1] = i0;

  // hullTri apunta a la media-arista opuesta a cada vértice
  hullTri[i0] = 0;
  hullTri[i1] = 1;
  hullTri[i2] = 2;

  // Almacenar los vértices del hull inicial en la tabla hash para búsqueda
  hullHash[hashKey({points[i0].x, points[i0].y})] = i0;
  hullHash[hashKey({points[i1].x, points[i1].y})] = i1;
  hullHash[hashKey({points[i2].x, points[i2].y})] = i2;

  unsigned int maxTriangles = 2 * n - 5;
  triangles.reserve(maxTriangles * 3);
  halfEdges.reserve(maxTriangles * 3);
  addTriangle(i0, i1, i2, INVALID, INVALID, INVALID);

  Point temp(numeric_limits<double>::quiet_NaN(), numeric_limits<double>::quiet_NaN());

  // Bucle principal de inserción incremental
  for(unsigned int k = 0; k < n; k++){
    unsigned int i = ids[k];
    Point p = {points[i].x, points[i].z};

    // Saltar puntos cercanos y el triangulo inicial
    if(k > 0 && abs(p.x - temp.x) <= numeric_limits<double>::epsilon() && abs(p.y - temp.y) <= numeric_limits<double>::epsilon()) continue;
    temp = p;
    
    if(i == i0 || i == i1 || i == i2) continue;

    // Localización del borde visible usando hash table y walking search
    unsigned int start = 0;
    unsigned int key = hashKey(p);
    for(unsigned int j = 0; j < hashSize; j++){
      start = hullHash[fast_mod(key + j, hashSize)];
      if(start != INVALID && start != hullNext[start]) break;
    }

    start = hullPrev[start];
    unsigned int e = start;
    unsigned int q;

    // Caminar hacia adelante hasta encontrar un borde
    while(q = hullNext[e], (orientation(p, {points[e].x, points[e].z}, {points[q].x, points[q].z}) != LEFT)){
      e = q;
      if(e == start){
        e = INVALID; // Punto interior o duplicado
        break;
      }
    }

    if(e == INVALID) continue;

    // Inserción del nuevo punto 'p'
    // Añadir el primer triángulo (e, i, hullNext[e])
    unsigned int t = addTriangle(e, i, hullNext[e], INVALID, INVALID, hullTri[e]);
    hullTri[i] = legalize(t + 2);
    hullTri[e] = t;
    
    unsigned int next = hullNext[e];
    // Caminar hacia adelante: añadir nuevos triángulos y legalizar
    while (q = hullNext[next], orientation(p, {points[next].x, points[next].z}, {points[q].x, points[q].z}) == LEFT){
      // Conexión crítica: se conecta con el triángulo vecino 'hullTri[next]'
      t = addTriangle(next, i, q, hullTri[i], INVALID, hullTri[next]);
      hullTri[i] = legalize(t + 2);
      hullNext[next] = next;
      next = q;
    }

    // Caminar hacia atrás: añadir nuevos triángulos y legalizar
    if(e == start){
      while(q = hullPrev[e], orientation(p, {points[q].x, points[q].z}, {points[e].x, points[e].z}) == LEFT){
        t = addTriangle(q, i, e, INVALID, hullTri[e], hullTri[q]);
        legalize(t + 2);
        hullTri[q] = t;
        hullNext[e] = e; 
        e = q;
      }
    }

    // Actualizar la envolvente convexa
    hullPrev[i] = e;
    hullStart = e;
    hullPrev[next] = i;
    hullNext[e] = i;
    hullNext[i] = next;

    hullHash[hashKey(p)] = i;
    hullHash[hashKey({points[e].x, points[e].z})] = e;
  }
}

double Delaunay::getHullArea(){
  vector<double> hullArea;
  unsigned int e = hullStart;
  do {
    hullArea.push_back((points[e].x - points[hullPrev[e]].x) * (points[e].z + points[hullPrev[e]].y));
    e = hullNext[e]; 
  } while(e != hullStart);
  
  double sum = 0;
  for(double v : hullArea) sum += v; 
  return sum;
}

// Legalización de aristas
unsigned int Delaunay::legalize(unsigned int a){
  unsigned int i = 0;
  unsigned int ar = 0;
  illegalEdgesStack.clear();

  while(true){
    unsigned int b = halfEdges[a];

    unsigned int a0 = 3 * (a / 3);
    ar = a0 + (a + 2) % 3;

    if(b == INVALID){
      if(i > 0){
        i--;
        a = illegalEdgesStack[i];
        continue;
      } else {
        break;
      }
    }

    unsigned int b0 = 3 * (b / 3);
    unsigned int al = a0 + (a + 1) % 3;
    unsigned int bl = b0 + (b + 2) % 3;

    unsigned int p0 = triangles[ar];
    unsigned int pr = triangles[a];
    unsigned int pl = triangles[al];
    unsigned int p1 = triangles[bl];

    // Verificamos si p1, el nuevo punto, esta en el circulo circunscrito del triangulo que ya existia
    bool illegal = inCircle({points[p0].x,points[p0].y}, {points[pr].x,points[pr].y}, {points[pl].x,points[pl].y}, {points[p1].x,points[p1].y});

    if(illegal){
      // Se hace el flip
      triangles[a] = p1;
      triangles[b] = p0;

      auto hbl = halfEdges[bl];

      if(hbl == INVALID){
        unsigned int e = hullStart;
        do {
          if(hullTri[e] == bl){
            hullTri[e] = a; 
            break;
          }
          e = hullNext[e];
        } while(e != hullStart);
      }

      // Actualizar punteros de halfEdges
      link(a, hbl);
      link(b, halfEdges[ar]);
      link(ar, bl);

      // Añadimos las nuevas aristas para comprobar iterativamente
      unsigned int br = b0 + (b + 1) % 3;

      if(i < illegalEdgesStack.size()){
        illegalEdgesStack[i] = br;
      } else {
        illegalEdgesStack.push_back(br);
      }
      i++;
    } else {
      // Si la arista es legal, desapilar y continuar
      if(i > 0){
        i--;
        a = illegalEdgesStack[i];
        continue;
      } else {
        break;
      }
    }
  }
  return ar;
}

unsigned int Delaunay::hashKey(Point p) const {

  // Calcular el vector relativo al circuncentro del triángulo inicial
  Point aux = p - circumCenterPoint;
  double angle = pseudoAngle(aux);
  
  // Escalar el pseudo-ángulo por hashSize para obtener la clave
  unsigned int key = static_cast<unsigned int>(std::llround(std::floor(angle * static_cast<double>(hashSize))));
  return fast_mod(key, hashSize);
}

unsigned int Delaunay::addTriangle(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int a, unsigned int b, unsigned int c){
  unsigned int t = triangles.size();

  // Añadir los índices de los 3 vértices
  triangles.push_back(i0);
  triangles.push_back(i1);
  triangles.push_back(i2);

  // Enlazar las media-aristas con sus vecinos
  link(t, a);
  link(t + 1, b);
  link(t + 2, c);

  return t;
}

void Delaunay::link(const unsigned int a, const unsigned int b){
  // Enlazar a -> b
  unsigned int s = halfEdges.size();
  if(a == s){
    halfEdges.push_back(b);
  } else if(a < s){
    halfEdges[a] = b;
  } else {
    throw runtime_error("Union invalida: a fuera de rango");
  }

  // Si el vecino es válido, enlazar b -> a
  if(b != INVALID){
    unsigned int s2 = halfEdges.size();
    if(b == s2){
      halfEdges.push_back(a);
    } else if (b < s2){
      halfEdges[b] = a;
    } else {
      throw runtime_error("Union invalida: b fuera de rango");
    }
  }
}

#endif