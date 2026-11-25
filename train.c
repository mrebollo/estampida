#include "parametros.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PARTIDAS 10000

int verbose = 0; // 0: Normal/Barra de progreso, 1: Matriz en vivo (-v)

// Matriz de aprendizaje Q-Table: 27 estados x 4 direcciones (0:W, 1:S, 2:A,
// 3:D)
float recompensa[NUM_ESTADOS][4];

// Prototipos de funciones del juego
void inicializar(char tablero[FILAS][COLUMNAS]);
void mostrar(char tablero[FILAS][COLUMNAS]);
void colocar_aleatorio(char tablero[FILAS][COLUMNAS]);
int mover(char tablero[FILAS][COLUMNAS], char jugador, char direccion);
bool esta_lleno(char tablero[FILAS][COLUMNAS]);
void contar_fichas(char tablero[FILAS][COLUMNAS], int *fx, int *fo);

// Prototipos de funciones de IA y aprendizaje (WOPR)
void save();
int obtener_estado(char tablero[FILAS][COLUMNAS], char jugador);
char elegir_movimiento(char tablero[FILAS][COLUMNAS], char jugador,
                       float exploracion);
void train(int num_partidas);
void mostrar_matriz_verbose(int actual, int total);

int main(int argc, char *argv[]) {
  int partidas = PARTIDAS;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0) {
      verbose = 1;
    } else {
      int p = atoi(argv[i]);
      if (p > 0)
        partidas = p;
    }
  }

  train(partidas);
  save();

  return 0;
}

// -----------------------------------------------------------------------
// LÓGICA DE APRENDIZAJE E INTELIGENCIA ARTIFICIAL
// -----------------------------------------------------------------------

// Calcula el índice de estado (0 a 26) según 3 métricas estáticas del tablero
int obtener_estado(char tablero[FILAS][COLUMNAS], char jugador) {
  int fx, fo;
  contar_fichas(tablero, &fx, &fo);
  int mis_fichas = (jugador == 'X') ? fx : fo;
  int sus_fichas = (jugador == 'X') ? fo : fx;
  int total = mis_fichas + sus_fichas;
  int dif = mis_fichas - sus_fichas;

  // 1. Diferencia de fichas
  int cat_dif = 1;
  if (dif < UMBRAL_DIF_BAJO)
    cat_dif = 0;
  else if (dif > UMBRAL_DIF_ALTO)
    cat_dif = 2;

  // 2. Fase de partida
  int cat_fase = 1;
  if (total < UMBRAL_FASE_INICIO)
    cat_fase = 0;
  else if (total > UMBRAL_FASE_FINAL)
    cat_fase = 2;

  // 3. Control del Centro (Contar fichas en el cuadrante central 4x4:
  // filas 3..6, columnas 3..6)
  int fichas_centro = 0;
  for (int r = 3; r <= 6; r++) {
    for (int c = 3; c <= 6; c++) {
      if (tablero[r][c] == jugador)
        fichas_centro++;
    }
  }

  int cat_centro = 1;
  if (fichas_centro < UMBRAL_CENTRO_BAJO)
    cat_centro = 0;
  else if (fichas_centro > UMBRAL_CENTRO_ALTO)
    cat_centro = 2;

  // Combinación única en base 3 -> Rango [0..26]
  return (cat_dif * 9) + (cat_fase * 3) + cat_centro;
}

// Elige la mejor dirección según la tabla Q o una aleatoria si está explorando
char elegir_movimiento(char tablero[FILAS][COLUMNAS], char jugador,
                       float exploracion) {
  char dirs[4] = {'W', 'S', 'A', 'D'};

  if ((rand() % 100) < (exploracion * 100)) {
    return dirs[rand() % 4];
  }

  int estado = obtener_estado(tablero, jugador);
  int mejor_idx = 0;
  float mejor_valor = recompensa[estado][0];

  for (int i = 1; i < 4; i++) {
    if (recompensa[estado][i] > mejor_valor) {
      mejor_valor = recompensa[estado][i];
      mejor_idx = i;
    }
  }

  return dirs[mejor_idx];
}

// Muestra la matriz de recompensas tabulada refrescando la pantalla
void mostrar_matriz_verbose(int actual, int total) {
  printf("\033[H\033[J"); // Limpiar pantalla en Linux/macOS
  printf("--- MATRIZ DE RECOMPENSAS Q-TABLE (Partida %d/%d) ---\n", actual,
         total);
  printf(" EST        W        S        A        D\n");
  for (int e = 0; e < NUM_ESTADOS; e++) {
    printf("[%02d] %8.1f %8.1f %8.1f %8.1f\n", e, recompensa[e][0],
           recompensa[e][1], recompensa[e][2], recompensa[e][3]);
  }
  fflush(stdout);
}

// Comprueba si un jugador puede hacer algún movimiento válido en cualquier
// dirección
bool puede_mover(char tablero[FILAS][COLUMNAS], char jugador) {
  char dirs[4] = {'W', 'S', 'A', 'D'};
  for (int i = 0; i < 4; i++) {
    char temp[FILAS][COLUMNAS];
    for (int r = 0; r < FILAS; r++)
      for (int c = 0; c < COLUMNAS; c++)
        temp[r][c] = tablero[r][c];

    if (mover(temp, jugador, dirs[i]) > 0)
      return true;
  }
  return false;
}

// Imprime una barra de progreso sencilla por consola
void mostrar_progreso(int actual, int total) {
  int ancho = 30;
  float porcentaje = (float)actual / total;
  int completado = porcentaje * ancho;

  printf("\r[WOPR] [");
  for (int i = 0; i < ancho; i++) {
    if (i < completado)
      printf("=");
    else if (i == completado)
      printf(">");
    else
      printf(" ");
  }
  printf("] %d/%d (%d%%)", actual, total, (int)(porcentaje * 100));
  fflush(stdout);
}

// Modo WOPR: La máquina simula partidas contra sí misma y aprende de los
// resultados
void train(int num_partidas) {
  if (!verbose) {
    printf("\n[WOPR] Simulando %d partidas a alta velocidad...\n",
           num_partidas);
  }
  char jugadores[2] = {'X', 'O'};
  srand(time(NULL));

  for (int p = 0; p < num_partidas; p++) {
    if (verbose) {
      if ((p + 1) % 50 == 0 || p == num_partidas - 1) {
        mostrar_matriz_verbose(p + 1, num_partidas);
        usleep(20000); // 20ms de retardo
      }
    } else {
      if ((p + 1) % 100 == 0 || p == num_partidas - 1) {
        mostrar_progreso(p + 1, num_partidas);
      }
    }

    char tablero[FILAS][COLUMNAS];
    inicializar(tablero);
    colocar_aleatorio(tablero);

    int historial_estado[100];
    int historial_mov[100];
    char historial_jugador[100];
    int num_pasos = 0;
    int total_fichas = FICHAS_INICIALES * 2;

    int turno = 0;
    int pasadas_seguidas = 0;

    while (total_fichas < FILAS * COLUMNAS && pasadas_seguidas < 2) {
      char jugador = jugadores[turno];

      if (!puede_mover(tablero, jugador)) {
        pasadas_seguidas++;
        turno = (turno + 1) % 2;
        continue;
      }

      pasadas_seguidas = 0;
      int est = 0;
      int mov_idx = 0;
      int nuevas = 0;

      // Buscar una dirección válida
      do {
        est = obtener_estado(tablero, jugador);
        char dir = elegir_movimiento(tablero, jugador, 0.2f);
        mov_idx = (dir == 'W') ? 0 : (dir == 'S') ? 1 : (dir == 'A') ? 2 : 3;

        nuevas = mover(tablero, jugador, dir);
      } while (nuevas == 0);

      total_fichas += nuevas;

      if (num_pasos < 100) {
        historial_estado[num_pasos] = est;
        historial_mov[num_pasos] = mov_idx;
        historial_jugador[num_pasos] = jugador;
        num_pasos++;
      }

      turno = (turno + 1) % 2;
    }

    // Evaluación del resultado final de la partida
    int fx, fo;
    contar_fichas(tablero, &fx, &fo);

    // Entrenamos ambas perspectivas: X y O
    for (int i = 0; i < num_pasos; i++) {
      char j = historial_jugador[i];
      float premio = 0.0f;

      if ((j == 'X' && fx > fo) || (j == 'O' && fo > fx)) {
        premio = 1.0f; // Ganador de la decisión
      } else if ((j == 'X' && fo > fx) || (j == 'O' && fx > fo)) {
        premio = -1.0f; // Perdedor de la decisión
      }

      recompensa[historial_estado[i]][historial_mov[i]] += premio;
    }
  }

  printf("\n[WOPR] Entrenamiento completado.\n");
  printf("Extraño juego, profesor Falken.\nLa única forma de ganar es no "
         "jugar.\n");
}

// -----------------------------------------------------------------------
// FUNCIONES AUXILIARES DE CONOCIMIENTO (FICHERO)
// -----------------------------------------------------------------------

void save() {
  FILE *f = fopen(REWARD_FILE, "w");
  if (!f)
    return;

  for (int e = 0; e < NUM_ESTADOS; e++) {
    for (int d = 0; d < 4; d++) {
      fprintf(f, "%.2f ", recompensa[e][d]);
    }
    fprintf(f, "\n");
  }
  fclose(f);
  printf("[WOPR] Recompensas guardadas en '%s'.\n", REWARD_FILE);
}

// -----------------------------------------------------------------------
// MODO DE JUEGO (HUMANO VS IA) Y FUNCIONES DE TABLERO
// -----------------------------------------------------------------------

void inicializar(char tablero[FILAS][COLUMNAS]) {
  for (int r = 0; r < FILAS; r++)
    for (int c = 0; c < COLUMNAS; c++)
      tablero[r][c] = VACIO;
}

void colocar_aleatorio(char tablero[FILAS][COLUMNAS]) {
  char jugadores[2] = {'X', 'O'};
  for (int i = 0; i < FICHAS_INICIALES * 2; i++) {
    char jugador = jugadores[i % 2];
    int r, c;
    do {
      r = rand() % FILAS;
      c = rand() % COLUMNAS;
    } while (tablero[r][c] != VACIO);
    tablero[r][c] = jugador;
  }
}

void contar_fichas(char tablero[FILAS][COLUMNAS], int *fx, int *fo) {
  *fx = 0;
  *fo = 0;
  for (int r = 0; r < FILAS; r++)
    for (int c = 0; c < COLUMNAS; c++)
      if (tablero[r][c] == 'X')
        (*fx)++;
      else if (tablero[r][c] == 'O')
        (*fo)++;
}

// Funciones de movimiento
int mover(char tablero[FILAS][COLUMNAS], char jugador, char direccion) {
  int df = 0, dc = 0;
  int inicioFila = 0, finFila = FILAS, incFila = 1;
  int inicioCol = 0, finCol = COLUMNAS, incCol = 1;

  if (direccion == 'W')
    df = -1;
  else if (direccion == 'S') {
    df = 1;
    inicioFila = FILAS - 1;
    finFila = -1;
    incFila = -1;
  } else if (direccion == 'A')
    dc = -1;
  else if (direccion == 'D') {
    dc = 1;
    inicioCol = COLUMNAS - 1;
    finCol = -1;
    incCol = -1;
  }

  int total = 0;
  for (int r = inicioFila; r != finFila; r += incFila) {
    for (int c = inicioCol; c != finCol; c += incCol) {
      if (tablero[r][c] == jugador) {
        int destFila = r + df;
        int destCol = c + dc;
        if (destFila >= 0 && destFila < FILAS && destCol >= 0 &&
            destCol < COLUMNAS) {
          if (tablero[destFila][destCol] == VACIO) {
            tablero[destFila][destCol] = jugador;
            total++;
          }
        }
      }
    }
  }
  return total;
}
