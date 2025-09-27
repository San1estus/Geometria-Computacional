## Prerrequisitos
Es necesario instalar lo siguiente en el entorno de desarrollo:
* [cmake](https://cmake.org/)
* Compilador de C++
* [GLFW](https://www.glfw.org/)
* [GLEW](https://glew.sourceforge.net/)

Para **Windows usar MSYS UCRT64**. En este se puede installar `cmake`, `g++`, `glfw` y `glew`, usando pacman. Para buscar se usa ``pacman -Ss (expresion regular a buscar)``.
Para instalar usar ``pacman -S (direccion del archivo)``, la direccion se encuentra con lo anterior, asegurarse de que sea para ucrt.

## Ejecución

Una vez instalado lo anterior, ir al directorio de animaciones y ejecutar lo siguiente para configurar el proyecto

    cmake -S . -B build

seguido construir el proyecto con

    cmake --build build

Para ejecutarlo:

    ./build/algoritmo

donde algoritmo puede ser 

* JarvisMarch
* GrahamScan
* MonotoneChain
