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

int pivotIdx = -1;      // Punto actual del hull
bool finished = false;
bool popping = false;

float lastTime = 0;

void stepGrahamScan(vector<point>& p) {
    int n = (int)p.size();
    if (finished || n <= 3) return; // n <=3 representa una linea o triangulo

    if (hull.empty()) {
        int l = 0;
        
        for (int i = 1; i < n; i++) {
            if (p[i].x < p[l].x || (fabs(p[i].x - p[l].x) < EPS && p[i].y < p[l].y)) l = i;
        }

        pivotIdx = l;
		swap(p[0], p[pivotIdx]);

        sort(++p.begin(), p.end(), [&](point a, point b) {
            if (ccw(p[0], a, b)) return true;
            if (ccw(p[0], b, a)) return false;
            return dist(p[0], a) < dist(p[0], b);
            });

		hull.push_back(p[0]);
		hull.push_back(p[1]);

        pivotIdx = 2;
        return;
    }

    if (pivotIdx < n) {
        point next = p[pivotIdx];
        if (!popping) {
			hull.push_back(next);
            popping = true;
        }
        else{
            if(hull.size() > 2 && !ccw(hull[hull.size() - 3], hull[hull.size() - 2], next)) {
                hull.erase(hull.end()-2); // Para nada optimo, pero la visualizacion se ve bonita.
			}
            else {
                pivotIdx++;
                popping = false;
            }
		}
    }
    else {
        finished = true;
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

    if (!finished && pivotIdx != -1 && pivotIdx < points.size()) {
        drawLine(points[pivotIdx], points[pivotIdx], 1, 1, 0.5);
    }

    if (finished) {
        drawLine(hull.back(), hull.front(), 1, 1, 1, 2.0f);
    }

    for (auto& p : hull) drawPoint(p, 1, 1, 1, 8.0f);
}

int main() {
    srand(time(NULL));
    points = randomPoints(30);

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Graham Scan", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        cout << "Error init GLEW\n"; return -1;
    }
    // Modificar esta variable para ajustar el tiempo entre pasos
    float timePerStep = 0.05;
    
    lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {

        float now = glfwGetTime();

        if (now - lastTime > timePerStep) {
            stepGrahamScan(points);
            lastTime = now;
        }

        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
