#include "serverGame.h"
#include <stdlib.h>
#include <string.h>

void
sendMessageToPlayer(int socket_client, char* message)
{
  unsigned int length = strlen(message) + 1;

  // Send length of the message
  check(send(socket_client, &length, sizeof(length), 0), "ERROR while sending message length");

  // Send the message
  check(send(socket_client, message, length, 0), "ERROR while sending message to the player");
}

void
receiveMessageFromPlayer(int socket_client, char* message)
{
  unsigned int length;

  // Receive length of the message
  check(recv(socket_client, &length, sizeof(length), 0), "ERROR while receiving length of message");

  memset(message, 0, length);

  // Receive the message
  check(recv(socket_client, message, length, 0), "ERROR while receiving message");
}

void
sendCodeToClient(int socket_client, unsigned int code)
{
  check(send(socket_client, &code, sizeof(code), 0), "ERROR while sending code to the player");
}

void
sendBoardToClient(int socket_client, tBoard board)
{
  check(send(socket_client, board, sizeof(tBoard), 0), "ERROR while sending board");
}

unsigned int
receiveMoveFromPlayer(int socket_client)
{
  unsigned int move;

  check(recv(socket_client, &move, sizeof(move), 0), "ERROR while receiving move");

  return move;
}

int
getSocketPlayer(tPlayer player, int player1socket, int player2socket)
{
  int socket;

  if (player == player1)
    socket = player1socket;
  else
    socket = player2socket;

  return socket;
}

tPlayer
switchPlayer(tPlayer current_player)
{
  tPlayer next_player;

  if (current_player == player1)
    next_player = player2;
  else
    next_player = player1;

  return next_player;
}

tPlayer
randomizeStartingPlayer()
{
  return rand() % 2 == 0 ? player1 : player2;
}

char
getPlayerChip(tPlayer current_player)
{
  return current_player == player1 ? PLAYER_1_CHIP : PLAYER_2_CHIP;
}

void
handleTurn(tPlayer current_player, tBoard board, int socket_p1, int socket_p2)
{
  tString message;

  printf("Sending TURN_MOVE to player %d\n", current_player);
  // Send TURN_MOVE to the current player
  sendCodeToClient(getSocketPlayer(current_player, socket_p1, socket_p2), TURN_MOVE);
  // Send board to the current player
  sendBoardToClient(getSocketPlayer(current_player, socket_p1, socket_p2), board);
  // Send player turn and chip to the current player
  sprintf(message, "Its your turn, you play with: %c", getPlayerChip(current_player));
  sendMessageToPlayer(getSocketPlayer(current_player, socket_p1, socket_p2), message);

  // Send TURN_WAIT to the rival player
  sendCodeToClient(getSocketPlayer(switchPlayer(current_player), socket_p1, socket_p2), TURN_WAIT);
  // Send board to the rival player
  sendBoardToClient(getSocketPlayer(switchPlayer(current_player), socket_p1, socket_p2), board);
  sprintf(message,
          "Your rival is thinking... please, wait! You play with: %c",
          getPlayerChip(switchPlayer(current_player)));
  sendMessageToPlayer(getSocketPlayer(switchPlayer(current_player), socket_p1, socket_p2), message);

  // Receive move from the current player
  unsigned int column =
    receiveMoveFromPlayer(getSocketPlayer(current_player, socket_p1, socket_p2));

  // Check if the movement is valid
  if (checkMove(board, column) == OK_move)
    insertChip(board, current_player, column);
  else {
    // Send a message to the player showing that the column is full
    tString error_message;
    sprintf(error_message, "Column %d is full, try another column.", column);
    sendMessageToPlayer(getSocketPlayer(current_player, socket_p1, socket_p2), error_message);
  }
}

int
main(int argc, char* argv[])
{
  int socketfd;                       /** Socket descriptor */
  struct sockaddr_in server_addr;     /** Server address structure */
  unsigned int port;                  /** Listening port */
  struct sockaddr_in player_one_addr; /** Client address structure for player 1 */
  struct sockaddr_in player_two_addr; /** Client address structure for player 2 */
  int socket_p1, socket_p2;           /** Socket descriptor for each player */
  long unsigned int client_length;    /** Length of client structure */

  tBoard board;           /** Board of the game */
  tPlayer current_player; /** Current player */
  tMove move_result;      /** Result of player's move */
  tString player1_name;   /** Name of player 1 */
  tString player2_name;   /** Name of player 2 */
  int end_of_game;        /** Flag to control the end of the game*/
  unsigned int column;    /** Selected column to insert the chip */

  // Check arguments
  if (argc != 2) {
    fprintf(stderr, "ERROR wrong number of arguments\n");
    fprintf(stderr, "Usage:\n$>%s port\n", argv[0]);
    exit(1);
  }

  // Get listening port
  port = atoi(argv[1]);

  // Seed
  srand(time(0));

  // Create server socket
  check(socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), "ERROR while creating socket");

  // Init server structure
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);

  // Bind
  check(bind(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)), "ERROR while binding");

  // Listen
  check(listen(socketfd, SERVER_BACKLOG), "ERROR while listening");

  // Accept player 1
  client_length = sizeof(player_one_addr);

  check(socket_p1 =
          accept(socketfd, (struct sockaddr*)&player_one_addr, (socklen_t*)&client_length),
        "ERROR while accepting player 1");
  printf("Connection accepted from %s:%d\n",
         inet_ntoa(player_one_addr.sin_addr),
         ntohs(player_one_addr.sin_port));

  // Accept player 2
  check(socket_p2 =
          accept(socketfd, (struct sockaddr*)&player_two_addr, (socklen_t*)&client_length),
        "ERROR while accepting player 2");
  printf("Connection accepted from %s:%d\n",
         inet_ntoa(player_two_addr.sin_addr),
         ntohs(player_two_addr.sin_port));

  // Receive player 1 name
  receiveMessageFromPlayer(socket_p1, player1_name);
  printf("Name of player 1 received: %s\n", player1_name);

  // Receive player 2 name
  receiveMessageFromPlayer(socket_p2, player2_name);
  printf("Name of player 2 received: %s\n", player2_name);

  // Send player 1 name to player 2
  sendMessageToPlayer(socket_p2, player1_name);

  // Send player 2 name to player 1
  sendMessageToPlayer(socket_p1, player2_name);

  // Set the current player
  current_player = randomizeStartingPlayer();

  // Game logic
  end_of_game = FALSE;

  initBoard(board);

  while (!end_of_game) {
    handleTurn(current_player, board, socket_p1, socket_p2);

    // Check if current player wins or if the board is actually full
    if (checkWinner(board, current_player)) {
      // Send GAMEOVER_WIN to the current player
      sendCodeToClient(getSocketPlayer(current_player, socket_p1, socket_p2), GAMEOVER_WIN);
      // Send board to the current player
      sendBoardToClient(getSocketPlayer(current_player, socket_p1, socket_p2), board);

      // Send GAMEOVER_LOSE to the rival player
      sendCodeToClient(getSocketPlayer(switchPlayer(current_player), socket_p1, socket_p2),
                       GAMEOVER_LOSE);
      sendBoardToClient(getSocketPlayer(switchPlayer(current_player), socket_p1, socket_p2), board);

      end_of_game = TRUE;

    } else if (isBoardFull(board)) {
      // Send GAMEOVER_TIE to both players
      sendCodeToClient(socket_p1, GAMEOVER_DRAW);
      sendCodeToClient(socket_p2, GAMEOVER_DRAW);

      // Send board to both players
      sendBoardToClient(socket_p1, board);
      sendBoardToClient(socket_p2, board);

      end_of_game = TRUE;
    } else {
      // Switch to the next player
      current_player = switchPlayer(current_player);
    }
  }

  close(socketfd);
  exit(EXIT_SUCCESS);
}
