#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "minmax.h"

void *malloc_verify(size_t memsize) {
	void *retval = malloc(memsize);
	if (retval == 0) {
		printf("Could not allocate memory");
		getchar();
		exit(1);
	}
	return retval;
}

int isnum(char *str) {
	int i = 0;
	if (str[0] == '\0') return 0;
	while (str[i] != '\0') {
		if (str[i]<'0' || str[i]>'9') return 0; //if any character is not a number return 0
		i++;
	}
	return 1; //else 1
}

void parse_command(char *buf, struct gameinfo *gi) {
	char *tok, *param;
	int i_param;
	tok = strtok(buf, " "); //split input
	if (tok != 0) {
		if (strcmp(tok, "newgame") == 0) {
			param = strtok(0, " "); //get parameter if there is one
			//call appropriate functions
			if (param == 0) {
				newgame(gi, gi->default_size);
				findlegal(gi);
				showstate(gi);
			}
			else {
				if (isnum(param)) {
					i_param = atoi(param);
					if (i_param >= 4 && i_param <= 26 && i_param%2 == 0) {
						newgame(gi, i_param);
						findlegal(gi);
						showstate(gi);
					}
					else printf("Invalid size for command newgame (4 <= size <= 26,  size is an even number)\n");
				}
				else printf("Invalid parameter for command newgame (parameter is an integer)\n");
			}
		}
		else if (strcmp(tok, "play") == 0) {
			param = strtok(0, " ");
			if (param == 0) printf("Missing parameter for command play\n");
			else if (gi->gamestarted == 0) printf("No game in progress\n");
			else if (gi->gs->nowplaying != gi->playercolor) printf("It's not your turn to play\n");
			else if (play(gi, param) == 1) showstate(gi); //if it's a valid move
		}
		else if (strcmp(tok, "cont") == 0) cont(gi);
		else if (strcmp(tok, "undo") == 0) undo(gi);
		else if (strcmp(tok, "suggest") == 0) suggest(gi);
		else if (strcmp(tok, "showstate") == 0) showstate(gi);
		else if (strcmp(tok, "save") == 0) {
			param = strtok(0, " ");
			if (param == 0) printf("Missing parameter for command save\n");
			else if(gi->gamestarted == 0) printf("No game in progress\n");
			else save(gi, param);
		}
		else if (strcmp(tok, "load") == 0) {
			param = strtok(0, " ");
			if (param == 0) printf("Missing parameter for command load\n");
			else load(gi, param);
		}
		else if (strcmp(tok, "selectcolor") == 0) {
			param = strtok(0, " ");
			if (param == 0) printf("Missing parameter for command selectcolor\n");
			else {
				if (strcmp(param, "black") == 0) gi->playercolor = 'B';
				else if (strcmp(param, "white") == 0) gi->playercolor = 'W';
				else printf("Invalid parameter for command selectcolor (parameter is 'black' or 'white')\n");
			}
		}
		else if (strcmp(tok, "showlegal") == 0) {
			param = strtok(0, " ");
			if (param == 0) printf("Missing parameter for command showlegal\n");
			else {
				if (strcmp(param, "on") == 0) gi->showlegal = 1;
				else if (strcmp(param, "off") == 0) gi->showlegal = 0;
				else printf("Invalid parameter for command showlegal (parameter is 'on' or 'off')\n");
			}
		}
		else if (strcmp(tok, "level") == 0) {
			param = strtok(0, " ");
			if (param == 0) printf("Current difficulty level: %d\n", gi->difficulty);
			else {
				if (isnum(param)) {
					i_param = atoi(param);
					if (i_param >= 1) gi->difficulty = i_param;
					else printf("Invalid parameter for command level (level >= 1)\n");
				}
				else printf("Invalid parameter for command level (parameter is an integer)\n");
			}
		}
		else if (strcmp(tok, "help") == 0) {
			printf("Available commands:\n"
				"newgame [<size>] - Start a new game,  optional parameter size (default %d)\n"
				"play <move> - Place a marker at the specified position\n"
				"cont - Allow the computer to play\n"
				"undo - Undo your last move\n"
				"suggest - Ask the computer to suggest a move\n"
				"selectcolor <black|white> - Select your color\n"
				"showlegal <on|off> - Show all possible moves\n"
				"level [<difficulty>] - Select a difficulty (difficulty >= 1) - If no parameter,  show the current difficulty\n"
				"save <filename> - Save the current game in a file\n"
				"load <filename> - Load game from a file\n"
				"showstate - Show the current state of the game\n"
				"quit - Exit the game\n"
				"help - Show the command list\n\n"
				, gi->default_size);
		}
		else printf("Unknown command %s (type help for a list of commands)\n", tok);
	}
}

void newgame(struct gameinfo *gi, unsigned int r_size) {
	unsigned int i, l;
	struct gamestate *prevstate;
	while (gi->gs != 0) {//free previous states' memory
		prevstate = gi->gs->prev;
		for (i = 0; i<gi->r_size; i++) free(gi->gs->board[i]);
		free(gi->gs->board);
		free(gi->gs);
		gi->gs = prevstate;
	}
	gi->gamestarted = 1;
	gi->r_size = r_size;
	//initialize gamestate struct
	gi->gs = (struct gamestate*)malloc_verify(sizeof(struct gamestate));
	gi->gs->whitecount = 2;
	gi->gs->blackcount = 2;
	gi->gs->nowplaying = 'B';
	strcpy(gi->gs->lastmove, "-"); //no last move
	gi->gs->prev = 0;
	gi->gs->board = (char**)malloc_verify(r_size*sizeof(char*)); //allocate board
	for (i = 0; i<r_size; i++) {
		gi->gs->board[i] = (char*)malloc_verify(r_size*sizeof(char));
		for (l = 0; l<r_size; l++) gi->gs->board[i][l] = ' '; //initialize board
	}
	gi->gs->board[r_size/2-1][r_size/2-1] = 'W';
	gi->gs->board[r_size/2-1][r_size/2] = 'B';
	gi->gs->board[r_size/2][r_size/2-1] = 'B';
	gi->gs->board[r_size/2][r_size/2] = 'W';
	build_eval_board(gi); //build board with values for each position
}

int play(struct gameinfo *gi, char *cell) {
	unsigned int x, y, i, l;
	char color = gi->gs->nowplaying;
	struct gamestate *newstate;
	x = cell[0]-'a'; //get x from letter (eg 'c'->3)
	if (isnum(cell+1)) {
		y = atoi(cell+1)-1; //get y
		if (x >= 0 && x<gi->r_size && y >= 0 && y<gi->r_size && gi->gs->board[x][y] == '*') {//if a legal move
			newstate = (struct gamestate*)malloc_verify(sizeof(struct gamestate)); //make a new state (for undo)
			newstate->board = (char**)malloc_verify(gi->r_size*sizeof(char*));
			for (i = 0; i<gi->r_size; i++) {
				newstate->board[i] = (char*)malloc_verify(gi->r_size*sizeof(char));
				memcpy(newstate->board[i], gi->gs->board[i], gi->r_size*sizeof(char)); //copy board to new state
				for (l = 0; l<gi->r_size; l++)
					if (newstate->board[i][l] == '*') newstate->board[i][l] = ' '; //legal moves aren't legal anymore
			}
			newstate->board[x][y] = color; //add current move
			newstate->blackcount = gi->gs->blackcount;
			newstate->whitecount = gi->gs->whitecount;
			if (color == 'B') newstate->blackcount++; //copy counts and increase the appropriate count
			else newstate->whitecount++;
			strcpy(newstate->lastmove, cell); //save last move (for showstate)
			newstate->prev = gi->gs; //current state is now last state
			gi->gs = newstate; //updated state is now current state
			updatecolors(gi, x, y); //change opposing pieces between player's old pieces and new piece
			if (gi->gs->prev->nowplaying == 'B') gi->gs->nowplaying = 'W'; //change current player
			else gi->gs->nowplaying = 'B';
			if (findlegal(gi) == 0) {//if there are no legal moves
				//printf("Player %s (%s) has no available moves!\n\n", gi->gs->nowplaying == 'W'?"White":"Black", gi->gs->nowplaying == gi->playercolor?"human":"computer");
				if (gi->gs->nowplaying == 'B') gi->gs->nowplaying = 'W'; //change player
				else gi->gs->nowplaying = 'B';
				if (findlegal(gi) == 0)//if the other player doesn't have legal moves either
					gi->gamestarted = 2; //gamestarted = 2  =  neither player has moves
			}
			return 1;
		}
		else printf("Invalid move\n");
	}
	else printf("Invalid move\n");
	return 0;
}

void cont(struct gameinfo *gi) {
	char *move;
	if (gi->gamestarted != 1) printf("No game in progress\n");
	else if (gi->gs->nowplaying == gi->playercolor) printf("It's your turn to play\n");
	else {
		move = find_move(gi);
		play(gi, move);
		showstate(gi);
		free(move);
	}
}

void undo(struct gameinfo *gi) {
	unsigned int i;
	struct gamestate *prevstate;
	if (gi->gamestarted == 0) printf("No game in progress\n");
	else if (gi->gs->prev == 0) printf("No previous states found\n\n");
	else {
		do {
			prevstate = gi->gs->prev; //go to previous state
			for (i = 0; i<gi->r_size; i++) free(gi->gs->board[i]);
			free(gi->gs->board);
			free(gi->gs); //free current state memory
			gi->gs = prevstate;
		} while (gi->gs->prev != 0 && gi->gs->nowplaying != gi->playercolor); //while there is a previous state and it isn't the player's turn
		showstate(gi);
	}
}

void suggest(struct gameinfo *gi) {
	char *move;
	if (gi->gamestarted != 1) printf("No game in progress\n");
	else if (gi->playercolor != gi->gs->nowplaying) printf("It's not your turn to play\n");
	else {
		move = find_move(gi);
		printf("Suggested move: %s\n", move);
		free(move);
	}
}

void save(struct gameinfo *gi, char *filename) {
	unsigned int i, l;
	FILE *f = fopen(filename, "wb"); //open file
	if (f) {
		fprintf(f, "%d", gi->r_size); //write board size
		for (i = 0; i<gi->r_size; i++)
			for (l = 0; l<gi->r_size; l++)
				switch (gi->gs->board[l][i]) {//write piece color
					case 'B':
						fputc('b', f);
						break;
					case 'W':
						fputc('w', f);
						break;
					default:
						fputc('n', f);
			}
		fputc(gi->gs->nowplaying, f); //write current player in the end (so it's compatible with other saves)
		fclose(f);
		printf("Game saved successfully!\n");
	}
	else printf("Unable to open file %s\n", filename);
}

void load(struct gameinfo *gi, char *filename) {
	unsigned int i, l, r_size;
	FILE *f = fopen(filename, "rb"); //open file
	if (f) {
		fscanf(f, "%d", &r_size); //read board size
		newgame(gi, r_size); //start a new game
		gi->gs->blackcount = 0;
		gi->gs->whitecount = 0;
		for (i = 0; i<r_size; i++)
			for (l = 0; l<r_size; l++)
				switch (fgetc(f)) {//read and count pieces
					case 'b':
						gi->gs->blackcount++;
						gi->gs->board[l][i] = 'B';
						break;
					case 'w':
						gi->gs->whitecount++;
						gi->gs->board[l][i] = 'W';
						break;
					default:
						gi->gs->board[l][i] = ' ';
			}
		gi->gs->nowplaying = fgetc(f); //get current player
		fclose(f);
		findlegal(gi);
		showstate(gi);
	}
	else printf("Unable to open file %s\n", filename);
}

void showstate(struct gameinfo *gi) {
	char winner;
	unsigned int i, l;
	if (gi->gamestarted == 0) printf("No game in progress\n");
	else {//print board
		putchar('\t');
		for (i = 0; i<gi->r_size; i++) printf("  %c ", 'a'+i);
		putchar('\n');
		for (i = 0; i<gi->r_size; i++) {
			putchar('\t');
			for (l = 0; l<gi->r_size; l++) printf("+---");
			printf("+\n%d\t", i+1);
			for (l = 0; l<gi->r_size; l++)
				if (gi->showlegal == 0 && gi->gs->board[l][i] == '*')
					printf("|   ");
				else
					printf("| %c ", gi->gs->board[l][i]);
			printf("|\n");
		}
		putchar('\t');
		for (l = 0; l<gi->r_size; l++) printf("+---");
		printf("+\n\n");
		if (gi->gs->lastmove[0] != '-') printf("Move played: %s\n", gi->gs->lastmove); //if there is a last move
		printf("Black: %d - White: %d\n", 
			gi->gs->blackcount, gi->gs->whitecount);
		if (gi->gamestarted == 2) printf("Neither player has available moves!\n");
		else if (gi->gs->prev != 0 && gi->gs->prev->nowplaying == gi->gs->nowplaying)
			printf("%s player (%s) has no available moves!\n", gi->gs->nowplaying == 'W'?"Black":"White", gi->gs->nowplaying == gi->playercolor?"computer":"human");
		if (gi->gs->whitecount == 0 || gi->gs->blackcount == 0 || gi->gs->whitecount+gi->gs->blackcount == gi->r_size*gi->r_size || gi->gamestarted == 2) {
			//if there are no white/black pieces or the board is full or neither side can play
			printf("END OF GAME\n\n");
			if (gi->gs->whitecount == gi->gs->blackcount) printf("Draw!\n");
			else {
				winner = gi->gs->whitecount>gi->gs->blackcount?'W':'B';
				printf("%s player (%s) wins the game!\n", winner == 'W'?"White":"Black", gi->playercolor == winner?"human":"computer");
			}
			gi->gamestarted = 0; //game not started
			for (i = 0; i<gi->r_size; i++) free(gi->eval_board[i]); //free eval board memory
			free(gi->eval_board);
		}
		else {
			printf("%s player (%s) plays now\n\n", 
				gi->gs->nowplaying == 'W'?"White":"Black", 
				gi->gs->nowplaying == gi->playercolor?"human":"computer");
		}
	}
}

unsigned int findlegal(struct gameinfo *gi) {
	unsigned int i, l, legaln;
	char oppcolor;
	legaln = 0; //number of legal moves
	if (gi->gs->nowplaying == 'W') oppcolor = 'B'; //find opponent's color
	else oppcolor = 'W';
	for (i = 0; i<gi->r_size; i++)
		for (l = 0; l<gi->r_size; l++)
			if (gi->gs->board[i][l] == oppcolor) {//if there is an empty position next to one of the opponent's pieces, check if it's a legal move
				if (i>0) {//prevent array overflows
					if (gi->gs->board[i-1][l] == ' ' && islegal(gi, i-1, l, i, l, oppcolor) == 1) {gi->gs->board[i-1][l] = '*'; legaln++; }
					if (l>0 && gi->gs->board[i-1][l-1] == ' ' && islegal(gi, i-1, l-1, i, l, oppcolor) == 1) {gi->gs->board[i-1][l-1] = '*'; legaln++; }
					if (l<(gi->r_size-1) && gi->gs->board[i-1][l+1] == ' ' && islegal(gi, i-1, l+1, i, l, oppcolor) == 1) {gi->gs->board[i-1][l+1] = '*'; legaln++; }
				}
				if (i<(gi->r_size-1)) {
					if (gi->gs->board[i+1][l] == ' ' && islegal(gi, i+1, l, i, l, oppcolor) == 1) {gi->gs->board[i+1][l] = '*'; legaln++; }
					if (l>0 && gi->gs->board[i+1][l-1] == ' ' && islegal(gi, i+1, l-1, i, l, oppcolor) == 1) {gi->gs->board[i+1][l-1] = '*'; legaln++; }
					if (l<(gi->r_size-1) && gi->gs->board[i+1][l+1] == ' ' && islegal(gi, i+1, l+1, i, l, oppcolor) == 1) {gi->gs->board[i+1][l+1] = '*'; legaln++; }
				}
				if (l>0 && gi->gs->board[i][l-1] == ' ' && islegal(gi, i, l-1, i, l, oppcolor)) {gi->gs->board[i][l-1] = '*'; legaln++; }
				if (l<(gi->r_size-1) && gi->gs->board[i][l+1] == ' ' && islegal(gi, i, l+1, i, l, oppcolor)) {gi->gs->board[i][l+1] = '*'; legaln++; }
			}
	return legaln; //return number of legal moves
}

void updatecolors(struct gameinfo *gi, unsigned int x, unsigned int y) {//change the pieces after each move
	signed int i, l, dirx, diry;
	char oppcolor;
	if (gi->gs->board[x][y] == 'B') oppcolor = 'W'; //find opponent's color
	else oppcolor = 'B';
	for (dirx = -1; dirx <= 1; dirx++)
		for (diry = -1; diry <= 1; diry++) {
			if (dirx == 0 && diry == 0) continue; //check all directions besides 0, 0 (current position)
			if ((dirx == 1 && x >= gi->r_size-2) || (dirx == -1 && x<2) || (diry == 1 && y >= gi->r_size-2) || (diry == -1 && y<2))
				continue; //check if we're on the edges of the board
			if (gi->gs->board[x+dirx][y+diry] != oppcolor) continue; //if there isn't the opposing color in this direction,  continue
			i = x+dirx;
			l = y+diry;
			while (i >= 0 && i<(signed int)gi->r_size && l >= 0 && l<(signed int)gi->r_size && gi->gs->board[i][l] == oppcolor) {
				//go over all of the opposing pieces
				i += dirx;
				l += diry;
			}
			if (i >= 0 && i<(signed int)gi->r_size && l >= 0 && l<(signed int)gi->r_size && gi->gs->board[i][l] == gi->gs->board[x][y]){
				//if we're still in the board limits, we found the pieces that need to change colors
				for (i = x+dirx, l = y+diry; gi->gs->board[i][l] == oppcolor; i += dirx, l += diry) {
					gi->gs->board[i][l] = gi->gs->board[x][y]; //change colors
					if (gi->gs->board[x][y] == 'B') {gi->gs->blackcount++; gi->gs->whitecount--; }//change white and black count
					else {gi->gs->whitecount++; gi->gs->blackcount--; }
				}
			}
		}
}

unsigned int islegal(struct gameinfo *gi, unsigned int x, unsigned int y, unsigned int dirx, unsigned int diry, char oppcolor) {
	signed int i = (signed int)dirx, l = (signed int)diry;
	while (gi->gs->board[i][l] == oppcolor) {//while the current piece is of the opposing color
		i += dirx-x; //move to next piece
		l += diry-y;
		if (i<0 || l<0 || i >= (signed int)gi->r_size || l >= (signed int)gi->r_size) return 0; //if we went outside the board limits,  it's illegal
	}
	if (gi->gs->board[i][l] == gi->gs->nowplaying) return 1; //if we reached another piece with the current color, it's a legal move
	return 0;
}
