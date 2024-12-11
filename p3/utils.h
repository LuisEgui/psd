#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include <mpi.h>

extern unsigned short* world;
extern unsigned short* world_send_buffer;
extern unsigned short* world_recv_buffer;
extern unsigned short* gather_world_buffer;
extern int total_processes;
extern int* send_counts;
extern int recv_count;
extern int* displs;
extern int* recv_counts;
extern int* rdispls;

/**
 * Shows an error message if the initialization stage is not performed successfully.
 *
 * @param msg Error message.
 */
void
show_error(char* msg);

/**
 * Shows an error message if the input parameters are not correct.
 *
 * @param program_name Name of the executable program.
 * @param msg Error message.
 */
void
wrong_usage(int rank, char* program_name, char* msg);

#endif // UTILS_H