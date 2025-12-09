## Prerrequisitos
Es necesario instalar lo siguiente en el entorno de desarrollo:
* [cmake](https://cmake.org/)
* Compilador de C++
* [GLFW](https://www.glfw.org/)
* [GLEW](https://glew.sourceforge.net/)
* [GLM](https://github.com/g-truc/glm/releases/)

Para **Windows usar MSYS UCRT64**. En este se puede installar `cmake`, `g++`, `glfw` y `glew`, usando pacman. Para buscar se usa ``pacman -Ss (expresion regular a buscar)``.
Para instalar usar ``pacman -S (direccion del archivo)``, la direccion se encuentra con lo anterior, asegurarse de que sea para ucrt.

## Ejecución

Una vez instalado lo anterior, ir al directorio de animaciones y ejecutar lo siguiente para configurar el proyecto

  	cmake -S . -B build

seguido construir el proyecto con

    cmake --build build

Para ejecutarlo:

    ./build/Mallado.exe

## Manejo del programa

El input se hace desde un archivo externo, los puntos dados en coordenadas 3D, la triangulación es realizada con respecto al plano XZ

Para mover la cámara en el espacio se usa `WASD`, para rotar la cámara es con el movimiento del ratón. Con espacio se hace un paso de la triangulación, en la visualización no se ve la legalización de aristas, sino el resultado final de triangular lo puntos. Pulsando `ENTER` se hacen todos los triangulos en automático. Con la tecla `P` se alterna la vista de malla y estructura sólida 
