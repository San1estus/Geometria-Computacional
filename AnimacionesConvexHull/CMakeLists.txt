cmake_minimum_required(VERSION 3.28)
project(Animaciones)

set(CMAKE_CXX_STANDARD 20)

add_executable(JarvisMarch JarvisMarch.cpp)
add_executable(GrahamScan GrahamScan.cpp)
add_executable(MonotoneChain MonotoneChain.cpp)

# --- Bibliotecas ---

find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED) # Es una buena práctica usar mayúsculas: GLEW

target_link_libraries(JarvisMarch PRIVATE glfw GLEW::glew)
target_link_libraries(GrahamScan PRIVATE glfw GLEW::glew)
target_link_libraries(MonotoneChain PRIVATE glfw GLEW::glew)

if (WIN32)
    target_link_libraries(JarvisMarch PRIVATE opengl32)
    target_link_libraries(GrahamScan PRIVATE opengl32)
    target_link_libraries(MonotoneChain PRIVATE opengl32)
elseif(APPLE)
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(JarvisMarch PRIVATE ${OpenGL_LIBRARY})
    target_link_libraries(GrahamScan PRIVATE ${OpenGL_LIBRARY})
    target_link_libraries(MonotoneChain PRIVATE ${OpenGL_LIBRARY})
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(JarvisMarch PRIVATE OpenGL::GL)
    target_link_libraries(GrahamScan PRIVATE OpenGL::GL)
    target_link_libraries(MonotoneChain PRIVATE OpenGL::GL)
endif()
