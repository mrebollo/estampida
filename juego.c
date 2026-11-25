#include <ctype.h>
#include <stdio.h>

#define FILAS 10
#define COLUMNAS 10
#define FICHAS_INICIALES 8
#define VACIO '.'

// Prototipos de funciones
void inicializar(char tablero[FILAS][COLUMNAS]);
void mostrar(char tablero[FILAS][COLUMNAS]);
void colocar(char tablero[FILAS][COLUMNAS]);
void estampida(char tablero[FILAS][COLUMNAS]);
int mover_arriba(char tablero[FILAS][COLUMNAS], char jugador);
int mover_abajo(char tablero[FILAS][COLUMNAS], char jugador);
int mover_izquierda(char tablero[FILAS][COLUMNAS], char jugador);
int mover_derecha(char tablero[FILAS][COLUMNAS], char jugador);
int mover(char tablero[FILAS][COLUMNAS], char jugador, char direccion);
void resultado(char tablero[FILAS][COLUMNAS]);

// Prototipos opcionales/extensión (usando struct Limites)
typedef struct {
  int inicioFila, finFila, incFila;
  int inicioCol, finCol, incCol;
  int df, dc;
} limites;
limites obtener_limites(char direccion);
int mover_unica(char tablero[FILAS][COLUMNAS], char jugador, char direccion);

int main() {
  char tablero[FILAS][COLUMNAS];

  printf("===========================================\n");
  printf("         JUEGO DE ESTAMPIDA EN TABLERO 10x10   \n");
  printf("===========================================\n\n");

  // 1. Inicializar y mostrar tablero
  inicializar(tablero);
  mostrar(tablero);

  // 2. Fase de colocación (8 fichas por jugador)
  colocar(tablero);

  // 3. Fase de estampida por turnos
  estampida(tablero);

  // 4. Fin del juego y recuento
  resultado(tablero);

  return 0;
}

// Inicializa el tablero con puntos vacíos
void inicializar(char tablero[FILAS][COLUMNAS]) {
  for (int r = 0; r < FILAS; r++) {
    for (int c = 0; c < COLUMNAS; c++) {
      tablero[r][c] = VACIO;
    }
  }
}

// Muestra el tablero numerando filas y columnas del 1 al 10 para facilitar la lectura
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

// Fase 1: Cada jugador coloca 8 fichas por coordenadas (1..10) de forma alterna
void colocar(char tablero[FILAS][COLUMNAS]) {
  char jugadores[2] = {'X', 'O'};
  int fila, col;

  for (int i = 0; i < FICHAS_INICIALES * 2; i++) {
    char jugador = jugadores[i % 2];

    do {
      printf("\n[%d/%d] - Ficha '%c': ", (i / 2) + 1, FICHAS_INICIALES, jugador);
      scanf("%d %d", &fila, &col);
    } while (tablero[fila - 1][col - 1] != VACIO);

    tablero[fila - 1][col - 1] = jugador;
    mostrar(tablero);
  }
}

// Fase 2: Juego. Expansión de las fichas por turnos
void estampida(char tablero[FILAS][COLUMNAS]) {
  char jugadores[2] = {'X', 'O'};
  int turno = 0; // 0 para Jugador 1 ('X'), 1 para Jugador 2 ('O')
  int total_fichas = FICHAS_INICIALES * 2;

  printf("\n===========================================\n");
  printf("  Controles: W (Arriba), S (Abajo), A (Izquierda), D (Derecha), P (Pasar)\n");
  printf("===========================================\n");
  while (total_fichas < FILAS * COLUMNAS) {
    mostrar(tablero);
    char jugador = jugadores[turno];
    char direccion;

    printf("[%c] Elige dirección (W/A/S/D | P): ", jugador);
    scanf(" %c", &direccion);
    direccion = toupper(direccion);
    if (direccion == 'P')
      continue;
    total_fichas += mover(tablero, jugador, direccion);

    // Cambiar turno
    turno = (turno + 1) % 2;
  }
}

// Funciones de movimiento separadas según dirección
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
      if (tablero[r][c] == jugador && r < FILAS - 1 && tablero[r + 1][c] == VACIO) {
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
      if (tablero[r][c] == jugador && c < COLUMNAS - 1 && tablero[r][c + 1] == VACIO) {
        tablero[r][c + 1] = jugador;
        total++;
      }
    }
  }
  return total;
}

// Función principal que delega en las 4 funciones dedicadas (1º de Carrera)
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

// Cuenta fichas al final del juego e informa del ganador
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

  printf("\nRecuento Final:\n");
  printf(" - Jugador 'X': %d fichas\n", fichasX);
  printf(" - Jugador 'O': %d fichas\n\n", fichasO);
}

// =======================================================================
// EXTRA / OPCIONAL: Versión unificada usando retorno de struct Limites por valor
// =======================================================================
limites obtener_limites(char direccion) {
  limites lim = {0, FILAS, 1, 0, COLUMNAS, 1, 0, 0};

  if (direccion == 'W') {
    lim.df = -1;
  } else if (direccion == 'S') {
    lim.df = 1;
    lim.inicioFila = FILAS - 1;
    lim.finFila = -1;
    lim.incFila = -1;
  } else if (direccion == 'A') {
    lim.dc = -1;
  } else if (direccion == 'D') {
    lim.dc = 1;
    lim.inicioCol = COLUMNAS - 1;
    lim.finCol = -1;
    lim.incCol = -1;
  }

  return lim;
}

int mover_unica(char tablero[FILAS][COLUMNAS], char jugador, char direccion) {
  int total = 0;
  limites lim = obtener_limites(direccion);

  for (int r = lim.inicioFila; r != lim.finFila; r += lim.incFila) {
    for (int c = lim.inicioCol; c != lim.finCol; c += lim.incCol) {
      if (tablero[r][c] == jugador) {
        int destFila = r + lim.df;
        int destCol = c + lim.dc;

        if (destFila >= 0 && destFila < FILAS && destCol >= 0 && destCol < COLUMNAS) {
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
