#include "renderer.hpp"
#include "shader.hpp"
#include "delaunay.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool topView = false;           // Estado de la cámara
int trianglesToDraw = 0;        // Cuántos índices dibujar (empieza en 0 = solo puntos)
float animationTimer = 0.0f;    // Para controlar la velocidad
bool autoAnimate = false;
float lastX = 960;
float lastY = 540;
float yaw = -90.0, roll, pitch, fov = 45.0;
bool wireFrame = true;
// Posicion inicial de la camara
glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 2.0f);
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
    points.push_back({x, y, z});
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
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(action == GLFW_PRESS){
		switch(key){
			case GLFW_KEY_P:
				if(wireFrame){
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				} else{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
				wireFrame = !wireFrame;
				break;
			case GLFW_KEY_1:
				cameraPos = glm::vec3(0.0f, 4.0f, 0.0f); 
				cameraFront = glm::vec3(0.0f, -1.0f, 0.0f);
				cameraUp = glm::vec3(0.0f, 0.0f, -1.0f); 
							
				pitch = -89.9f;
				yaw = -90.0f;
			break;
			case GLFW_KEY_2:
				cameraPos = glm::vec3(0.0f, 0.5f, 2.0f);
    		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    		pitch = 0.0f;
    		yaw = -90.0f;
			break;
		}
			
	}
}
void processInput(GLFWwindow* window, float deltaTime){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  static bool spacePressed = false;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    if (!spacePressed) {
      trianglesToDraw += 3;
      spacePressed = true;
    }
  } else {
    spacePressed = false;
  }

  static bool enterPressed = false;
  if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
    if (!enterPressed) {
      autoAnimate = !autoAnimate;
      enterPressed = true;
    }
    } else {
      enterPressed = false;
    }
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
	vector<Point3> points = readInput("input4.txt");
	Delaunay d(points);

	unsigned int totalIndices = d.triangles.size();
	// Triangles tiene los indices en tercias para cada triangulo
	cout << d.triangles.size() << '\n';
	for(size_t i =0; i < totalIndices; i+=3){
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
	glEnable(GL_DEPTH_TEST); 
  // Habilitar Face Culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
  glfwSetKeyCallback(window, keyCallback);
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
		lastFrame = currentFrame;
		processInput(window, deltaTime);
		if (autoAnimate) {
			animationTimer += deltaTime;
    	if (animationTimer >= 0.1f) { // Cada 0.05 segundos añade un triángulo
        trianglesToDraw += 3; 
        animationTimer = 0.0f;
      }
    }
    glm::mat4 globalModelMatrix = glm::mat4(1.0f);
    glm::mat4 proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos+cameraFront, cameraUp);

		glm::mat4 mvp = proj * view * globalModelMatrix; 

		if (trianglesToDraw > totalIndices) trianglesToDraw = totalIndices;
    renderer->drawTriangles(shader.ID, mvp, globalModelMatrix, {1.0, 0.3, 0.5}, trianglesToDraw);
		glPointSize(5.0f);

		renderer->drawPoints(shader.ID, mvp, globalModelMatrix, {0,1,0});
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
	delete renderer;
  glfwTerminate();
	return 0;
}