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
