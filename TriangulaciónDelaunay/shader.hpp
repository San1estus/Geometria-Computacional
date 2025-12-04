#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader{
    public:
    unsigned int ID;

    Shader();
    void init(std::string vertexPath, std::string fragmentPath);
    void use() const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setBool(const std::string &name, const bool values) const;
    void setInt(const std::string &name, const int values) const;
    void setFloat(const std::string &name, const float values) const;

    private:
    unsigned int CompileShader(unsigned int type, const std::string source);
};

void Shader::init(std::string vertexPath, std::string fragmentPath){
    std::ifstream streamvertex(vertexPath);
    std::stringstream ssvertex;
    ssvertex << streamvertex.rdbuf();

    std::ifstream streamfragment(fragmentPath);
    std::stringstream ssfragment;
    ssfragment << streamfragment.rdbuf();

    std::string vertexShader = ssvertex.str();
    std::string fragmentShader = ssfragment.str();

    unsigned int vertex = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glValidateProgram(ID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() const{
    glUseProgram(ID);
}

unsigned int Shader::CompileShader(unsigned int type, const std::string source){
    unsigned int id = glCreateShader(type);
	const char* src = source.c_str(); 
	glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length); 
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << '\n';
        std::cout << message << '\n';

        glDeleteShader(id);

        return 0;
    }

    return id;
}

Shader::Shader(){}
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const{
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()),1,GL_FALSE, glm::value_ptr(mat));
}

void Shader::setBool(const std::string &name, bool value) const{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}
#endif