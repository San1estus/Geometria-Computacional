#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "geo.h"

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

void drawPoint(point p, float r, float g, float b, float size = 5.0f) {
    glPointSize(size);
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
    glVertex2f(p.x, p.y);
    glEnd();
}

void drawLine(point p, point q, float r, float g, float b, float width = 1.0f) {
    glLineWidth(width);
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex2f(p.x, p.y);
    glVertex2f(q.x, q.y);
    glEnd();
}

vector<point> points;
vector<point> hull;

int pivotIdx = 0;      // Punto actual del hull
bool finished = false;
bool lowerHull = true;
int lowerHullSize = 0;
bool sorted = false;

float lastTime = 0;

// Algoritmo paso a paso

void stepMonotoneChain(vector<point>& p) {
    int n = (int)p.size();
    if (finished || n <= 3) return; // n <=3 representa una linea o triangulo
    
    if(lowerHull){
        if(pivotIdx < n){
            while(hull.size() >= 2 && !ccw(hull[hull.size()-2], hull.back(), p[pivotIdx])){
                hull.pop_back();
            }
            hull.push_back(p[pivotIdx++]);
        }
        else{
            lowerHull = false;
            lowerHullSize = hull.size();
            pivotIdx = n - 2;
        }
    }
    else{
        if(pivotIdx >= 0){
            while(hull.size() > lowerHullSize && !ccw(hull[hull.size()-2], hull.back(), p[pivotIdx])){
                hull.pop_back();
            }
            hull.push_back(p[pivotIdx--]);
        }
        else{
            finished = true;
            if(!hull.empty()){
                hull.pop_back();
            }
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


    if (finished) {
        drawLine(hull.back(), hull.front(), 1, 1, 1, 2.0f);
    }

    for (auto& p : hull) drawPoint(p, 1, 1, 1, 8.0f);
}



int main() {
    srand(time(NULL));
    // Generar puntos aleatorios
    points = randomPoints(100);
    sort(points.begin(), points.end());
    if (!glfwInit()) return -1;
    // Maximizar ventana
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Monotone Chain", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    // Modificar esta variable para ajustar el tiempo entre pasos
    float timePerStep = 0.05;
    
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        cout << "Error init GLEW\n"; return -1;
    }

    lastTime = glfwGetTime();
    // Mostrar la ventana hasta que se cierre
    while (!glfwWindowShouldClose(window)) {

        float now = glfwGetTime();

        if (now - lastTime > timePerStep) {
            stepMonotoneChain(points);
            lastTime = now;
        }

        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
