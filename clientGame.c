#include "clientGame.h"

void
sendMessageToServer(int socketServer, char* message)
{
  unsigned int length = strlen(message);

  // Send length of the message
  check(send(socketServer, &length, length, 0), "ERROR while sending message length");

  // Send the message
  check(send(socketServer, message, length, 0), "ERROR while sending meesage to the server");
}

void
receiveMessageFromServer(int socketServer, char* message)
{
  unsigned int length;

  // Receive length of the message
  check(recv(socketServer, &length, sizeof(int), 0), "ERROR while receiving length of message");

  memset(message, 0, length);

  // Receive the message
  check(recv(socketServer, message, length, 0), "ERROR while receiving message");
}

void
receiveBoard(int socketServer, tBoard board)
{
  check(recv(socketServer, board, sizeof(tBoard), 0), "ERROR while receiving board");
}

unsigned int
receiveCode(int socketServer)
{
  unsigned int code = ERROR;

  check(send(socketServer, &code, sizeof(unsigned int), 0),
        "ERROR while sending code to the server");

  return code;
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
  char* server_ip;                   /** Server IP */
  tBoard board;                      /** Board to be displayed */
  tString player_name;               /** Name of the player */
  tString rival_name;                /** Name of the rival */
  tString message;                   /** Message received from server */
  unsigned int column;               /** Selected column */
  unsigned int code;                 /** Code sent/receive to/from server */
  unsigned int end_of_game;          /** Flag to control the end of the game */

  // Check arguments!
  if (argc != 3) {
    fprintf(stderr, "ERROR wrong number of arguments\n");
    fprintf(stderr, "Usage:\n$>%s server_ip port\n", argv[0]);
    exit(0);
  }

  // Get the server address
  server_ip = argv[1];

  // Get the port
  port = atoi(argv[2]);

  // Create socket
  check((socketfd = socket(AF_INET, SOCK_STREAM, 0), socketfd), "ERROR while creating the socket");

  // Fill server address structure
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(server_ip);
  server_address.sin_port = htons(port);

  printf("Connecting to the server: %s:%u\n", server_ip, port);

  // Connect with server
  check(connect(socketfd, (struct sockaddr*)&server_address, sizeof(server_address)),
        "ERROR while connecting to the server");

  // Init player's name
  do {
    memset(player_name, 0, STRING_LENGTH);
    printf("Enter player name:");
    fgets(player_name, STRING_LENGTH - 1, stdin);

    // Remove '\n'
    player_name[strlen(player_name) - 1] = 0;

  } while (strlen(player_name) <= 2);

  // Send player's name to server
  sendMessageToServer(socketfd, player_name);

  // Receive rival's name
  receiveMessageFromServer(socketfd, rival_name);

  printf("You are playing against %s\n", rival_name);

  // Init
  end_of_game = FALSE;

  // Game starts
  printf("Game starts!\n\n");

  // While game continues...

  // Close socket
}
