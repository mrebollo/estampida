# Juego Estampida

Un juego de tablero estratégico con dos modos: humano contra humano y humano contra IA entrenada mediante **Q-Learning**.

## Descripción del Juego

**Estampida** es un juego de dos jugadores en un tablero de 10×10 donde cada jugador controla 8 fichas iniciales. El objetivo es conseguir tener el mayor número fichas cuando se llena el tablero.

### Mecánicas del Juego

- **Tablero**: 10×10 celdas (100 casillas totales)
- **Fichas iniciales**: 8 por jugador
- **Jugadores**: 
  - Jugador 1: X
  - Jugador 2 (humano/máquina): O
  - Casillas vacías: .
  
- **Movimientos**: Cada jugador elige una de las cuatro direcciones:
  - **W**: Arriba
  - **S**: Abajo
  - **A**: Izquierda
  - **D**: Derecha

El movimiento lo que hace es replicar las fichas del jugador en la siguientes celdas libres en la dirección seleccionada.

### Fases del Juego

1. **Fase de Colocación**: Cada jugador coloca sus 8 fichas en el tablero
2. **Fase de Estampida**: Los jugadores se turnan para mover sus fichas
3. **Fin de la Partida**: Se cuenta el total de fichas y se determina el ganador

## Estructura del Proyecto

estampida/
├── juego.c              # Juego básico (2 jugadores humanos)
├── juego_rf.c           # Juego contra IA entrenada
├── train.c              # Script de entrenamiento con Q-Learning
├── parametros.h         # Constantes y definiciones del juego
├── rewards.txt          # Matriz Q-Table guardada (resultado del entrenamiento)
└── README.md            # Este archivo


### Descripción de Archivos

#### parametros.h
Define todas las constantes del juego:
- Dimensiones del tablero (10×10)
- Número de fichas iniciales (8)
- Número de estados para Q-Learning (27)
- Umbrales para categorización de estados

#### juego.c
Implementa el juego básico entre dos jugadores humanos.
- Inicializar tablero
- Mostrar tablero
- Colocar fichas
- Mover fichas en todas direcciones
- Contar fichas y determinar ganador

#### train.c
Implementa el algoritmo de entrenamiento Q-Learning.
- Entrena la IA jugando 10,000 partidas contra sí misma
- Guarda la matriz de recompensas en rewards.txt
- Soporta modo verbose (-v) para ver progreso en tiempo real
- Permite especificar número de partidas: ./train 50000

#### juego_rf.c (RF = Reinforcement Learning)
Juego interactivo donde el humano juega contra la máquina entrenada.
- Carga el conocimiento previo desde rewards.txt
- Humano coloca fichas y toma decisiones
- Máquina usa Q-Learning para elegir movimientos óptimos
- Evalúa el desempeño de la IA

#### rewards.txt
Matriz Q-Table guardada (27 estados × 4 direcciones).
Contiene los valores de recompensa aprendidos durante el entrenamiento.
Formato: 27 líneas, 4 valores por línea (uno por dirección: W, S, A, D)

## Inteligencia Artificial: Q-Learning

### Concepto de Estados

La IA discretiza la complejidad del juego en **27 estados** mediante 3 dimensiones:

#### 1. **Diferencia de Fichas** (3 categorías)
- 0: Perdiendo (mis_fichas - sus_fichas < -6)
- 1: Igualado (-6 ≤ diferencia ≤ 6)
- 2: Ganando (mis_fichas - sus_fichas > 6)

#### 2. **Fase de la Partida** (3 categorías)
- 0: Inicio (fichas totales < 35)
- 1: Medio (35 ≤ fichas totales ≤ 70)
- 2: Final (fichas totales > 70)

#### 3. **Control del Centro** (3 categorías)
Control del área central 4×4 (filas 4-7, columnas 4-7):
- 0: Bajo (< 3 fichas propias)
- 1: Medio (3-6 fichas propias)
- 2: Alto (> 6 fichas propias)

**Total**: 3 × 3 × 3 = **27 estados únicos**

### Algoritmo Q-Learning

La IA utiliza el algoritmo Q-Learning para aprender la mejor acción en cada estado:

$$
Q(estado, acción) = Q(estado, acción) + \alpha × (recompensa + \gamma × \max(Q(siguiente_estado, _)) - Q(estado, acción))
$$

Donde:
- **$\alpha$ (Learning Rate)**: Velocidad de aprendizaje (0-1)
- **$\gamma$ (Discount Factor)**: Importancia de recompensas futuras (0-1)
- **recompensa**: Cambio en fichas capturadas en este turno

### Acciones

La IA puede elegir entre 4 direcciones de movimiento:
- 0: W (Arriba)
- 1: S (Abajo)
- 2: A (Izquierda)
- 3: D (Derecha)

**Matriz de Recompensas**: 27 × 4 (estados × acciones)

## Proceso de Entrenamiento

### Pasos para Entrenar la IA

1. **Compilar el programa de entrenamiento**:
   ```bash
   gcc -o train train.c -lm
   ```

2. **Entrenar la IA** (por defecto 10.000 partidas ):
   ```bash
   ./train
   ```

   O especificar un número diferente de partidas:
   ```bash
   ./train 50000
   ```

   Para modo verbose (ver matriz en tiempo real):
   ```bash
   ./train -v
   ./train 50000 -v
   ```

3. **Resultado**:
   - Se genera/actualiza el archivo `rewards.txt` con la matriz Q-Table entrenada
   - Contiene los valores de recompensa aprendidos para cada estado-acción

### Parámetros de Entrenamiento

Los parámetros de Q-Learning pueden ajustarse en `train.c`:
- **Learning Rate (α)**: Influye en la velocidad de convergencia
- **Discount Factor (γ)**: Equilibra recompensas inmediatas vs futuras
- **Exploration Rate (ε)**: Probabilidad de acciones aleatorias (epsilon-greedy)
- **Número de partidas**: Más partidas = mejor aprendizaje (con tiempo de ejecución)

### Estrategia de Juego

La IA entrenada considera:

1. **Cambio en la diferencia de fichas**: Favorece movimientos que capturan fichas
2. **Fase del juego**: Adapta su estrategia según el avance de la partida
3. **Control del centro**: Prioriza el control de la zona central (4×4)
4. **Exploración vs Explotación**: Equilibra probar nuevas estrategias vs usar las conocidas


## Cómo Jugar

### Compilar

Juego básico (2 jugadores)
```bash
gcc -o juego juego.c -lm
```
Juego persona contra máquina
```bash
gcc -o juego_rf juego_rf.c -lm
```
Entrenar la IA
```bash
gcc -o train train.c -lm
```

### Ejecutar Juego 2 Jugadores
```bash
./juego
```
Ambos jugadores son humanos. Sigue las instrucciones en pantalla.

### Ejecutar Juego vs Máquina
```bash
./juego_rf
```
- La persona juega como `X`
- La máquina juega como `O`
- Coloca tus fichas estratégicamente
- Juega contra la IA entrenada

**Requisito previo**: Debe existir `rewards.txt` generado por el entrenamiento.


