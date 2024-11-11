#include "server.h"
#include "game.h"
#include "soapStub.h"
#include <assert.h>

/** Flag to enable debugging */
#define DEBUG_SERVER 1

/** Array with games */
tGame games[MAX_GAMES];

void
initServerStructures()
{
  if (DEBUG_SERVER)
    printf("[Server init] Initializing...\n");

  // Init seed
  srand(time(NULL));

  // Init each game
  for (int i = 0; i < MAX_GAMES; i++) {

    // Allocate and init board
    games[i].board = (xsd__string)malloc(BOARD_WIDTH * BOARD_HEIGHT);

    if (games[i].board == NULL) {
      perror("[Server init] Error allocating memory for board\n");
      exit(1);
    }

    initBoard(games[i].board);

    // Calculate the first player to play
    if ((rand() % 2) == 0)
      games[i].current_player = player1;
    else
      games[i].current_player = player2;

    // Allocate and init player names
    games[i].player1_name = (xsd__string)malloc(STRING_LENGTH);

    if (games[i].player1_name == NULL) {
      perror("[Server init] Error allocating memory for player1_name\n");
      exit(1);
    }

    games[i].player2_name = (xsd__string)malloc(STRING_LENGTH);

    if (games[i].player2_name == NULL) {
      perror("[Server init] Error allocating memory for player2_name\n");
      exit(1);
    }

    memset(games[i].player1_name, 0, STRING_LENGTH);
    memset(games[i].player2_name, 0, STRING_LENGTH);

    // Game status
    games[i].end_of_game = FALSE;
    games[i].status = EMPTY;

    // Init mutex and cond variable
    pthread_mutex_init(&games[i].mutex, NULL);
    pthread_cond_init(&games[i].game_ready, NULL);
  }
}

conecta4ns__tPlayer
switchPlayer(conecta4ns__tPlayer current_player)
{
  return (current_player == player1) ? player2 : player1;
}

int
is_empty(int game_id)
{
  return games[game_id].status == EMPTY;
}

int
is_waiting_player(int game_id)
{
  return games[game_id].status == WAITING_FOR_PLAYER;
}

int
searchEmptyGame()
{
  int game_index = ERROR_SERVER_FULL;

  for (int i = 0; i < MAX_GAMES; i++) {
    if (is_waiting_player(i) || is_empty(i)) {
      game_index = i;
      break;
    }
  }

  return game_index;
}

int
checkPlayer(xsd__string playerName, int gameId)
{
  if (gameId < 0 || gameId >= MAX_GAMES)
    return FALSE;

  if (strcmp(games[gameId].player1_name, playerName) == 0)
    return TRUE;

  if (strcmp(games[gameId].player2_name, playerName) == 0)
    return TRUE;

  return FALSE;
}

void
freeGameByIndex(int index)
{
  if (index < 0 || index >= MAX_GAMES)
    return;

  // Free the board
  free(games[index].board);

  // Free the player names
  free(games[index].player1_name);
  free(games[index].player2_name);

  // Allocate and init board
  games[index].board = (xsd__string)malloc(BOARD_WIDTH * BOARD_HEIGHT);

  if (games[index].board == NULL) {
    perror("[Game reset] Error allocating memory for board\n");
    exit(1);
  }

  initBoard(games[index].board);

  // Calculate the first player to play
  if ((rand() % 2) == 0)
    games[index].current_player = player1;
  else
    games[index].current_player = player2;

  // Allocate and init player names
  games[index].player1_name = (xsd__string)malloc(STRING_LENGTH);

  if (games[index].player1_name == NULL) {
    perror("[Server init] Error allocating memory for player1_name\n");
    exit(1);
  }

  games[index].player2_name = (xsd__string)malloc(STRING_LENGTH);

  if (games[index].player2_name == NULL) {
    perror("[Server init] Error allocating memory for player2_name\n");
    exit(1);
  }

  memset(games[index].player1_name, 0, STRING_LENGTH);
  memset(games[index].player2_name, 0, STRING_LENGTH);

  // Game status
  games[index].end_of_game = FALSE;
  games[index].status = EMPTY;
}

void
copyGameStatusStructure(conecta4ns__tBlock* status, char* message, xsd__string board, int newCode)
{
  // Set the new code
  status->code = newCode;

  // Copy the message
  memset((status->msgStruct).msg, 0, STRING_LENGTH);
  strncpy((status->msgStruct).msg, message, strlen(message) + 1);
  (status->msgStruct).__size = strlen((status->msgStruct).msg) + 1;
  (status->msgStruct).msg[(status->msgStruct).__size - 1] = '\0';

  // Copy the board, only if it is not NULL
  if (board == NULL) {
    status->board = NULL;
    status->__size = 0;
  } else {
    strncpy(status->board, board, BOARD_WIDTH * BOARD_HEIGHT);
    status->__size = BOARD_WIDTH * BOARD_HEIGHT;
  }
}

char
getPlayerChip(conecta4ns__tPlayer current_player)
{
  return current_player == player1 ? PLAYER_1_CHIP : PLAYER_2_CHIP;
}

int
conecta4ns__register(struct soap* soap, conecta4ns__tMessage playerName, int* code)
{
  while (1) {
    int game_index = -1;

    // Set \0 at the end of the string
    playerName.msg[playerName.__size] = 0;

    if (DEBUG_SERVER)
      printf("[Register] Registering new player -> [%s]\n", playerName.msg);

    game_index = searchEmptyGame();

    if (game_index == ERROR_SERVER_FULL) {
      if (DEBUG_SERVER)
        printf("[Register] Server is full\n");
      *code = ERROR_SERVER_FULL;
      return SOAP_OK;
    }

    // Check if the player name is already registered in the game
    if (checkPlayer(playerName.msg, game_index)) {
      if (DEBUG_SERVER)
        printf(
          "[Register] Player %s is already registered in game %d\n", playerName.msg, game_index);
      *code = ERROR_PLAYER_REPEATED;
      return SOAP_OK;
    }

    pthread_mutex_lock(&games[game_index].mutex);

    // Register the player
    if (is_empty(game_index)) {
      assert(strlen(games[game_index].player1_name) == 0);
      strncpy(games[game_index].player1_name, playerName.msg, playerName.__size);
      games[game_index].player1_name[playerName.__size] = '\0';
      games[game_index].status = WAITING_FOR_PLAYER;

      if (DEBUG_SERVER)
        printf("[Register] Player %s registered in game %d\n",
               games[game_index].player1_name,
               game_index);

      pthread_cond_wait(&games[game_index].game_ready, &games[game_index].mutex);
      pthread_mutex_unlock(&games[game_index].mutex);
      *code = game_index;
      return SOAP_OK;

    } else if (is_waiting_player(game_index)) {
      assert(strlen(games[game_index].player2_name) == 0);
      strncpy(games[game_index].player2_name, playerName.msg, playerName.__size);
      games[game_index].player2_name[playerName.__size] = '\0';

      if (DEBUG_SERVER)
        printf("[Register] Player %s registered in game %d\n",
               games[game_index].player2_name,
               game_index);

      games[game_index].status = READY;

      if (DEBUG_SERVER)
        printf("[Register] Game %d is ready to start\n", game_index);

      pthread_cond_signal(&games[game_index].game_ready);
      pthread_mutex_unlock(&games[game_index].mutex);
      *code = game_index;
      return SOAP_OK;
    } else
      pthread_mutex_unlock(&games[game_index].mutex);
  }
}

void
handleGameOver(int gameId, conecta4ns__tMessage playerName, char* message_to_player, int* code)
{
  // Check if there's a winner
  if (checkWinner(games[gameId].board, games[gameId].current_player)) {
    // Current player is the winner
    if (DEBUG_SERVER)
      printf("[GameOver] Player %s wins game %d\n", playerName.msg, gameId);
    strncpy(message_to_player, "Congratulations, you won!", STRING_LENGTH - 1);
    *code = GAMEOVER_WIN;
  } else if (isBoardFull(games[gameId].board)) {
    // No winner and the board is full, so it's a tie
    if (DEBUG_SERVER)
      printf("[GameOver] Game %d is a tie\n", gameId);
    strncpy(message_to_player, "Game over. It's a tie!", STRING_LENGTH - 1);
    *code = GAMEOVER_DRAW;
  } else {
    // This player lost
    if (DEBUG_SERVER)
      printf("[GameOver] Player %s loses game %d\n", playerName.msg, gameId);
    strncpy(message_to_player, "You lose! Better luck next time!", STRING_LENGTH - 1);
    *code = GAMEOVER_LOSE;
  }
  message_to_player[STRING_LENGTH - 1] = '\0'; // Ensure the message is null-terminated
}

void
handleTurn(int gameId, conecta4ns__tMessage playerName, char* message_to_player, int* code)
{
  conecta4ns__tPlayer current_player = games[gameId].current_player;
  char player_chip = getPlayerChip(current_player);

  if ((current_player == player1 &&
       strncmp(games[gameId].player1_name, playerName.msg, playerName.__size) == 0) ||
      (current_player == player2 &&
       strncmp(games[gameId].player2_name, playerName.msg, playerName.__size) == 0)) {
    snprintf(message_to_player, STRING_LENGTH, "It's your turn, you play with: %c", player_chip);
    *code = TURN_MOVE;
  } else {
    snprintf(message_to_player,
             STRING_LENGTH,
             "Your rival is thinking... please, wait! You play with: %c",
             player_chip);
    *code = TURN_WAIT;
  }

  message_to_player[STRING_LENGTH - 1] = '\0';
}

int
conecta4ns__getStatus(struct soap* soap,
                      conecta4ns__tMessage playerName,
                      int gameId,
                      conecta4ns__tBlock* status)
{
  char message_to_player[STRING_LENGTH];
  int code;

  // Set \0 at the end of the string and alloc memory for the status
  playerName.msg[playerName.__size] = 0;
  allocClearBlock(soap, status);

  if (DEBUG_SERVER)
    printf("[GetStatus] Receiving getStatus() request from -> %s [%d] in game %d\n",
           playerName.msg,
           playerName.__size,
           gameId);

  // Check if the game exists
  if (gameId < 0 || gameId >= MAX_GAMES) {
    if (DEBUG_SERVER)
      printf("[GetStatus] Game %d does not exist\n", gameId);
    copyGameStatusStructure(status, "Wrong ID. Game does not exist", NULL, ERROR_WRONG_GAMEID);
    return SOAP_OK;
  }

  // Check if the player is registered in the game
  if (checkPlayer(playerName.msg, gameId) == FALSE) {
    if (DEBUG_SERVER)
      printf("[GetStatus] Player %s is not registered in game %d\n", playerName.msg, gameId);
    copyGameStatusStructure(status, "Player not found", NULL, ERROR_PLAYER_NOT_FOUND);
    return SOAP_OK;
  }

  pthread_mutex_lock(&games[gameId].mutex);

  // If it's not the player's turn, wait
  while ((strcmp(games[gameId].player1_name, playerName.msg) == 0 &&
          games[gameId].current_player != player1) || // Player 1
         (strcmp(games[gameId].player2_name, playerName.msg) == 0 &&
          games[gameId].current_player != player2)) { // Player 2
    pthread_cond_wait(&games[gameId].turn, &games[gameId].mutex);
  }

  // Check if the game is over
  if (games[gameId].end_of_game) {
    if (DEBUG_SERVER)
      printf("[GetStatus] Game %d is over\n", gameId);
    handleGameOver(gameId, playerName, message_to_player, &code);
    copyGameStatusStructure(status, message_to_player, games[gameId].board, code);
    games[gameId].current_player = switchPlayer(games[gameId].current_player);
    pthread_cond_signal(&games[gameId].turn);

    games[gameId].game_over_acks++;

    if (games[gameId].game_over_acks == 2)
      freeGameByIndex(gameId);

  } else {
    if (DEBUG_SERVER)
      printf("[GetStatus] Sending status to player %s in game %d\n", playerName.msg, gameId);
    handleTurn(gameId, playerName, message_to_player, &code);
    copyGameStatusStructure(status, message_to_player, games[gameId].board, code);
    pthread_cond_signal(&games[gameId].turn);
  }

  pthread_mutex_unlock(&games[gameId].mutex);

  return SOAP_OK;
}

int
conecta4ns__insertChip(struct soap* soap,
                       conecta4ns__tMessage playerName,
                       int gameId,
                       int column,
                       conecta4ns__tBlock* status)
{
  char message_to_player[STRING_LENGTH];
  int code;

  // Set \0 at the end of the string and alloc memory for the status
  playerName.msg[playerName.__size] = 0;
  allocClearBlock(soap, status);

  // Check if the game exists
  if (gameId < 0 || gameId >= MAX_GAMES) {
    if (DEBUG_SERVER)
      printf("[InsertChip] Game %d does not exist\n", gameId);
    copyGameStatusStructure(status, "Wrong ID. Game does not exist", NULL, ERROR_WRONG_GAMEID);
    return SOAP_OK;
  }

  // Check if the player is registered in the game
  if (checkPlayer(playerName.msg, gameId) == FALSE) {
    if (DEBUG_SERVER)
      printf("[InsertChip] Player %s is not registered in game %d\n", playerName.msg, gameId);
    copyGameStatusStructure(status, "Player not found", NULL, ERROR_PLAYER_NOT_FOUND);
    return SOAP_OK;
  }

  pthread_mutex_lock(&games[gameId].mutex);

  // Check the player's move
  if (checkMove(games[gameId].board, column) == INVALID_MOVE_FULL_COLUMN) {
    if (DEBUG_SERVER)
      printf("[InsertChip] Invalid move from player %s in game %d\n", playerName.msg, gameId);
    copyGameStatusStructure(status, "Column is full, try another column", NULL, ERROR_INVALID_MOVE);
    pthread_mutex_unlock(&games[gameId].mutex);
    return SOAP_OK;
  }

  // Insert the chip
  insertChip(games[gameId].board, games[gameId].current_player, column);

  if (DEBUG_SERVER) {
    char player_chip = getPlayerChip(games[gameId].current_player);
    printf("[InsertChip] Player %s inserted chip %c in column %d in game %d\n",
           playerName.msg,
           player_chip,
           column,
           gameId);
  }

  // Check if the game is over
  if (checkWinner(games[gameId].board, games[gameId].current_player) ||
      isBoardFull(games[gameId].board)) {
    if (DEBUG_SERVER)
      printf("[InsertChip] Game %d is over\n", gameId);
    games[gameId].end_of_game = TRUE;
    pthread_cond_signal(&games[gameId].turn);
  } else {
    if (DEBUG_SERVER)
      printf("[InsertChip] Player %s's turn in game %d\n", playerName.msg, gameId);
    games[gameId].current_player = switchPlayer(games[gameId].current_player);
    pthread_cond_signal(&games[gameId].turn);
  }

  pthread_mutex_unlock(&games[gameId].mutex);
  return SOAP_OK;
}

void*
processRequest(void* soap)
{
  pthread_detach(pthread_self());

  soap_serve((struct soap*)soap);
  soap_destroy((struct soap*)soap);
  soap_end((struct soap*)soap);
  soap_done((struct soap*)soap);
  free(soap);

  return NULL;
}

int
main(int argc, char** argv)
{
  struct soap soap;
  struct soap* tsoap;
  pthread_t tid;
  int port;
  SOAP_SOCKET m, s;

  // Init soap environment
  soap_init(&soap);

  // Configure timeouts
  soap.send_timeout = 60;     // 60 seconds
  soap.recv_timeout = 60;     // 60 seconds
  soap.accept_timeout = 3600; // server stops after 1 hour of inactivity
  soap.max_keep_alive = 100;  // max keep-alive sequence

  initServerStructures();

  // Get listening port
  port = atoi(argv[1]);

  // Bind
  m = soap_bind(&soap, NULL, port, 100);

  if (!soap_valid_socket(m)) {
    exit(1);
  }

  printf("[Server init] Server is ON ...\n");

  while (TRUE) {

    // Accept a new connection
    s = soap_accept(&soap);

    // Socket is not valid :(
    if (!soap_valid_socket(s)) {

      if (soap.errnum) {
        soap_print_fault(&soap, stderr);
        exit(1);
      }

      fprintf(stderr, "Time out!\n");
      break;
    }

    // Copy the SOAP environment
    tsoap = soap_copy(&soap);

    if (!tsoap) {
      printf("SOAP copy error!\n");
      break;
    }

    // Create a new thread to process the request
    pthread_create(&tid, NULL, (void* (*)(void*))processRequest, (void*)tsoap);
  }

  // Detach SOAP environment
  soap_done(&soap);
  return 0;
}
