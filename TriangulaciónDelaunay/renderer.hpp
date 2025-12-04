#ifndef RENDERER_H
#define RENDERER_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

using namespace std;
class Renderer{
    private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
		unsigned int vertexCount;

    public:
    Renderer(const vector<float> vertices, const vector<unsigned int> indices):indexCount(indices.size()), vertexCount(vertices.size()/3){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        GLsizei stride = 3*sizeof(float);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void drawTriangles(unsigned int shaderProgram, const glm::mat4 mvp, const glm::mat4& model, const glm::vec3& color){
        int mvpLocation = glGetUniformLocation(shaderProgram, "u_MVP"); 
        int modelLocation = glGetUniformLocation(shaderProgram, "u_Model"); 
        int colorLocation = glGetUniformLocation(shaderProgram, "u_ObjectColor"); 

        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(colorLocation, 1, glm::value_ptr(color));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
				glDrawArrays(GL_POINTS, 0, indexCount);
        glBindVertexArray(0);
    }

		void drawPoints(unsigned int shaderProgram, const glm::mat4 mvp, const glm::mat4& model, const glm::vec3& color){
        int mvpLocation = glGetUniformLocation(shaderProgram, "u_MVP"); 
        int modelLocation = glGetUniformLocation(shaderProgram, "u_Model"); 
        int colorLocation = glGetUniformLocation(shaderProgram, "u_ObjectColor"); 

        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(colorLocation, 1, glm::value_ptr(color));

        glBindVertexArray(VAO);

        glDrawArrays(GL_POINTS, 0, vertexCount);
        glBindVertexArray(0);
    }

    ~Renderer() {
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};
#endif