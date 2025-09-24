cmake_minimum_required(VERSION 3.28)
project(Visualizacion)

set(CMAKE_CXX_STANDARD 20)

add_executable(JarvisMarch JarvisMarch.cpp)
add_executable(GrahamScan GrahamScan.cpp)

# --- Bibliotecas ---

# Busca los paquetes necesarios
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED) # Es una buena práctica usar mayúsculas: GLEW

# Enlaza las bibliotecas principales usando sus targets de CMake
# Esto es más robusto que enlazar por nombre
target_link_libraries(JarvisMarch PRIVATE glfw GLEW::glew)
target_link_libraries(GrahamScan PRIVATE glfw GLEW::glew)

# Enlaza OpenGL de forma específica para cada plataforma
if (WIN32)
    # En Windows, se enlaza directamente a opengl32
    target_link_libraries(JarvisMarch PRIVATE opengl32)
    target_link_libraries(GrahamScan PRIVATE opengl32)
elseif(APPLE)
    # En macOS
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenGL_LIBRARY})
else()
    # En Linux y otros
    find_package(OpenGL REQUIRED)
    target_link_libraries(Visualizacion PRIVATE OpenGL::GL)
endif()
