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

// La mera mera
DCEL dcel;

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