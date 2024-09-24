#include "game.h"

/** Server connections */
#define SERVER_BACKLOG 10

/**
 * Send a message to the player. This includes the length of the message and the message itself
 * @param socket_client Socket descriptor
 * @param message Message to be sent
 */
void
sendMessageToPlayer(int socket_client, char* message);

/**
 * Receive a message from the player. This includes the length of the message and the message itself
 * @param socket_client Socket descriptor
 * @param message Message to be received
 */
void
receiveMessageFromPlayer(int socket_client, char* message);

/**
 * Send a code to the player.
 * @param socket_client Socket descriptor
 * @param code Code to be send
 */
void
sendCodeToClient(int socket_client, unsigned int code);

/**
 * Send a board to the player.
 * @param socket_client Socket descriptor
 * @param board Board of the game
 */
void
sendBoardToClient(int socket_client, tBoard board);

/**
 * Receive a move from the player.
 * @param socket_client Socket descriptor
 * @return Move performed by the player
 */
unsigned int
receiveMoveFromPlayer(int socket_client);

/**
 * Get the associated socket to player
 *
 * @param player Current player
 * @param player1socket Socket that connects with player 1
 * @param player1socket Socket that connects with player 2
 * @return Associated socket to player
 */
int
getSocketPlayer(tPlayer player, int player1socket, int player2socket);

/**
 * Switch player
 *
 * @param current_player Current player
 * @return Rival player
 */
tPlayer
switchPlayer(tPlayer current_player);

/**
 * Randomize the starting player
 * @return Starting player
 */
tPlayer
randomize_starting_player();

/**
 *
 */
void
handleTurn(tPlayer current_player, tBoard board, int socket_p1, int socket_p2);