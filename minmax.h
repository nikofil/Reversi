#define MOBILITY_CE 220//mobility coefficient
#define FRONTIER_CE 180//frontier coefficient
#define SCORE_CE 12//score coefficient

char *find_move(struct gameinfo *);
signed int eval_move(struct gameinfo *, unsigned int, unsigned int, signed int, signed int, unsigned int, char);
signed int eval_state(struct gameinfo *, char);
int isfrontier(struct gameinfo *, unsigned int, unsigned int);
void build_eval_board(struct gameinfo *);