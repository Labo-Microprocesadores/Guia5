/*
 * TetrisGame.h
 *
 *      Author: Grupo 2
 */

#ifndef SOURCES_TETRIS_H_
#define SOURCES_TETRIS_H_

typedef struct {
  unsigned char numOfRotates;
  unsigned char currentRotate;
  unsigned char pieceType;
  unsigned char x;
  unsigned char y;
  char attached;
  const unsigned char *shapes;
} piece;

typedef enum {
  TETRIS_INIT,
  TETRIS_WAIT_FOR_START,
  TETRIS_START,
  TETRIS_PLAY,
  TETRIS_LOST,
  TETRIS_END
} TETRIS_State;

void TETRIS_Start(void);

int TETRIS_Run(void);

#endif /* SOURCES_TETRIS_H_ */