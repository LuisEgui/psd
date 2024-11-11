#include "client.h"
#include "soapH.h"

#define DEBUG_CLIENT 1

unsigned int
readMove()
{
  xsd__string enteredMove;
  unsigned int move;
  unsigned int isRightMove;

  // Init...
  enteredMove = (xsd__string)malloc(STRING_LENGTH);

  if (enteredMove == NULL) {
    perror("Error allocating memory for entered move\n");
    exit(1);
  }

  memset(enteredMove, 0, STRING_LENGTH);
  isRightMove = FALSE;
  move = STRING_LENGTH;

  while (!isRightMove) {

    printf("Enter a move [0-%d]:", BOARD_WIDTH - 1);

    // Read move
    fgets(enteredMove, STRING_LENGTH - 1, stdin);

    // Remove new-line char
    enteredMove[strlen(enteredMove) - 1] = 0;

    // Length of entered move is not correct
    if (strlen(enteredMove) != 1) {
      printf("Entered move is not correct. It must be a number in the interval [0-%d]\n",
             BOARD_WIDTH - 1);
    }

    // Check if entered move is a number
    else if (isdigit(enteredMove[0])) {

      // Convert move to an int
      move = enteredMove[0] - '0';

      if (move >= BOARD_WIDTH)
        printf("Entered move is not correct. It must be a number in the interval [0-%d]\n",
               BOARD_WIDTH - 1);
      else
        isRightMove = TRUE;
    }

    // Entered move is not a number
    else
      printf("Entered move is not correct. It must be a number in the interval [0-%d]\n",
             BOARD_WIDTH - 1);
  }

  return move;
}

int
main(int argc, char** argv)
{
  struct soap soap;                 /** Soap struct */
  char* server_url;                 /** Server URL */
  unsigned int end_of_game;         /** Flag to control the end of the game */
  conecta4ns__tMessage player_name; /** Player name */
  conecta4ns__tBlock game_status;   /** Game status */
  unsigned int player_move;         /** Player move */
  int res_code;                     /** Return code from server */
  int game_id;                      /** Game ID */

  // Init gSOAP environment
  soap_init(&soap);

  // Check arguments!
  if (argc != 2) {
    fprintf(stderr, "ERROR wrong number of arguments\n");
    fprintf(stderr, "Usage:\n$>%s http://localhost:port\n", argv[0]);
    exit(0);
  }

  // Obtain server address
  server_url = argv[1];

  // Allocate memory for player name and init
  player_name.msg = (xsd__string)malloc(STRING_LENGTH);

  if (player_name.msg == NULL) {
    perror("Error allocating memory for player name\n");
    exit(1);
  }

  memset(player_name.msg, 0, STRING_LENGTH);

  // Allocate memory for game status and init
  allocClearBlock(&soap, &game_status);

  // Init
  res_code = -1;
  end_of_game = FALSE;

  do {
    // Get player name
    printf("Enter your name: ");
    fgets(player_name.msg, STRING_LENGTH - 1, stdin);

    // Remove '\n'
    player_name.msg[strlen(player_name.msg) - 1] = 0;
    player_name.__size = strlen(player_name.msg);

    res_code = soap_call_conecta4ns__register(&soap, server_url, "", player_name, &game_status);

    if (game_status.code == ERROR_SERVER_FULL) {
      printf("Server is full. Try again later.\n");
      exit(1);
    } else if (game_status.code == ERROR_PLAYER_REPEATED) {
      printf("Player %s is already registered. Try with a different name.\n", player_name.msg);
    }
  } while (game_status.code == ERROR_PLAYER_REPEATED);

  game_id = game_status.code;

  printf("Connected to server. Game ID: %d\n", game_id);

  while (TRUE)
    ;

  // Clean the environment
  soap_destroy(&soap);
  soap_end(&soap);
  soap_done(&soap);
  free(player_name.msg);

  return 0;
}
