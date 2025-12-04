#include "renderer.hpp"
#include "shader.hpp"
#include "delaunay.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


float lastX = 960;
float lastY = 540;
float yaw = -90.0, roll, pitch, fov = 45.0;

// Posicion inicial de la camara
glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Se puede ajustar para que la camara rote mas rapido
const float MOUSE_SENSITIVITY = 0.1f;
bool firstMouse = true;

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

void mouseCallback(GLFWwindow* window, double xpos, double ypos){

    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    yoffset *= MOUSE_SENSITIVITY;
    xoffset *= MOUSE_SENSITIVITY;

    yaw+= xoffset;
    pitch += yoffset;

    if(pitch > 89.0f){
        pitch = 89.0f;
    }
    if(pitch < -89.0f){
        pitch = -89.0f;
    }
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void processInput(GLFWwindow* window, float deltaTime){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
    
  // Movimiento camara con WASD
    
  const float cameraSpeed = 0.01f;
  if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    cameraPos += cameraSpeed * cameraFront;
  }
  if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    cameraPos -= cameraSpeed * cameraFront;
  }
  if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp))* cameraSpeed;
  }
  if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp))* cameraSpeed;
  }
}


vector<float> Point3ToVertices(const vector<Point3> &points){
	vector<float> vertices;
	for(Point3 p: points){
		vertices.push_back(p.x);
		vertices.push_back(p.y);
		vertices.push_back(p.z);
	}
	return vertices;
}

int main(){
	vector<Point3> points = readInput("input.txt");

	Delaunay d(points);
	// Triangles tiene los indices en tercias para cada triangulo
	cout << d.triangles.size() << '\n';
	for(size_t i =0; i < d.triangles.size(); i+=3){
		cout << "Puntos del triangulo " << i/3+1 << ":\n";
		cout << d.points[d.triangles[i]].x << ' ' << d.points[d.triangles[i]].y <<  ' ' << d.points[d.triangles[i]].z << '\n';
		cout << d.points[d.triangles[i+1]].x << ' ' << d.points[d.triangles[i+1]].y << ' ' << d.points[d.triangles[i+1]].z << '\n';
		cout << d.points[d.triangles[i+2]].x << ' ' << d.points[d.triangles[i+2]].y << ' ' << d.points[d.triangles[i+2]].z <<'\n';
		cout << "Indices del triangulo: " << '\n';
		cout << d.triangles[i] << ' '<< d.triangles[i+1] << ' ' << d.triangles[i+2] << '\n';
	}

	
	GLFWwindow* window;

  /* Initialize the library */
  if (!glfwInit())
    return -1;
  
  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(1920, 1080, "Mallado", NULL, NULL);
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

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Carga de shaders
  Shader shader;
	shader.init("shaders/vertex.vs", "shaders/fragment.fs");
	shader.use();
	
	// Configuración Inicial de Matrices y Luz
    
  glm::vec3 lightPos(0.0f, 1.0f, 3.0f); 
    
  int lightPosLocation = glGetUniformLocation(shader.ID, "u_LightPos");
	int viewPosLocation = glGetUniformLocation(shader.ID, "u_ViewPos");
  glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	float rotationSpeed = 0.1f;

  float lastFrame = 0.0f;

	
	vector<unsigned int> indices = d.triangles;
	vector<float> vertices = Point3ToVertices(points);
	
	Renderer* renderer = new Renderer(vertices, indices);
  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
		processInput(window, deltaTime);
    glm::mat4 globalModelMatrix = glm::mat4(1.0f);
    glm::mat4 proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos+cameraFront, cameraUp);

		glm::mat4 mvp = proj * view * globalModelMatrix; 
    renderer->drawTriangles(shader.ID, mvp, globalModelMatrix, {1.0, 0.3, 0.5});
		glPointSize(5.0f);
		renderer->drawPoints(shader.ID, mvp, globalModelMatrix, {0,1,0});
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
	delete renderer;
  glfwTerminate();
	return 0;
}