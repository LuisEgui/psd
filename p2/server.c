#include "server.h"
#include "soapStub.h"

/** Flag to enable debugging */
#define DEBUG_SERVER 1

/** Array with games */
tGame games[MAX_GAMES];

/** Mutex to protect the game status field in the array of games */
pthread_mutex_t mutex_status_array;

void
initServerStructures()
{
  if (DEBUG_SERVER)
    printf("Initializing...\n");

  // Init seed
  srand(time(NULL));

  // Init each game
  for (int i = 0; i < MAX_GAMES; i++) {

    // Allocate and init board
    games[i].board = (xsd__string)malloc(BOARD_WIDTH * BOARD_HEIGHT);

    if (games[i].board == NULL) {
      perror("Error allocating memory for board\n");
      exit(1);
    }

    initBoard(games[i].board);

    // Calculate the first player to play
    if ((rand() % 2) == 0)
      games[i].currentPlayer = player1;
    else
      games[i].currentPlayer = player2;

    // Allocate and init player names
    games[i].player1Name = (xsd__string)malloc(STRING_LENGTH);

    if (games[i].player1Name == NULL) {
      perror("Error allocating memory for player1Name\n");
      exit(1);
    }

    games[i].player2Name = (xsd__string)malloc(STRING_LENGTH);

    if (games[i].player2Name == NULL) {
      perror("Error allocating memory for player2Name\n");
      exit(1);
    }

    memset(games[i].player1Name, 0, STRING_LENGTH);
    memset(games[i].player2Name, 0, STRING_LENGTH);

    // Game status
    games[i].endOfGame = FALSE;
    games[i].status = gameEmpty;

    // Init mutex and cond variable
    pthread_mutex_init(&games[i].mutex, NULL);
  }
}

conecta4ns__tPlayer
switchPlayer(conecta4ns__tPlayer currentPlayer)
{
  return (currentPlayer == player1) ? player2 : player1;
}

int
searchEmptyGame()
{
  int game_index = ERROR_SERVER_FULL;

  for (int i = 0; i < MAX_GAMES; i++) {
    if (games[i].status == gameEmpty || games[i].status == gameWaitingPlayer) {
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

  if (strcmp(games[gameId].player1Name, playerName) == 0)
    return TRUE;

  if (strcmp(games[gameId].player2Name, playerName) == 0)
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
  free(games[index].player1Name);
  free(games[index].player2Name);

  // Reset the game status
  games[index].status = gameEmpty;
}

void
copyGameStatusStructure(conecta4ns__tBlock* status, char* message, xsd__string board, int newCode)
{
  // Set the new code
  status->code = newCode;

  // Copy the message
  memset((status->msgStruct).msg, 0, STRING_LENGTH);
  strcpy((status->msgStruct).msg, message);
  (status->msgStruct).__size = strlen((status->msgStruct).msg);

  // Copy the board, only if it is not NULL
  if (board == NULL) {
    status->board = NULL;
    status->__size = 0;
  } else {
    strncpy(status->board, board, BOARD_WIDTH * BOARD_HEIGHT);
    status->__size = BOARD_WIDTH * BOARD_HEIGHT;
  }
}

int
conecta4ns__register(struct soap* soap,
                     conecta4ns__tMessage playerName,
                     conecta4ns__tBlock* game_status)
{
  int game_index = -1;

  // Set \0 at the end of the string
  playerName.msg[playerName.__size] = 0;

  if (DEBUG_SERVER)
    printf("[Register] Registering new player -> [%s]\n", playerName.msg);

  // Search for an empty game
  pthread_mutex_lock(&mutex_status_array);
  game_index = searchEmptyGame();

  if (game_index == ERROR_SERVER_FULL) {
    pthread_mutex_unlock(&mutex_status_array);
    if (DEBUG_SERVER)
      printf("Server is full\n");
    game_status->code = ERROR_SERVER_FULL;
    return SOAP_OK;
  }

  // Check if the player name is already registered in the game
  if (checkPlayer(playerName.msg, game_index)) {
    pthread_mutex_unlock(&mutex_status_array);
    if (DEBUG_SERVER)
      printf("Player %s is already registered in game %d\n", playerName.msg, game_index);
    game_status->code = ERROR_PLAYER_REPEATED;
    return SOAP_OK;
  }

  // Register the player
  if (strlen(games[game_index].player1Name) == 0) {
    strncpy(games[game_index].player1Name, playerName.msg, playerName.__size);
    games[game_index].player1Name[playerName.__size] = '\0';
    games[game_index].status = gameWaitingPlayer;

    if (DEBUG_SERVER)
      printf("Player %s registered in game %d\n", games[game_index].player1Name, game_index);

  } else if (strlen(games[game_index].player2Name) == 0) {
    strncpy(games[game_index].player2Name, playerName.msg, playerName.__size);
    games[game_index].player2Name[playerName.__size] = '\0';
    games[game_index].status = gameWaitingPlayer;

    if (DEBUG_SERVER)
      printf("Player %s registered in game %d\n", games[game_index].player2Name, game_index);
  }

  // Check if the game is ready to start
  if (strlen(games[game_index].player1Name) > 0 && strlen(games[game_index].player2Name) > 0) {
    games[game_index].status = gameReady;
    if (DEBUG_SERVER)
      printf("Game %d is ready to start\n", game_index);
  }

  pthread_mutex_unlock(&mutex_status_array);

  game_status->code = game_index;

  return SOAP_OK;
}

int
conecta4ns__getStatus(struct soap* soap,
                      conecta4ns__tMessage playerName,
                      int gameId,
                      conecta4ns__tBlock* status)
{
  char messageToPlayer[STRING_LENGTH];

  // Set \0 at the end of the string and alloc memory for the status
  playerName.msg[playerName.__size] = 0;
  allocClearBlock(soap, status);

  if (DEBUG_SERVER)
    printf("Receiving getStatus() request from -> %s [%d] in game %d\n",
           playerName.msg,
           playerName.__size,
           gameId);

  return SOAP_OK;
}

void*
processRequest(void* soap)
{
  pthread_detach(pthread_self());

  if (DEBUG_SERVER)
    printf("Processing a new request...");

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
  pthread_mutex_init(&mutex_status_array, NULL);

  // Get listening port
  port = atoi(argv[1]);

  // Bind
  m = soap_bind(&soap, NULL, port, 100);

  if (!soap_valid_socket(m)) {
    exit(1);
  }

  printf("Server is ON ...\n");

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
