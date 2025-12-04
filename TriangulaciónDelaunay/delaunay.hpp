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

#define INVALID std::numeric_limits<size_t>::max()

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

  bool operator()(size_t i, size_t j){
		Point p1 = {points[i].x, points[i].y};
		Point p2 = {points[j].x, points[j].y};
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

size_t fast_mod(const size_t i, const size_t j){
  return i >= j ? i % j : i; 
}

class Delaunay {
public:
  vector<Point3> const& points;
  vector<size_t> triangles;
  vector<size_t> halfEdges;
  vector<size_t> hullPrev;
  vector<size_t> hullNext;
  vector<size_t> hullTri;
  size_t hullStart;

  Delaunay(vector<Point3> const& input);
  double getHullArea();

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

Delaunay::Delaunay(vector<Point3> const& input) : points(input) {
  size_t n = points.size();
  if(n < 3) return;

  vector<size_t> ids;
  ids.reserve(n);
  Point maxPoint(numeric_limits<double>::min(), numeric_limits<double>::min());
  Point minPoint(numeric_limits<double>::max(), numeric_limits<double>::max());

  // Encontrar la Bounding Box
  for(size_t i = 0; i < n; i++){
    if(points[i].x < minPoint.x) minPoint.x = points[i].x;
    if(points[i].y < minPoint.y) minPoint.y = points[i].y;
    if(points[i].x > maxPoint.x) maxPoint.x = points[i].x;
    if(points[i].y > maxPoint.y) maxPoint.y = points[i].y;
    ids.push_back(i);
  }

  // Calcular el centroide de la caja
  Point centroid((minPoint + maxPoint) / 2.0);
  
  // Selección del triángulo inicial
  size_t i0 = INVALID;
  double minDist = numeric_limits<double>::max();
  // i0: punto más cercano al centroide
  for(size_t i = 0; i < n; i++){
		Point p = {points[i].x, points[i].y};
    double d = dist(centroid, p);
    if(d < minDist){
      i0 = i;
      minDist = d;
    }
  }

  Point p0 = {points[i0].x, points[i0].y};

  size_t i1 = INVALID;
  minDist = numeric_limits<double>::max();
  // i1: punto más cercano a i0
  for(size_t i = 0; i < n; i++){
    if(i == i0) continue;
		Point p = {points[i].x, points[i].y};
    double d = dist(p0, p);
    if(d < minDist){
      i1 = i;
      minDist = d;
    }
  }
  Point p1 = {points[i1].x, points[i1].y};

  size_t i2 = INVALID;
  double minRadius = numeric_limits<double>::max();
  // i2 es el punto que forma el circuncírculo más pequeño con p0 y p1
  for(size_t i = 0; i < n; i++){
    if(i == i0 || i == i1) continue;
		Point p = {points[i].x, points[i].y};
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
  hashSize = static_cast<size_t>(llround(ceil(sqrt(n))));
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

  size_t maxTriangles = 2 * n - 5;
  triangles.reserve(maxTriangles * 3);
  halfEdges.reserve(maxTriangles * 3);
  addTriangle(i0, i1, i2, INVALID, INVALID, INVALID);

  Point temp(numeric_limits<double>::quiet_NaN(), numeric_limits<double>::quiet_NaN());

  // Bucle principal de inserción incremental
  for(size_t k = 0; k < n; k++){
    size_t i = ids[k];
    Point p = {points[i].x, points[i].y};

    // Saltar puntos cercanos y el triangulo inicial
    if(k > 0 && abs(p.x - temp.x) <= numeric_limits<double>::epsilon() && abs(p.y - temp.y) <= numeric_limits<double>::epsilon()) continue;
    temp = p;
    
    if(i == i0 || i == i1 || i == i2) continue;

    // Localización del borde visible usando hash table y walking search
    size_t start = 0;
    size_t key = hashKey(p);
    for(size_t j = 0; j < hashSize; j++){
      start = hullHash[fast_mod(key + j, hashSize)];
      if(start != INVALID && start != hullNext[start]) break;
    }

    start = hullPrev[start];
    size_t e = start;
    size_t q;

    // Caminar hacia adelante hasta encontrar un borde
    while(q = hullNext[e], (orientation(p, {points[e].x, points[e].y}, {points[q].x, points[q].y}) != LEFT)){
      e = q;
      if(e == start){
        e = INVALID; // Punto interior o duplicado
        break;
      }
    }

    if(e == INVALID) continue;

    // Inserción del nuevo punto 'p'
    // Añadir el primer triángulo (e, i, hullNext[e])
    size_t t = addTriangle(e, i, hullNext[e], INVALID, INVALID, hullTri[e]);
    hullTri[i] = legalize(t + 2);
    hullTri[e] = t;
    
    size_t next = hullNext[e];
    // Caminar hacia adelante: añadir nuevos triángulos y legalizar
    while (q = hullNext[next], orientation(p, {points[next].x, points[next].y}, {points[q].x, points[q].y}) == LEFT){
      // Conexión crítica: se conecta con el triángulo vecino 'hullTri[next]'
      t = addTriangle(next, i, q, hullTri[i], INVALID, hullTri[next]);
      hullTri[i] = legalize(t + 2);
      hullNext[next] = next;
      next = q;
    }

    // Caminar hacia atrás: añadir nuevos triángulos y legalizar
    if(e == start){
      while(q = hullPrev[e], orientation(p, {points[q].x, points[q].y}, {points[e].x, points[e].y}) == LEFT){
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
    hullHash[hashKey({points[e].x, points[e].y})] = e;
  }
}

double Delaunay::getHullArea(){
  vector<double> hullArea;
  size_t e = hullStart;
  do {
    hullArea.push_back((points[e].x - points[hullPrev[e]].x) * (points[e].y + points[hullPrev[e]].y));
    e = hullNext[e]; 
  } while(e != hullStart);
  
  double sum = 0;
  for(double v : hullArea) sum += v; 
  return sum;
}

// Legalización de aristas
size_t Delaunay::legalize(size_t a){
  size_t i = 0;
  size_t ar = 0;
  illegalEdgesStack.clear();

  while(true){
    size_t b = halfEdges[a];

    size_t a0 = 3 * (a / 3);
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

    size_t b0 = 3 * (b / 3);
    size_t al = a0 + (a + 1) % 3;
    size_t bl = b0 + (b + 2) % 3;

    size_t p0 = triangles[ar];
    size_t pr = triangles[a];
    size_t pl = triangles[al];
    size_t p1 = triangles[bl];

    // Verificamos si p1, el nuevo punto, esta en el circulo circunscrito del triangulo que ya existia
    bool illegal = inCircle({points[p0].x,points[p0].y}, {points[pr].x,points[pr].y}, {points[pl].x,points[pl].y}, {points[p1].x,points[p1].y});

    if(illegal){
      // Se hace el flip
      triangles[a] = p1;
      triangles[b] = p0;

      auto hbl = halfEdges[bl];

      if(hbl == INVALID){
        size_t e = hullStart;
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
      size_t br = b0 + (b + 1) % 3;

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

size_t Delaunay::hashKey(Point p) const {

  // Calcular el vector relativo al circuncentro del triángulo inicial
  Point aux = p - circumCenterPoint;
  double angle = pseudoAngle(aux);
  
  // Escalar el pseudo-ángulo por hashSize para obtener la clave
  size_t key = static_cast<size_t>(std::llround(std::floor(angle * static_cast<double>(hashSize))));
  return fast_mod(key, hashSize);
}

size_t Delaunay::addTriangle(size_t i0, size_t i1, size_t i2, size_t a, size_t b, size_t c){
  size_t t = triangles.size();

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

void Delaunay::link(const size_t a, const size_t b){
  // Enlazar a -> b
  size_t s = halfEdges.size();
  if(a == s){
    halfEdges.push_back(b);
  } else if(a < s){
    halfEdges[a] = b;
  } else {
    throw runtime_error("Union invalida: a fuera de rango");
  }

  // Si el vecino es válido, enlazar b -> a
  if(b != INVALID){
    size_t s2 = halfEdges.size();
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