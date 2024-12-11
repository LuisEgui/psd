#include "worker.h"

void
worker(int world_width)
{
  unsigned short n_rows, index, current_row = 0;
  int world_height;
  unsigned short* world = NULL;
  unsigned short* next_world_state = NULL;

  // Receive the number of rows
  MPI_Recv(&n_rows, 1, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Top + area + bottom
  world_height = n_rows + 2;

  world = malloc(world_width * world_height * sizeof(unsigned short));

  if (world == NULL) {
    fprintf(stderr, "Memory allocation for world failed.\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
    exit(EXIT_FAILURE);
  }

  next_world_state = malloc(world_width * world_height * sizeof(unsigned short));

  if (next_world_state == NULL) {
    fprintf(stderr, "Memory allocation for next_world_state failed.\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
    exit(EXIT_FAILURE);
  }

  clear_world(world, world_width, world_height);
  clear_world(next_world_state, world_width, world_height);

  do {
    // Receive the index
    MPI_Recv(&index, 1, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Receive the top row
    MPI_Recv(world, world_width, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    current_row++;

    // Receive the effective working area
    MPI_Recv(world + (current_row * world_width),
             n_rows * world_width,
             MPI_UNSIGNED_SHORT,
             MASTER,
             0,
             MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    current_row += n_rows;

    // Receive the bottom row
    MPI_Recv(world + (current_row + world_width),
             world_width,
             MPI_UNSIGNED_SHORT,
             MASTER,
             0,
             MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    // Reset current_row
    current_row = 0;

    // Compute the next world state
    compute_next_world_state(world, next_world_state, world_width, world_height);

    // Send the number of rows
    MPI_Send(&n_rows, 1, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD);

    // Send the index
    MPI_Send(&index, 1, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD);

    // Send the world (skip top row)
    MPI_Send(next_world_state + world_width,
             n_rows * world_width,
             MPI_UNSIGNED_SHORT,
             MASTER,
             0,
             MPI_COMM_WORLD);

    // Receive the number of rows
    MPI_Recv(&n_rows, 1, MPI_UNSIGNED_SHORT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  } while (n_rows > 0);

  // Free memory
  free(world);
  free(next_world_state);

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
}
