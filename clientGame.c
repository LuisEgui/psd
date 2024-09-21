#include "clientGame.h"

void
sendMessageToServer(int socketServer, char* message)
{
}

void
receiveMessageFromServer(int socketServer, char* message)
{
}

void
receiveBoard(int socketServer, tBoard board)
{
}

unsigned int
receiveCode(int socketServer)
{
}

unsigned int
readMove()
{

  tString enteredMove;
  unsigned int move;
  unsigned int isRightMove;

  // Init...
  isRightMove = FALSE;
  move = STRING_LENGTH;

  while (!isRightMove) {

    printf("Enter a move [0-6]:");

    // Read move
    fgets(enteredMove, STRING_LENGTH - 1, stdin);

    // Remove new-line char
    enteredMove[strlen(enteredMove) - 1] = 0;

    // Length of entered move is not correct
    if (strlen(enteredMove) != 1) {
      printf("Entered move is not correct. It must be a number between [0-6]\n");
    }

    // Check if entered move is a number
    else if (isdigit(enteredMove[0])) {

      // Convert move to an int
      move = enteredMove[0] - '0';

      if (move > 6)
        printf("Entered move is not correct. It must be a number between [0-6]\n");
      else
        isRightMove = TRUE;
    }

    // Entered move is not a number
    else
      printf("Entered move is not correct. It must be a number between [0-6]\n");
  }

  return move;
}

void
sendMoveToServer(int socketServer, unsigned int move)
{
}

int
main(int argc, char* argv[])
{

  int socketfd;                      /** Socket descriptor */
  unsigned int port;                 /** Port number (server) */
  struct sockaddr_in server_address; /** Server address structure */
  char* serverIP;                    /** Server IP */

  tBoard board;           /** Board to be displayed */
  tString playerName;     /** Name of the player */
  tString rivalName;      /** Name of the rival */
  tString message;        /** Message received from server */
  unsigned int column;    /** Selected column */
  unsigned int code;      /** Code sent/receive to/from server */
  unsigned int endOfGame; /** Flag to control the end of the game */

  // Check arguments!
  if (argc != 3) {
    fprintf(stderr, "ERROR wrong number of arguments\n");
    fprintf(stderr, "Usage:\n$>%s serverIP port\n", argv[0]);
    exit(0);
  }

  // Get the server address
  serverIP = argv[1];

  // Get the port
  port = atoi(argv[2]);

  // Create socket

  // Check if the socket has been successfully created
  if (socketfd < 0)
    showError("ERROR while creating the socket");

  // Fill server address structure

  // Connect with server

  printf("Connection established with server!\n");

  // Init player's name
  do {
    memset(playerName, 0, STRING_LENGTH);
    printf("Enter player name:");
    fgets(playerName, STRING_LENGTH - 1, stdin);

    // Remove '\n'
    playerName[strlen(playerName) - 1] = 0;

  } while (strlen(playerName) <= 2);

  // Send player's name to the server

  // Receive rival's name

  printf("You are playing against %s\n", rivalName);

  // Init
  endOfGame = FALSE;

  // Game starts
  printf("Game starts!\n\n");

  // While game continues...

  // Close socket
}
