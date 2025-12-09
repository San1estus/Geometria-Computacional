#ifndef DELAUNAY_H
#define DELAUNAY_H

#include <algorithm>
#include <vector>
#include <memory>
#include <utility>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <iostream>

#define INVALID std::numeric_limits<unsigned int>::max()
#define EPSILON 1e-9 

using namespace std;

struct Point3 {
  double x, y, z;
  Point3() : x(0), y(0), z(0) {}
  Point3(double x0, double y0, double z0) : x(x0), y(y0), z(z0) {}
};

inline double distSq(double ax, double ay, double bx, double by) {
  double dx = ax - bx;
  double dy = ay - by;
  return dx * dx + dy * dy;
}

// Calcula el radio al cuadrado del círculo circunscrito al triángulo (a,b,c)
inline double circumradius(double ax, double ay, double bx, double by, double cx, double cy) {
  double dx = bx - ax;
  double dy = by - ay;
  double ex = cx - ax;
  double ey = cy - ay;
  
  double bl = dx * dx + dy * dy;  
  double cl = ex * ex + ey * ey;  
  double d = dx * ey - dy * ex;   
  
  // Puntos colineales si d ≈ 0
  if (fabs(d) < EPSILON) {
    return std::numeric_limits<double>::max();
  }
  
  // Centro del círculo circunscrito relativo al punto a
  double x = (ey * bl - dy * cl) * 0.5 / d;
  double y = (dx * cl - ex * bl) * 0.5 / d;
  
  return x * x + y * y;  
}

// Prueba de orientación CCW
inline bool orient(double px, double py, double qx, double qy, double rx, double ry) {
  return (qy - py) * (rx - qx) - (qx - px) * (ry - qy) < EPSILON;
}

// Retorna true si punto p está dentro del círculo circunscrito de triángulo a-b-c
// Calcula usando determinante de matriz 4x4 simplificado
inline bool InCircle(double ax, double ay, double bx, double by, double cx, double cy, double px, double py) {
  double dx = ax - px;
  double dy = ay - py;
  double ex = bx - px;
  double ey = by - py;
  double fx = cx - px;
  double fy = cy - py;
  
  double ap = dx * dx + dy * dy;
  double bp = ex * ex + ey * ey;
  double cp = fx * fx + fy * fy;
  
  // Determinante de la matriz de puntos orientados
  return (dx * (ey * cp - bp * fy) - dy * (ex * cp - bp * fx) + ap * (ex * fy - ey * fx)) < 0.0;
}

// Compara si dos puntos 2D son iguales 
inline bool check_pts_equal(double x1, double y1, double x2, double y2) {
  return fabs(x1 - x2) <= EPSILON && fabs(y1 - y2) <= EPSILON;
}

// Monotona respecto al ángulo pero más eficiente que atan2
inline double pseudo_angle(double dx, double dy) {
  double p = dx / (fabs(dx) + fabs(dy));
  return (dy > 0.0 ? 3.0 - p : 1.0 + p) / 4.0;  
}

// Evita operación % cuando no es necesario
inline unsigned int fast_mod(unsigned int i, unsigned int c) {
  return i >= c ? i % c : i;
}

// Comparador para ordenar puntos por distancia desde el centro 
// Si distancias iguales, desempata por coordenada x, luego z
struct compare {
  const vector<Point3>& points;
  double cx, cy;
  
  compare(const vector<Point3>& p, double center_x, double center_y) : points(p), cx(center_x), cy(center_y) {}
  
  bool operator()(unsigned int i, unsigned int j) const {
    double d1 = distSq(points[i].x, points[i].z, cx, cy);
    double d2 = distSq(points[j].x, points[j].z, cx, cy);
    
    if (fabs(d1 - d2) > EPSILON) {
      return d1 < d2;
    }
    
    double diff_x = points[i].x - points[j].x;
    if (fabs(diff_x) > EPSILON) {
      return diff_x < 0.0;
    }
    
    return points[i].z < points[j].z;
  }
};

class Delaunay {
public:
  const vector<Point3>& points;       
  vector<unsigned int> triangles;     
  vector<unsigned int> halfedges;      
  vector<unsigned int> hull_prev;
  vector<unsigned int> hull_next;
  vector<unsigned int> hull_tri;      
  unsigned int hull_start;
  
  Delaunay(const vector<Point3>& input);
  double getHullArea();                
  
private:
  vector<unsigned int> m_hash;         
  double m_center_x, m_center_y;       
  unsigned int m_hash_size;            
  vector<unsigned int> m_edge_stack;
  
  unsigned int legalize(unsigned int a);                           
  unsigned int hashKey(double x, double y) const;                  
  unsigned int addTriangle(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int a, unsigned int b, unsigned int c);  
  void link(unsigned int a, unsigned int b);                       
};

// OpenGL usa Y como altura vertical, por eso trabajamos en plano XZ
// pero en código usamos nombres x, y por simplicidad

Delaunay::Delaunay(const vector<Point3>& input) : points(input), hull_start(0) {
  
  unsigned int n = points.size();
  if (n < 3) return;  // Mínimo 3 puntos para triangulación
  
  double min_x = numeric_limits<double>::max();
  double min_y = numeric_limits<double>::max();
  double max_x = numeric_limits<double>::lowest();
  double max_y = numeric_limits<double>::lowest();
  
  vector<unsigned int> ids;
  ids.reserve(n);

  // Encontrar bounding box 
  for (unsigned int i = 0; i < n; i++) {
    double x = points[i].x;
    double y = points[i].z;
    
    if (x < min_x) min_x = x;
    if (y < min_y) min_y = y;
    if (x > max_x) max_x = x;
    if (y > max_y) max_y = y;
    
    ids.push_back(i);  
  }
  
  // Centro del bounding box
  double cx = (min_x + max_x) / 2.0;
  double cy = (min_y + max_y) / 2.0;
  
  // Seleccionar triángulo inicial
  // i0: punto más cercano al centro del bounding box
  unsigned int i0 = INVALID;
  double min_dist = numeric_limits<double>::max();
  
  for (unsigned int i = 0; i < n; i++) {
    double d = distSq(points[i].x, points[i].z, cx, cy);
    if (d < min_dist) {
      i0 = i;
      min_dist = d;
    }
  }
  
  double i0x = points[i0].x;
  double i0y = points[i0].z;
  
  // i1: punto más cercano a i0 (y no coincidente)
  unsigned int i1 = INVALID;
  min_dist = numeric_limits<double>::max();
  
  for (unsigned int i = 0; i < n; i++) {
    if (i == i0) continue;
    double d = distSq(i0x, i0y, points[i].x, points[i].z);
    if (d < min_dist && d > EPSILON) {
      i1 = i;
      min_dist = d;
    }
  }
  
  if (i1 == INVALID) {
    throw runtime_error("Puntos colineales o coincidentes");
  }
  
  double i1x = points[i1].x;
  double i1y = points[i1].z;
  
  // i2: punto que forma el círculo circunscrito más pequeño con i0 e i1
  // Esto minimiza el radio del triángulo inicial
  unsigned int i2 = INVALID;
  double min_radius = numeric_limits<double>::max();
  
  for (unsigned int i = 0; i < n; i++) {
    if (i == i0 || i == i1) continue;
    double r = circumradius(i0x, i0y, i1x, i1y, points[i].x, points[i].z);
    if (r < min_radius) {
      i2 = i;
      min_radius = r;
    }
  }
  
  if (min_radius == numeric_limits<double>::max()) {
    throw runtime_error("No se puede crear triangulación");
  }
  
  double i2x = points[i2].x;
  double i2y = points[i2].z;
  
  // Asegurar orientación CCW del triángulo inicial
  if (!orient(i0x, i0y, i1x, i1y, i2x, i2y)) {
    swap(i1, i2);
    swap(i1x, i2x);
    swap(i1y, i2y);
  }
  
  // Calcular centro del círculo circunscrito del triángulo inicial
  double dx = i1x - i0x;
  double dy = i1y - i0y;
  double ex = i2x - i0x;
  double ey = i2y - i0y;
  double bl = dx * dx + dy * dy;
  double cl = ex * ex + ey * ey;
  double d = dx * ey - dy * ex;
  
  m_center_x = i0x + (ey * bl - dy * cl) * 0.5 / d;
  m_center_y = i0y + (dx * cl - ex * bl) * 0.5 / d;
  
  // Ordenar puntos por distancia radial al centro
  sort(ids.begin(), ids.end(), compare(points, m_center_x, m_center_y));
  
  // Inicializar estructuras de datos
  m_hash_size = static_cast<unsigned int>(ceil(sqrt(n)));
  m_hash.resize(m_hash_size, INVALID);
  
  hull_prev.resize(n, INVALID);
  hull_next.resize(n, INVALID);
  hull_tri.resize(n, INVALID);
  
  // Inicializar envolvente convexa con triángulo inicial
  hull_start = i0;
  
  // Crear lista doblemente enlazada
  hull_next[i0] = i1;
  hull_prev[i2] = i1;
  hull_next[i1] = i2;
  hull_prev[i0] = i2;
  hull_next[i2] = i0;
  hull_prev[i1] = i0;
  
  // Asignar triángulos incidentes iniciales
  hull_tri[i0] = 0;
  hull_tri[i1] = 1;
  hull_tri[i2] = 2;
  
  // Inicializar tabla hash con vértices del triángulo inicial
  m_hash[hashKey(i0x, i0y)] = i0;
  m_hash[hashKey(i1x, i1y)] = i1;
  m_hash[hashKey(i2x, i2y)] = i2;
  
  // Reservar memoria para triángulos
  unsigned int max_triangles = n < 3 ? 1 : 2 * n - 5;
  triangles.reserve(max_triangles * 3);
  halfedges.reserve(max_triangles * 3);

  // Agregar el triángulo inicial
  addTriangle(i0, i1, i2, INVALID, INVALID, INVALID);
  
  // Inicio del algorimto incremental
  double xp = numeric_limits<double>::quiet_NaN();
  double yp = numeric_limits<double>::quiet_NaN();
  
  for (unsigned int k = 0; k < n; k++) {
    unsigned int i = ids[k];
    double x = points[i].x;
    double y = points[i].z;
    
    // Saltar puntos duplicados (respecto a 2D no 3D)
    if (k > 0 && check_pts_equal(x, y, xp, yp)) {
      continue;
    }

    xp = x;
    yp = y;
    
    // Saltar puntos del triángulo inicial
    if (i == i0 || i == i1 || i == i2) continue;
    
    // Buscar vértice del hull cercano usando tabla hash
    unsigned int start = 0;
    unsigned int key = hashKey(x, y);
    
    // Sondeo lineal para manejar colisiones
    for (unsigned int j = 0; j < m_hash_size; j++) {
      start = m_hash[fast_mod(key + j, m_hash_size)];
      if (start != INVALID && start != hull_next[start]) break;
    }
    
    start = hull_prev[start];
    unsigned int e = start;
    unsigned int q;
    
    // Buscar primera arista del envolvente visible desde el punto P
    while (true) {
      q = hull_next[e];  
      if (orient(x, y, points[e].x, points[e].z, points[q].x, points[q].z)) {
        break;
      }
      e = q;
      if (e == start) {
        e = INVALID;  
        break;
      }
    }
    
    if (e == INVALID) continue;  
    
    // Crear primer triángulo conectando P a la arista visible (
    unsigned int t = addTriangle(e, i, hull_next[e], INVALID, INVALID, hull_tri[e]);
    
    // Legalizar la nueva arista p-q y actualizar referencias
    hull_tri[i] = legalize(t + 2);  
    hull_tri[e] = t; 
    
    // Extender en sentido antihorario mientras sea visible
    unsigned int next = hull_next[e];
    while (true) {
      q = hull_next[next];
      // ¿Siguiente vértice también visible desde P?
      if (!orient(x, y, points[next].x, points[next].z, points[q].x, points[q].z)) {
        break; 
      }
      // Crear triángulo (next, P, q)
      t = addTriangle(next, i, q, hull_tri[i], INVALID, hull_tri[next]);
      hull_tri[i] = legalize(t + 2);
      hull_next[next] = next;  
      next = q;  
    }
    
    // Extender en sentido horario si es necesario
    if (e == start) {
      while (true) {
        q = hull_prev[e];
        // ¿Vértice anterior también visible desde P?
        if (!orient(x, y, points[q].x, points[q].z, points[e].x, points[e].z)) {
          break;
        }
        // Crear triángulo (q, P, e)
        t = addTriangle(q, i, e, INVALID, hull_tri[e], hull_tri[q]);
        legalize(t + 2);
        hull_tri[q] = t;
        hull_next[e] = e;
        e = q;
      }
    }
    
    // Actualizar envolvente convexa
    hull_prev[i] = e;
    hull_start = e;
    hull_prev[next] = i;
    hull_next[e] = i;
    hull_next[i] = next;
    
    // Actualizar tabla hash con nuevos vértices del casco
    m_hash[hashKey(x, y)] = i;
    m_hash[hashKey(points[e].x, points[e].z)] = e;
  }
}

double Delaunay::getHullArea() {
  vector<double> hull_area;
  unsigned int e = hull_start;
  
  do {
    double area = (points[e].x - points[hull_prev[e]].x) * (points[e].z + points[hull_prev[e]].z);
    hull_area.push_back(area);
    e = hull_next[e];
  } while (e != hull_start);
  
  double sum = 0.0;
  for (double val : hull_area) sum += val;
  return fabs(sum) * 0.5; 
}

// Legalizar aristas iterativamente
unsigned int Delaunay::legalize(unsigned int a) {
  unsigned int i = 0;           
  unsigned int ar = 0;
  m_edge_stack.clear();
  
  while (true) {
    unsigned int b = halfedges[a]; 
    
    // Calcular índices de vértices del triángulo que contiene a
    unsigned int a0 = 3 * (a / 3);          
    ar = a0 + (a + 2) % 3;
    
    // Si no hay arista opuesta, retroceder en pila o terminar
    if (b == INVALID) {
      if (i > 0) {
        i--;
        a = m_edge_stack[i];
        continue;
      } else {
        break;
      }
    }
    
    // Calcular todos los vértices del cuadrilátero formado por dos triángulos adyacentes
    unsigned int b0 = 3 * (b / 3);          
    unsigned int al = a0 + (a + 1) % 3;
    unsigned int bl = b0 + (b + 2) % 3;     
    
    unsigned int p0 = triangles[ar];  
    unsigned int pr = triangles[a];   
    unsigned int pl = triangles[al];  
    unsigned int p1 = triangles[bl];
    
    // Prueba de la circunferencia
    bool illegal = InCircle(
      points[p0].x, points[p0].z,
      points[pr].x, points[pr].z,
      points[pl].x, points[pl].z,
      points[p1].x, points[p1].z);
    
    if (illegal) {
      // Flipping de la arista
      triangles[a] = p1;
      triangles[b] = p0;
      
      // Actualizar referencias de aristas
      unsigned int hbl = halfedges[bl];
      
      // Si bl estaba en el casco, actualizar su triángulo incidente
      if (hbl == INVALID) {
        unsigned int e = hull_start;
        do {
          if (hull_tri[e] == bl) {
            hull_tri[e] = a;
            break;
          }
          e = hull_next[e];
        } while (e != hull_start);
      }
      
      // Re-enlazar todas las aristas afectadas
      link(a, hbl);
      link(b, halfedges[ar]);
      link(ar, bl);
      
      // Apilar nueva arista para verificar recursivamente
      unsigned int br = b0 + (b + 1) % 3;
      
      if (i < m_edge_stack.size()) {
        m_edge_stack[i] = br;
      } else {
        m_edge_stack.push_back(br);
      }
      i++;
    } else {
      if (i > 0) {
        i--;
        a = m_edge_stack[i];
        continue;
      } else {
        break;
      }
    }
  }
  
  return ar;
}

// Calcula hash basada en pseudo-ángulo 
unsigned int Delaunay::hashKey(double x, double y) const {
  double dx = x - m_center_x;
  double dy = y - m_center_y;
  double angle = pseudo_angle(dx, dy);
  unsigned int key = static_cast<unsigned int>(floor(angle * m_hash_size));
  return fast_mod(key, m_hash_size);
}

unsigned int Delaunay::addTriangle(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int a, unsigned int b, unsigned int c) {
  unsigned int t = triangles.size();  
  triangles.push_back(i0);
  triangles.push_back(i1);
  triangles.push_back(i2);
  
  // Enlazar cada arista con su twin
  link(t, a);      
  link(t + 1, b);  
  link(t + 2, c);  
  
  return t;
}

// Establece relación de dualidad entre dos aristas twins
void Delaunay::link(unsigned int a, unsigned int b) {
  // Establecer b como twin de a

  if (a == halfedges.size()) {
    halfedges.push_back(b);
  } else if (a < halfedges.size()) {
    halfedges[a] = b;
  } else {
    throw runtime_error("link: a fuera de rango");
  }
  
  // Si b es válida, establecer a como twin de b
  if (b != INVALID) {
    if (b == halfedges.size()) {
      halfedges.push_back(a);
    } else if (b < halfedges.size()) {
      halfedges[b] = a;
    } else {
      throw runtime_error("link: b fuera de rango");
    }
  }
}

#endif