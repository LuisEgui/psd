#include "world.h"

void
get_cell_up(tCoordinate* c, tCoordinate* dest_cell)
{
  dest_cell->row = c->row - 1;
  dest_cell->col = c->col;
}

void
get_cell_down(tCoordinate* c, tCoordinate* dest_cell)
{
  dest_cell->row = c->row + 1;
  dest_cell->col = c->col;
}

void
get_cell_left(tCoordinate* c, int world_width, tCoordinate* dest_cell)
{
  dest_cell->row = c->row;
  dest_cell->col = c->col > 0 ? c->col - 1 : world_width - 1;
}

void
get_cell_right(tCoordinate* c, int world_width, tCoordinate* dest_cell)
{
  dest_cell->row = c->row;
  dest_cell->col = c->col < world_width - 1 ? c->col + 1 : 0;
}

unsigned short int
get_cell_at_world(tCoordinate* c, unsigned short* world, int world_width)
{
  return (world[(c->row * world_width) + c->col]);
}

void
set_cell_at(tCoordinate* c, unsigned short* world, int world_width, unsigned short int type)
{
  world[(c->row * world_width) + c->col] = type;
}

void
init_random_world(unsigned short* w, int world_width, int world_height)
{
  tCoordinate cell;

  for (int col = 0; col < world_width; col++)
    for (int row = 0; row < world_height; row++)
      if ((rand() % 100) < INITIAL_CELLS_PERCENTAGE) {
        cell.row = row;
        cell.col = col;
        set_cell_at(&cell, w, world_width, CELL_LIVE);
      }
}

void
clear_world(unsigned short* w, int world_width, int world_height)
{
  tCoordinate cell;

  for (int col = 0; col < world_width; col++)
    for (int row = 0; row < world_height; row++) {
      cell.row = row;
      cell.col = col;
      set_cell_at(&cell, w, world_width, CELL_EMPTY);
    }
}

void
calculate_lonely_cell()
{
  int value, total = 0;
  tMatrix matrixA = (tMatrix)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
  tMatrix matrixB = (tMatrix)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
  tMatrix matrixC = (tMatrix)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));

  // Random matrix A
  for (int i = 0; i < (MATRIX_SIZE * MATRIX_SIZE); i++)
    matrixA[i] = (rand() % 1000);

  // Random matrix B
  for (int i = 0; i < (MATRIX_SIZE * MATRIX_SIZE); i++)
    matrixB[i] = (rand() % 1000);

  for (int i = 0; i < MATRIX_SIZE; i++)
    for (int j = 0; j < MATRIX_SIZE; j++) {
      matrixC[(i * MATRIX_SIZE) + j] = 0;
      for (int k = 0; k < MATRIX_SIZE; k++)
        matrixC[(i * MATRIX_SIZE) + j] +=
          matrixA[(i * MATRIX_SIZE) + k] * matrixB[(k * MATRIX_SIZE) + j];
    }

  free(matrixA);
  free(matrixB);
  free(matrixC);
}

unsigned short*
get_top_row(unsigned short* world, int world_width, int world_height, int current_row)
{
  return (current_row == 0) ? world + (world_height - 1) * world_width
                            : world + (current_row - 1) * world_width;
}

unsigned short*
get_bottom_row(unsigned short* world, int world_width, int world_height, int current_row)
{
  return (current_row == world_height) ? world : world + (current_row * world_width);
}

void
update_cell(tCoordinate* cell,
            int world_width,
            unsigned short* current_world,
            unsigned short* next_world_state)
{
  int neighbours = 0;
  tCoordinate* dest_cell = (tCoordinate*)malloc(sizeof(tCoordinate));

  if (dest_cell == NULL) {
    fprintf(stderr, "Memory allocation for dest_cell failed.\n");
    exit(EXIT_FAILURE);
  }

  // Check up
  get_cell_up(cell, dest_cell);

  if (get_cell_at_world(dest_cell, current_world, world_width) == CELL_LIVE)
    neighbours++;

  // Check up-left
  get_cell_left(cell, world_width, dest_cell);
  get_cell_up(dest_cell, dest_cell);

  if (get_cell_at_world(dest_cell, current_world, world_width) == CELL_LIVE)
    neighbours++;

  // Check right
  get_cell_right(cell, world_width, dest_cell);

  if (get_cell_at_world(dest_cell, current_world, world_width) == CELL_LIVE)
    neighbours++;

  // Check down-right
  get_cell_right(cell, world_width, dest_cell);
  get_cell_down(dest_cell, dest_cell);

  if (get_cell_at_world(dest_cell, current_world, world_width) == CELL_LIVE)
    neighbours++;

  // Check down
  get_cell_down(cell, dest_cell);

  if (get_cell_at_world(dest_cell, current_world, world_width) == CELL_LIVE)
    neighbours++;

  // Check down-left
  get_cell_left(cell, world_width, dest_cell);
  get_cell_down(dest_cell, dest_cell);

  if (get_cell_at_world(dest_cell, current_world, world_width) == CELL_LIVE)
    neighbours++;

  // Check left
  get_cell_left(cell, world_width, dest_cell);

  if (get_cell_at_world(dest_cell, current_world, world_width) == CELL_LIVE)
    neighbours++;

  // Check up-right
  get_cell_right(cell, world_width, dest_cell);
  get_cell_up(dest_cell, dest_cell);

  if (get_cell_at_world(dest_cell, current_world, world_width) == CELL_LIVE)
    neighbours++;

  if (get_cell_at_world(cell, current_world, world_width) == CELL_LIVE && (neighbours == 0))
    calculate_lonely_cell();

  if (get_cell_at_world(cell, current_world, world_width) == CELL_LIVE && (neighbours == 2) ||
      (neighbours == 3))
    set_cell_at(cell, next_world_state, world_width, CELL_LIVE);
  else if (get_cell_at_world(cell, current_world, world_width) == CELL_EMPTY && (neighbours == 3))
    set_cell_at(cell, next_world_state, world_width, CELL_LIVE);
  else
    set_cell_at(cell, next_world_state, world_width, CELL_EMPTY);

  free(dest_cell);
}

void
compute_next_world_state(unsigned short* current_world,
                         unsigned short* next_world_state,
                         int world_width,
                         int world_height)
{
  tCoordinate cell;

  for (int row = 1; row < world_height - 1; row++) {
    for (int col = 0; col < world_width; col++) {
      cell.col = col;
      cell.row = row;
      update_cell(&cell, world_width, current_world, next_world_state);
    }
  }
}