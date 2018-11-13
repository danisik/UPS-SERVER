#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
// kvuli iotctl
#include <sys/ioctl.h>
#include "header.h"

void create_wanna_play(wanna_play **wanna_plays) {
	(*wanna_plays) = calloc(1, sizeof(wanna_play));
	(*wanna_plays) -> size = 0;
	(*wanna_plays) -> socket_IDs = calloc(1, sizeof(int));
}

void add_wanna_play(wanna_play **wanna_plays, int socket_ID) {
	(*wanna_plays) -> size++;
	printf("socket_ID %d want play a game\n", socket_ID);
	(*wanna_plays) -> socket_IDs = realloc((*wanna_plays) -> socket_IDs, (*wanna_plays) -> size * sizeof(int));
	(*wanna_plays) -> socket_IDs[((*wanna_plays) -> size) - 1] = socket_ID;
	printf("%d client/s wanna play a game:\n", (*wanna_plays) -> size);
}

void remove_wanna_play(wanna_play **wanna_plays, int socket_ID) {
	int i;
	int socket;
	int count = (*wanna_plays) -> size;
	for(i = 0; i < count; i++) {
		socket = (*wanna_plays) -> socket_IDs[i];
		if (socket == socket_ID) {
			(*wanna_plays) -> size--;			
			if (i < (count - 1)) {
				(*wanna_plays) -> socket_IDs[i] = (*wanna_plays) -> socket_IDs[((*wanna_plays) -> size)];								
			}	
			(*wanna_plays) -> socket_IDs = realloc((*wanna_plays) -> socket_IDs, (*wanna_plays) -> size * sizeof(wanna_play));
			return;
		}
	}
	printf("socket ID %d removed from queue", socket_ID);
}

void create_games(games **all_games) {
	(*all_games) = calloc(1, sizeof(games));
	(*all_games) -> games_count = 0;
	(*all_games) -> games = calloc(1, sizeof(game));	
}

void create_game(game **gm, char *name_1, char *name_2) {
	(*gm) = calloc(1, sizeof(game));
	(*gm) -> size = 10;
	(*gm) -> name_1 = name_1;
	(*gm) -> name_2 = name_2;
}

void add_game(games **all_games, char *name_1, char *name_2) {
	(*all_games) -> games_count++;
	printf("Games count: %d\n", (*all_games) -> games_count);
	(*all_games) -> games = realloc((*all_games) -> games, (*all_games) -> games_count * sizeof(game));
	game *game = NULL;
	create_game(&game, name_1, name_2);
	(*all_games) -> games[((*all_games) -> games_count) - 1] = game;
}

void remove_game() {

}

void restart_game() {

}

//král všemi směry, ale pouze o krok
//pokud mám krále a muže, můžu hýbat kým chci
//skákání je povinné 
//pokud lze vzít protihráči figurku (za figurkou je volné místo), musí jí přeskočit a přejít na volné políčko
//pokud hráč nemůže hrát, prohrál (je zablokovaný, nemá šutry)
void process_move(games **all_games, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, char *color, char *type) {

	//kontrola zda určitý hráč může hýbat alespoň s jednou figurkou

	if ((cp_row == dp_row) || (cp_col == dp_col)) {
		//spatny tah, lze se hybat pouze diagonalne
	}
	else {
		game *current_game = (*all_games) -> games[game_ID];
		if (strcmp(color, "white") == 0) {
			if (strcmp(type, "man") == 0) {
				if (cp_row < dp_row) {
					//spatny tah, bily man muze pouze dolu (current vzdy vetsi nez destination - row)
				}
				else {
					
				}
			}
			else {
			
			}
		}
		else {
			if (strcmp(type, "man") == 0) {
				if (cp_row > dp_row) {
					//spatny tah, cerny man muze pouze dolu (current vzdy mensi nez destination - row)
				}
				else {
	
				}
			}
			else {
			
			}
		}
	}
}
