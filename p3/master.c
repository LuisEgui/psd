#include "master.h"

void
master(int world_width,
       int world_height,
       int iterations,
       int execution_mode,
       int distribution_mode,
       int grain_size,
       SDL_Window* window,
       SDL_Renderer* renderer)
{
  MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

  // Allocate memory for the world
  world = malloc(world_width * world_height * sizeof(unsigned short));

  if (world == NULL) {
    fprintf(stderr, "Memory allocation for world failed.\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  clear_world(world, world_width, world_height);

  init_random_world(world, world_width, world_height);

  if (distribution_mode == STATIC) {
    int total_data = sizeof(world) / sizeof(world[0]);

    world_send_buffer = malloc(total_data * sizeof(unsigned short));

    if (world_send_buffer == NULL) {
      fprintf(stderr, "Memory allocation for world_send_buffer failed.\n");
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

    gather_world_buffer = malloc(total_data * sizeof(unsigned short));

    if (gather_world_buffer == NULL) {
      fprintf(stderr, "Memory allocation for gather_world_buffer failed.\n");
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

    send_counts = malloc(total_processes * sizeof(int));

    if (send_counts == NULL) {
      fprintf(stderr, "Memory allocation for send_counts failed.\n");
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

    displs = malloc(total_processes * sizeof(int));

    if (displs == NULL) {
      fprintf(stderr, "Memory allocation for displs failed.\n");
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

    for (int itertation = 0, offset = 0; iterations < iterations; iterations++) {

      for (int i = 0; i < total_processes; i++) {
        if (i == 0) { // Master receives no data
          send_counts[i] = 0;
          displs[i] = 0;
        } else {
          send_counts[i] = total_data / (total_processes - 1); // Equal division for workers

          if (i <= total_data % (total_processes - 1)) // Handle remainder
            send_counts[i] += 1;

          displs[i] = offset;
          offset += send_counts[i];
        }
      }

      // Scatter data sizes
      MPI_Scatter(
        send_counts, 1, MPI_UNSIGNED_SHORT, &recv_count, 1, MPI_UNSIGNED_SHORT, 0, MPI_COMM_WORLD);

      // Scatterv for variable-sized data
      MPI_Scatterv(world_send_buffer,
                   send_counts,
                   displs,
                   MPI_UNSIGNED_SHORT,
                   world_recv_buffer,
                   recv_count,
                   MPI_UNSIGNED_SHORT,
                   0,
                   MPI_COMM_WORLD);

      // Send boundary rows to each worker
      for (int i = 1; i < total_processes; i++) {
        // Send the top boundary row
        MPI_Send(get_top_row(world, world_width, world_height, displs[i]),
                 world_width,
                 MPI_UNSIGNED_SHORT,
                 i,
                 0,
                 MPI_COMM_WORLD);
        // Send the bottom boundary row
        MPI_Send(get_bottom_row(world, world_width, world_height, displs[i]),
                 world_width,
                 MPI_UNSIGNED_SHORT,
                 i,
                 0,
                 MPI_COMM_WORLD);
      }

      // Prepare for gathering results back to master
      recv_counts = malloc(total_processes * sizeof(int));

      if (recv_counts == NULL) {
        fprintf(stderr, "Memory allocation for recv_counts failed.\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
      }

      rdispls = malloc(total_processes * sizeof(int));

      if (rdispls == NULL) {
        fprintf(stderr, "Memory allocation for rdispls failed.\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
      }

      for (int i = 0; i < total_processes; i++) {
        if (i == 0) {
          recv_counts[i] = 0; // Maser doesn't receive data from itself
          rdispls[i] = 0;
        } else {
          recv_counts[i] = send_counts[i]; // Same as send_counts
          rdispls[i] = displs[i];
        }
      }

      // Gather processed data back to master
      MPI_Gatherv(world_recv_buffer,
                  recv_count,
                  MPI_UNSIGNED_SHORT,
                  gather_world_buffer,
                  recv_counts,
                  rdispls,
                  MPI_UNSIGNED_SHORT,
                  0,
                  MPI_COMM_WORLD);

      // Clear renderer
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
      SDL_RenderClear(renderer);

      // Draw the world
      draw_world(
        world, gather_world_buffer, renderer, 0, world_height - 1, world_width, world_height);

      // Update the surface
      SDL_RenderPresent(renderer);

      // Update world with the next world state: gather_world_buffer
      memcpy(world, gather_world_buffer, world_width * world_height * sizeof(unsigned short));

      if (execution_mode == STEP) {
        printf("Press Enter to continue...");
        getchar();
      }
    }

    free(world_send_buffer);
    free(world_recv_buffer);
    free(gather_world_buffer);
  }

  if (distribution_mode == DYNAMIC) {
    // TODO
  }

  free(world);
}
