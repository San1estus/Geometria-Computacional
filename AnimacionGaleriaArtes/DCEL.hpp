#ifndef DCEL_GEO
#define DCEL_GEO
#include "../geo.hpp"
#include <vector>
#include <algorithm>
#include <list>
#include <iostream> 
#include <iterator>
#include <map>
#include <set>
#include <stack>

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
    class Face *incidentFace = nullptr;
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
    Face(int key): key(key), incidentEdge(nullptr) {}
};

class DCEL
{
  public:
    vector<Vertex*> vertices;
    vector<HalfEdge*> halfEdges;
    vector<Face*> faces;
    int vertexCount = 0;

    // Esto es para el dibujo, para no hacer un algoritmo paso a paso
    vector<pair<int,int>> diags;
    vector<int> colors;
    vector<int> colorOrder;

    Face* outer = nullptr;
    Face* inner = nullptr;

    void init(const vector<Point>& points){
      int n = (int)points.size();
      vertices.clear();
      halfEdges.clear();
      faces.clear();
      diags.clear();
      colors.clear();
      colorOrder.clear();

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

        // Asignar arista saliente al vértice
        x->incidentEdge = e;
      }
      
      // Conectar aristas y asignar incidentFace
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
      // Fin creación de aristas

      // Asignar arista representante a las caras
      if(!halfEdges.empty()){
        inner->incidentEdge = halfEdges[0];
        outer->incidentEdge = halfEdges[1];
      } else {
        inner->incidentEdge = nullptr;
        outer->incidentEdge = nullptr;
      }

      vertexCount = (int)vertices.size();
    }

    void clear(){
      for(auto p: vertices) delete p;
      for(auto e: halfEdges) delete e;
      for(auto f: faces) delete f;
      vertices.clear();
      halfEdges.clear();
      faces.clear();
      diags.clear();
      colors.clear();
      colorOrder.clear();
      inner = outer = nullptr;
      vertexCount = 0;
    }

    void print(){
      for(auto vertex: vertices){
        cout << "Vertice: "<< vertex->key;
        if(vertex->incidentEdge) cout << " arista saliente: E"<< vertex->incidentEdge->key;
        cout << '\n';
      }
      for(auto edge: halfEdges){
        cout << "Arista: E" << edge->key << " del vertice " << edge->origin_v << " al vertice " << edge->end_v;
        if(edge->twin) cout << " twin E" << edge->twin->key;
        if(edge->incidentFace) cout << " en la cara "<< edge->incidentFace->key;
        cout << '\n';
      }
      for(auto face: faces){
        cout << "Cara: " << face->key;
        if(face->incidentEdge) cout << " incidentEdge E" << face->incidentEdge->key;
        cout << '\n';
      }
    }

    // ---------- Triangulación (ear clipping) ----------
    void triangulate(){
      if(inner == nullptr){
        cout << "No hay cara interna definida\n";
        return;
      }
      
      vector<Vertex*> remainingVertices = vertices;
      // Usando alguna otra estructura el borrado podía ser más rápido, pero como manejamos pocos puntos no hay mucho problema.

      while(remainingVertices.size() > 3){
        int m = (int)remainingVertices.size();
        bool earFound = false;
        for(int i = 0; i < m; ++i){
          if(!isEar(remainingVertices, i)) continue;

          // Oreja encontrada (i-1,i,i+)
          Vertex* a = remainingVertices[(i-1+m)%m];
          Vertex* b = remainingVertices[i];
          Vertex* c = remainingVertices[(i+1)%m];

          // Inicialmente todos pertenecen a inner, por lo cual las buscamos ahi
          HalfEdge* e_ab = findHalfEdge(a->key, b->key, inner);
          HalfEdge* e_bc = findHalfEdge(b->key, c->key, inner);
          
          HalfEdge* e_before = e_ab->prev; 
          HalfEdge* e_after  = e_bc->next; 

          if(e_before == nullptr || e_after == nullptr){
            cout << "Error topológico: prev/next es nullptr en triangulacion\n";
            return;
          }

          HalfEdge* d  = new HalfEdge(c, a); 
          HalfEdge* dt = new HalfEdge(a, c); 
          diags.push_back({c->key, a->key});

          int baseKey = (int)halfEdges.size();
          d->key = baseKey;
          dt->key = baseKey + 1;
          d->twin = dt;
          dt->twin = d;

          halfEdges.push_back(d);
          halfEdges.push_back(dt);

          Face* tri = new Face((int)faces.size());
          faces.push_back(tri);

          e_ab->next = e_bc;
          e_bc->prev = e_ab;

          e_bc->next = d;
          d->prev = e_bc;

          d->next = e_ab;
          e_ab->prev = d;

          e_ab->incidentFace = tri;
          e_bc->incidentFace = tri;
          d->incidentFace = tri;
          tri->incidentEdge = e_ab;

          e_before->next = dt;
          dt->prev = e_before;

          dt->next = e_after;
          e_after->prev = dt;
          
          dt->incidentFace = inner;

          inner->incidentEdge = e_after;

          a->incidentEdge = findHalfEdge(a->key, dt->end_v, inner); 
          if(a->incidentEdge == nullptr) a->incidentEdge = dt;

          c->incidentEdge = findHalfEdge(c->key, e_after->end_v, inner);
          if(c->incidentEdge == nullptr) c->incidentEdge = e_after;

          remainingVertices.erase(remainingVertices.begin() + i);

          earFound = true;
          break;
        } 

        if(!earFound){
          cout << "Error: no se encontro oreja (poligono posiblemente degenerado o no simple)\n";
          return;
        }
      } 

      // Crear la última cara triangular con las 3 aristas restantes
      HalfEdge* startFinal = inner->incidentEdge;
      if(startFinal == nullptr){
        cout << "Error: inner->incidentEdge == nullptr al finalizar\n";
        return;
      }

      HalfEdge* s = startFinal;
      vector<HalfEdge*> finalEdges;
      int safety = 0;
      do{
        finalEdges.push_back(s);
        s = s->next;
      } while(s != startFinal);

      Face* triLast = new Face((int)faces.size());
      faces.push_back(triLast);
      for(auto he: finalEdges){
        he->incidentFace = triLast;
      }
      triLast->incidentEdge = finalEdges[0];
      inner = triLast;
    }

    // ---------- Obtener indices para triangulación ----------
    // Devuelve vector de triángulos, regresa los indices para cada cara.
    vector<vector<int>> getTriangles(){
      vector<vector<int>> triangles;
      for(Face* f : faces){
        // Para debugear
        // if(f == nullptr) continue;
        //   cout << "face key = " << f->key 
        //        << " incidentEdge = " << (f->incidentEdge ? to_string(f->incidentEdge->key) : string("NULL")) << '\n';

        //   if (f->incidentEdge == nullptr) {
        //       cout << "WARNING: Face sin incidentEdge: " << f->key << '\n';
        //       continue;
        //   }

        if(f->key == -1) continue; 

        HalfEdge* e = f->incidentEdge;
        vector<int> verts;
        HalfEdge* cur = e;
        int safety = 0;
        do {
            if(cur == nullptr){
                cout << "Una arista no se asigno bien en la cara: " << f->key << '\n';
                break;
            }
            verts.push_back(cur->origin_v);
            cur = cur->next;
        } while(cur != e);

        if (verts.size() == 3) {
            triangles.push_back(verts);
        } else {
            cout << "Cara " << f->key << " tiene " << verts.size() << " vertices, ignorada\n";
        }

        cout << "Triangulo: ";
        for(int k : verts) cout << k << " ";
        cout << '\n';
      }
      return triangles;
    }

    // ---------- GRAFO DUAL ----------
    vector<vector<int>> buildDualGraph(const vector<vector<int>>& triangles) {
      int T = (int)triangles.size();
      vector<vector<int>> adj(T);
      // Mapa arista -> triángulo que la contiene
      map<pair<int,int>, int> edgeOwner;

      for(int ti = 0; ti < T; ++ti){
        const auto& tri = triangles[ti];
        if(tri.size() != 3) continue;
        for(int k = 0; k < 3; ++k){
          int a = tri[k];
          int b = tri[(k+1)%3];
          pair<int,int> e = (a < b) ? make_pair(a,b) : make_pair(b,a);
          auto it = edgeOwner.find(e);
          if(it == edgeOwner.end()){
            edgeOwner[e] = ti;
          } else {
            int tj = it->second;
            // Agregar adyacencia si no está ya
            if(find(adj[ti].begin(), adj[ti].end(), tj) == adj[ti].end()){
              adj[ti].push_back(tj);
            }
            if(find(adj[tj].begin(), adj[tj].end(), ti) == adj[tj].end()){
              adj[tj].push_back(ti);
            }
          }
        }
      }
      return adj;
    }

    // ---------- COLOREO DE VÉRTICES usando DFS en el grafo DUAL ----------
    vector<int> colorVerticesByDualDFS(const vector<vector<int>>& triangles) {
      int T = (int)triangles.size();
      vector<int> vertexColor(vertexCount, -1);
      if(T == 0) return vertexColor;

      auto dual = buildDualGraph(triangles);

      // Para marcar los triangulos visitados, char para optimizar (algo aunque sea) de espacio
      vector<char> visitedT(T, 0);

      stack<int> st;

      st.push(0);
      visitedT[0] = 1;

      // Asignamos lo del triangulo inicial y seguimos de ahi
      if(triangles[0].size() == 3){
        vertexColor[triangles[0][0]] = 0;
        vertexColor[triangles[0][1]] = 1;
        vertexColor[triangles[0][2]] = 2;
        colorOrder.push_back(triangles[0][0]);
        colorOrder.push_back(triangles[0][1]);
        colorOrder.push_back(triangles[0][2]);
      }

      while(!st.empty()){
        int t = st.top(); st.pop();
        
        for(int neighbor : dual[t]){
          if(visitedT[neighbor]) continue;

          // Como es vecino, comparten arista en consecuencia comparten vertices
          const auto &Ta = triangles[t];
          const auto &Tb = triangles[neighbor];

          vector<int> shared;
          for(int va : Ta) for(int vb : Tb) if(va == vb) shared.push_back(va);
          // Para que no haya duplicados
          sort(shared.begin(), shared.end());
          shared.erase(unique(shared.begin(), shared.end()), shared.end());

          int v0 = shared[0];
          int v1 = shared[1];

          int color_v0 = vertexColor[v0];
          int color_v1 = vertexColor[v1];

          // Para marcar los colores usados
          bool used[3] = {false,false,false};
          if(color_v0 >= 0 && color_v0 < 3) used[color_v0] = true;
          if(color_v1 >= 0 && color_v1 < 3) used[color_v1] = true;

          // Encontramos el vertices faltante
          int third = -1;
          for(int x : Tb) if(x != v0 && x != v1) { third = x; break; }

          // En caso que no haya (no debería)
          if(third == -1){
            visitedT[neighbor] = 1;
            st.push(neighbor);
            continue;
          }
          
          // Asignar el color restante al vertice restante
          int assign = -1;
          for(int c = 0; c < 3; ++c) if(!used[c]) { assign = c; break; }
          
          vertexColor[third] = assign;

          colorOrder.push_back(third);
          visitedT[neighbor] = 1;
          st.push(neighbor);
        }
      } 
      return vertexColor;
    }

    // ---------- La parte importante ----------
    // Devuelve índices de vértices donde colocar guardias (color con menos vértices)
    vector<int> getGuardsFromDual(){
      // Obtener triángulos actuales
      auto triangles = getTriangles();
      if(triangles.empty()){
        cout << "No hay triángulos para construir el grafo dual\n";
        return {};
      }

      // Colorear vértices mediante DFS sobre dual
      colors = colorVerticesByDualDFS(triangles);

      // Contar y elegir color mínimo
      int cnt[3] = {0,0,0};
      for(int c : colors) if(c>=0 && c<3) cnt[c]++;

      int best = 0;
      if (cnt[1] < cnt[best]) best = 1;
      if (cnt[2] < cnt[best]) best = 2;

      vector<int> guards;
      for(int i = 0; i < vertexCount; ++i){
        if(colors[i] == best) guards.push_back(i);
      }

      cout << "Frecuencia de cada color 0:" << cnt[0] << " 1:" << cnt[1] << " 2:" << cnt[2]
           << " Color con menos: " << best << " hay: " << guards.size() << "guardias \n";

      return guards;
    }

  // ---------- Para comprobar si los puntos son una oreja ----------
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
        if(area > 0) return (area1 > 0 && area2 > 0 && area3 > 0);
        else return (area1 < 0 && area2 < 0 && area3 < 0);
    }

    bool isEar(const vector<Vertex*>& verts, int i) {
        int n = verts.size();
        if(n < 3) return false;
        Vertex* a = verts[(i-1+n)%n];
        Vertex* b = verts[i];
        Vertex* c = verts[(i+1)%n];

        if(!isConvex(a,b,c)) return false;

        // Verificar si algún otro vértice está dentro
        for(int j=0;j<n;j++){
            if(j==i || j==(i-1+n)%n || j==(i+1)%n) continue;
            if(pointInTriangle(verts[j], a,b,c)) return false;
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
