#include "gameTypes.h"

/** Error Constant */
#define ERROR -1

/**
 * Function that shows an error message
 *
 * @param msg Error message
 */
void
showError(const char* msg);

/**
 * Handles system calls and prints the error if needed.
 * @param exp System call to be checked.
 * @param msg Message to be printed if the system call fails.
 */
int
check(int exp, const char* msg);

/**
 * Prints a board in the screen
 *
 * @param board	Board of current game
 * @param message Message with info about the game
 */
void
printBoard(tBoard board, char* message);
