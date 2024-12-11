#include "worker.h"

void
worker(int world_height, int world_width)
{
  int recv_count;

  world_recv_buffer = malloc(recv_count * sizeof(unsigned short));

  if (world_recv_buffer == NULL) {
    fprintf(stderr, "Memory allocation for world_recv_buffer failed.\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  unsigned short* tmp_world_buffer =
    malloc(recv_count + (2 * world_width) * sizeof(unsigned short));

  if (tmp_world_buffer == NULL) {
    fprintf(stderr, "Memory allocation for tmp_world_buffer failed.\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  // TODO: clear world_recv_buffer

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

  // Receive top row
  MPI_Recv(
    tmp_world_buffer, world_width, MPI_UNSIGNED_SHORT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Copy world_recv_buffer into tmp_world_buffer after the first row (world_width size)
  memcpy(tmp_world_buffer + world_width, world_recv_buffer, recv_count * sizeof(unsigned short));

  // Receive bottom row
  MPI_Recv(world_recv_buffer + recv_count,
           world_width,
           MPI_UNSIGNED_SHORT,
           0,
           0,
           MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);

  compute_next_world_state(world, tmp_world_buffer, world_width, world_height);

  // Copy after the first row and before the last column to world_recv_buffer
  memcpy(world_recv_buffer,
         tmp_world_buffer + world_width,
         (recv_count - 1) * world_width * sizeof(unsigned short));

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

  free(tmp_world_buffer);
}
