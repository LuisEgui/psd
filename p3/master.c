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
    if (distribution_mode == STATIC) {
      unsigned short n_rows = world_height / (total_processes - 1);
      unsigned short index = 0;

      // Distribute the workload between the workers
      for (int i = 1; i < total_processes; i++, index += n_rows) {
        if (i == (total_processes - 1))
          n_rows += world_height % (total_processes - 1);

        // Send the number of rows to each worker
        MPI_Send(&n_rows, 1, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);
        // Send index to each worker
        MPI_Send(&index, 1, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);
        // Send the top row to each worker
        MPI_Send(get_top_row(world, world_width, world_height, index),
                 world_width,
                 MPI_UNSIGNED_SHORT,
                 i,
                 0,
                 MPI_COMM_WORLD);
        // Send effective working area to each worker
        MPI_Send(world + (index * world_width),
                 n_rows * world_width,
                 MPI_UNSIGNED_SHORT,
                 i,
                 0,
                 MPI_COMM_WORLD);
        // Send the bottom row to each worker
        MPI_Send(get_bottom_row(world, world_width, world_height, index + n_rows),
                 world_width,
                 MPI_UNSIGNED_SHORT,
                 i,
                 0,
                 MPI_COMM_WORLD);
      }

      // printf("Sent the workload to the workers\n");

      MPI_Status status;
      int rank;

      // Receive the computed workload from each worker
      for (int i = 1; i < total_processes; i++) {
        // Receive the number of rows from each worker
        MPI_Recv(&n_rows, 1, MPI_UNSIGNED_SHORT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        rank = status.MPI_SOURCE;

        // Receive index from each worker
        MPI_Recv(&index, 1, MPI_UNSIGNED_SHORT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Receive the effective working area from each worker
        MPI_Recv(next_world_state + (index * world_width),
                 n_rows * world_width,
                 MPI_UNSIGNED_SHORT,
                 rank,
                 0,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        // Draw the computed workload
        draw_world(
          world, next_world_state, renderer, index, index + n_rows, world_width, world_height);
      }

      // printf("Received the computed workload from the workers\n");

    } else if (distribution_mode == DYNAMIC) {
      unsigned short n_rows = grain_size;
      unsigned short index = 0;
      unsigned short n_rows_send = 0;
      unsigned short n_rows_recv = 0;

      // Distribute the workload between the workers dynamically
      for (int i = 1; i < total_processes; i++, index += n_rows, n_rows_send += n_rows) {
        // Send the number of rows to each worker
        MPI_Send(&n_rows, 1, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);

        // Send index to each worker
        MPI_Send(&index, 1, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);

        // Send the top row to each worker
        MPI_Send(get_top_row(world, world_width, world_height, index),
                 world_width,
                 MPI_UNSIGNED_SHORT,
                 i,
                 0,
                 MPI_COMM_WORLD);
        // Send effective working area to each worker
        MPI_Send(world + (index * world_width),
                 n_rows * world_width,
                 MPI_UNSIGNED_SHORT,
                 i,
                 0,
                 MPI_COMM_WORLD);
        // Send the bottom row to each worker
        MPI_Send(get_bottom_row(world, world_width, world_height, index + n_rows),
                 world_width,
                 MPI_UNSIGNED_SHORT,
                 i,
                 0,
                 MPI_COMM_WORLD);
      }

      MPI_Status status;
      int rank;

      // Receive the computed workload from each worker until there are no more rows to process
      do {
        // Receive the number of rows from each worker
        MPI_Recv(&n_rows, 1, MPI_UNSIGNED_SHORT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        rank = status.MPI_SOURCE;

        // Receive index from each worker
        MPI_Recv(&index, 1, MPI_UNSIGNED_SHORT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Receive the effective working area from each worker
        MPI_Recv(next_world_state + (index * world_width),
                 n_rows * world_width,
                 MPI_UNSIGNED_SHORT,
                 rank,
                 0,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        // Draw the computed workload
        draw_world(
          world, next_world_state, renderer, index, index + n_rows, world_width, world_height);

        n_rows_recv += n_rows;

        if (n_rows_send < world_height) {

          // Send leftover rows to the worker if there are less rows than the grain size
          n_rows =
            ((n_rows_send + grain_size) > world_height) ? world_height - n_rows_send : grain_size;

          // Send the number of rows to each worker
          MPI_Send(&n_rows, 1, MPI_UNSIGNED_SHORT, rank, 0, MPI_COMM_WORLD);

          // Send index to each worker
          MPI_Send(&n_rows_send, 1, MPI_UNSIGNED_SHORT, rank, 0, MPI_COMM_WORLD);

          // Send the top row to each worker
          MPI_Send(get_top_row(world, world_width, world_height, n_rows_send),
                   world_width,
                   MPI_UNSIGNED_SHORT,
                   rank,
                   0,
                   MPI_COMM_WORLD);
          // Send effective working area to each worker
          MPI_Send(world + (n_rows_send * world_width),
                   n_rows * world_width,
                   MPI_UNSIGNED_SHORT,
                   rank,
                   0,
                   MPI_COMM_WORLD);
          // Send the bottom row to each worker
          MPI_Send(get_bottom_row(world, world_width, world_height, n_rows_send + n_rows),
                   world_width,
                   MPI_UNSIGNED_SHORT,
                   rank,
                   0,
                   MPI_COMM_WORLD);

          n_rows_send += n_rows;
        }
      } while (n_rows_recv < world_height);
    }

    // Draw next world
    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(window);

    if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
      quit = TRUE;

    swap(&world, &next_world_state);

    // Clear the next_world_state for the next iteration
    clear_world(next_world_state, world_width, world_height);
  }

  unsigned short n_rows = 0;

  // Send END_PROCESSING to all workers
  for (int i = 1; i < total_processes; i++)
    MPI_Send(&n_rows, 1, MPI_UNSIGNED_SHORT, i, 0, MPI_COMM_WORLD);

  free(world);
  free(next_world_state);
}
