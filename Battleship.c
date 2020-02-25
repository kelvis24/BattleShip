/*----------------------------------------------------------------------------
-								Includes									 -
-----------------------------------------------------------------------------*/
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
/*----------------------------------------------------------------------------
-								Defines	    								 -
-----------------------------------------------------------------------------*/
//length and width of each board array
#define M 4

/*----------------------------------------------------------------------------
-								Structs	    								 -
-----------------------------------------------------------------------------*/
//Represents a position on a board array
typedef struct position_struct {
	char x;
	char y;
}position;

//Represents a ship
typedef struct ship_struct {
	char x;
	char y;
	char l;
	char d;
}ship;

/*----------------------------------------------------------------------------
-	                            Prototypes                                   -
-----------------------------------------------------------------------------*/
//See function for description

void drawDefaultBoard();

void placePlayerFleet(char board[M][M]);
void placeCompFleet(char board[M][M]);

void placeShip(char board[M][M], ship boat);

char testBoarder(ship boat);
char testConflict(char board[M][M], ship boat);

char playerAttack(char board[M][M]);
char compAttack(char board[M][M]);

void updateSetBoard(char board[M][M]);
void updatePlayerBoard(char board[M][M]);
void updateCompBoard(char board[M][M]);

char getInputDir(char row);
void getInputPos(char row, position* pos);

void toPlayerBoard(position pos, char* c);
void toCompBoard(position pos, char* c);

void toTerminal(char row, char* c);
void clearTerminal();

/*----------------------------------------------------------------------------
-	                                Notes                                    -
-----------------------------------------------------------------------------*/
/* BOARD LOGIC:
	0: No ship; not tried; empty
	1: Ship;    not tried; empty
	2: No ship; tried;     miss
	3: Ship;    tried;     hit

*/
/*----------------------------------------------------------------------------
-								Implementation                               -
-----------------------------------------------------------------------------*/

int main(){
	// seed random
	srand(time(NULL));
	char playerBoard[M][M];
	char compBoard[M][M];

	// initializing the 2D array
	for (char i = 0; i < M; i++){
		for (char j = 0; j < M; j++){
			playerBoard[i][j] = 0;
			compBoard[i][j] = 0;
		}
	}
	position pos;
	char playerScore = 0;
	char compScore = 0;
	char turn = 0;

	initscr();
	drawDefaultBoard();
	refresh();

	placePlayerFleet(playerBoard);
	placeCompFleet(compBoard);
	// control game based on the score
	while (playerScore < 6 && compScore < 6){
		switch (turn) {
			case 0:
				playerScore += playerAttack(compBoard);
				updateCompBoard(compBoard);
				refresh();
				break;
			case 1:
				compScore += compAttack(playerBoard);
				updatePlayerBoard(playerBoard);
				refresh();
				break;
		}
		turn = !turn;
	}

	endwin();
	// print if the player won or not based on the score
	if (playerScore >= 6){
		printf("You've won!");
	}
	else {
		printf("You've lost!");
	}

	return 0;
}

//Has player manually place their fleet
void placePlayerFleet(char board[M][M]) {
	// create variables for placing ships
	position pos;
	char name[3][15];
	ship fleet[3];
	strcpy(name[0], "Patrol Boat");
	strcpy(name[1], "Destroyer");
	strcpy(name[2], "Carrier");
	fleet[0].l = 1;
	fleet[1].l = 2;
	fleet[2].l = 3;
	char i;
	// iterate through to place alll the ships on the board
	for (i = 0; i < 3; i++) {
		toTerminal(1, "Please place your: ");
		toTerminal(2, name[i]);
		refresh();
		getInputPos(3, &pos);
		clearTerminal();
		fleet[i].x = pos.x;
		fleet[i].y = pos.y;
		// if the ship is bigger than 1 unit, we need to ask which direction it faces
		if (fleet[i].l > 1) {
			toTerminal(1, "In which direction sould that boat face?");
			toTerminal(2, "(\"right\" or \"down\")");
			refresh();
			fleet[i].d = getInputDir(3);
			clearTerminal();
		}
		else {
			fleet[i].d = 0;
		}
		// if we try to go out of bounds or if a ship already exists, then restart the placing process again
		if (testBoarder(fleet[i])) {
			toTerminal(4, "Ship is out of bounds");
			i--;
		}
		else if (testConflict(board, fleet[i])) {
			toTerminal(4, "A ship is already there");
			i--;
		}
		else {
			placeShip(board, fleet[i]);
		}
		updateSetBoard(board);
	}
}

//Automatically places the computer's fleet
void placeCompFleet(char board[M][M]) {
	position pos;
	ship fleet[3];
	fleet[0].l = 1;
	fleet[1].l = 2;
	fleet[2].l = 3;
	char i;
	// place the three ships randomly
	for (i = 0; i < 3; i++) {
		// use a random generator to have the computer place the ships
		fleet[i].x = rand() % M;
		fleet[i].y = rand() % M;
		fleet[i].d = rand() & 2;
		if (testBoarder(fleet[i])) {
			i--;
		}
		else if (testConflict(board, fleet[i])) {
			i--;
		}
		else {
			placeShip(board, fleet[i]);
		}
	}
}

//Places a ship on the board
void placeShip(char board[M][M], ship boat) {
	char i;
	// place ships on 2D array
	if (boat.d) {
		for (i = 0; i < boat.l; i++) {
			board[boat.x][boat.y + i] = 1;
		}
	}
	else {
		for (i = 0; i < boat.l; i++) {
			board[boat.x + i][boat.y] = 1;
		}
	}
}

//Returns true if a ship is out of bounds; else false
char testBoarder(ship boat) {
	char i;
	if (boat.d) {
		for (i = 1; i < boat.l; i++) {
			if (boat.y + i >= M) {
				return 1;
			}
		}
	}
	else {
		for (i = 1; i < boat.l; i++) {
			if (boat.x + i >= M) {
				return 1;
			}
		}
	}
	return 0;
}

//Returns true if a ship's placement would confict with another ship's; else false
char testConflict(char board[M][M], ship boat) {
	char i;
	if (boat.d) {
		for (i = 0; i < boat.l; i++) {
			if (board[boat.x][boat.y + i]) {
				return 1;
			}
		}
	}
	else {
		for (i = 0; i < boat.l; i++) {
			if (board[boat.x + i][boat.y]) {
				return 1;
			}
		}
	}
	return 0;
}

//Prompts player to fire on a place on the computer's board
char playerAttack(char board[M][M]){
	toTerminal(1, "Fire at will!");
	position pos;
	char ok;
	do {
		ok = 1;
		refresh();
		getInputPos(2, &pos);
		if (board[pos.x][pos.y]/2) {
			toTerminal(1, "Place was already fired upon");
			ok = 0;
		}
	} while(!ok);
	board[pos.x][pos.y] += 2;
	if (board[pos.x][pos.y] == 3) {
		toTerminal(3, "Player: Hit!");
		return 1;
	}
	else {
		toTerminal(3, "Player: Miss!");
		return 0;
	}
}

//Has the computer fire on a rand place on the player's board
//Returns 1 if it's a hit, 0 if it's a miss
char compAttack(char board[M][M]){
	position pos;

	//While that position has not been attack, generate new rand numbers
	//Returns 1 if there is a hit; returns 0 if there is a miss
	do {
		pos.x = rand() % 4;
		pos.y = rand() % 4;
	} while(board[pos.x][pos.y]/2);
	board[pos.x][pos.y] += 2;
	//If there is not a ship
	if(board[pos.x][pos.y] == 3){
		//Indicate that the position has been attacked
		toTerminal(4, "Computer: Hit!");
		refresh();
		return 1;
	}
	//If there is a ship
	else {
		//Indicate that the position has been attacked
		toTerminal(4, "Computer: Miss!");
		refresh();
		return 0;
	}

}

//Updates the players board durring the boat setting phase
void updateSetBoard(char board[M][M]) {
	position pos;
	for (pos.y = 0; pos.y < M; pos.y++){
		for (pos.x = 0; pos.x < M; pos.x++) {
			if (board[pos.x][pos.y] == 0){
				toPlayerBoard(pos, " ");
			}
			else if (board[pos.x][pos.y] == 1){
				toPlayerBoard(pos, "O");
			}
		}
	}
}

//Updates the player's board
void updatePlayerBoard(char board[M][M]){
	position pos;
	//write the character that is determined by the number in our board array.
	for (pos.y = 0; pos.y < M; pos.y++){
		for (pos.x = 0; pos.x < M; pos.x++) {
			if (board[pos.x][pos.y] == 0) {
				toPlayerBoard(pos, " ");
			}
			else if (board[pos.x][pos.y] == 1) {
				toPlayerBoard(pos, "O");
			}
			else if (board[pos.x][pos.y] == 2){
				toPlayerBoard(pos, "M");
			}
			else if (board[pos.x][pos.y] == 3) {
				toPlayerBoard(pos, "X");
			}
		}
	}
}

//Updates the computer's board
void updateCompBoard(char board[M][M]){
	position pos;
	for (pos.y = 0; pos.y < M; pos.y++){
		for (pos.x = 0; pos.x < M; pos.x++) {
			// based on number on board array, print a character to the board
			if (board[pos.x][pos.y] == 2){
				toCompBoard(pos, "O");
			}
			else if (board[pos.x][pos.y] == 3) {
				toCompBoard(pos, "X");
			}
			else {
				toCompBoard(pos, " ");
			}
		}
	}
}

//Retrieves a character as input from user typeing on a given terminal row
//Only used to retrieve a direction "right" or "down"
char getInputDir(char row){
	char s[6];
	do {
		refresh();
		// offset row to account for ascii characters
		mvscanw(37 + row, 1,"%s", s);
		// set the null character at the end of the string
		s[5] = '\0';
		toTerminal(row, "");
		toTerminal(4, "Invalid Input");
	} while (strcmp(s, "right") && strcmp(s, "down"));
	toTerminal(4, "");
	refresh();
	// if we are left, return 0, if we are right return 1
	if (!strcmp(s, "right")) {
		return 0;
	}
	else {
		return 1;
	}
}

//Retrieves a position as input from user typeing on a given terminal row
//Only used to get position in form (A-D) (1-4)
void getInputPos(char row, position* pos){
	do {
		refresh();
		mvscanw(37 + row, 1,"%c %c", &pos->x, &pos->y);
		pos->x = tolower(pos->x) - 'a';
		pos->y -= '1';
		toTerminal(row, "");
		toTerminal(4, "Invalid Input");
	} while (pos->x < 0 || pos->x >= M || pos->y < 0 || pos->y >= M);
	toTerminal(4, "");
	refresh();
}

//Draws the board in the default position
void drawDefaultBoard(){
	mvprintw( 0,0,"|---------------------------------------------------|");
	mvprintw( 1,0,"|                                                   |");
	mvprintw( 2,0,"|                                                   |");
	mvprintw( 3,0,"|               |-------------------|               |");
	mvprintw( 4,0,"|               |    BATTLESHIP!    |               |");
	mvprintw( 5,0,"|               |-------------------|               |");
	mvprintw( 6,0,"|               |                   |               |");
	mvprintw( 7,0,"|               |  Computer's Board |               |");
	mvprintw( 8,0,"|               |                   |               |");
	mvprintw( 9,0,"|               |-------------------|               |");
	mvprintw(10,0,"|               |   | a | b | c | d |               |");
	mvprintw(11,0,"|               |-------------------|               |");
	mvprintw(12,0,"|               | 1 |   |   |   |   |               |");
	mvprintw(13,0,"|               |-------------------|               |");
	mvprintw(14,0,"|               | 2 |   |   |   |   |               |");
	mvprintw(15,0,"|               |-------------------|               |");
	mvprintw(16,0,"|               | 3 |   |   |   |   |               |");
	mvprintw(17,0,"|               |-------------------|               |");
	mvprintw(18,0,"|               | 4 |   |   |   |   |               |");
	mvprintw(19,0,"|               |-------------------|               |");
	mvprintw(20,0,"|               |                   |               |");
	mvprintw(21,0,"|               |  Player's Board   |               |");
	mvprintw(22,0,"|               |                   |               |");
	mvprintw(23,0,"|               |-------------------|               |");
	mvprintw(24,0,"|               |   | a | b | c | d |               |");
	mvprintw(25,0,"|               |-------------------|               |");
	mvprintw(26,0,"|               | 1 |   |   |   |   |               |");
	mvprintw(27,0,"|               |-------------------|               |");
	mvprintw(28,0,"|               | 2 |   |   |   |   |               |");
	mvprintw(29,0,"|               |-------------------|               |");
	mvprintw(30,0,"|               | 3 |   |   |   |   |               |");
	mvprintw(31,0,"|               |-------------------|               |");
	mvprintw(32,0,"|               | 4 |   |   |   |   |               |");
	mvprintw(33,0,"|               |-------------------|               |");
	mvprintw(34,0,"|                                                   |");
	mvprintw(35,0,"|                                                   |");
	mvprintw(36,0,"|                                                   |");
	mvprintw(37,0,"|---------------------------------------------------|");
	mvprintw(38,0,"|                                                   |");
	mvprintw(39,0,"|                                                   |");
	mvprintw(40,0,"|                                                   |");
	mvprintw(41,0,"|                                                   |");
	mvprintw(42,0,"|---------------------------------------------------|");
}

//Draws to the player's board a character at a certain position
void toPlayerBoard(position pos, char* c){
	mvprintw(2 * pos.y + 26, 4 * pos.x + 22, c);
}

//Draws to the computer's board a character at a certain position
void toCompBoard(position pos, char* c){
	mvprintw(2 * pos.y + 12, 4 * pos.x + 22, c);
}

//Draws text in a chosen row in the terminal
void toTerminal(char row, char* c){
	mvprintw(37 + row, 0, "|                                                   |");
	mvprintw(37 + row, 1, c);
}

//Clears all text in terminal
void clearTerminal(){
	mvprintw(38,0,"|                                                   |");
	mvprintw(39,0,"|                                                   |");
	mvprintw(40,0,"|                                                   |");
}