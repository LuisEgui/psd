#include "master.h"
#include "mpi.h"
#include "utils.h"
#include "worker.h"
#include <SDL2/SDL.h>

unsigned short* world;
unsigned short* world_send_buffer;
unsigned short* world_recv_buffer;
unsigned short* gather_world_buffer;
int total_processes;
int* send_counts;
int recv_count;
int* displs;
int* recv_counts;
int* rdispls;

int
main(int argc, char* argv[])
{
  // The window to display the cells
  SDL_Window* window = NULL;

  // The window renderer
  SDL_Renderer* renderer = NULL;

  // World size and execution modes
  int world_width, world_height, execution_mode, distribution_mode;

  // Iterations, grain size, and processes' info
  int iterations, grain_size, rank, size;

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
  iterations = atoi(argv[3]);

  // Execution mode
  if (strcmp(argv[4], "step") == 0)
    execution_mode = STEP;
  else if (strcmp(argv[4], "auto") == 0)
    execution_mode = AUTO;
  else
    wrong_usage(rank, "Wrong execution mode, please select [step|auto]\n", argv[0]);

  // Read input parameters
  output_file = argv[5];

  // Distribution mode
  if (strcmp(argv[6], "static") == 0 && argc == 7)
    distribution_mode = STATIC;
  else if (strcmp(argv[6], "dynamic") == 0 && argc == 8) {
    distribution_mode = DYNAMIC;
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
      fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
      MPI_Abort(MPI_COMM_WORLD, -1);
      exit(0);
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
      fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
      MPI_Abort(MPI_COMM_WORLD, -1);
      exit(0);
    }

    // Show parameters...
    printf("Executing with:\n");
    printf("\tWorld size:%d x %d\n", world_width, world_height);
    printf("\tScreen size:%d x %d\n", world_width * CELL_SIZE, world_height * CELL_SIZE);
    printf("\tNumber of iterations:%d\n", iterations);
    printf("\tExecution mode:%s\n", argv[4]);
    printf("\tOutputFile:[%s]\n", output_file);
    printf("\tDistribution mode: %s", argv[6]);

    if (distribution_mode == STATIC)
      printf("\n");
    else
      printf(" - [%d rows]\n", grain_size);

    // Set timer
    start_time = MPI_Wtime();

    // Invoke the master subprogram
    master(world_width,
           world_height,
           iterations,
           execution_mode,
           distribution_mode,
           grain_size,
           window,
           renderer);

    // Set timer
    end_time = MPI_Wtime();
    printf("Total execution time:%f seconds\n", end_time - start_time);

  }

  // Workers
  else
    // Invoke the worker subprogram
    worker(world_height, world_width);

  return 0;
}
