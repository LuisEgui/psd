#include "serverGame.h"

void sendMessageToPlayer (int socketClient, char* message){


}

void receiveMessageFromPlayer (int socketClient, char* message){


}

void sendCodeToClient (int socketClient, unsigned int code){


}

void sendBoardToClient (int socketClient, tBoard board){


}

unsigned int receiveMoveFromPlayer (int socketClient){


}

int getSocketPlayer (tPlayer player, int player1socket, int player2socket){

	int socket;

		if (player == player1)
			socket = player1socket;
		else
			socket = player2socket;

	return socket;
}

tPlayer switchPlayer (tPlayer currentPlayer){

	tPlayer nextPlayer;

		if (currentPlayer == player1)
			nextPlayer = player2;
		else
			nextPlayer = player1;

	return nextPlayer;
}



int main(int argc, char *argv[]){

	int socketfd;						/** Socket descriptor */
	struct sockaddr_in serverAddress;	/** Server address structure */
	unsigned int port;					/** Listening port */
	struct sockaddr_in player1Address;	/** Client address structure for player 1 */
	struct sockaddr_in player2Address;	/** Client address structure for player 2 */
	int socketPlayer1, socketPlayer2;	/** Socket descriptor for each player */
	unsigned int clientLength;			/** Length of client structure */

	tBoard board;						/** Board of the game */
	tPlayer currentPlayer;				/** Current player */
	tMove moveResult;					/** Result of player's move */
	tString player1Name;				/** Name of player 1 */
	tString player2Name;				/** Name of player 2 */
	int endOfGame;						/** Flag to control the end of the game*/
	unsigned int column;				/** Selected column to insert the chip */
	tString message;					/** Message sent to the players */



		// Check arguments
		if (argc != 2) {
			fprintf(stderr,"ERROR wrong number of arguments\n");
			fprintf(stderr,"Usage:\n$>%s port\n", argv[0]);
			exit(1);
		}

		// Init seed
		srand(time(NULL));

		// Create the socket


		// Check
		if (socketfd < 0)
			showError("ERROR while opening socket");

		// Init server structure
		memset(&serverAddress, 0, sizeof(serverAddress));

		// Get listening port
		port = atoi(argv[1]);

		// Fill server structure


		// Bind


		// Listen

}
