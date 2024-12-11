#include "utils.h"

void
show_error(char* msg)
{
  printf("ERROR: %s\n", msg);
  MPI_Abort(MPI_COMM_WORLD, -1);
}

void
wrong_usage(int rank, char* program_name, char* msg)
{
  // Only the master process show this message
  if (rank == MASTER) {
    printf("%s\n", msg);
    printf("Usage: >%s world_width world_height iterations [step|auto] outputImage [static|dynamic "
           "grain_size] \n",
           program_name);
  }

  MPI_Finalize();
}
