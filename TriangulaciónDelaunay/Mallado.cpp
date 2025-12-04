#include "renderer.hpp"
#include "delaunay.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

vector<Point3> readInput(const string& filename){
	vector<Point3> points;
	ifstream file(filename);
	if(!file.is_open()){
		throw runtime_error("No se encontro el archivo.");
	}

	string line;
	while(getline(file, line)){
		stringstream ss(line);
		double x, y, z;
		if(!(ss >> x >> y >> z)){
			continue;
		}
		points.push_back({x,y,z});
	}
	return points;
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods){}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){}

void render(){
	
}
int main(){
	vector<Point3> points = readInput("input.txt");

	Delaunay d(points);
	cout << d.triangles.size() << '\n';
	for(size_t i =0; i < d.triangles.size(); i+=3){
		cout << "Puntos del triangulo " << i/3 << ":\n";
		cout << d.points[d.triangles[i]].x << ' ' << d.points[d.triangles[i]].y << '\n';
		cout << d.points[d.triangles[i+1]].x << ' ' << d.points[d.triangles[i+1]].y << '\n';
		cout << d.points[d.triangles[i+2]].x << ' ' << d.points[d.triangles[i+2]].y << '\n';
	}
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
    

    render();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  

  glfwTerminate();
	return 0;
}