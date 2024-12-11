#include "mpi.h"
#include "types.h"
#include "utils.h"
#include "world.h"
#include <string.h>

// Enables/Disables the log messages from worker processes
#define DEBUG_WORKER 0

void
worker(int world_height, int world_width);
