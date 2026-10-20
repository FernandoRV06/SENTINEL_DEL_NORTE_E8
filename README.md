# SENTINEL Digital Twin · Sentinela del Norte

Hackathon integrador de TC2038 (Análisis y Diseño de Algoritmos Avanzados). Plataforma que integra 7 algoritmos de infraestructura de red y ciberseguridad sobre la red GÉANT y transmisiones derivadas de CIC-IDS2017.

## Estructura

```
src/        Núcleo en C++: un único main.cpp más un .h por desafío
app/        Interfaz (SENTINEL Digital Twin)
tests/      Casos de prueba
results/    Mediciones de los benchmarks
```

| Archivo | Contenido |
| --- | --- |
| `src/main.cpp` | Lee la petición JSON, la envía al módulo del modo y escribe la respuesta |
| `src/json.h` | Lectura y escritura de JSON (sin librerías externas) |
| `src/mst.h` | Desafío 01 · árbol de expansión mínima |
| `src/tsp.h` | Desafío 02 · ruta de mantenimiento |
| `src/flujoMaximo.h` | Desafío 03 · flujo máximo |
| `src/voronoi.h` | Desafío 04 · regiones de Voronoi |
| `src/transmision.h` | Limpieza de transmisiones (compartido por 05, 06 y 07) |
| `src/firmas.h` | Desafío 05 · detección de firmas |
| `src/palindromo.h` | Desafío 06 · palíndromo más largo |
| `src/subcadenaComun.h` | Desafío 07 · subcadena común más larga |

## Compilación

Mismas opciones que el autograder:

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel
```

(Con Clang: cambiar `g++` por `clang++`.)

## Ejecución

El ejecutable lee un objeto JSON por la entrada estándar y escribe un objeto JSON por la salida estándar:

```
./sentinel < tests/peticiones/mst.json
```

Los modos son `mst`, `tsp`, `flow`, `voronoi`, `matching`, `palindrome` y `common`. En `tests/peticiones/` hay una petición de ejemplo por modo y en `tests/esperados.md` su respuesta esperada.

## Arquitectura

Pendiente de completar: capas de datos, núcleo en C++, módulos y aplicación.

## Dependencias

Núcleo: solo la biblioteca estándar de C++17. Aplicación: pendiente.

## Datos

Pendiente de completar: fuentes (SNDlib GÉANT y CIC-IDS2017), transformaciones aplicadas y aclaración de que `mapping.csv` es una asociación simulada.
