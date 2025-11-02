#pragma once
#include "geo.hpp"

class Vertex{
    public:
    int key;
    double x, y;
};

class HalfEdge{
    public:
    int originVertex, endVertex;
    Vertex* origin, end;
    HalfEdge* twin;
};

class Face{
    public:
    int key = -1;
};

class VertexTable{
    public:
    Vertex* v;
    HalfEdge* e;
};

class FaceTable{
    public:
    Face* face = nullptr;
    vector<HalfEdge> innerComponent;
    HalfEdge* outerComponent = nullptr;
    float area = -1;
};

class HalfEdgeTable{
    public:
    HalfEdge *halfEdge, *next, *prev;
    Face* incidentFace = nullptr;
};

void printHalfEdge(vector<HalfEdge> &edges, vector<Vertex> &vertex){
    for(auto edge : edges){
        cout << "Arista :" << edge.originVertex << '-' << edge.endVertex<<'\n';
        cout << "Coordenadas : (" << vertex[edge.originVertex].x << " , " << vertex[edge.originVertex].y << ") , (" << vertex[edge.endVertex].x << " , " << vertex[edge.endVertex].y << ")\n";
        cout << "Twin edge: " << edge.twin->originVertex << '-' << edge.twin->endVertex << '\n';
    }
}

int searchHalfEdge(int s, int e, vector<HalfEdge> &edges){
    int n = sz(edges);
    for(int i = 0; i < n; i++){
        if(edges[i].originVertex == s && edges[i].endVertex == e){
            return i;
        }
    }
    cout << "No encontrado\n";
    return -1;
}

int searchHalfEdgeTable(HalfEdge* halfEdge, vector<HalfEdgeTable> halfEdgeTable){
    int n = sz(halfEdgeTable);
    for(int i = 0; i < n; i++){
        if(halfEdgeTable[i].halfEdge == halfEdge) return i;
    }
    return 0;
}

void fillVertexTable(vector<VertexTable>& vertexTable, vector<vector<float>> adj, vector<HalfEdge> &edges, vector<Vertex>& vertexes){
    int n = sz(vertexes);

    for(int i = 0; i < n; i++){
        int s = vertexes[i].key;
        int e = adj[s][0];
        vertexTable[i].v = &vertexes[i];
        vertexTable[i].e = &edges[searchHalfEdge(s,e,edges)];
    }
}

