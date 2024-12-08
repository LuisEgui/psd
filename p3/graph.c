#include "graph.h"

void
draw_cell(tCoordinate* cell, unsigned short type, SDL_Renderer* renderer)
{
  SDL_Rect fill_rect = { cell->col * CELL_SIZE, cell->row * CELL_SIZE, CELL_SIZE, CELL_SIZE };

  // Set color for this cell
  switch (type) {

    case CELL_NEW:
      SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);
      break;

    case CELL_LIVE:
      SDL_SetRenderDrawColor(renderer, 0x00, 0x99, 0x00, 0x00);
      break;

    case CELL_DEAD:
      SDL_SetRenderDrawColor(renderer, 0xCC, 0x00, 0x00, 0x00);
      break;

    case CELL_EMPTY:
      SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
      break;

    case CELL_CATACLYSM:
      SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0xFF, 0x00);
      break;
  }

  // Render cell (point or rectangle)
  if (CELL_SIZE == 1)
    SDL_RenderDrawPoint(renderer, cell->col, cell->row);
  else
    SDL_RenderFillRect(renderer, &fill_rect);
}

void
draw_world(unsigned short* current_world,
           unsigned short* new_world,
           SDL_Renderer* renderer,
           int first_row,
           int last_row,
           int world_width,
           int world_height)
{
  tCoordinate cell;

  for (int col = 0; col < world_width; col++)
    for (int row = first_row; row <= last_row; row++) {

      cell.row = row;
      cell.col = col;

      // New cell
      if (get_cell_at_world(&cell, current_world, world_width) == CELL_EMPTY &&
          get_cell_at_world(&cell, new_world, world_width) == CELL_LIVE)
        draw_cell(&cell, CELL_NEW, renderer);

      // Current cell remains alive
      else if (get_cell_at_world(&cell, current_world, world_width) == CELL_LIVE &&
               get_cell_at_world(&cell, new_world, world_width) == CELL_LIVE)
        draw_cell(&cell, CELL_LIVE, renderer);

      // Current cell dies
      else if (get_cell_at_world(&cell, current_world, world_width) == CELL_LIVE &&
               get_cell_at_world(&cell, new_world, world_width) == CELL_EMPTY)
        draw_cell(&cell, CELL_DEAD, renderer);

      // Cataclysm cell
      else if (get_cell_at_world(&cell, current_world, world_width) == CELL_CATACLYSM &&
               get_cell_at_world(&cell, new_world, world_width) == CELL_EMPTY)
        draw_cell(&cell, CELL_CATACLYSM, renderer);
    }
}

void
save_image(SDL_Renderer* renderer, char* file_name, int screen_width, int screen_height)
{
  const Uint32 format = SDL_PIXELFORMAT_RGB24;

  SDL_Surface* surface =
    SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, screen_width, screen_height, 24, format);
  SDL_RenderReadPixels(renderer, NULL, format, surface->pixels, surface->pitch);
  SDL_LockSurface(surface);
  SDL_SaveBMP(surface, file_name);
  SDL_UnlockSurface(surface);
  SDL_FreeSurface(surface);
}
