# SENTINEL Digital Twin · Sentinela del Norte

Hackathon integrador de TC2038 (Análisis y Diseño de Algoritmos Avanzados). SENTINEL es un "gemelo digital" de una red de telecomunicaciones: resuelve 7 problemas de infraestructura de red y de ciberseguridad con un núcleo en C++, y los muestra en una aplicación web sobre la red europea GÉANT.

**Contenido**

1. [Cómo probar la página paso a paso](#cómo-probar-la-página-paso-a-paso)
2. [Los algoritmos y por qué los elegimos](#los-algoritmos-y-por-qué-los-elegimos)
3. [Estructura del repositorio](#estructura-del-repositorio)
4. [Uso del núcleo desde la terminal](#uso-del-núcleo-desde-la-terminal)
5. [Pruebas y benchmarks](#pruebas-y-benchmarks)
6. [Arquitectura](#arquitectura)
7. [Datos](#datos)

---

## Cómo probar la página paso a paso

### 1. Descargar el proyecto

```
git clone https://github.com/FernandoRV06/SENTINEL_DEL_NORTE_E8.git
cd SENTINEL_DEL_NORTE_E8
```

Si tienes el ZIP de la entrega, descomprímelo y abre una terminal dentro de la carpeta.

### 2. Abrir la página

Desde la carpeta raíz del proyecto:

```
python3 app/servidor.py
```

El servidor compila el núcleo en C++ si hace falta y abre el navegador en `http://localhost:8000`. Si no se abre solo, escribe esa dirección en el navegador. Arriba a la derecha debe aparecer el indicador verde **"Núcleo C++ conectado"**.

### 3. Recorrido sugerido

Todos los resultados los calcula el núcleo en C++ en ese momento; la página no guarda resultados precalculados. Los valores esperados corresponden al primer escenario, "Demostración GÉANT: Londres a Atenas".

| Pestaña | Qué hacer | Qué debes ver |
| --- | --- | --- |
| Network Explorer | Elegir **MST** en la barra lateral | Enlaces verdes que conectan las 14 centrales; "Costo total: 6,367" |
| Network Explorer | Elegir **TSP** | Ruta naranja numerada que sale de Viena y regresa a ella; "10,174 km" |
| Network Explorer | Elegir **Flujo máximo** | Líneas azules más gruesas donde pasa más tráfico de Londres a Atenas; "30 Gbps" |
| Network Explorer | Elegir **Voronoi** y hacer clic en cualquier punto del mapa | Una región de color por central y el mensaje "El cliente se conecta a …" con la central que le corresponde |
| Scenario Manager | Desmarcar el enlace **AT-HU**, presionar **Recalcular** y volver a Network Explorer | El enlace aparece punteado en rojo. El MST sube a 6,595, el TSP a 10,653 km y el flujo baja a 20 Gbps |
| Scenario Manager | Elegir la tarjeta **"Caída del enlace Austria–Alemania"** | El enlace AT-DE cae y el flujo se mantiene en 30 Gbps, porque la red lo redirige por otros enlaces. Al terminar, vuelve a elegir la tarjeta "Demostración GÉANT" |
| Algorithm Laboratory | Problema **"Subcadena común más larga"**, Datos **"Datos aleatorios grandes"** y **Comparar** | Las dos técnicas dan las mismas posiciones, pero la programación dinámica tarda decenas de veces más que el arreglo de sufijos |
| Cybersecurity Console | Con el escenario "Demostración GÉANT" activo, presionar **Analizar** | ABBA en las posiciones 13 y 11, FF en 11 y 9, DEAD no aparece; palíndromos en 2–8 y 7–12; fragmento común en 3–8 (BCDCBA). Abajo se ve la salida con el formato de texto del caso |

En el Scenario Manager también se puede cargar un escenario propio en `.json` o descargar el actual.

### 4. Cerrar

En la terminal donde corre el servidor, presiona `Ctrl + C`.

### Si algo falla

| Problema | Solución |
| --- | --- |
| "No se encontro g++ ni clang++" al abrir el servidor | Instala un compilador de C++ (`g++` o `clang++`) y vuelve a abrir el servidor |
| "Address already in use" al abrir el servidor | El puerto 8000 está ocupado, normalmente por otro servidor abierto. Ciérralo con `Ctrl + C` y vuelve a intentar |
| La página dice "Sin servidor" | Abriste `index.html` directamente. La página necesita el servidor: usa `python3 app/servidor.py` y entra a `http://localhost:8000` |
| El mapa se ve gris | No hay internet para descargar el mapa de fondo. La red y los resultados se dibujan igual |
| `python3` no existe (Windows) | Usa `python` |

---

## Los algoritmos y por qué los elegimos

Para cada desafío implementamos una **técnica principal**, que es la que recibe el autograder, y una **alternativa** para comparar. Elegimos la principal por dos razones: su complejidad teórica y lo que medimos al correr ambas sobre los mismos datos (resultados completos en `results/benchmarks.md`, medidos en un Apple M5). En todas las mediciones, las dos técnicas dan la misma respuesta (en el TSP, la heurística se compara contra el óptimo).

| Desafío | Qué resuelve | Técnica principal | Complejidad | Alternativa |
| --- | --- | --- | --- | --- |
| 01 · MST | Conectar todas las centrales gastando lo menos posible | Kruskal | O(E log E) | Prim |
| 02 · TSP | Ruta más corta del técnico que visita todas las centrales | Held-Karp (hasta 16 centrales); vecino más cercano + 2-opt (más de 16) | O(2ⁿ · n²) exacto; O(n²) aproximado | Cada técnica por separado |
| 03 · Flujo máximo | Cuánta información cabe entre dos centrales | Edmonds-Karp | O(V · E²) | Ford-Fulkerson |
| 04 · Voronoi | Qué zona atiende cada central | Recorte por semiplanos con poda | O(n²) en el peor caso | El mismo recorte sin poda |
| 05 · Firmas | Encontrar firmas de malware en el tráfico | KMP | O(n + m) | Búsqueda ingenua |
| 06 · Palíndromo | Encontrar la secuencia "espejo" más larga | Manacher | O(n) | Expansión desde el centro |
| 07 · Subcadena común | Encontrar el fragmento más largo que comparten dos transmisiones | Arreglo de sufijos + LCP | O(n log n) | Programación dinámica |

*n, V = centrales o caracteres; E = enlaces; m = largo de la firma.*

### 01 · Árbol de expansión mínima (MST) · Kruskal

**Cómo funciona.** Ordena los enlaces del más barato al más caro y los va agregando, saltándose cualquiera que forme un ciclo. Para saber rápido si dos centrales ya están conectadas usa conjuntos disjuntos (*union-find*).

**Por qué.** Los datos llegan como una lista de enlaces, que es justo lo que Kruskal necesita: solo hay que ordenarla. En nuestras mediciones rinde igual que Prim (50,000 centrales y 250,000 enlaces: 0.80 s cada uno) y es más fácil de verificar. Prim se queda como alternativa.

### 02 · Ruta de mantenimiento (TSP) · Held-Karp y vecino más cercano + 2-opt

**El reto.** El problema del viajante no tiene un algoritmo exacto y rápido conocido: el tiempo exacto crece de forma exponencial.

**Cómo funciona.** Hasta 16 centrales usamos **Held-Karp**, programación dinámica que prueba todos los subconjuntos de centrales sin repetir trabajo y garantiza la ruta óptima. Con más centrales usamos **vecino más cercano** (siempre ir a la central más cercana no visitada) y después **2-opt**, que deshace los cruces de la ruta mientras eso la acorte.

**Por qué.** Held-Karp es exacto, pero su tiempo y su memoria crecen más del doble con cada central extra. Con 16 centrales tarda 0.013 s; con 18 ya usa 74 MB. Por eso el corte está en 16. La red GÉANT tiene 14 centrales, así que en la aplicación la ruta siempre es la óptima. En instancias de 8 a 18 centrales, donde se puede comparar contra el óptimo, el vecino más cercano solo quedó hasta 8 % arriba, y con 2-opt la diferencia bajó a entre 0 y 2 %. La versión aproximada resuelve 2,000 centrales en 1.5 s.

### 03 · Flujo máximo · Edmonds-Karp

**Cómo funciona.** Busca un camino de origen a destino que todavía tenga capacidad libre, lo llena al máximo y repite hasta que ya no exista ninguno. Siempre elige el camino con menos enlaces, usando búsqueda en anchura (BFS).

**Por qué.** Ford-Fulkerson con búsqueda en profundidad (DFS) puede tomar caminos largos, y su tiempo depende de qué tan grandes son las capacidades. Edmonds-Karp depende solo del tamaño de la red. Con 1,000 centrales y 10,000 enlaces tardó 0.034 s, contra 0.36 s de Ford-Fulkerson (unas 10 veces más rápido).

### 04 · Cobertura geográfica (Voronoi) · Recorte por semiplanos

**Cómo funciona.** La zona de cada central empieza siendo el rectángulo completo del mapa. Para cada otra central se traza la línea que queda a la misma distancia de ambas (la mediatriz) y se recorta la parte que queda más cerca de la otra. Con **poda**, las centrales se revisan de la más cercana a la más lejana y se deja de recortar cuando las demás ya no pueden cambiar la zona.

**Por qué.** El algoritmo de Fortune es más rápido en teoría, O(n log n), pero es mucho más complejo de implementar y más propenso a errores. Para los tamaños del caso, el recorte es suficiente, fácil de verificar y, con la poda, rápido: 5,000 centrales en 0.94 s, contra 2.89 s sin poda.

### 05 · Detección de firmas de malware · KMP

**Cómo funciona.** Antes de buscar, analiza la firma para saber cuánto puede avanzar cuando hay una diferencia. Así nunca regresa a leer otra vez la transmisión.

**Por qué.** La búsqueda ingenua vuelve a comparar desde cada posición. Con texto aleatorio rinden parecido, pero con datos repetitivos (por ejemplo `AAAA…`) la ingenua se vuelve mucho más lenta: con un millón de caracteres, 0.85 s contra 0.13 s de KMP. Elegimos KMP porque garantiza tiempo lineal en cualquier caso.

### 06 · Palíndromo más largo · Manacher

**Cómo funciona.** Aprovecha que un palíndromo es simétrico: lo que ya se sabe de la mitad izquierda sirve para la derecha, así que no repite comparaciones. Encuentra palíndromos de longitud par e impar en tiempo lineal.

**Por qué.** La expansión desde cada centro es más simple, pero en el peor caso es cuadrática. Con 100,000 caracteres iguales tardó 2.46 s, contra 0.015 s de Manacher.

### 07 · Subcadena común más larga · Arreglo de sufijos + LCP

**Cómo funciona.** Une las dos transmisiones con un separador y ordena todos sus sufijos (el **arreglo de sufijos**). Después calcula cuántos caracteres comparte cada sufijo con el siguiente en ese orden (el arreglo **LCP**, con el algoritmo de Kasai). La respuesta es el mayor valor entre un sufijo de la primera transmisión y uno de la segunda.

**Por qué.** La programación dinámica compara cada carácter de una transmisión contra cada carácter de la otra, O(n · m). Con 200,000 caracteres tardó 39.5 s, contra 0.067 s del arreglo de sufijos.

### Comparar las técnicas por tu cuenta

- En la aplicación: pestaña **Algorithm Laboratory** (paso 3).
- Desde la terminal: agrega `"technique"` dentro de `input` (ver [Uso del núcleo desde la terminal](#uso-del-núcleo-desde-la-terminal)).
- Todas las mediciones: `python3 results/benchmarks.py` (ver [Pruebas y benchmarks](#pruebas-y-benchmarks)).

---

## Estructura del repositorio

```
src/        Núcleo en C++: un main.cpp y un .h por desafío
app/        Aplicación web (servidor en Python y página)
tests/      Pruebas automáticas y peticiones de ejemplo
results/    Script y resultados de los benchmarks
team.txt    Integrantes y reparto de tareas
```

| Archivo | Contenido | Nombre de la técnica en `technique` |
| --- | --- | --- |
| `src/main.cpp` | Lee la petición JSON, la envía al desafío correspondiente y escribe la respuesta | |
| `src/json.h` | Lectura y escritura de JSON, sin librerías externas | |
| `src/mst.h` | Desafío 01 · MST | `kruskal` (principal), `prim` |
| `src/tsp.h` | Desafío 02 · TSP | `auto` (principal), `held-karp`, `nearest-neighbor`, `2-opt` |
| `src/flujoMaximo.h` | Desafío 03 · flujo máximo | `edmonds-karp` (principal), `ford-fulkerson` |
| `src/voronoi.h` | Desafío 04 · Voronoi | `half-plane` (principal), `half-plane-naive` |
| `src/transmision.h` | Limpieza de transmisiones, compartida por los desafíos 05, 06 y 07 | |
| `src/firmas.h` | Desafío 05 · firmas | `kmp` (principal), `naive` |
| `src/palindromo.h` | Desafío 06 · palíndromo | `manacher` (principal), `center` |
| `src/subcadenaComun.h` | Desafío 07 · subcadena común | `suffix-array` (principal), `dp` |

## Uso del núcleo desde la terminal

Compilación, con las mismas opciones que usa el autograder (con Clang, cambia `g++` por `clang++`):

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel
```

El ejecutable lee **un** objeto JSON por la entrada estándar y escribe **un** objeto JSON por la salida estándar. Los modos son `mst`, `tsp`, `flow`, `voronoi`, `matching`, `palindrome` y `common`.

Para usar la técnica alternativa, agrega `"technique"` dentro de `input`. Si no se envía, se usa la principal, que es la que recibe el autograder:

```
echo '{"mode": "mst", "input": {"nodes": [0, 1, 2], "edges": [{"u": 0, "v": 1, "weight": 4}, {"u": 1, "v": 2, "weight": 1}], "technique": "prim"}}' | ./sentinel
```

Si la entrada no es válida, responde `{"error": "..."}`.

## Pruebas y benchmarks

```
python3 tests/probarModulos.py         # MST, flujo, firmas, palíndromo y subcadena común
python3 tests/probarTspVoronoi.py      # TSP y Voronoi
```

Necesitan el ejecutable `sentinel` ya compilado (ver la sección anterior). Cada script compara las respuestas contra soluciones escritas aparte en Python e incluye casos borde, casos aleatorios con el formato del autograder, pruebas de estrés y las técnicas alternativas. Se puede pasar una semilla como argumento para cambiar los casos aleatorios (por defecto, 2038). Detalle de los casos en `tests/esperados.md`.

```
python3 results/benchmarks.py           # 3 repeticiones por medición (unos 3 minutos)
python3 results/benchmarks.py --rapido  # 1 repetición
```

Genera instancias chicas, medianas y grandes con semilla fija, corre todas las técnicas sobre los mismos datos y escribe `results/benchmarks.csv` y `results/benchmarks.md` con tiempo, memoria, crecimiento medido contra la complejidad teórica y si las técnicas coinciden.

## Arquitectura

```
Núcleo:      stdin (JSON) -> main.cpp -> desafío (.h) -> stdout (JSON)
Aplicación:  navegador -> servidor.py -> ./sentinel -> servidor.py -> navegador
```

- `main.cpp` solo lee la petición, elige el desafío según `mode` y escribe la respuesta. Cada desafío vive en su propio `.h`. No hay estado global: cada ejecución atiende una sola petición.
- El núcleo no lee archivos: todo lo recibe en `input`. Así el autograder, las pruebas y la aplicación lo usan exactamente igual.
- `app/servidor.py` sirve la página y, por cada consulta, ejecuta `./sentinel` y devuelve su respuesta. No calcula nada por su cuenta.
- La página (`app/index.html` y `app/js/`) tiene cuatro pestañas: Network Explorer, Algorithm Laboratory, Cybersecurity Console y Scenario Manager. `app/js/api.js` convierte el escenario activo en la petición de cada modo. Para el TSP, primero calcula las distancias mínimas entre centrales siguiendo los enlaces (Floyd-Warshall), porque no todas tienen enlace directo.
- `app/datos/red.json` es la red de demostración y `app/datos/escenarios.json` contiene los escenarios iniciales.
- El modo `flow` acepta `"detail": true` para devolver también el flujo de cada enlace (`"edgeFlows"`), que la página usa para dibujarlo.

**Dependencias.** Núcleo: solo la biblioteca estándar de C++17. El autograder compila sin soporte de excepciones (convierte cada `throw` en `abort` y elimina `try` y `catch`); por eso el núcleo solo lanza errores ante entradas inválidas. Servidor, pruebas y benchmarks: Python 3 con su biblioteca estándar. Página: [Leaflet](https://leafletjs.com) 1.9.4 (licencia BSD-2), incluida en `app/vendor/leaflet/`, y el mapa de fondo de [OpenStreetMap](https://www.openstreetmap.org/copyright).

## Datos

El caso combina dos fuentes públicas que provienen de **entornos independientes**:

| Fuente | Qué aporta | Archivos del caso | Módulo |
| --- | --- | --- | --- |
| [SNDlib](https://sndlib.put.poznan.pl/networks.overview.action), red GÉANT | Topología, coordenadas, costos, capacidades y demandas de la red troncal europea | `nodes.csv`, `edges.csv`, `capacities.csv`, `demands.csv`, `scenarios.json` | I · infraestructura |
| [CIC-IDS2017](https://www.unb.ca/cic/datasets/ids-2017.html) | Tráfico de red real, benigno y de ataque, capturado en archivos PCAP | `transmission1.txt`, `transmission2.txt`, `mcode1.txt`, `mcode2.txt`, `mcode3.txt` | II · ciberseguridad |

**Real y simulado.** `mapping.csv` asocia cada transmisión con un enlace de la red: `transmission1.txt` con Austria–Suiza (tráfico benigno) y `transmission2.txt` con Austria–Alemania (tráfico DDoS). Esa asociación es una **construcción de simulación** del caso, porque las capturas de CIC-IDS2017 no ocurrieron en la red GÉANT. Por eso, encontrar una firma en una transmisión indica un patrón sospechoso, pero no confirma una infección en ese enlace.

**Transformaciones aplicadas.**

- Antes de procesar las transmisiones (hexadecimal en mayúsculas) se eliminan saltos de línea, espacios y tabuladores. Las posiciones se reportan desde 1 sobre esa secuencia limpia.
- El palíndromo y la subcadena común se buscan sobre caracteres hexadecimales, no sobre bits.
- En Voronoi, las coordenadas se tratan como puntos en el plano y las zonas se recortan al rectángulo indicado en `bounds`.
- El flujo máximo usa las capacidades de los enlaces; las demandas no intervienen.
- El TSP trabaja sobre la matriz de distancias que recibe: el costo es la suma de `distances[ruta[i]][ruta[i + 1]]`.

**Lo que no se incluye.** Los PCAP completos de CIC-IDS2017 no forman parte del repositorio; solo se documentan su fuente y las transformaciones.

**Estado.** La página del caso lista los archivos de entrada, pero sin enlace de descarga. Mientras tanto:

- La aplicación usa `app/datos/red.json`, la topología GÉANT del laboratorio de la página: 14 centrales con sus coordenadas, 24 enlaces y capacidades de 10, 40 o 100 Gbps. La distancia de cada enlace se calcula con la fórmula de haversine y se usa también como costo de instalación.
- Las transmisiones de los escenarios son el ejemplo de la página o cadenas sintéticas generadas con semilla 2038, marcadas como tales; no provienen de CIC-IDS2017.
- Las pruebas y los benchmarks usan instancias generadas con semilla fija y el formato del autograder.

Cuando lleguen los archivos oficiales, basta con convertirlos al formato de `red.json` y de un escenario, y cargarlos desde el Scenario Manager.
