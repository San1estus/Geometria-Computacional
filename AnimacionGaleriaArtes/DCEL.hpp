#ifndef DCEL_GEO
#define DCEL_GEO
#include "../geo.hpp"
#include <vector>
#include <list>
#include <iostream> 
#include <iterator> 

using namespace std;

class Vertex
{
  public:
    int key;
    float x, y;
    class HalfEdge* incidentEdge;
    Vertex(int key, float x, float y) : key(key), x(x), y(y), incidentEdge(nullptr) {}
};

class HalfEdge
{
  public:
    int origin_v, end_v, key;
    class Vertex *origin = nullptr, *end= nullptr;
    class HalfEdge *twin= nullptr, *next=nullptr, *prev=nullptr;
    class Face *incidentFace = nullptr;;
    HalfEdge(Vertex* a, Vertex* b){
      origin_v = a->key;
      end_v = b->key;
      origin = a; end = b;
    }
};

class Face
{
  public:
    int key = -1;
    class HalfEdge* incidentEdge = nullptr;
    Face(int key): key(key) {}
};

class DCEL
{
  public:
    vector<Vertex*> vertices;
    vector<HalfEdge*> halfEdges;
    vector<Face*> faces;
    Face* outer;
    Face* inner;

    void init(const vector<Point>& points){
      int n = points.size();
      vertices.reserve(n);
      halfEdges.reserve(2*n);
      faces.reserve(2);

      // Crear caras
      inner = new Face(0);
      faces.push_back(inner);

      outer = new Face(-1);
      faces.push_back(outer);

      // Crear vertices
      for(int i = 0; i < n; i++){
        vertices.push_back(new Vertex(i, points[i].x, points[i].y));
      }

      // Crear aristas
      for(int i = 0; i < n; i++){
        Vertex *x = vertices[i], *y = vertices[(i+1)%n];
        HalfEdge *e = new HalfEdge(x,y);
        HalfEdge *t = new HalfEdge(y,x);

        e->twin = t;
        t->twin = e;

        e->key = 2*i;
        t->key = 2*i+1;

        halfEdges.push_back(e);
        halfEdges.push_back(t);

        x->incidentEdge = e;
      }
      
      // Conectar aristas
      for(int i = 0; i < n; i++){
        HalfEdge *e = halfEdges[2*i];
        HalfEdge *t = halfEdges[2*i+1];

        HalfEdge *e_next  = halfEdges[2*((i+1)%n)];
        HalfEdge *e_prev  = halfEdges[2*((i-1+n)%n)];

        HalfEdge *t_next  = halfEdges[2*((i-1+n)%n)+1];
        HalfEdge *t_prev  = halfEdges[2*((i+1)%n)+1];

        e->next = e_next;
        e->prev = e_prev;

        t->next = t_next;
        t->prev = t_prev;

        e->incidentFace = inner;
        t->incidentFace = outer;
      }

      inner->incidentEdge = halfEdges[0];
      outer->incidentEdge = halfEdges[1];
    }
    void clear(){
      for(auto p: vertices) delete p;
      for(auto e: halfEdges) delete e;
      for(auto f: faces) delete f;
      vertices.clear();
      halfEdges.clear();
      faces.clear();
      inner = outer = nullptr;
    }
    void print(){
      for(auto vertex: vertices){
        cout << "Vertice: "<< vertex->key << " arista saliente: E"<< vertex->incidentEdge->key << '\n';
      }
      for(auto edge: halfEdges){
        cout << "Arista: E" << edge->key << " del vertice " << edge->origin_v << " al vertice " << edge->end_v << " twin E" << edge->twin->key << " en la cara "<< edge->incidentFace->key << '\n';
      }
      for(auto face: faces){
        cout << "Cara: " << face->key << '\n';
      }
    }
    void triangulate(){
      if(inner == nullptr){
        cout << "No hay cara interna definida\n";
        return;
      }
      while(vertices.size() > 3){
        int m = (int)vertices.size();
        bool earFound = false;
        for(int i = 0; i < m; ++i){
          if(!isEar(vertices, i)) continue;

          // oreja encontrada en vertices (a,b,c)
          Vertex* a = vertices[(i-1+m)%m];
          Vertex* b = vertices[i];
          Vertex* c = vertices[(i+1)%m];

          // halfedges actuales en la DCEL que representan a->b y b->c (deben pertenecer a inner)
          HalfEdge* e_ab = findHalfEdge(a->key, b->key, inner);
          HalfEdge* e_bc = findHalfEdge(b->key, c->key, inner);

          if(e_ab == nullptr || e_bc == nullptr){
            // si algo raro pasa, intentar encontrar sin filtrar por face
            e_ab = findHalfEdge(a->key, b->key, nullptr);
            e_bc = findHalfEdge(b->key, c->key, nullptr);
            if(e_ab == nullptr || e_bc == nullptr){
              cout << "Error interno: no se encontraron las aristas a->b o b->c en la DCEL\n";
              return;
            }
          }

          // vecinos en el ciclo
          HalfEdge* e_before = e_ab->prev; // termina en a
          HalfEdge* e_after  = e_bc->next; // empieza en c

          // Crear diagonal: d = c->a (pertenece al triángulo), dt = a->c (pertenece a la cara restante)
          HalfEdge* d  = new HalfEdge(c, a); // c->a
          HalfEdge* dt = new HalfEdge(a, c); // a->c
          d->key = (int)halfEdges.size();
          dt->key = (int)halfEdges.size() + 1;
          d->twin = dt;
          dt->twin = d;

          halfEdges.push_back(d);
          halfEdges.push_back(dt);
          // Crear nueva cara triangular
          Face* tri = new Face((int)faces.size());
          faces.push_back(tri);

          // Ajustes para la cara triangular (a,b,c) : ciclo a->b, b->c, c->a (d)
          // e_ab (a->b) , e_bc (b->c) , d (c->a)
          e_ab->next = e_bc;
          e_bc->prev = e_ab;

          e_bc->next = d;
          d->prev = e_bc;

          d->next = e_ab;
          e_ab->prev = d;

          // asignar incidentFace del triángulo
          e_ab->incidentFace = tri;
          e_bc->incidentFace = tri;
          d->incidentFace = tri;
          tri->incidentEdge = e_ab;

          // Ajustes para la cara restante (inner) : quitamos b y conectamos a->c (dt)
          // e_before -> ... -> a (termina en a) ; e_after (empieza en c) ...
          e_before->next = dt;
          dt->prev = e_before;

          dt->next = e_after;
          e_after->prev = dt;

          // dt pertenece a la cara restante (inner)
          dt->incidentFace = inner;

          // Los edges que antes pertenecían a inner y que no cambiaron siguen con inner (ya estaban así).
          // Necesitamos actualizar incidentEdge de inner si apuntaba a una arista eliminada (por ejemplo e_ab o e_bc)
          if(inner->incidentEdge == e_ab || inner->incidentEdge == e_bc){
            inner->incidentEdge = dt; // asignar a una arista válida del ciclo restante
          }

          // actualizar incidentEdge de vértices a y c si apuntaban a aristas 'eliminadas'
          // elegir una arista saliente válida para cada vértice en la cara restante
          a->incidentEdge = findHalfEdge(a->key, dt->end_v, inner); 
          if(a->incidentEdge == nullptr) a->incidentEdge = dt;

          c->incidentEdge = findHalfEdge(c->key, e_after->end_v, inner);
          if(c->incidentEdge == nullptr) c->incidentEdge = e_after;

          // b queda eliminado del ciclo restante; su incidentEdge puede permanecer, pero no será usado en inner
          // Nota: no borramos e_ab ni e_bc de halfEdges porque siguen existiendo (pertenecen al triángulo ahora)

          // lista 'vertices' se actualiza: quitar el vértice i (b)
          vertices.erase(vertices.begin() + i);
          // también actualizar la lista de edges si la mantuviéramos; en este algoritmo usamos findHalfEdge
          earFound = true;
          break;
        } // for i

        if(!earFound){
          cout << "Error: no se encontró oreja (polígono posiblemente degenerado o no simple)\n";
          return;
        }
      } // while > 3

      // Al final quedan 3 vértices -> crear la última cara triangular con las 3 aristas restantes
      // reconstruir ciclo final tomando inner->incidentEdge
      HalfEdge* startFinal = inner->incidentEdge;
      if(startFinal == nullptr){
        cout << "Error: inner->incidentEdge == nullptr al finalizar\n";
        return;
      }
      HalfEdge* s = startFinal;
      vector<HalfEdge*> finalEdges;
      do{
        finalEdges.push_back(s);
        s = s->next;
      } while(s != startFinal);

      // crear la última cara
      Face* triLast = new Face((int)faces.size());
      faces.push_back(triLast);
      for(auto he: finalEdges){
        he->incidentFace = triLast;
      }

      triLast->incidentEdge = finalEdges[0];
      inner = triLast;
    }
    
  private:
    bool isConvex(Vertex* a, Vertex* b, Vertex* c) {
      float cross = (b->x - a->x)*(c->y - a->y) - (b->y - a->y)*(c->x - a->x);
      return cross > 0; 
    }
    
    bool pointInTriangle(Vertex* p, Vertex* a, Vertex* b, Vertex* c) {
        float area = (b->x - a->x)*(c->y - a->y) - (b->y - a->y)*(c->x - a->x);
        float area1 = (a->x - p->x)*(b->y - p->y) - (a->y - p->y)*(b->x - p->x);
        float area2 = (b->x - p->x)*(c->y - p->y) - (b->y - p->y)*(c->x - p->x);
        float area3 = (c->x - p->x)*(a->y - p->y) - (c->y - p->y)*(a->x - p->x);
        return (area > 0 && area1 > 0 && area2 > 0 && area3 > 0);
    }

    bool isEar(const vector<Vertex*>& vertices, int i) {
        int n = vertices.size();
        Vertex* a = vertices[(i-1+n)%n];
        Vertex* b = vertices[i];
        Vertex* c = vertices[(i+1)%n];

        if(!isConvex(a,b,c)) return false;

        // Verificar si algún otro vértice está dentro
        for(int j=0;j<n;j++){
            if(j==i || j==(i-1+n)%n || j==(i+1)%n) continue;
            if(pointInTriangle(vertices[j], a,b,c)) return false;
        }
        return true;
    }

    HalfEdge* findHalfEdge(int origin_k, int end_k, Face* faceFilter = nullptr){
      for(auto e: halfEdges){
        if(e->origin_v == origin_k && e->end_v == end_k){
          if(faceFilter == nullptr) return e;
          if(e->incidentFace == faceFilter) return e;
        }
      }
      return nullptr;
    }
};
#endif