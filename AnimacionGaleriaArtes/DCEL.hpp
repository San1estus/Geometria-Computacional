#ifndef DCEL
#define DCEL
#include "../geo.hpp"
#include <vector>
#include <list>
#include <iostream> 
#include <iterator> // Para std::next

using namespace std;

// Declaraciones anticipadas
class HalfEdge;
class Face; 
class Vertex;

// Definición de las clases con IDs en sus constructores
class Face{
    public:
    int id; // Nuevo ID
    HalfEdge* edge;
    Face(int id): edge(nullptr), id(id) {}
    int getEdgeCount();
};

class HalfEdge{
    public:
    int id; // Nuevo ID
    HalfEdge* twin;
    Vertex* origin;
    HalfEdge* next;
    Face* face;
    bool visited;

    HalfEdge(int id): origin(nullptr), twin(nullptr), next(nullptr), face(nullptr), visited(false), id(id) {}
    Vertex* getDestination();
    HalfEdge* getPrevious();
    Face* getFace();
};

class Vertex{
    public:
    int id; // Nuevo ID
    Point point;
    HalfEdge* leaving;

    Vertex(const Point& point, int id): point(point), leaving(nullptr), id(id) {}
    HalfEdge* getEdgeTo(Vertex* node);
};


class DoubleConnectedEdgeList{
    public:
    list<Vertex*> vertices;
    list<HalfEdge*> edges;
    list<Face*> faces;
    
    // Contadores de IDs
    int next_vertex_id = 0;
    int next_halfedge_id = 0;
    int next_face_id = 0;

    DoubleConnectedEdgeList(const vector<Point>& points){
        init(points);
    }
    ~DoubleConnectedEdgeList(){
        for(auto edge: edges) delete edge;
        for(auto face: faces) delete face;
        for(auto vertex: vertices) delete vertex;
    }
    void init(const vector<Point>& points){
        int n = points.size(); 
        
        Face* face = new Face(next_face_id++); // Asignar ID
        faces.push_back(face);
        
        HalfEdge* prevLeft = nullptr;
        HalfEdge* prevRight = nullptr;

        for(int i = 0; i < n; i++){
            Point point = points[i];
            Vertex* vertex = new Vertex(point, next_vertex_id++); // Asignar ID
            HalfEdge* left =  new HalfEdge(next_halfedge_id++);  // Asignar ID
            HalfEdge* right = new HalfEdge(next_halfedge_id++); // Asignar ID  

            left->face = face;
            left->next = nullptr;
            left->origin = vertex;
            left->twin = right;

            right->face = nullptr;
            right->next = prevRight;
            right->origin = nullptr;
            right->twin = left;

            edges.push_back(left);
            edges.push_back(right);

            vertex->leaving = left;
            vertices.push_back(vertex);

            if(prevLeft !=nullptr){
                prevLeft->next = left;
            }
            if(prevRight !=nullptr){
                prevRight->origin = vertex;
            }

            prevLeft = left;
            prevRight = right;
        }

        HalfEdge* firstLeft = edges.front();
        prevLeft->next = firstLeft;
        
        auto second = next(edges.begin(), 1); 
        HalfEdge* firstRight = *second;
        firstRight->next = prevRight;

        prevRight->origin = vertices.front();
        face->edge = firstLeft;
    }

    HalfEdge* getPreviousEdge(Vertex* vertex, Face* face){
        HalfEdge* twin = vertex->leaving->twin;
        HalfEdge* edge = vertex->leaving->twin->next->twin;
        while(edge != twin){
            if(edge->face == face){
                return edge;
            }
            edge = edge->next->twin;
        }

        return edge;
    }
    
    Face* getReferenceFace(Vertex* v1, Vertex* v2){
        if (v1->leaving->face == v2->leaving->face)
        {
            return v1->leaving->face;
        }
            
        auto e1 = v1->leaving->twin->next->twin;
        while (e1 != v1->leaving->twin)
        {
            auto e2 = v2->leaving->twin->next->twin;
            while (e2 != v2->leaving->twin)
            {
                if (e1->face == e2->face) 
                {
                    return e1->face;
                }
                e2 = e2->next->twin;
            }
            e1 = e1->next->twin;
        }

        return v1->leaving->face;
    }

    void addHalfEdges(Vertex* v1, Vertex* v2){
        Face* face = new Face(next_face_id++); // Asignar ID

        HalfEdge* left = new HalfEdge(next_halfedge_id++); // Asignar ID
        HalfEdge* right = new HalfEdge(next_halfedge_id++); // Asignar ID

        Face* referenceFace = getReferenceFace(v1, v2);

        HalfEdge* prev1 = getPreviousEdge(v1, referenceFace);
        HalfEdge* prev2 = getPreviousEdge(v2, referenceFace);

        face->edge = left;
        referenceFace->edge = right;

        left->face = face;
        left->next = prev2->next;
        left->origin = v1;
        left->twin = right;


        right->face = referenceFace;
        right->next = prev1->next;
        right->origin = v2;
        right->twin = left;

        prev1->next = left;
        prev2->next = right;

        HalfEdge* curr = left->next;


        while(curr != left){
            curr->face = face;
            curr = curr->next;
        }

        edges.push_back(left);
        edges.push_back(right);

        faces.push_back(face);
    }

    void removeHalfEdges(HalfEdge* edge){
        Face* face =edge->twin->face;
        
        HalfEdge* ePrev = edge->getPrevious();
        HalfEdge* eNext = edge->next;
        HalfEdge* tPrev = edge->twin->getPrevious();
        HalfEdge* tNext = edge->twin->next;

        ePrev->next = tNext;
        tPrev->next = eNext;

        face->edge =  eNext;

        HalfEdge* te = eNext;
        while(te != tNext){ // Condición corregida
            te->face = face;
            te = te->next;
        }
        
        faces.remove(edge->face);
        delete edge->face;

        HalfEdge* twin = edge->twin;
        edges.remove(edge);
        edges.remove(twin);

        delete edge;
        delete twin;
    }

    void printDCEL(){
        cout << "------------Vertices (" << vertices.size() << ")-----------\n";
        for(auto vertex : vertices){
            cout << "V" << vertex->id << ": ("<< vertex->point.x << ", " << vertex->point.y << ") - Leaving E" << vertex->leaving->id << "\n";
        }
        cout << "\n------------Faces (" << faces.size() << ")-----------\n";
        for(auto face : faces){
            cout << "F" << face->id << ": Edges=" << face->getEdgeCount() << " | Ref HalfEdge E" << face->edge->id << '\n';
        }
        cout << "\n------------Half-Edges (" << edges.size() << ")-----------\n";
        for(auto edge : edges){
        string face_id_str;
        if (edge->face == nullptr) {
            face_id_str = "F-1 (Exterior)";
        } else {
            face_id_str = "F" + to_string(edge->face->id);
        }

        cout << "E" << edge->id << ": Origin V" << edge->origin->id;
        cout << " -> Dest V" << edge->getDestination()->id << " (Face " << face_id_str << ")\n";
        cout << "    |-> Next E" << edge->next->id << " | Twin E" << edge->twin->id << "\n";
    }
    }
};

// Métodos de Vertex
HalfEdge* Vertex::getEdgeTo(Vertex* node){
    if(leaving != nullptr){
        if(leaving->twin->origin == node){
            return leaving;
        }
        else{
            HalfEdge* edge = leaving->twin->next;
            while(edge != leaving){
                if(edge->twin->origin == node){
                    return edge;
                }
                else {
                    edge = edge->twin->next;
                }
            }
        }
    }
    return nullptr;
}

// Métodos de HalfEdge
Vertex* HalfEdge::getDestination(){
    return this->next->origin;
}

HalfEdge* HalfEdge::getPrevious(){
    HalfEdge* current = this->next;
    
    if (current == this) return this; 

    while(current->next != this){
        current = current->next;
    }
    return current;
}
Face* HalfEdge::getFace(){
    return this->face;
}

// Métodos de Face
int Face::getEdgeCount(){
    HalfEdge* edge = this->edge;
    int count = 0;
    if(edge != nullptr){
        count++;
        while(edge->next != this->edge){
            count++;
            edge = edge->next;
        }
    }
    return count;
}
#endif