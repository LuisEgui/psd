#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include "types.h"

/**
 * Calculates the coordinate above the current cell.
 *
 * @param c Cell's coordinate
 * @param dest_cell Coordinate above the current cell.
 */
void
get_cell_up(tCoordinate* c, tCoordinate* dest_cell);

/**
 * Calculates the coordinate below the current cell.
 *
 * @param c Cell's coordinate
 * @param dest_cell Coordinate below the current cell.
 */
void
get_cell_down(tCoordinate* c, tCoordinate* dest_cell);

/**
 * Calculates the coordinate of the cell at the left of the current cell.
 *
 * @param c Cell's coordinate
 * @param world_width Width of the world (in number of cells).
 * @param dest_cell Coordinate at the left of the current cell.
 */
void
get_cell_left(tCoordinate* c, int world_width, tCoordinate* dest_cell);

/**
 * Calculates the coordinate of the cell at the right of the current cell.
 *
 * @param c Cell's coordinate
 * @param world_width Width of the world (in number of cells).
 * @param dest_cell Coordinate at the right of the current cell.
 */
void
get_cell_right(tCoordinate* c, int world_width, tCoordinate* dest_cell);

/**
 * Gets the cell at coordinate c located in a world.
 *
 * @param c Cell's coordinate
 * @param world World where the requested cell is placed.
 * @param world_width Width of the world (in number of cells).
 * @return State of the cell at coordinate c.
 */
unsigned short int
get_cell_at_world(tCoordinate* c, unsigned short* world, int world_width);

/**
 * Sets the cell at coordinate c.
 *
 * @param c Cell's coordinate
 * @param world World where is plced the requested cell.
 * @param world_width Width of the world (in number of cells).
 * @param value New state of the cell.
 */
void
set_cell_at(tCoordinate* c, unsigned short* world, int world_width, unsigned short int type);

/**
 * Inits the world (randomly)
 *
 * @param w World to be initialized.
 * @param world_width Width of the world (in number of cells).
 * @param world_height Height of the world (in number of cells).
 */
void
init_random_world(unsigned short* w, int world_width, int world_height);

/**
 * Clears the world.
 *
 * @param w World to be cleared (all cells are set to CELL_EMPTY).
 * @param world_width Width of the world (in number of cells).
 * @param world_height Height of the world (in number of cells).
 *
 */
void
clear_world(unsigned short* w, int world_width, int world_height);

/**
 * Calculations performed when a cell is dead and has no neighbours.
 */
void
calculate_lonely_cell();

unsigned short*
get_top_row(unsigned short* world, int world_width, int world_height, int current_row);

unsigned short*
get_bottom_row(unsigned short* world, int world_width, int world_height, int current_row);

void
update_cell(tCoordinate* cell,
            int world_width,
            unsigned short* current_world,
            unsigned short* next_world_state);

void
compute_next_world_state(unsigned short* current_world,
                         unsigned short* next_world_state,
                         int world_width,
                         int world_height);

#endif // WORLD_H_INCLUDED