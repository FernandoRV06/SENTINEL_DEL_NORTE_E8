# Reporte de reflexión · SENTINEL Digital Twin

Equipo 8 · Sentinela del Norte · TC2038 Análisis y Diseño de Algoritmos Avanzados, Grupo 603 · Prof. Nezih Nieto Gutiérrez

| Integrante | Matrícula |
| --- | --- |
| Carlos Emiliano Nava Flores | A00842650 |
| Luis Enrique Castillo Carrillo | A00842390 |
| Desiderio Iván Ortegón Mortón | A00840591 |
| Fernando Robles Vidrio | A00843290 |

Este documento complementa el README: allí está el *qué* y el *cómo* de cada algoritmo; aquí, lo que aprendimos al elegirlos, sus límites y lo que cambiaríamos.

## 1. Visión general de la solución

SENTINEL modela una red de telecomunicaciones como un grafo (infraestructura) y su tráfico como cadenas hexadecimales (ciberseguridad). Un único núcleo en C++17 resuelve los siete desafíos; el autograder, las pruebas automáticas y la aplicación web lo usan exactamente con el mismo contrato JSON (stdin → stdout). Esta decisión fue la más importante del proyecto: al no haber una segunda implementación en la página, todo lo que se ve en el mapa es lo mismo que se califica.

## 2. Reflexión por módulo

### Infraestructura · MST y TSP (responsable: Carlos Emiliano Nava Flores, A00842650)

- **MST.** Kruskal y Prim empataron en tiempo (0.80 s con 50,000 centrales). La lección es que, cuando la complejidad asintótica es equivalente, decide la forma de los datos: la entrada llega como lista de aristas y Kruskal la usa sin convertirla.
- **TSP.** Fue el único desafío sin solución exacta eficiente. Medir el crecimiento de Held-Karp (memoria de 74 MB con 18 centrales) nos obligó a fijar un corte explícito en 16 y a ofrecer una heurística para tamaños mayores. 2-opt redujo el error del vecino más cercano de hasta 8 % a 0–2 %.
- **Límite reconocido.** Con matrices asimétricas 2-opt no aplica, así que la calidad de la ruta aproximada depende solo del vecino más cercano. Una mejora sería Or-opt o 3-opt dirigido.

### Infraestructura · Flujo máximo y Voronoi (responsable: Luis Enrique Castillo Carrillo, A00842390)

- **Flujo máximo.** Comparar Edmonds-Karp con Ford-Fulkerson (DFS) mostró en la práctica por qué importa que la complejidad no dependa de las capacidades: ~10× más rápido con 1,000 centrales.
- **Voronoi.** Elegimos recorte por semiplanos en lugar de Fortune conscientemente: perdimos el O(n log n) teórico a cambio de un código verificable. La poda por distancia recuperó buena parte del rendimiento (0.94 s vs 2.89 s con 5,000 centrales) sin cambiar el resultado.
- **Límite reconocido.** Las coordenadas se tratan como plano (longitud = x, latitud = y). A escala continental eso distorsiona las regiones; una versión más fiel proyectaría las coordenadas o trabajaría sobre la esfera.

### Ciberseguridad · Firmas, palíndromo y subcadena común (responsable: Desiderio Iván Ortegón Mortón, A00840591)

- **Firmas (KMP).** Con texto aleatorio la búsqueda ingenua rinde parecido; la diferencia aparece con datos repetitivos (0.85 s vs 0.13 s). En seguridad conviene la garantía de peor caso, porque un atacante puede elegir la entrada.
- **Palíndromo (Manacher).** La expansión desde el centro es cuadrática en el peor caso (2.46 s vs 0.015 s con 100,000 caracteres iguales). Manacher reutiliza la simetría ya conocida y nunca repite comparaciones.
- **Subcadena común (arreglo de sufijos + LCP).** Fue la mayor diferencia medida: 39.5 s con programación dinámica contra 0.067 s. El separador entre transmisiones y las reglas de desempate fueron los detalles que más casos borde produjeron.
- **Límite reconocido.** Encontrar una firma indica un patrón sospechoso, no una infección confirmada, porque la asociación transmisión–enlace es una construcción de simulación.

### Integración, aplicación y pruebas (responsable: Fernando Robles Vidrio, A00843290)

- **Integración.** Separar `main.cpp` (solo despacho) de un `.h` por desafío permitió trabajar en paralelo sin conflictos y mantener cada módulo con una sola responsabilidad.
- **Pruebas.** Cada modo se valida contra un oráculo escrito aparte en Python con otro algoritmo (por ejemplo, Dinic para flujo y fuerza bruta para TSP chico): 3,825 casos entre bordes, aleatorios con semilla fija, estrés y técnicas alternativas.
- **Restricción del autograder.** Compilar sin excepciones nos obligó a validar entradas de forma explícita y a responder `{"error": ...}` en lugar de depender de `try/catch`.

## 3. Decisiones transversales

| Decisión | Alternativa descartada | Por qué |
| --- | --- | --- |
| Núcleo único en C++ para autograder y página | Recalcular en JavaScript | Una sola fuente de verdad; lo que se ve es lo que se califica |
| Técnica principal + alternativa por desafío | Solo la principal | Justificar la elección con mediciones propias, no solo con la complejidad teórica |
| Benchmarks con semilla fija | Datos ad hoc | Reproducibilidad: cualquier integrante obtiene los mismos casos |
| JSON propio sin librerías | Librería externa | El autograder compila solo con la biblioteca estándar |

## 4. Limitaciones y trabajo futuro

- Los archivos oficiales del caso (SNDlib GÉANT y CIC-IDS2017) no tenían enlace de descarga; la aplicación usa la topología GÉANT del laboratorio y transmisiones de ejemplo o sintéticas con semilla 2038, marcadas como tales.
- Voronoi esférico o con proyección, y el algoritmo de Fortune para instancias muy grandes.
- Mejoras locales más fuertes para TSP asimétrico.
- Integración continua que compile y corra las pruebas en cada *commit*.

## 5. Aprendizaje como equipo

La complejidad teórica fue el punto de partida, pero las decisiones finales salieron de medir: en MST la teoría empataba y decidió la forma de los datos; en TSP la teoría marcaba el límite y la medición fijó el corte exacto; en los desafíos de texto, los peores casos medidos confirmaron que las garantías lineales valen la pena.
