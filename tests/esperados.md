# Respuestas esperadas

Cada archivo de `tests/peticiones/` es una petición con el formato exacto del autograder. Se ejecuta así:

```
./sentinel < tests/peticiones/mst.json
```

Y la salida debe coincidir con la de esta tabla (el orden de los campos no importa).

| Petición | Respuesta esperada | Qué revisa |
| --- | --- | --- |
| `mst.json` | `{"cost": 4, "edges": [[0,1,1],[2,3,1],[1,2,2]]}` | Exactamente n − 1 aristas, sin ciclos, y que el costo coincida con la suma |
| `tsp.json` | `{"cost": 80, "route": [0,1,3,2,0]}` | La ruta empieza y termina en 0 y visita cada nodo una vez (también vale `[0,2,3,1,0]`, mismo costo) |
| `flow.json` | `{"flow": 5}` | Las aristas son dirigidas: de `u` a `v` |
| `voronoi.json` | `{"cells": [[[0,0],[50,0],[50,100],[0,100]], [[50,0],[100,0],[100,100],[50,100]]]}` | Una celda por sitio, en el mismo orden; se compara el área (5000 cada una) |
| `matching1.json` | `{"matches": [[3,-1,-1],[7,3,-1]]}` | Una fila por transmisión, una posición por firma; `-1` si no aparece |
| `matching2.json` | `{"matches": [[13,11,-1],[11,9,-1]]}` | Es el ejemplo de la página |
| `palindrome1.json` | `{"ranges": [[2,8],[6,11]]}` | Palíndromo impar (`ABCDCBA`) y par (`123321`) |
| `palindrome2.json` | `{"ranges": [[1,1],[1,9]]}` | Empate de longitud 1: gana la menor posición inicial |
| `common1.json` | `{"range": [3,6]}` | Posiciones en la primera transmisión |
| `common2.json` | `{"range": [1,3]}` | Empate entre `ABC` y `CBA`: gana el que empieza antes |

Los casos de cadenas (`matching`, `palindrome`, `common`) son los mismos casos fijos que usa el autograder de la página. Los de grafos son casos pequeños resueltos a mano.

## Casos propios de TSP y Voronoi

Se validan todos juntos con:

```
python3 tests/probarTspVoronoi.py          # semilla 2038
python3 tests/probarTspVoronoi.py 7        # otra semilla para los aleatorios
```

El script compara cada respuesta contra oráculos independientes del código C++ (fuerza bruta y Held-Karp en Python para TSP; recorte de referencia, muestreo por sitio más cercano e invariantes para Voronoi). Además corre 150 casos aleatorios por modo con el mismo formato que la página y las pruebas de estrés de los perfiles `lite`, `normal` y `huge`.

| Petición | Respuesta esperada | Qué revisa |
| --- | --- | --- |
| `tsp_vacio.json` | `{"cost": 0, "route": [0]}` | Matriz vacía: la ruta tiene n + 1 = 1 nodo |
| `tsp_un_nodo.json` | `{"cost": 0, "route": [0,0]}` | Una sola central |
| `tsp_dos_nodos.json` | `{"cost": 14, "route": [0,1,0]}` | Ida y vuelta |
| `tsp_triangulo.json` | costo 12 | Tres centrales (3 + 4 + 5) |
| `tsp_asimetrico.json` | `{"cost": 5, "route": [0,1,2,3,4,0]}` | d[i][j] ≠ d[j][i]: solo el ciclo en un sentido es barato |
| `tsp_no_metrico.json` | costo 4 | No cumple la desigualdad del triángulo; el costo se mide sobre la matriz tal cual |
| `tsp_iguales.json` | costo 30 | Todas las distancias iguales (cualquier ruta es óptima) |
| `tsp_ceros.json` | costo 0 | Todas las distancias en cero |
| `tsp_decimales.json` | costo 6.625 | Distancias no enteras |
| `tsp_grandes.json` | costo 4800000 | Valores del orden de millones |
| `tsp_circulo_10.json` | costo 618.034 | Puntos en un círculo: el óptimo es el perímetro del polígono |
| `tsp_aleatorio_12.json` | costo 99 | Óptimo confirmado con Held-Karp en Python |
| `tsp_limite_16.json` | costo 2086 | Mayor tamaño que se resuelve exacto (`LIMITE_EXACTO_TSP`) |
| `tsp_aproximado_17.json` | ciclo válido | Primer tamaño que usa vecino más cercano + 2-opt |
| `tsp_circulo_40.json` | costo 6276.728 | El aproximado también llega al perímetro (2-opt quita todos los cruces) |
| `tsp_asimetrico_30.json` | ciclo válido | Aproximado con matriz asimétrica (solo vecino más cercano) |
| `voronoi_un_sitio.json` | área 10000 | Un sitio: su celda es todo el dominio |
| `voronoi_cuadrantes.json` | 2500 cada una | Cuatro cuadrantes iguales |
| `voronoi_colineales.json` | 2000, 3500, 4500 | Sitios alineados: franjas verticales en x = 20 y x = 55 |
| `voronoi_diagonal.json` | 5000 cada una | Mediatriz diagonal: dos triángulos |
| `voronoi_borde.json` | 5000 cada una | Sitios sobre el borde del dominio |
| `voronoi_duplicados.json` | 5000, 5000, 5000 | Sitios repetidos comparten la misma celda (igual que la referencia de la página) |
| `voronoi_rectangulo.json` | 2000 cada una | Dominio no cuadrado |
| `voronoi_negativos.json` | suma 4800 | Coordenadas negativas y dominio que no empieza en 0 |
| `voronoi_decimales.json` | suma 9 | Coordenadas no enteras |
| `voronoi_cuadricula_9.json` | 100 cada una | Cuadrícula 3×3: nueve cuadrados iguales |
| `voronoi_aleatorio_40.json` | suma 1000000 | 40 sitios aleatorios, áreas contra la referencia |

## Pruebas de MST, flujo, firmas, palíndromo y subcadena común

```
python3 tests/probarModulos.py          # semilla 2038
python3 tests/probarModulos.py 7        # otra semilla para los aleatorios
```

Compara cada respuesta con un oráculo escrito aparte en Python: Prim para `mst`, Dinic para `flow` (distinto del Edmonds-Karp del C++), `str.find` para `matching`, fuerza bruta desde cada centro para `palindrome` y programación dinámica O(n·m) para `common`. Incluye casos borde (un nodo, aristas repetidas, lazos, pesos negativos, sumidero inalcanzable, aristas antiparalelas, fuente igual a sumidero, transmisiones vacías, traslapes, empates par/impar, sin caracteres en común), 300–400 casos aleatorios por modo y las pruebas de estrés de los perfiles `lite`, `normal` y `huge` (hasta 5000 nodos, transmisiones de 1 000 000 de caracteres y 200 000 en subcadena común).
