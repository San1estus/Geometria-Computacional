#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "DCEL.hpp"

using namespace std;

vector<Point> vertices;
vector<Point> diags;
DCEL dcel;

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
                diags.clear();
                dcel.clear();
                finished = false;
                break;
            case GLFW_KEY_C:
                if(!finished){
                    finished = true;
                    dcel.init(vertices);
                }
                break;
            case GLFW_KEY_T:
                if(finished){
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
}

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

    
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    

    glfwTerminate();
    return 0;
}