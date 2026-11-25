#include "parametros.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Matriz de aprendizaje Q-Table: 27 estados x 4 direcciones (0:W, 1:S, 2:A,
// 3:D)
float recompensa[NUM_ESTADOS][4];

// Prototipos de funciones
void inicializar(char tablero[FILAS][COLUMNAS]);
void mostrar(char tablero[FILAS][COLUMNAS]);
void cargar_conocimiento();
int contar_fichas(char tablero[FILAS][COLUMNAS], char jugador);
int obtener_estado(char tablero[FILAS][COLUMNAS], char jugador);

void colocar_humano(char tablero[FILAS][COLUMNAS], int num_ficha);
void colocar_maquina(char tablero[FILAS][COLUMNAS], int num_ficha);
void colocar_random(char tablero[FILAS][COLUMNAS], int num_ficha, char jugador);
void colocar(char tablero[FILAS][COLUMNAS]);

char estampida_humano();
char estampida_maquina(char tablero[FILAS][COLUMNAS], char jugador);
void estampida(char tablero[FILAS][COLUMNAS]);

int mover_arriba(char tablero[FILAS][COLUMNAS], char jugador);
int mover_abajo(char tablero[FILAS][COLUMNAS], char jugador);
int mover_izquierda(char tablero[FILAS][COLUMNAS], char jugador);
int mover_derecha(char tablero[FILAS][COLUMNAS], char jugador);
int mover(char tablero[FILAS][COLUMNAS], char jugador, char direccion);
void resultado(char tablero[FILAS][COLUMNAS]);

int main() {
  srand(time(NULL));
  cargar_conocimiento();

  char tablero[FILAS][COLUMNAS];

  printf("===========================================\n");
  printf("   JUEGO DE ESTAMPIDA vs MÁQUINA (Q-LEARNING/TRAINED)   \n");
  printf("===========================================\n\n");

  // 1. Inicializar y mostrar tablero
  inicializar(tablero);
  mostrar(tablero);

  // 2. Fase de colocación (Humano 'X' elige, Máquina 'O' al azar)
  colocar(tablero);

  // 3. Fase de estampida (Humano vs Máquina entrenada por train)
  estampida(tablero);

  // 4. Fin del juego y recuento
  resultado(tablero);

  return 0;
}

// Carga los pesos aprendidos generados por train.c
void cargar_conocimiento() {
  FILE *f = fopen(REWARD_FILE, "r");
  if (!f) {
    printf("[Aviso] No se encontro '%s'. La maquina usara movimientos por "
           "defecto.\n",
           REWARD_FILE);
    for (int e = 0; e < NUM_ESTADOS; e++)
      for (int d = 0; d < 4; d++)
        recompensa[e][d] = 0.0f;
    return;
  }

  for (int e = 0; e < NUM_ESTADOS; e++) {
    for (int d = 0; d < 4; d++) {
      fscanf(f, "%f", &recompensa[e][d]);
    }
  }
  fclose(f);
  printf("[Info] Conocimiento entrenado de '%s' cargado con exito.\n",
         REWARD_FILE);
}

int contar_fichas(char tablero[FILAS][COLUMNAS], char jugador) {
  int count = 0;
  for (int r = 0; r < FILAS; r++)
    for (int c = 0; c < COLUMNAS; c++)
      if (tablero[r][c] == jugador)
        count++;
  return count;
}

// Calcula el índice de estado (0 a 26) según 3 métricas estáticas del tablero
int obtener_estado(char tablero[FILAS][COLUMNAS], char jugador) {
  char rival = (jugador == 'X') ? 'O' : 'X';
  int mis_fichas = contar_fichas(tablero, jugador);
  int sus_fichas = contar_fichas(tablero, rival);
  int total = mis_fichas + sus_fichas;
  int dif = mis_fichas - sus_fichas;

  int cat_dif = 1;
  if (dif < UMBRAL_DIF_BAJO)
    cat_dif = 0;
  else if (dif > UMBRAL_DIF_ALTO)
    cat_dif = 2;

  int cat_fase = 1;
  if (total < UMBRAL_FASE_INICIO)
    cat_fase = 0;
  else if (total > UMBRAL_FASE_FINAL)
    cat_fase = 2;

  // Control del Centro (Contar fichas en el cuadrante central 4x4: filas 3..6,
  // columnas 3..6)
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

  return (cat_dif * 9) + (cat_fase * 3) + cat_centro;
}

// Inicializa el tablero con puntos vacíos
void inicializar(char tablero[FILAS][COLUMNAS]) {
  for (int r = 0; r < FILAS; r++) {
    for (int c = 0; c < COLUMNAS; c++) {
      tablero[r][c] = VACIO;
    }
  }
}

// Muestra el tablero numerando filas y columnas del 1 al 10
void mostrar(char tablero[FILAS][COLUMNAS]) {
  printf("\n   ");
  for (int c = 1; c <= COLUMNAS; c++) {
    printf("%2d ", c);
  }
  printf("\n");

  for (int r = 0; r < FILAS; r++) {
    printf("%2d ", r + 1);
    for (int c = 0; c < COLUMNAS; c++) {
      printf(" %c ", tablero[r][c]);
    }
    printf("\n");
  }
  printf("\n");
}

// Subfunciones de colocación
void colocar_humano(char tablero[FILAS][COLUMNAS], int num_ficha) {
  int fila, col, r, c;
  do {
    printf("\nX [%d/%d]: ", num_ficha, FICHAS_INICIALES);
    scanf("%d %d", &fila, &col);
    r = fila - 1;
    c = col - 1;
  } while (r < 0 || r >= FILAS || c < 0 || c >= COLUMNAS ||
           tablero[r][c] != VACIO);
  tablero[r][c] = 'X';
}

void colocar_maquina(char tablero[FILAS][COLUMNAS], int num_ficha) {
  int r, c;
  do {
    r = rand() % FILAS;
    c = rand() % COLUMNAS;
  } while (tablero[r][c] != VACIO);
  tablero[r][c] = 'O';
  printf("\nO [%d/%d]: (%d, %d)\n", num_ficha, FICHAS_INICIALES, r + 1, c + 1);
}

void colocar_random(char tablero[FILAS][COLUMNAS], int num_ficha,
                    char jugador) {
  int r, c;
  do {
    r = rand() % FILAS;
    c = rand() % COLUMNAS;
  } while (tablero[r][c] != VACIO);
  tablero[r][c] = jugador;
  printf("\n%c [%d/%d]: (%d, %d)\n", jugador, num_ficha, FICHAS_INICIALES,
         r + 1, c + 1);
}

// Fase 1: Coordinador de colocación por turnos
void colocar(char tablero[FILAS][COLUMNAS]) {
  char jugadores[2] = {'X', 'O'};

  for (int i = 0; i < FICHAS_INICIALES * 2; i++) {
    char jugador = jugadores[i % 2];
    int num_ficha = (i / 2) + 1;

    if (jugador == 'X') {
      colocar_random(tablero, num_ficha, 'X');
    } else {
      colocar_random(tablero, num_ficha, 'O');
    }
  }
  mostrar(tablero);
}

// Subfunciones de estampida
char estampida_humano() {
  char direccion;
  printf("[Tu Turno 'X'] Elige dirección (W/A/S/D): ");
  scanf(" %c", &direccion);
  return toupper(direccion);
}

char estampida_maquina(char tablero[FILAS][COLUMNAS], char jugador) {
  char dirs[4] = {'W', 'S', 'A', 'D'};
  int estado = obtener_estado(tablero, jugador);

  int mejor_idx = -1;
  float mejor_valor = -999999.0f;

  for (int i = 0; i < 4; i++) {
    char temp[FILAS][COLUMNAS];
    for (int r = 0; r < FILAS; r++)
      for (int c = 0; c < COLUMNAS; c++)
        temp[r][c] = tablero[r][c];

    if (mover(temp, jugador, dirs[i]) > 0) {
      if (recompensa[estado][i] > mejor_valor) {
        mejor_valor = recompensa[estado][i];
        mejor_idx = i;
      }
    }
  }

  if (mejor_idx == -1)
    mejor_idx = 0;

  char direccion = dirs[mejor_idx];
  printf("[Turno Máquina '%c'] Dirección: %c "
         "(Estado: %d |  Puntuación: %.1f)\n",
         jugador, direccion, estado, mejor_valor);
  return direccion;
}

// Fase 2: Coordinador de estampida por turnos
void estampida(char tablero[FILAS][COLUMNAS]) {
  char jugadores[2] = {'X', 'O'};
  int turno =
      rand() % 2; // Sorteo aleatorio: 0 para Humano ('X'), 1 para Máquina ('O')
  int total_fichas = FICHAS_INICIALES * 2;

  printf("\n===========================================\n");
  printf("  Empieza el jugador '%c'\n", jugadores[turno]);
  printf("  Controles Humano: W (Arriba), S (Abajo), A (Izquierda), D "
         "(Derecha)\n");
  printf("===========================================\n");

  while (total_fichas < FILAS * COLUMNAS) {
    mostrar(tablero);
    char jugador = jugadores[turno];
    char direccion;

    if (jugador == 'X') {
      direccion = estampida_humano();
    } else {
      direccion = estampida_maquina(tablero, jugador);
    }

    total_fichas += mover(tablero, jugador, direccion);

    // Cambiar turno
    turno = (turno + 1) % 2;
  }
}

// Funciones de movimiento
int mover_arriba(char tablero[FILAS][COLUMNAS], char jugador) {
  int total = 0;
  for (int r = 0; r < FILAS; r++) {
    for (int c = 0; c < COLUMNAS; c++) {
      if (tablero[r][c] == jugador && r > 0 && tablero[r - 1][c] == VACIO) {
        tablero[r - 1][c] = jugador;
        total++;
      }
    }
  }
  return total;
}

int mover_abajo(char tablero[FILAS][COLUMNAS], char jugador) {
  int total = 0;
  for (int r = FILAS - 1; r >= 0; r--) {
    for (int c = 0; c < COLUMNAS; c++) {
      if (tablero[r][c] == jugador && r < FILAS - 1 &&
          tablero[r + 1][c] == VACIO) {
        tablero[r + 1][c] = jugador;
        total++;
      }
    }
  }
  return total;
}

int mover_izquierda(char tablero[FILAS][COLUMNAS], char jugador) {
  int total = 0;
  for (int r = 0; r < FILAS; r++) {
    for (int c = 0; c < COLUMNAS; c++) {
      if (tablero[r][c] == jugador && c > 0 && tablero[r][c - 1] == VACIO) {
        tablero[r][c - 1] = jugador;
        total++;
      }
    }
  }
  return total;
}

int mover_derecha(char tablero[FILAS][COLUMNAS], char jugador) {
  int total = 0;
  for (int r = 0; r < FILAS; r++) {
    for (int c = COLUMNAS - 1; c >= 0; c--) {
      if (tablero[r][c] == jugador && c < COLUMNAS - 1 &&
          tablero[r][c + 1] == VACIO) {
        tablero[r][c + 1] = jugador;
        total++;
      }
    }
  }
  return total;
}

int mover(char tablero[FILAS][COLUMNAS], char jugador, char direccion) {
  if (direccion == 'W')
    return mover_arriba(tablero, jugador);
  if (direccion == 'S')
    return mover_abajo(tablero, jugador);
  if (direccion == 'A')
    return mover_izquierda(tablero, jugador);
  if (direccion == 'D')
    return mover_derecha(tablero, jugador);
  return 0;
}

void resultado(char tablero[FILAS][COLUMNAS]) {
  int fichasX = 0;
  int fichasO = 0;

  for (int r = 0; r < FILAS; r++) {
    for (int c = 0; c < COLUMNAS; c++) {
      if (tablero[r][c] == 'X')
        fichasX++;
      else if (tablero[r][c] == 'O')
        fichasO++;
    }
  }

  printf("\n===========================================\n");
  printf("               FIN DEL JUEGO               \n");
  printf("===========================================\n");
  printf("Recuento Final: Humano ('X'): %d | Máquina ('O'): %d\n\n", fichasX,
         fichasO);
}
