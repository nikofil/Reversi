struct gamestate {
	char **board, lastmove[4];
	char nowplaying;
	unsigned int whitecount, blackcount;
	struct gamestate *prev;
};

struct gameinfo {
	unsigned int r_size, difficulty, showlegal, gamestarted, default_size;
	char playercolor;
	struct gamestate *gs;
	signed int **eval_board;
};

void *malloc_verify(size_t);
int isnum(char *);
void parse_command(char *, struct gameinfo *);
void newgame(struct gameinfo *, unsigned int);
int play(struct gameinfo *, char *);
void cont(struct gameinfo *);
void undo(struct gameinfo *);
void suggest(struct gameinfo *);
void save(struct gameinfo *, char *);
void load(struct gameinfo *, char *);
void showstate(struct gameinfo *);
unsigned int findlegal(struct gameinfo *);
void updatecolors(struct gameinfo *, unsigned int, unsigned int);
unsigned int islegal(struct gameinfo *, unsigned int, unsigned int, unsigned int, unsigned int, char);