#include "master.h"
#include "mpi.h"
#include "worker.h"

/**
 * Shows an error message if the input parameters are not correct.
 *
 * @param program_name Name of the executable program.
 * @param msg Error message.
 */
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

/**
 * Shows an error message if the initialization stage is not performed successfully.
 *
 * @param msg Error message.
 */
void
show_error(char* msg)
{
  printf("ERROR: %s\n", msg);
  MPI_Abort(MPI_COMM_WORLD, -1);
}

int
main(int argc, char* argv[])
{
  // The window to display the cells
  SDL_Window* window = NULL;

  // The window renderer
  SDL_Renderer* renderer = NULL;

  // World size and execution modes
  int world_width, world_height, auto_mode, dist_mode_static;

  // Iterations, grain size, and processes' info
  int total_iterations, grain_size, rank, size;

  // Output file
  char* output_file = NULL;

  // Time
  double start_time, end_time;

  // Init MPI
  MPI_Init(&argc, &argv);

  // Get rank and size
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Check the number of processes
  if (size < 3) {
    printf("The number of processes must be > 2\n");
    MPI_Finalize();
    exit(0);
  }

  // Check the number of parameters
  if ((argc < 7) || (argc > 8)) {
    wrong_usage(rank, "Wrong number of parameters!\n", argv[0]);
    exit(0);
  }

  // Read world's width (common for master and workers)
  world_width = atoi(argv[1]);
  world_height = atoi(argv[2]);
  total_iterations = atoi(argv[3]);

  // Execution mode
  if (strcmp(argv[4], "step") == 0)
    auto_mode = 0;
  else if (strcmp(argv[4], "auto") == 0)
    auto_mode = 1;
  else
    wrong_usage(rank, "Wrong execution mode, please select [step|auto]\n", argv[0]);

  // Read input parameters
  output_file = argv[5];

  // Distribution mode
  if (strcmp(argv[6], "static") == 0 && argc == 7)
    dist_mode_static = 1;
  else if (strcmp(argv[6], "dynamic") == 0 && argc == 8) {
    dist_mode_static = 0;
    grain_size = atoi(argv[7]);
  } else
    wrong_usage(
      rank, "Wrong distribution mode, please select [static|dynamic grain_size]\n", argv[0]);

  // Randomize the generator
  srand(SEED);

  // Master process
  if (rank == MASTER) {

    // Init video mode
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      show_error("Error initializing SDL\n");
      exit(0);
    }

    // Create window
    window = SDL_CreateWindow("Práctica 3 de PSD",
                              0,
                              0,
                              world_width * CELL_SIZE,
                              world_height * CELL_SIZE,
                              SDL_WINDOW_SHOWN);

    // Check if the window has been successfully created
    if (window == NULL) {
      show_error("Window could not be created!\n");
      exit(0);
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Show parameters...
    printf("Executing with:\n");
    printf("\tWorld size:%d x %d\n", world_width, world_height);
    printf("\tScreen size:%d x %d\n", world_width * CELL_SIZE, world_height * CELL_SIZE);
    printf("\tNumber of iterations:%d\n", total_iterations);
    printf("\tExecution mode:%s\n", argv[4]);
    printf("\tOutputFile:[%s]\n", output_file);
    printf("\tDistribution mode: %s", argv[6]);

    if (dist_mode_static)
      printf("\n");
    else
      printf(" - [%d rows]\n", grain_size);

    // Set timer
    start_time = MPI_Wtime();

    // TODO: Invoke the master subprogram

    // Set timer
    end_time = MPI_Wtime();
    printf("Total execution time:%f seconds\n", end_time - start_time);

  }

  // Workers
  else
    // TODO: Invoke the worker subprogram

    return 0;
}
