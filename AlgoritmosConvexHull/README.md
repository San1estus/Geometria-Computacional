## Ejecución del programa

Es necesario tener un compilador de C++, una vez descargada la carpeta, en la terminal moverse a esta carpeta y usar la siguiente linea

    g++ -o solver ConvexHulls.cpp

Esto creara el ejecutable solver, para ejectuar el programa usar en la misma terminal

    .\solver

dependiendo del entorno puede que sea necesario cambiar \ por /.

## Generacion de puntos

Al ejecutar el programa se debe indicar la cantidad de puntos aleatorios a generar, una vez hecho esto, se ejecutaran los tres algoritmos, si todo funciono bien deben regresar los mismos puntos de la envolvente convexa, adicionalmente despues de cada ejecución debe imprimirse un 1, esto indica que en efecto el conjunto de puntos dado es convexo.

## Aclaración

Los puntos dados de la envolvente convexa no es la envolvente convexa cerrada, para la verificaición de convexidad, se duplica el primer vértice del conjunto de puntos dado, por tanto la salida dada debe considerarse como el camino de puntos que se sigue hasta antes de cerrarse. 