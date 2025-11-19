#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "DCEL.hpp"

using namespace std;

vector<Point> vertices;
int diagIndex = 0;
float timer = 0.0f;
float delay = 0.5f;
// Para dibujar diagonales
vector<Point> diags;
// Para dibujar guardias y coloreo
vector<int> colorVertices;
vector<int> selectedGuards;
int colorIndex = 0;
bool colorFinished = 0;
int selectedGuard = 0;
// Para poligonos de visibilidad
map<int, vector<Point>> visPolys;
// La mera mera
DCEL dcel;

float getRaySegmentIntersection(Point p, Point r, Point a, Point b) {
    Point s = b-a;
    Point q_minus_p = a-p;

    float r_cross_s = cross(r, s);

    if (abs(r_cross_s) < 1e-5f) return -1.0f;

    float t = cross(q_minus_p, s) / r_cross_s;
    float u = cross(q_minus_p, r) / r_cross_s;

    if (t > 1e-4f && u >= 0.0f && u <= 1.0f) {
        return t;
    }

    return -1.0f;
}

struct AngularPoint {
    Point p;
    float angle;
    float dist;

    // Para ordenar por ángulo
    bool operator<(const AngularPoint& other) const {
        return angle < other.angle;
    }
};

Point virtualGuard(int vertexIndex){
    Vertex* v = dcel.vertices[vertexIndex];
    HalfEdge* e = v->incidentEdge;

    Vertex* v1 = e->origin;          // Este es el guardia
    Vertex* v2 = e->next->origin;
    Vertex* v3 = e->next->next->origin;

    // Calculamos el centroide de este triángulo pequeño
    float centroidX = (v1->x + v2->x + v3->x) / 3.0f;
    float centroidY = (v1->y + v2->y + v3->y) / 3.0f;

    // --- AQUI ESTA LA MAGIA ---
    // Creamos un vector dirección: Vértice -> Centroide
    float dirX = centroidX - v1->x;
    float dirY = centroidY - v1->y;

    // Factor de desplazamiento pequeño (ej. 1% de la distancia al centro)
    // Esto despega al guardia de la pared lo suficiente para que las matemáticas no fallen,
    // pero visualmente es imperceptible para el polígono resultante.
    float epsilonFactor = 0.01f; 

    return Point(v1->x + dirX * epsilonFactor, v1->y + dirY * epsilonFactor);
}

vector<Point> visibilityPolygon(Point p) {
    vector<AngularPoint> detectedPoints;
    int n = vertices.size();

    // Algoritmo:
    // 1. Para cada vértice v del polígono (el posible destino del rayo)
    for (int i = 0; i < n; ++i) {
        Point v = vertices[i];
        
        // Definir dirección del rayo (r) y distancia máxima inicial (al vértice objetivo)
        Point r = v-p;
        
        // Angulo theta (para ordenar después)
        float theta = atan2(r.y, r.x);
        
        // Distancia inicial asumiendo que el vértice v es visible
        float maxDist = sqrt(r.x*r.x + r.y*r.y);
        float minT = 1.0f; // t=1 significa que llegamos exactamente al vértice v (p + 1*r = v)

        // 2. Verificar intersección con TODOS los obstáculos (aristas del polígono)
        for (int j = 0; j < n; ++j) {
            Point edgeStart = vertices[j];
            Point edgeEnd = vertices[(j + 1) % n];

            float t = getRaySegmentIntersection(p, r, edgeStart, edgeEnd);

            // Si hay intersección y es más cercana que la actual
            if (t != -1.0f) {
                if (t < minT) {
                    minT = t;
                }
            }
        }

        // Calculamos el punto final real de visibilidad
        Point result;
        result.x = p.x + r.x * minT;
        result.y = p.y + r.y * minT;

        detectedPoints.push_back({result, theta, maxDist * minT});
    }

    // 3. Ordenar por ángulo para poder dibujarlo como un TRIANGLE_FAN
    sort(detectedPoints.begin(), detectedPoints.end());

    // Convertir de regreso a vector<Point>
    vector<Point> resultPoly;
    for (const auto& ap : detectedPoints) {
        resultPoly.push_back(ap.p);
    }
    
    // Cerramos el ciclo repitiendo el primer punto si es necesario para GL_LINE_STRIP,
    // aunque para GL_TRIANGLE_FAN no es estrictamente necesario.
    if (!resultPoly.empty()) {
        resultPoly.push_back(resultPoly[0]);
    }

    return resultPoly;
}

// Para dibujar puntos
void mouseCallback(GLFWwindow* window, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){

        double posX, posY;
        glfwGetCursorPos(window, &posX, &posY);
        cout << "(x,y) coordenadas: "<< posX << " " << posY << '\n';

        int width, height;
		glfwGetWindowSize(window, &width, &height);
        float x = -1.0f + 2 * posX / width;
        float y = 1.0f - 2 * posY / height;
        Point newPoint(x, y);
        cout << "X: " << x << '\n';
        cout << "Y: " << y << '\n';
        
        vertices.push_back(newPoint);
        
        int verticeSize = vertices.size();
        cout << verticeSize << '\n';
    }
}

bool finished = false;


// Para hacer los pasos
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(mods == GLFW_MOD_CONTROL && action == GLFW_PRESS){
        switch(key){
            case GLFW_KEY_Z:
                vertices.pop_back();
                if(finished){
                    dcel.clear();
                    finished = false;
                }
                break;
        }
    }
    else if(action == GLFW_PRESS){
        switch (key){
            case GLFW_KEY_R:
                vertices.clear();
                dcel.clear();
                diags.clear();
                selectedGuards.clear();
                colorVertices.clear();
                visPolys.clear();
                finished = false;
                colorFinished = false;
                diagIndex = 0;
                colorIndex = 0;
                selectedGuard = 0;
                break;
            case GLFW_KEY_C:
                if(!finished){
                    finished = true;
                    dcel.init(vertices);
                    colorVertices.assign(vertices.size(), -1);
                }
                break;
            case GLFW_KEY_T:
                if(finished){
                    dcel.diags.clear();
                    dcel.triangulate();
                }
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_P:
                if(finished){
                    dcel.print();
                }
                break;
            case GLFW_KEY_G:
                if (finished) {

                    // Triangulación si no está hecha
                    if (dcel.diags.empty()) {
                        cout << "Triangulando DCEL antes de colorear.\n";
                        dcel.triangulate();
                    }

                    selectedGuards = dcel.getGuardsFromDual();

                    cout << "Guardias necesarios: " << selectedGuards.size() << "\n";
                    for (int g : selectedGuards)
                        cout << "Guardia en vertice: " << g 
                            << " pos=(" << vertices[g].x << "," << vertices[g].y << ")\n";
                    
                }
                break;    
            case GLFW_KEY_V:
                if(!selectedGuards.empty()){
                    if(visPolys.find(selectedGuard) == visPolys.end()){
                        visPolys[selectedGuard] = visibilityPolygon(virtualGuard(selectedGuards[selectedGuard]));
                    }
                    selectedGuard = (selectedGuard+1)%selectedGuards.size();
                }
                break;
            }
        }
}

void drawPoint(Point p, float r, float g, float b, float size = 10.0f) {
    glPointSize(size);
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
    glVertex2f(p.x, p.y ); 
    glEnd();
}

void drawColoredVertex(int v, int colorIndex) {
    float r,g,b;

    switch(colorIndex){
        case 0: r = 0.26f; g = 0.52f; b = 0.96f; break;   
        case 1: r = 1.0f;  g = 0.60f; b = 0.0f;  break;   
        case 2: r = 0.30f; g = 0.85f; b = 0.55f; break;   
    }

    drawPoint({vertices[v].x, vertices[v].y}, r,g,b);
}

void drawLine(Point a, Point b, float r, float g, float bl, float width = 3.0f) {
    glLineWidth(width);
    glColor3f(r, g, bl);
    glBegin(GL_LINES);
    glVertex2f(a.x , a.y);
    glVertex2f(b.x, b.y);
    glEnd();
}



void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if(visPolys.size() != 0){
        for(int i = 0; i < visPolys.size(); i++){
            glColor4f(1.0f, 1.0f, 0.0f, 0.5f); 
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(vertices[selectedGuards[i]].x, vertices[selectedGuards[i]].y);
            for(auto& p : visPolys[i]){
                glVertex2f(p.x, p.y);
            }
            glEnd();
        }
    }
    
    for (auto& p : vertices) drawPoint(p, 1, 0, 0);
    if (vertices.size() > 1) {
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_STRIP);
        for (auto& p : vertices)
            glVertex2f(p.x, p.y);
            if(finished){
                glVertex2f(vertices.begin()->x,vertices.begin()->y);
            }
        glEnd();
    }
    
    for(int i = 0; i + 1 < diags.size(); i += 2){
        drawLine(diags[i], diags[i+1], 0.0f, 1.0f, 0.0f, 2.0f);
    }

    if(!colorFinished){
        for(int i = 0; i < colorVertices.size(); i++){
            if(colorVertices[i] != -1)
                drawColoredVertex(i, colorVertices[i]);
        }
    }

    else{
        for (int g : selectedGuards) {
            drawPoint({vertices[g].x, vertices[g].y}, 0, 1, 0);
        }
    }
    
}

// ---------------------------- IMPORTANTISISISISIMO ---------------------------- //
// Los vertices tienen que estar dados en sentido antihorario o no va a funcionar la triangulacion

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Galeria", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        cout << "Error" << '\n';
    }

    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);

    float timePerStep = 0.3f;
    
    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        float now = glfwGetTime();

        if (now - lastTime > timePerStep) {
            lastTime = now;
            if(diagIndex < dcel.diags.size()){
                
                auto [ak, ck] = dcel.diags[diagIndex];

                Point A = vertices[ak];
                Point C = vertices[ck];

                diags.push_back(A);
                diags.push_back(C);
                
                diagIndex++;
            }

            if(colorIndex < dcel.colorOrder.size()){
                int v = dcel.colorOrder[colorIndex];   
                int c = dcel.colors[v];                
                colorVertices[v] = c;                  
                colorIndex++;
                colorFinished = (colorIndex == dcel.colorOrder.size());
            }

        }


        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    

    glfwTerminate();
    return 0;
}