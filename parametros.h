#ifndef PARAMETROS_H
#define PARAMETROS_H

// -----------------------------------------------------------------------
// PARÁMETROS DEL TABLERO Y JUEGO
// -----------------------------------------------------------------------
#define FILAS 10
#define COLUMNAS 10
#define FICHAS_INICIALES 8
#define VACIO '.'

// -----------------------------------------------------------------------
// PARÁMETROS DE INTELIGENCIA ARTIFICIAL Y APRENDIZAJE (Q-LEARNING)
// -----------------------------------------------------------------------
// Representación de estados: 3x3x3 = 27 estados posibles
// - Diferencia de fichas: 0 (perdiendo < -6), 1 (igualado), 2 (ganando > 6)
// - Fase de la partida: 0 (inicio < 35), 1 (medio 35-70), 2 (final > 70)
// - Control del Centro del tablero (4x4 central): 0 (bajo < 3), 1 (medio 3-6), 2 (alto > 6)
#define NUM_ESTADOS 27
#define REWARD_FILE "rewards.txt"

// Umbrales para la categoría de diferencia de fichas (dif = mis_fichas - sus_fichas)
#define UMBRAL_DIF_BAJO -6
#define UMBRAL_DIF_ALTO 6

// Umbrales para la categoría de fase de partida (total = mis_fichas + sus_fichas)
#define UMBRAL_FASE_INICIO 35
#define UMBRAL_FASE_FINAL 70

// Umbrales para el control del cuadrante central del tablero (filas 4..7, columnas 4..7)
#define UMBRAL_CENTRO_BAJO 3
#define UMBRAL_CENTRO_ALTO 6

#endif // PARAMETROS_H
