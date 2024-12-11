#include "world.h"
#include <SDL2/SDL.h>

/**
 * Draws a cell in a renderer.
 *
 * @param cell Cell's coordinates.
 * @param type Type of the cell.
 * @param renderer Rendered where the cell is drawn.
 */
void
draw_cell(tCoordinate* cell, unsigned short type, SDL_Renderer* renderer);

/**
 * Draws a world.
 *
 * @param current_world Current state of the world.
 * @param newWorld Next state of the world.
 * @param renderer Renderer where the cell is drawn.
 * @param first_row First row to be drawn.
 * @param last_row Last row to be drawn.
 * @param world_width Width of the world (in number of cells).
 * @param world_height Height of the world (in number of cells).
 */
void
draw_world(unsigned short* current_world,
           unsigned short* new_world,
           SDL_Renderer* renderer,
           int first_row,
           int last_row,
           int world_width,
           int world_height);

/**
 * Saves the current state of the screen in a bmp image.
 *
 * @param renderer Renderer that contais the world (in pixels).
 * @param file_name Name of the output bmp file.
 * @param screen_width Width of the screen (in pixels).
 * @param screen_height Height of the screen (in pixels).
 *
 */
void
save_image(SDL_Renderer* renderer, char* file_name, int screen_width, int screen_height);
