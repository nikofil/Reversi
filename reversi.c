#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"

int main(int argc, char *argv[]) {
	unsigned int doexit = 0, i, tmp;
	char *buf;
	struct gameinfo gi;
	struct gamestate *prevstate;
	//initialize gameinfo struct
	gi.default_size = 8;
	gi.difficulty = 1;
	gi.showlegal = 0;
	gi.playercolor = 'B';
	gi.gs = 0;
	gi.gamestarted = 0;
	gi.eval_board = 0;
	buf = (char*)malloc_verify(100);
	//read arguments
	for (i = 1; i<(unsigned int)argc; i++) {
		if (strcmp(argv[i], "-n") == 0) {
			i++;
			if (i<(unsigned int)argc) {
				if (isnum(argv[i])) {
					tmp = atoi(argv[i]);
					if (tmp >= 4 && tmp <= 26 && tmp%2 == 0) gi.default_size = tmp;
					else {
						printf("Invalid size (4 <= size <= 26, size is an even number)\n");
						i--;
					}
				}
				else {
					printf("Invalid parameter for flag -n (parameter is an integer)\n");
					i--;
				}
			}
			else printf("Missing parameter for flag -n\n");
		}
		else if (strcmp(argv[i], "-d") == 0) {
			i++;
			if (i<(unsigned int)argc) {
				if (isnum(argv[i])) {
					tmp = atoi(argv[i]);
					if (tmp >= 1) gi.difficulty = tmp;
					else {
						printf("Invalid difficulty (difficulty >= 1)\n");
						i--;
					}
				}
				else {
					printf("Invalid parameter for flag -d (parameter is an integer)\n");
					i--;
				}
			}
			else printf("Missing parameter for flag -d\n");
		}
		else if (strcmp(argv[i], "-l") == 0) gi.showlegal = 1;
		else if (strcmp(argv[i], "-w") == 0) gi.playercolor = 'W';
	}
	//read commands
	while (doexit == 0) {
		gets(buf);
		if (strcmp(buf, "quit") == 0) doexit = 1; //exit when the "quit" command is given
		else parse_command(buf, &gi);
		fflush(stdout);
	}
	//free memor
	free(buf);
	while (gi.gs != 0) {
		prevstate = gi.gs->prev;
		for (i = 0; i<gi.r_size; i++) free(gi.gs->board[i]);
		free(gi.gs->board);
		free(gi.gs);
		gi.gs = prevstate;
	}
	if (gi.eval_board != 0) {
		for (i = 0; i<gi.r_size; i++) free(gi.eval_board[i]);
		free(gi.eval_board);
	}
	return 0;
}
