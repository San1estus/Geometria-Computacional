#version 330 core
layout (location = 0) in vec3 aPos;     // Posición
layout (location = 1) in vec3 aNormal;  // Normal

out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 u_MVP;    // Proyección * Vista * Modelo
uniform mat4 u_Model;  

void main()
{
    // Posición final del vértice
    gl_Position = u_MVP * vec4(aPos, 1.0);
    
    // Posición en el espacio del mundo
    v_FragPos = vec3(u_Model * vec4(aPos, 1.0));
    
    // Transformar la normal 
    v_Normal = mat3(u_Model) * aNormal;
}