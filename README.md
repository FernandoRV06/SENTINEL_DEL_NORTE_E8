# SENTINEL Digital Twin · Sentinela del Norte

Hackathon integrador de TC2038 (Análisis y Diseño de Algoritmos Avanzados). Plataforma que integra 7 algoritmos de infraestructura de red y ciberseguridad sobre la red GÉANT y transmisiones derivadas de CIC-IDS2017.

## Estructura

```
src/        Núcleo en C++: un único main.cpp más un .h por desafío
app/        Interfaz (SENTINEL Digital Twin)
tests/      Casos de prueba
results/    Mediciones de los benchmarks
```

| Archivo | Contenido | Técnica principal | Alternativa (`technique`) |
| --- | --- | --- | --- |
| `src/main.cpp` | Lee la petición JSON, la envía al módulo del modo y escribe la respuesta | | |
| `src/json.h` | Lectura y escritura de JSON (sin librerías externas) | | |
| `src/mst.h` | Desafío 01 · árbol de expansión mínima | Kruskal (`kruskal`) | Prim con montículo (`prim`) |
| `src/tsp.h` | Desafío 02 · ruta de mantenimiento | Held-Karp exacto hasta 16 nodos, si no vecino más cercano + 2-opt (`auto`) | `held-karp`, `nearest-neighbor`, `2-opt` |
| `src/flujoMaximo.h` | Desafío 03 · flujo máximo | Edmonds-Karp (`edmonds-karp`) | Ford-Fulkerson con DFS (`ford-fulkerson`) |
| `src/voronoi.h` | Desafío 04 · regiones de Voronoi | Recorte por semiplanos con poda (`half-plane`) | Sin poda (`half-plane-naive`); Fortune solo se analiza |
| `src/transmision.h` | Limpieza y lectura de transmisiones (compartido por 05, 06 y 07) | | |
| `src/firmas.h` | Desafío 05 · detección de firmas | KMP (`kmp`) | Búsqueda ingenua (`naive`) |
| `src/palindromo.h` | Desafío 06 · palíndromo más largo | Manacher (`manacher`) | Expansión desde el centro (`center`) |
| `src/subcadenaComun.h` | Desafío 07 · subcadena común más larga | Arreglo de sufijos + LCP de Kasai (`suffix-array`) | Programación dinámica (`dp`) |

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

Dentro de `input` se puede agregar el campo opcional `"technique"` para usar la técnica alternativa de la tabla anterior (sirve para los benchmarks). Si no se envía, se usa la técnica principal, que es lo que recibe el autograder:

```
echo '{"mode": "mst", "input": {"nodes": [0, 1, 2], "edges": [{"u": 0, "v": 1, "weight": 4}, {"u": 1, "v": 2, "weight": 1}], "technique": "prim"}}' | ./sentinel
```

## Pruebas

Requieren Python 3 (solo biblioteca estándar) y el ejecutable ya compilado:

```
python3 tests/probarTspVoronoi.py      # TSP y Voronoi
python3 tests/probarModulos.py         # MST, flujo, firmas, palíndromo y subcadena común
```

Cada script compara las respuestas contra oráculos escritos aparte en Python e incluye casos borde, casos aleatorios con el formato de la página, pruebas de estrés de los perfiles `lite`, `normal` y `huge`, y las técnicas alternativas. Se puede pasar una semilla como argumento para cambiar los casos aleatorios. Detalle de los casos en `tests/esperados.md`.

## Benchmarks

```
python3 results/benchmarks.py           # 3 repeticiones por medición (unos 3 minutos)
python3 results/benchmarks.py --rapido  # 1 repetición
```

Genera instancias chicas, medianas y grandes con semilla fija, corre todas las técnicas sobre la misma instancia y escribe `results/benchmarks.csv` y `results/benchmarks.md` con tiempo, memoria, exponente empírico contra la complejidad teórica, coincidencia entre técnicas y los datos del entorno (procesador, sistema, compilador y banderas).

## Arquitectura

Núcleo en C++ (`src/`):

```
stdin (JSON) -> main.cpp -> LectorJson (json.h) -> despacharModo -> resolverX() del módulo -> escribirJson -> stdout (JSON)
```

- `main.cpp` solo lee la petición, elige el módulo por el campo `mode` y escribe la respuesta; si la entrada es inválida responde `{"error": "..."}`.
- Cada desafío vive en su propio `.h` con una función `resolverX(const ValorJson &entrada)` que lee su campo `input`, elige la técnica y arma la respuesta. Los tres desafíos de cadenas comparten `transmision.h` para normalizar las transmisiones.
- No hay estado global: cada ejecución atiende una sola petición, así que la aplicación puede llamar al ejecutable por cada consulta.

Pendiente de completar: capa de datos (archivos de entrada del caso) y aplicación.

## Dependencias

Núcleo: solo la biblioteca estándar de C++17. El autograder de la página compila sin soporte de excepciones, por eso convierte cada `throw` en un `abort` y elimina los `try` y `catch`. El código solo lanza errores ante entradas inválidas, y no debe escribir esas palabras en comentarios. Pruebas y benchmarks: Python 3 con su biblioteca estándar. Aplicación: pendiente.

## Datos

Pendiente de completar: fuentes (SNDlib GÉANT y CIC-IDS2017), transformaciones aplicadas y aclaración de que `mapping.csv` es una asociación simulada.
