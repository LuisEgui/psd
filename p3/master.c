#include "master.h"

void
master(int total_processes,
       int world_width,
       int world_height,
       int iterations,
       int execution_mode,
       int distribution_mode,
       int grain_size,
       SDL_Window* window,
       SDL_Renderer* renderer)
{
  int quit = FALSE;
  SDL_Event event;

  // Allocate memory for the world
  unsigned short* world = malloc(world_width * world_height * sizeof(unsigned short));

  if (world == NULL) {
    fprintf(stderr, "Memory allocation for world failed.\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  unsigned short* next_world_state = malloc(world_width * world_height * sizeof(unsigned short));

  if (next_world_state == NULL) {
    fprintf(stderr, "Memory allocation for next_world_state failed.\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  clear_world(world, world_width, world_height);
  clear_world(next_world_state, world_width, world_height);

  init_random_world(world, world_width, world_height);

  // Setup render with black color and draw randomly generated world
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);
  draw_world(next_world_state, world, renderer, 0, world_height, world_width, world_height);
  SDL_RenderPresent(renderer);
  SDL_UpdateWindowSurface(window);

  if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
    quit = TRUE;

  printf("World created (iteration 0)\n");
  printf("Press Enter to start the simulation...\n");
  getchar();

  unsigned short no_cataclysm_counter = 0;

  for (int iteration = 1; iteration <= iterations && quit == FALSE;
       iteration++, no_cataclysm_counter++) {

    if (execution_mode == STEP) {
      printf("Press Enter to continue...\n");
      getchar();
    }

    // Clear the world
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    // There could be cataclysm every ITER_CATACLYSM iterations with a probability of PROB_CATACLYSM
    if (no_cataclysm_counter > ITER_CATACLYSM && (rand() % 100) < PROB_CATACLYSM) {
      printf("Cataclysm!\n");
      cataclysm(world, world_width, world_height);
      no_cataclysm_counter = 0;
    }

    // Static case
    if (distribution_mode == STATIC)
      printf("Static case\n");
    else
      printf("Dynamic case\n");

    // Draw next world
    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(window);

    if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
      quit = TRUE;

    swap(&world, &next_world_state);

    // Clear the next_world_state for the next iteration
    clear_world(next_world_state, world_width, world_height);
  }

  int end_processing = END_PROCESSING;

  // Send END_PROCESSING to all workers
  for (int i = 1; i < total_processes; i++)
    MPI_Send(&end_processing, 1, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);

  free(world);
  free(next_world_state);
}
