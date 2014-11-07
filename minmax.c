#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "minmax.h"

char *find_move(struct gameinfo *gi) {
	struct gamestate *gs_tmp;
	unsigned int i, l, x, y;
	signed int eval_cur = -10000000, eval_tmp;
	char *move = (char*)malloc_verify(4);
	gs_tmp = gi->gs;
	gi->gs = (struct gamestate*)malloc_verify(sizeof(struct gamestate)); //allocate temporary gamestate
	memcpy(gi->gs, gs_tmp, sizeof(struct gamestate));
	gi->gs->board = (char**)malloc_verify(gi->r_size*sizeof(char*)); //allocate temporary board
	for (i = 0; i<gi->r_size; i++) {
		gi->gs->board[i] = (char*)malloc_verify(gi->r_size*sizeof(char)); //allocate temporary board
		memcpy(gi->gs->board[i], gs_tmp->board[i], gi->r_size*sizeof(char)); //copy contents over
	}
	for (i = 0; i<gi->r_size; i++)
		for (l = 0; l<gi->r_size; l++)
			if (gi->gs->board[i][l] == '*') {
				eval_tmp = eval_move(gi, i, l, eval_cur, 10000000, gi->difficulty-1, gi->gs->nowplaying); //evaluate all possible moves
				if (eval_tmp>eval_cur) {//find the best
					x = i;
					y = l;
					eval_cur = eval_tmp;
				}
			}
	move[0] = 'a'+x;
	sprintf(&move[1], "%d", y+1); //move in letter form (eg 2, 3->"b3")
	for (i = 0; i<gi->r_size; i++) free(gi->gs->board[i]); //free memory for temporary board
	free(gi->gs->board); //free memory for temporary board
	free(gi->gs); //free memory for temporary gamestate
	gi->gs = gs_tmp;
	return move;
}

signed int eval_move(struct gameinfo *gi, unsigned int x, unsigned int y, signed int alpha, signed int beta, unsigned int depth, char maxplayer) {
	char move[4];
	struct gamestate *prevstate;
	unsigned int i, l, loopbreak = 0, mobility = 0;
	signed int eval_cur, eval_tmp;
	move[0] = 'a'+x;
	sprintf(&move[1], "%d", y+1);
	play(gi, move);
	if (gi->gs->whitecount == 0 || gi->gs->blackcount == 0 ||
		gi->gs->whitecount+gi->gs->blackcount == gi->r_size*gi->r_size ||
		gi->gamestarted == 2) {//if a move has led to the end of the game
			gi->gamestarted = 1;
			if (gi->gs->blackcount == gi->gs->whitecount) eval_cur = 0; //draw
			else if ((gi->gs->blackcount>gi->gs->whitecount && maxplayer == 'B')||(gi->gs->whitecount>gi->gs->blackcount && maxplayer == 'W')) eval_cur = 1000000; //maxplayer win
			else eval_cur = -1000000; //maxplayer lose
			prevstate = gi->gs->prev; //go to previous state
			for (i = 0; i<gi->r_size; i++) free(gi->gs->board[i]);
			free(gi->gs->board);
			free(gi->gs); //free current state memory
			gi->gs = prevstate;
			return eval_cur;
	}
	if (depth == 0) {//if it's the final depth or a move has led to the end of the game
			gi->gamestarted = 1; //in case the game has ended reset the gamestarted variable
			//start final state evaluation
			eval_cur = eval_state(gi, maxplayer)-eval_state(gi, maxplayer == 'B'?'W':'B'); //max player's evaluation minus min player's
			for (i = 0; i<gi->r_size; i++)
				for (l = 0; l<gi->r_size; l++)
					if (gi->gs->board[i][l] == '*') mobility++; //number of available moves
			mobility *= MOBILITY_CE; //mobility coefficient
			if (gi->gs->nowplaying == maxplayer) eval_cur += mobility; //better to have more moves for the maximizing player
			else eval_cur -= mobility;
			//end final state evaluation
			prevstate = gi->gs->prev; //go to previous state
			for (i = 0; i<gi->r_size; i++) free(gi->gs->board[i]);
			free(gi->gs->board);
			free(gi->gs); //free current state memory
			gi->gs = prevstate;
			return eval_cur;
	}
	eval_cur = (maxplayer == gi->gs->nowplaying?alpha:beta); //if it's max player's turn we return alpha,  if min then beta
	for (i = 0; i<gi->r_size && loopbreak == 0; i++)
		for (l = 0; l<gi->r_size && loopbreak == 0; l++)
			if (gi->gs->board[i][l] == '*') {//alpha-beta pruning
				if (maxplayer == gi->gs->nowplaying) {
					eval_tmp = eval_move(gi, i, l, alpha, beta, depth-1, maxplayer); //eval possible moves at this state
					if (alpha<eval_tmp) {alpha = eval_tmp; eval_cur = alpha; }//select the one with maximum evaluation
					if (beta <= alpha) loopbreak = 1; //beta cut-off
				}
				else {
					eval_tmp = eval_move(gi, i, l, alpha, beta, depth-1, maxplayer); //eval possible moves at this state
					if (beta>eval_tmp) {beta = eval_tmp; eval_cur = beta; }//select the one with minimum evaluation (best for min player)
					if (beta <= alpha) loopbreak = 1; //alpha cut-off
				}
			}
	prevstate = gi->gs->prev; //go to previous state
	for (i = 0; i<gi->r_size; i++) free(gi->gs->board[i]);
	free(gi->gs->board);
	free(gi->gs); //free current state memory
	gi->gs = prevstate;
	return eval_cur;
}

signed int eval_state(struct gameinfo *gi, char color) {
	unsigned int i, l;
	signed int eval_cur = 0, frontierdiscs = 0, score_eval;
	for (i = 0; i<gi->r_size; i++)
		for (l = 0; l<gi->r_size; l++)
			if (gi->gs->board[i][l] == color) {
				if (isfrontier(gi, i, l)) frontierdiscs++;
				eval_cur += gi->eval_board[i][l]; //evaluation of current board state
			}
	frontierdiscs *= FRONTIER_CE; //frontier coefficient
	eval_cur -= frontierdiscs;
	score_eval = (gi->gs->blackcount+gi->gs->whitecount)-34; //moves played - 34
	if (color == 'B') score_eval *= gi->gs->blackcount; //better to have few pieces early game (when there are fewer moves played) and many at endgame
	else score_eval *= gi->gs->whitecount;
	score_eval *= SCORE_CE; //score coefficient
	eval_cur += score_eval;
	return eval_cur;
}

int isfrontier(struct gameinfo *gi, unsigned int x, unsigned int y) {//check if any neighbouring fields are empty
	if (y>0) {
		if (gi->gs->board[x][y-1] == ' ' || gi->gs->board[x][y-1] == '*') return 1;
		if (x>0 && (gi->gs->board[x-1][y-1] == ' ' || gi->gs->board[x-1][y-1] == '*')) return 1;
		if (x<gi->r_size-1 && (gi->gs->board[x+1][y-1] == ' ' || gi->gs->board[x+1][y-1] == '*')) return 1;
	}
	if (y<gi->r_size-1) {
		if (gi->gs->board[x][y+1] == ' ' || gi->gs->board[x][y+1] == '*') return 1;
		if (x>0 && (gi->gs->board[x-1][y+1] == ' ' || gi->gs->board[x-1][y+1] == '*')) return 1;
		if (x<gi->r_size-1 && (gi->gs->board[x+1][y+1] == ' ' || gi->gs->board[x+1][y+1] == '*')) return 1;
	}
	if (x>0 && (gi->gs->board[x-1][y] == ' ' || gi->gs->board[x-1][y] == '*')) return 1;
	if (x<gi->r_size-1 && (gi->gs->board[x+1][y] == ' ' || gi->gs->board[x+1][y] == '*')) return 1;
	return 0;
}

void build_eval_board(struct gameinfo *gi) {
	unsigned int i, l;
	gi->eval_board = (int**)malloc(gi->r_size*sizeof(int*)); //allocate board
	for (i = 0; i<gi->r_size; i++) gi->eval_board[i] = (int*)malloc(gi->r_size*sizeof(int));
	for (i = 3; i <= gi->r_size-4; i++)//assign values of the importance of each position
		for (l = 3; l <= gi->r_size-4; l++)
			gi->eval_board[i][l] = 80; //center
	for (i = 3; i <= gi->r_size-4; i++) {
		gi->eval_board[i][gi->r_size-3] = gi->eval_board[gi->r_size-3][i] = gi->eval_board[i][2] = gi->eval_board[2][i] = 150; //around the center
		gi->eval_board[i][gi->r_size-2] = gi->eval_board[gi->r_size-2][i] = gi->eval_board[i][1] = gi->eval_board[1][i] = -500; //next to edges
		gi->eval_board[i][gi->r_size-1] = gi->eval_board[gi->r_size-1][i] = gi->eval_board[i][0] = gi->eval_board[0][i] = 900; //edges
	}
	gi->eval_board[2][2] = gi->eval_board[2][gi->r_size-3] = gi->eval_board[gi->r_size-3][2] = gi->eval_board[gi->r_size-3][gi->r_size-3] = 200; //around the center
	gi->eval_board[1][1] = gi->eval_board[1][gi->r_size-2] = gi->eval_board[gi->r_size-2][1] = gi->eval_board[gi->r_size-2][gi->r_size-2] = -5000; //next to corners
	gi->eval_board[0][0] = gi->eval_board[0][gi->r_size-1] = gi->eval_board[gi->r_size-1][0] = gi->eval_board[gi->r_size-1][gi->r_size-1] = 50000; //corners
	gi->eval_board[0][1] = gi->eval_board[1][0] = gi->eval_board[gi->r_size-2][0] = gi->eval_board[gi->r_size-1][1] = 
		gi->eval_board[0][gi->r_size-2] = gi->eval_board[1][gi->r_size-1] = gi->eval_board[gi->r_size-2][gi->r_size-1] = gi->eval_board[gi->r_size-1][gi->r_size-2] = -3200; //next to corners
	gi->eval_board[0][2] = gi->eval_board[2][0] = gi->eval_board[gi->r_size-3][0] = gi->eval_board[gi->r_size-1][2] = 
		gi->eval_board[0][gi->r_size-3] = gi->eval_board[2][gi->r_size-1] = gi->eval_board[gi->r_size-3][gi->r_size-1] = gi->eval_board[gi->r_size-1][gi->r_size-3] = 1100; //edges close to corners
	gi->eval_board[1][2] = gi->eval_board[2][1] = gi->eval_board[gi->r_size-3][1] = gi->eval_board[gi->r_size-2][2] = 
		gi->eval_board[1][gi->r_size-3] = gi->eval_board[2][gi->r_size-2] = gi->eval_board[gi->r_size-3][gi->r_size-2] = gi->eval_board[gi->r_size-2][gi->r_size-3] = -400; //next to edges
	/*for (i = 0; i<gi->r_size; i++) {//print eval board (for debugging)
		for (l = 0; l<gi->r_size; l++)
			printf("%d\t", gi->eval_board[i][l]);
		putchar('\n');
	}*/
}
