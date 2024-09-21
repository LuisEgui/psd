#include "serverGame.h"
#include <stdlib.h>

void
sendMessageToPlayer(int socketClient, char* message)
{
}

void
receiveMessageFromPlayer(int socketClient, char* message)
{
}

void
sendCodeToClient(int socketClient, unsigned int code)
{
}

void
sendBoardToClient(int socketClient, tBoard board)
{
}

unsigned int
receiveMoveFromPlayer(int socketClient)
{
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
switchPlayer(tPlayer currentPlayer)
{

  tPlayer nextPlayer;

  if (currentPlayer == player1)
    nextPlayer = player2;
  else
    nextPlayer = player1;

  return nextPlayer;
}

int
check(int exp, const char* msg)
{
  if (exp == ERROR) {
    perror(msg);
    exit(1);
  }

  return exp;
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
  tString message;        /** Message sent to the players */

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

  close(socketfd);
  exit(EXIT_SUCCESS);
}
