# Benchmarks

Generado por `python3 results/benchmarks.py`. Datos completos en `benchmarks.csv`.

## Entorno

- **Fecha:** 2026-10-20
- **Procesador:** Apple M5
- **Sistema:** Darwin 25.6.0 (arm64)
- **Compilador:** Apple clang version 21.0.0 (clang-2100.3.34.2)
- **Banderas:** -std=c++17 -O2 -Wall -Wextra -Wpedantic
- **Python:** 3.14.6
- **Semilla:** 2038
- **Repeticiones:** 3 (se reporta la mediana)

## Cómo leer las columnas

- **segundos**: mediana del tiempo total del proceso (incluye leer y escribir el JSON).
- **memoria**: memoria residente máxima del proceso.
- **exponente**: pendiente de log(t) contra log(n) respecto al tamaño anterior; se compara con la complejidad teórica (por ejemplo 1 para O(n), 2 para O(n²)). Solo se calcula si el tiempo supera 0.05 s, porque abajo de eso domina el arranque del proceso.
- **coincide**: si todas las técnicas exactas dan la misma respuesta en la misma instancia; en TSP, la brecha de la heurística contra el óptimo de Held-Karp.

## Limitaciones conocidas

- En Held-Karp el crecimiento es exponencial, así que el exponente log-log no aplica; para contrastarlo con la teoría conviene el cociente t(n) / t(n - 2), que debería acercarse a 4 · (n / (n - 2))² cuando el tiempo ya no lo domina el arranque.
- La memoria de las entradas grandes la domina el lector `src/json.h`, que guarda cada número como un `ValorJson` completo (TSP con 2000 nodos: unos 4 millones de valores). El algoritmo en sí usa mucho menos (la matriz de 2000 × 2000 son 32 MB). Los tamaños del autograder (hasta 450 nodos en TSP) quedan muy por debajo.
- Los tiempos incluyen arrancar el proceso y leer el JSON, así que en instancias chicas (menos de 0.01 s) las técnicas no se distinguen.


## mst · aleatorio E = 5V

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| kruskal | 1000 | 4999 | 0.0173 | 8.5 |  | O(E log E) | si |
| prim | 1000 | 4999 | 0.0234 | 10.3 |  | O(E log V) | si |
| kruskal | 10000 | 49999 | 0.1559 | 70.5 | 0.95 | O(E log E) | si |
| prim | 10000 | 49999 | 0.1541 | 71.8 | 0.82 | O(E log V) | si |
| kruskal | 50000 | 249999 | 0.7976 | 340.4 | 1.01 | O(E log E) | si |
| prim | 50000 | 249999 | 0.7974 | 356.7 | 1.02 | O(E log V) | si |

## tsp · puntos en el plano

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| held-karp | 8 | 8 | 0.0019 | 1.5 |  | O(2^n n^2) | brecha 0.00 % |
| nearest-neighbor | 8 | 8 | 0.0016 | 1.5 |  | O(n^2) | brecha 3.16 % |
| 2-opt | 8 | 8 | 0.0015 | 1.5 |  | O(n^2) por pasada | brecha 0.00 % |
| held-karp | 12 | 12 | 0.002 | 2.3 |  | O(2^n n^2) | brecha 0.00 % |
| nearest-neighbor | 12 | 12 | 0.0017 | 1.5 |  | O(n^2) | brecha 6.13 % |
| 2-opt | 12 | 12 | 0.0016 | 1.5 |  | O(n^2) por pasada | brecha 1.82 % |
| held-karp | 16 | 16 | 0.013 | 17.6 |  | O(2^n n^2) | brecha 0.00 % |
| nearest-neighbor | 16 | 16 | 0.0018 | 1.6 |  | O(n^2) | brecha 8.17 % |
| 2-opt | 16 | 16 | 0.0017 | 1.6 |  | O(n^2) por pasada | brecha 0.00 % |
| held-karp | 18 | 18 | 0.0533 | 73.7 | 11.98 | O(2^n n^2) | brecha 0.00 % |
| nearest-neighbor | 18 | 18 | 0.002 | 1.6 |  | O(n^2) | brecha 6.68 % |
| 2-opt | 18 | 18 | 0.0018 | 1.6 |  | O(n^2) por pasada | brecha 0.00 % |

## tsp · puntos en el plano (grande)

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| nearest-neighbor | 100 | 100 | 0.0053 | 4.6 |  | O(n^2) | heuristica |
| 2-opt | 100 | 100 | 0.0053 | 4.6 |  | O(n^2) por pasada | heuristica |
| nearest-neighbor | 500 | 500 | 0.1019 | 70.6 | 1.84 | O(n^2) | heuristica |
| 2-opt | 500 | 500 | 0.0989 | 70.6 | 1.82 | O(n^2) por pasada | heuristica |
| nearest-neighbor | 1000 | 1000 | 0.3692 | 248.3 | 1.86 | O(n^2) | heuristica |
| 2-opt | 1000 | 1000 | 0.3653 | 248.4 | 1.89 | O(n^2) por pasada | heuristica |
| nearest-neighbor | 2000 | 2000 | 1.4853 | 984.8 | 2.01 | O(n^2) | heuristica |
| 2-opt | 2000 | 2000 | 1.4601 | 983.7 | 2.0 | O(n^2) por pasada | heuristica |

## flow · aleatorio E = 10V

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| edmonds-karp | 100 | 989 | 0.0049 | 3.2 |  | O(V E^2) | si |
| ford-fulkerson | 100 | 989 | 0.0049 | 3.2 |  | O(E f) | si |
| edmonds-karp | 300 | 2991 | 0.0108 | 5.7 |  | O(V E^2) | si |
| ford-fulkerson | 300 | 2991 | 0.0148 | 5.8 |  | O(E f) | si |
| edmonds-karp | 1000 | 9992 | 0.0338 | 14.9 |  | O(V E^2) | si |
| ford-fulkerson | 1000 | 9992 | 0.3636 | 15.0 | 2.66 | O(E f) | si |

## voronoi · sitios aleatorios

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| half-plane | 100 | 100 | 0.0031 | 2.4 |  | O(n k log n) con poda | si |
| half-plane-naive | 100 | 100 | 0.0035 | 2.3 |  | O(n^2 k) | si |
| half-plane | 500 | 500 | 0.012 | 4.7 |  | O(n k log n) con poda | si |
| half-plane-naive | 500 | 500 | 0.0323 | 4.7 |  | O(n^2 k) | si |
| half-plane | 2000 | 2000 | 0.1434 | 12.6 | 1.79 | O(n k log n) con poda | si |
| half-plane-naive | 2000 | 2000 | 0.4612 | 12.7 | 1.92 | O(n^2 k) | si |
| half-plane | 5000 | 5000 | 0.9402 | 30.0 | 2.05 | O(n k log n) con poda | si |
| half-plane-naive | 5000 | 5000 | 2.8854 | 30.0 | 2.0 | O(n^2 k) | si |

## matching · aleatorio

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| kmp | 10000 | 10000 | 0.0036 | 1.7 |  | O(n + m) | si |
| naive | 10000 | 10000 | 0.0033 | 1.7 |  | O(n m) peor caso | si |
| kmp | 100000 | 100000 | 0.0161 | 2.9 |  | O(n + m) | si |
| naive | 100000 | 100000 | 0.0152 | 2.9 |  | O(n m) peor caso | si |
| kmp | 1000000 | 1000000 | 0.133 | 12.8 | 0.92 | O(n + m) | si |
| naive | 1000000 | 1000000 | 0.1256 | 12.8 | 0.92 | O(n m) peor caso | si |

## matching · peor caso (AAAA...)

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| kmp | 10000 | 10000 | 0.0035 | 1.7 |  | O(n + m) | si |
| naive | 10000 | 10000 | 0.01 | 1.7 |  | O(n m) peor caso | si |
| kmp | 100000 | 100000 | 0.0165 | 3.0 |  | O(n + m) | si |
| naive | 100000 | 100000 | 0.0862 | 2.9 | 0.94 | O(n m) peor caso | si |
| kmp | 1000000 | 1000000 | 0.1299 | 12.8 | 0.9 | O(n + m) | si |
| naive | 1000000 | 1000000 | 0.8503 | 12.8 | 0.99 | O(n m) peor caso | si |

## palindrome · aleatorio

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| manacher | 10000 | 10000 | 0.0035 | 1.9 |  | O(n) | si |
| center | 10000 | 10000 | 0.0034 | 1.7 |  | O(n^2) peor caso | si |
| manacher | 100000 | 100000 | 0.0153 | 4.2 |  | O(n) | si |
| center | 100000 | 100000 | 0.0158 | 2.9 |  | O(n^2) peor caso | si |
| manacher | 1000000 | 1000000 | 0.1305 | 28.1 | 0.93 | O(n) | si |
| center | 1000000 | 1000000 | 0.1278 | 12.8 | 0.91 | O(n^2) peor caso | si |

## palindrome · peor caso (AAAA...)

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| manacher | 1000 | 1000 | 0.0023 | 1.5 |  | O(n) | si |
| center | 1000 | 1000 | 0.0023 | 1.4 |  | O(n^2) peor caso | si |
| manacher | 10000 | 10000 | 0.0033 | 1.9 |  | O(n) | si |
| center | 10000 | 10000 | 0.0293 | 1.7 |  | O(n^2) peor caso | si |
| manacher | 30000 | 30000 | 0.0063 | 2.4 |  | O(n) | si |
| center | 30000 | 30000 | 0.2269 | 2.0 | 1.86 | O(n^2) peor caso | si |
| manacher | 100000 | 100000 | 0.0154 | 4.2 |  | O(n) | si |
| center | 100000 | 100000 | 2.4634 | 2.9 | 1.98 | O(n^2) peor caso | si |

## common · bloque compartido n/8

| técnica | n | m | segundos | memoria (MB) | exponente | teoría | coincide |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| suffix-array | 1000 | 1000 | 0.0023 | 1.6 |  | O(n log n) | si |
| dp | 1000 | 1000 | 0.003 | 1.5 |  | O(n m) | si |
| suffix-array | 5000 | 5000 | 0.0031 | 2.2 |  | O(n log n) | si |
| dp | 5000 | 5000 | 0.0262 | 1.7 |  | O(n m) | si |
| suffix-array | 20000 | 20000 | 0.0072 | 4.2 |  | O(n log n) | si |
| dp | 20000 | 20000 | 0.3859 | 2.2 | 1.94 | O(n m) | si |
| suffix-array | 200000 | 200000 | 0.0667 | 26.2 | 0.97 | O(n log n) | si |
| dp | 200000 | 200000 | 39.4763 | 6.8 | 2.01 | O(n m) | si |
