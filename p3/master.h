#include "graph.h"
#include "types.h"
#include "utils.h"
#include "world.h"
#include <SDL2/SDL.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Enables/Disables the log messages from the master process
#define DEBUG_MASTER 0

// Probability that a cataclysm may occur [0-100] :(
#define PROB_CATACLYSM 100

// Number of iterations between two possible cataclysms
#define ITER_CATACLYSM 5

/**
 * Master process.
 *
 * @param world_width Width of the world.
 * @param world_height Height of the world.
 * @param iterations Number of iterations to be executed.
 * @param execution_mode Execution mode (0: step, 1: auto).
 * @param distribution_mode Distribution mode (0: static, 1: distribution_mode).
 * @param grain_size Number of rows to be assigned to each worker (only for dynamic mode).
 * @param window SDL window to display the world.
 * @param renderer SDL renderer to draw the world.
 */
void
master(int world_width,
       int world_height,
       int iterations,
       int execution_mode,
       int distribution_mode,
       int grain_size,
       SDL_Window* window,
       SDL_Renderer* renderer);
