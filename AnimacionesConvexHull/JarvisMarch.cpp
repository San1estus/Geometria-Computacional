#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "../geo.hpp"

using namespace std;

vector<point> randomPoints(int n) {
    vector<point>points;
    for (int i = 0; i < n; i++) {
        float xrand = -0.9 + 1.5 * (float)rand() / (RAND_MAX + 1);
        float yrand = -0.9 + 1.5 * (float)rand() / (RAND_MAX + 1);

        points.push_back({ xrand, yrand });
    }

    return points;
}

vector<point> points;
vector<point> hull;

int pivot = -1;      // Punto actual del hull
int bestIdx = -1;    // Mejor candidato actual
int testIdx = -1;    // Índice en barrido
bool finished = false;

float lastTime = 0;

void drawPoint(point p, float r, float g, float b, float size = 5.0f) {
    glPointSize(size);
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
    glVertex2f(p.x, p.y ); 
    glEnd();
}

void drawLine(point a, point b, float r, float g, float bl, float width = 1.0f) {
    glLineWidth(width);
    glColor3f(r, g, bl);
    glBegin(GL_LINES);
    glVertex2f(a.x , a.y);
    glVertex2f(b.x, b.y);
    glEnd();
}

void stepJarvisMarch(vector<point>& p) {
    int n = (int)p.size();
    if (finished || n <= 3) return; // n <=3 representa una linea o triangulo

    if (hull.empty()) {
        int l = 0;

        for (int i = 1; i < n; i++) {
            if (p[i] < p[l]) l = i;
        }

        pivot = l;
        hull.push_back(p[pivot]);

        bestIdx = (pivot + 1) % n;
        testIdx = 0;
        return;
    }

    if (testIdx != pivot) {
        if (ccw(p[pivot], p[testIdx], p[bestIdx])) {
            bestIdx = testIdx; 
        }
    }

    testIdx++;

    if (testIdx >= n) {
        if (p[bestIdx] == hull[0]) {
            finished = true;
        }
        else {
            hull.push_back(p[bestIdx]);
            pivot = bestIdx;

            bestIdx = (pivot + 1) % n;
            testIdx = testIdx % n;
        }
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto& p : points) drawPoint(p, 1, 0, 0);

    if (hull.size() > 1) {
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_STRIP);
        for (auto& p : hull)
            glVertex2f(p.x, p.y);
        glEnd();
    }

    if (!finished && pivot != -1 && testIdx < points.size()) {
        drawLine(points[pivot], points[testIdx], 1, 1, 0.5);
        drawLine(points[pivot], points[bestIdx], 0, 1, 0, 2.0f); 
    }

    if (finished) {
		drawLine(hull.back(), hull.front(), 1, 1, 1, 2.0f); 
    }

    for (auto& p : hull) drawPoint(p, 1, 1, 1, 8.0f);
}

int main() {
    srand(time(NULL));
    // Generar puntos aleatorios
	points = randomPoints(100);

    if (!glfwInit()) return -1;
    // Maximizar ventana
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Jarvis March", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        cout << "Error init GLEW\n"; return -1;
    }

    // Modificar esta variable para ajustar el tiempo entre pasos
    float timePerStep = 0.05;

    lastTime = glfwGetTime();
    // Mostrar la ventana hasta que se cierre
    while (!glfwWindowShouldClose(window)) {

        float now = glfwGetTime();

        if (now - lastTime > timePerStep) { 
            stepJarvisMarch(points);
            lastTime = now;
        }

        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
