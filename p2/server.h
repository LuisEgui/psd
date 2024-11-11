#include "conecta4ns.nsmap"
#include "game.h"
#include "soapH.h"
#include <pthread.h>

/** Maximum number of active games in the server */
#define MAX_GAMES 5

/** Type for game status */
typedef enum
{
  gameEmpty,
  gameWaitingPlayer,
  gameReady
} tGameState;

/**
 * Struct that contains a game for 2 players
 */
typedef struct game
{
  xsd__string board;                  /** Board of the game */
  conecta4ns__tPlayer current_player; /** Current player */
  xsd__string player1_name;           /** Name of player 1 */
  xsd__string player2_name;           /** Name of player 2 */
  int endOfGame;                      /** Flag to control the end of the game*/
  tGameState status;                  /** Flag to indicate the status of this game */
  pthread_mutex_t mutex;              /** Mutex to protect the game status field */
  pthread_cond_t game_ready;          /** Condition variable to signal that the game is ready */
} tGame;

/**
 * Initialize server structures
 */
void
initServerStructures();

/**
 * Gets the other player
 *
 * @param current_player Current player
 * @return The player that is not current_player
 */
conecta4ns__tPlayer
switchPlayer(conecta4ns__tPlayer current_player);

/**
 * Checks if a game is empty
 * @param game_id ID of the game
 * @return TRUE if the game is empty, or FALSE in another case.
 */
int
is_empty(int game_id);

/**
 * Checks if a game is waiting for a player
 * @param game_id ID of the game
 * @return TRUE if the game is waiting for a player, or FALSE in another case.
 */
int
is_waiting_player(int game_id);

/**
 * Search for a game.
 *
 * This function returns a number between [0-MAX_GAMES] if there
 * exists an empty game, or ERROR_SERVER_FULL in other case.
 */
int
searchEmptyGame();

/**
 * Checks if a player is already registered in the game with ID=gameId
 *
 * @param playerName Name of the player
 * @param gameId ID of the game
 * @return TRUE if the player is registered in the game with ID=gameID, or FALSE in another case.
 */
int
checkPlayer(xsd__string playerName, int gameId);

/**
 * Initializes the game structure at the index position in the array
 * @param index Position of the game in the array
 */
void
freeGameByIndex(int index);

/**
 * Copies the data to be sent in a conecta4ns__tBlock structure.
 *
 * @param status Structure where the data is copied.
 * @param message Message to be sent.
 * @param board Board to be sent.
 * @param newCode Code to be sent.
 */
void
copyGameStatusStructure(conecta4ns__tBlock* status, char* message, xsd__string board, int newCode);

/**
 * Thread function to process client requests
 */
void*
processRequest(void*);
