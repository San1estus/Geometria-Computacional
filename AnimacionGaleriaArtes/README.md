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

    ./build/Galeria.exe

## Manejo del programa

Se puedne poner puntos dando click en la pantalla, los puntos deben ser dados en orden antihorario, de lo contrario no funcionaran de manera correcta los algoritmos, se pueden deshacer los puntos usando `CTRL+Z`. Una vez que se pusieron todos los puntos, presionar la tecla `C` para cerrar el poligono.

Una vez cerrado el poligono presionar `T` para triangular el poligono, una vez realizada la triangulación presionar `G` para hacer el coloreo de los vertices y de ahi se hace la seleccion de la colocación de los guardias. 

Por último presionar `V` para mostrar la visibilidad de cada guardia.

Si se quiere probar con otro poligono se puede presionar `R` para reiniciar toda la memoria.
