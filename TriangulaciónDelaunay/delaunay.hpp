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
#include <iostream>

#define INVALID std::numeric_limits<unsigned int>::max()
#define EPSILON 1e-9  // Usando la misma EPS que en geo.hpp

using namespace std;

struct Point3 {
  double x, y, z;
  Point3() : x(0), y(0), z(0) {}
  Point3(double x0, double y0, double z0) : x(x0), y(y0), z(z0) {}
  Point3(Point p) : x(p.x), y(p.y), z(p.y) {}  // Note: z = y para 2D
  
  operator Point() const { return Point(x, z); }
  Point toPoint2D() const { return Point(x, z); }
};

inline double dist2(double ax, double ay, double bx, double by) {
  double dx = ax - bx;
  double dy = ay - by;
  return dx * dx + dy * dy;
}

inline double circumradius(double ax, double ay, double bx, double by, double cx, double cy) {
  double dx = bx - ax;
  double dy = by - ay;
  double ex = cx - ax;
  double ey = cy - ay;
  
  double bl = dx * dx + dy * dy;
  double cl = ex * ex + ey * ey;
  double d = dx * ey - dy * ex;
  
  if (fabs(d) < EPSILON) {
    return std::numeric_limits<double>::max();
  }
  
  double x = (ey * bl - dy * cl) * 0.5 / d;
  double y = (dx * cl - ex * bl) * 0.5 / d;
  
  return x * x + y * y;
}

inline bool orient(double px, double py, double qx, double qy, double rx, double ry) {
  return (qy - py) * (rx - qx) - (qx - px) * (ry - qy) < EPSILON;
}

inline bool in_circle(double ax, double ay, double bx, double by,
                     double cx, double cy, double px, double py) {
  double dx = ax - px;
  double dy = ay - py;
  double ex = bx - px;
  double ey = by - py;
  double fx = cx - px;
  double fy = cy - py;
  
  double ap = dx * dx + dy * dy;
  double bp = ex * ex + ey * ey;
  double cp = fx * fx + fy * fy;
  
  return (dx * (ey * cp - bp * fy) -
         dy * (ex * cp - bp * fx) +
         ap * (ex * fy - ey * fx)) < 0.0;
}

inline bool check_pts_equal(double x1, double y1, double x2, double y2) {
  return fabs(x1 - x2) <= EPSILON && fabs(y1 - y2) <= EPSILON;
}

inline double pseudo_angle(double dx, double dy) {
  double p = dx / (fabs(dx) + fabs(dy));
  return (dy > 0.0 ? 3.0 - p : 1.0 + p) / 4.0;
}

inline unsigned int fast_mod(unsigned int i, unsigned int c) {
  return i >= c ? i % c : i;
}

struct compare {
  const vector<Point3>& points;
  double cx, cy;
  
  compare(const vector<Point3>& p, double center_x, double center_y) 
    : points(p), cx(center_x), cy(center_y) {}
  
  bool operator()(unsigned int i, unsigned int j) const {
    double d1 = dist2(points[i].x, points[i].z, cx, cy);
    double d2 = dist2(points[j].x, points[j].z, cx, cy);
    
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
  double get_hull_area();
  
private:
  vector<unsigned int> m_hash;
  double m_center_x, m_center_y;
  unsigned int m_hash_size;
  vector<unsigned int> m_edge_stack;
  
  unsigned int legalize(unsigned int a);
  unsigned int hash_key(double x, double y) const;
  unsigned int add_triangle(unsigned int i0, unsigned int i1, unsigned int i2,
                           unsigned int a, unsigned int b, unsigned int c);
  void link(unsigned int a, unsigned int b);
};

Delaunay::Delaunay(const vector<Point3>& input) 
  : points(input), hull_start(0) {
  
  unsigned int n = points.size();
  if (n < 3) return;
  
  double min_x = numeric_limits<double>::max();
  double min_y = numeric_limits<double>::max();
  double max_x = numeric_limits<double>::lowest();
  double max_y = numeric_limits<double>::lowest();
  
  vector<unsigned int> ids;
  ids.reserve(n);
  
  for (unsigned int i = 0; i < n; i++) {
    double x = points[i].x;
    double y = points[i].z;  // Usar z como coordenada y 2D
    
    if (x < min_x) min_x = x;
    if (y < min_y) min_y = y;
    if (x > max_x) max_x = x;
    if (y > max_y) max_y = y;
    
    ids.push_back(i);
  }
  
  double cx = (min_x + max_x) / 2.0;
  double cy = (min_y + max_y) / 2.0;
  
  unsigned int i0 = INVALID;
  double min_dist = numeric_limits<double>::max();
  
  for (unsigned int i = 0; i < n; i++) {
    double d = dist2(points[i].x, points[i].z, cx, cy);
    if (d < min_dist) {
      i0 = i;
      min_dist = d;
    }
  }
  
  double i0x = points[i0].x;
  double i0y = points[i0].z;
  
  unsigned int i1 = INVALID;
  min_dist = numeric_limits<double>::max();
  
  for (unsigned int i = 0; i < n; i++) {
    if (i == i0) continue;
    double d = dist2(i0x, i0y, points[i].x, points[i].z);
    if (d < min_dist && d > EPSILON) {  // Evitar puntos coincidentes
      i1 = i;
      min_dist = d;
    }
  }
  
  if (i1 == INVALID) {
    throw runtime_error("Puntos colineales o coincidentes");
  }
  
  double i1x = points[i1].x;
  double i1y = points[i1].z;
  
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
  
  // Asegurar orientación CCW
  if (!orient(i0x, i0y, i1x, i1y, i2x, i2y)) {
    swap(i1, i2);
    swap(i1x, i2x);
    swap(i1y, i2y);
  }
  
  double dx = i1x - i0x;
  double dy = i1y - i0y;
  double ex = i2x - i0x;
  double ey = i2y - i0y;
  double bl = dx * dx + dy * dy;
  double cl = ex * ex + ey * ey;
  double d = dx * ey - dy * ex;
  
  m_center_x = i0x + (ey * bl - dy * cl) * 0.5 / d;
  m_center_y = i0y + (dx * cl - ex * bl) * 0.5 / d;
  
  sort(ids.begin(), ids.end(), compare(points, m_center_x, m_center_y));
  
  m_hash_size = static_cast<unsigned int>(ceil(sqrt(n)));
  m_hash.resize(m_hash_size, INVALID);
  
  hull_prev.resize(n, INVALID);
  hull_next.resize(n, INVALID);
  hull_tri.resize(n, INVALID);
  
  hull_start = i0;
  
  hull_next[i0] = i1;
  hull_prev[i2] = i1;
  hull_next[i1] = i2;
  hull_prev[i0] = i2;
  hull_next[i2] = i0;
  hull_prev[i1] = i0;
  
  hull_tri[i0] = 0;
  hull_tri[i1] = 1;
  hull_tri[i2] = 2;
  
  m_hash[hash_key(i0x, i0y)] = i0;
  m_hash[hash_key(i1x, i1y)] = i1;
  m_hash[hash_key(i2x, i2y)] = i2;
  
  unsigned int max_triangles = n < 3 ? 1 : 2 * n - 5;
  triangles.reserve(max_triangles * 3);
  halfedges.reserve(max_triangles * 3);
  
  add_triangle(i0, i1, i2, INVALID, INVALID, INVALID);
  
  double xp = numeric_limits<double>::quiet_NaN();
  double yp = numeric_limits<double>::quiet_NaN();
  
  for (unsigned int k = 0; k < n; k++) {
    unsigned int i = ids[k];
    double x = points[i].x;
    double y = points[i].z;
    
    // Saltar puntos duplicados
    if (k > 0 && check_pts_equal(x, y, xp, yp)) {
      continue;
    }
    xp = x;
    yp = y;
    
    // Saltar puntos del triángulo inicial
    if (i == i0 || i == i1 || i == i2) continue;
    
    unsigned int start = 0;
    unsigned int key = hash_key(x, y);
    
    for (unsigned int j = 0; j < m_hash_size; j++) {
      start = m_hash[fast_mod(key + j, m_hash_size)];
      if (start != INVALID && start != hull_next[start]) break;
    }
    
    start = hull_prev[start];
    unsigned int e = start;
    unsigned int q;
    
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
    
    unsigned int t = add_triangle(e, i, hull_next[e], 
                                 INVALID, INVALID, hull_tri[e]);
    
    hull_tri[i] = legalize(t + 2);
    hull_tri[e] = t;
    
    unsigned int next = hull_next[e];
    while (true) {
      q = hull_next[next];
      if (!orient(x, y, points[next].x, points[next].z, points[q].x, points[q].z)) {
        break;
      }
      t = add_triangle(next, i, q, hull_tri[i], INVALID, hull_tri[next]);
      hull_tri[i] = legalize(t + 2);
      hull_next[next] = next;
      next = q;
    }
    
    if (e == start) {
      while (true) {
        q = hull_prev[e];
        if (!orient(x, y, points[q].x, points[q].z, points[e].x, points[e].z)) {
          break;
        }
        t = add_triangle(q, i, e, INVALID, hull_tri[e], hull_tri[q]);
        legalize(t + 2);
        hull_tri[q] = t;
        hull_next[e] = e;
        e = q;
      }
    }
    
    hull_prev[i] = e;
    hull_start = e;
    hull_prev[next] = i;
    hull_next[e] = i;
    hull_next[i] = next;
    
    m_hash[hash_key(x, y)] = i;
    m_hash[hash_key(points[e].x, points[e].z)] = e;
  }
}

double Delaunay::get_hull_area() {
  vector<double> hull_area;
  unsigned int e = hull_start;
  
  do {
    double area = (points[e].x - points[hull_prev[e]].x) * 
                 (points[e].z + points[hull_prev[e]].z);
    hull_area.push_back(area);
    e = hull_next[e];
  } while (e != hull_start);
  
  double sum = 0.0;
  for (double val : hull_area) sum += val;
  return fabs(sum) * 0.5;  // Área del polígono
}

unsigned int Delaunay::legalize(unsigned int a) {
  unsigned int i = 0;
  unsigned int ar = 0;
  m_edge_stack.clear();
  
  while (true) {
    unsigned int b = halfedges[a];
    
    unsigned int a0 = 3 * (a / 3);
    ar = a0 + (a + 2) % 3;
    
    if (b == INVALID) {
      if (i > 0) {
        i--;
        a = m_edge_stack[i];
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
    
    bool illegal = in_circle(
      points[p0].x, points[p0].z,
      points[pr].x, points[pr].z,
      points[pl].x, points[pl].z,
      points[p1].x, points[p1].z);
    
    if (illegal) {
      triangles[a] = p1;
      triangles[b] = p0;
      
      unsigned int hbl = halfedges[bl];
      
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
      
      link(a, hbl);
      link(b, halfedges[ar]);
      link(ar, bl);
      
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

unsigned int Delaunay::hash_key(double x, double y) const {
  double dx = x - m_center_x;
  double dy = y - m_center_y;
  double angle = pseudo_angle(dx, dy);
  unsigned int key = static_cast<unsigned int>(floor(angle * m_hash_size));
  return fast_mod(key, m_hash_size);
}

unsigned int Delaunay::add_triangle(unsigned int i0, unsigned int i1, unsigned int i2,
                                   unsigned int a, unsigned int b, unsigned int c) {
  unsigned int t = triangles.size();
  triangles.push_back(i0);
  triangles.push_back(i1);
  triangles.push_back(i2);
  
  link(t, a);
  link(t + 1, b);
  link(t + 2, c);
  
  return t;
}

void Delaunay::link(unsigned int a, unsigned int b) {
  if (a == halfedges.size()) {
    halfedges.push_back(b);
  } else if (a < halfedges.size()) {
    halfedges[a] = b;
  } else {
    throw runtime_error("link: a fuera de rango");
  }
  
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