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

void inicialize_pieces(fields **fields, char *color, int row, int col) {
	int pieces_row = 4, fields_row = 10;
	int i, j;
	char *type_man = "man";
	for(i = 0; i < pieces_row; i++) {
		for(j = 0; j < fields_row; j++) {
			if (( (row+i) + (col+j) ) % 2 != 0) {
				piece *piece = calloc(1, sizeof(piece));
				piece -> color = color;
				piece -> type = type_man;
				(*fields) -> all_fields[row+i][col+j] -> piece = piece;
			}
		}
	}
}

void create_fields(game **gm) {
	int size = 10;		
	char *color_white = "white";
	char *color_black = "black";
	fields *fields = calloc(1, sizeof(fields));
	
	fields -> size = size;
	fields -> all_fields = calloc(1, (fields -> size) * sizeof(field)); 
	
	int i, j;
	for(i = 0; i < size; i++) {
		fields -> all_fields[i] = calloc(1, (fields -> size) * sizeof(field));
		for(j = 0; j < size; j++) {
			fields -> all_fields[i][j] = calloc(1, sizeof(field));
			fields -> all_fields[i][j] -> row = i;
			fields -> all_fields[i][j] -> col = j;
			
			if ((i+j) % 2 == 0) {
				fields -> all_fields[i][j] -> color = color_white;
			}
			else { 
				fields -> all_fields[i][j] -> color = color_black;
			}
		}
	}

	inicialize_pieces(&fields, color_black, 0, 0);
	inicialize_pieces(&fields, color_white, 6, 0);
	(*gm) -> fields = &fields;
}

void create_game(game **gm, char *name_1, char *name_2) {
	(*gm) = calloc(1, sizeof(game));
	//create fields
	(*gm) -> name_1 = name_1;
	(*gm) -> name_2 = name_2;	
	create_fields(gm);
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

//král všemi směry, ale pouze o krok - HOTOVO
//pokud mám krále a muže, můžu hýbat kým chci - NEŘEŠIT
//skákání je povinné - NEŘEŠIT
//pokud lze vzít protihráči figurku (za figurkou je volné místo), musí jí přeskočit a přejít na volné políčko - HOTOVO
//pokud hráč nemůže hrát, prohrál (je zablokovaný, nemá šutry)
void process_move(games **all_games, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, char *color, char *type) {

	//kontrola zda určitý hráč může hýbat alespoň s jednou figurkou
	
	if ((cp_row == dp_row) || (cp_col == dp_col)) {
		//spatny tah, lze se hybat pouze diagonalne
		return;
	}

	game *current_game = (*all_games) -> games[game_ID];
	fields **fields = current_game -> fields;

	if ((*fields) -> all_fields[cp_row][cp_col] -> piece == NULL) {
		//spatny tah, nepohybujeme s piece	
		return;
	}

	int can_kill = check_if_can_kill(*fields, cp_row, cp_col, color, type);		


	int first_position, second_position;

	if (strcmp(color, "white") == 0) {
		first_position = 1;
		second_position = 2;
		if ((cp_row < dp_row) && (strcmp(type, "man") == 0)) {
			//spatny tah, cerny man muze pouze dolu (current vzdy mensi nez destination - row)
		}
	}
	else {
		first_position = -1;
		second_position = -2;
		if ((cp_row > dp_row) && (strcmp(type, "man") == 0)) {
			//spatny tah, cerny man muze pouze dolu (current vzdy mensi nez destination - row)
		}
	}

	//udělat kontrolu, když kliknu někde na kraji boardu jestli nepřekročím pole fieldů při kontrole 
	if (can_kill == 1) {
		if (dp_row == (cp_row - second_position)) {
			if ((dp_col == (cp_col - second_position)) && ((*fields) -> all_fields[cp_row - first_position][cp_col - first_position] -> piece != NULL)) {
				if (strcmp((*fields) -> all_fields[cp_row - first_position][cp_col - first_position] -> piece -> color, color) == 1) { 
					//spravny tah, poslat message se změnou

				}					
				else {
					//spatny tah, nelze preskakovat vlastni piece
				}
			}
			else if ((dp_col == (cp_col + second_position)) && ((*fields) -> all_fields[cp_row - first_position][cp_col + first_position] -> piece != NULL)) {
				if (strcmp((*fields) -> all_fields[cp_row - first_position][cp_col + first_position] -> piece -> color, color) == 1) { 
					//spravny tah, poslat message se změnou

				}					
				else {
					//spatny tah, nelze preskakovat vlastni piece
				}

			}
			else {
				//spatny tah, preskakuju prazdne misto
			}
		}
		else {
			//spatny tah, man musi preskocit protihracovo piece
		}
	}
	else {
		if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col - first_position)) ) {
			if ((*fields) -> all_fields[cp_row - first_position][cp_col - first_position] -> piece == NULL) { 
				//spravny tah, poslat message se změnou

			}					
			else {
				//spatny tah, nelze jit na plne policko
			}
		}
		else if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col + first_position)) ) {
			if ((*fields) -> all_fields[cp_row - first_position][cp_col + first_position] -> piece == NULL) { 
				//spravny tah, poslat message se změnou

			}					
			else {
				//spatny tah, nelze preskakovat vlastni piece
			}
		}
		else {
			//spatny tah, man se musi posouvat pouze dopredu
		}
	}	

	if (strcmp(type, "king") == 0) {
		if (can_kill == 1) {
			if (dp_row == (cp_row + second_position)) {
				if ((dp_col == (cp_col + second_position)) && ((*fields) -> all_fields[cp_row + first_position][cp_col + first_position] -> piece != NULL)) {
					if (strcmp((*fields) -> all_fields[cp_row + first_position][cp_col + first_position] -> piece -> color, color) == 1) { 
						//spravny tah, poslat message se změnou
	
					}					
					else {
						//spatny tah, nelze preskakovat vlastni piece
					}
				}
				else if ((dp_col == (cp_col - second_position)) && ((*fields) -> all_fields[cp_row + first_position][cp_col - first_position] -> piece != NULL)) {
					if (strcmp((*fields) -> all_fields[cp_row + first_position][cp_col - first_position] -> piece -> color, color) == 1) { 
						//spravny tah, poslat message se změnou
	
					}					
					else {
						//spatny tah, nelze preskakovat vlastni piece
					}
	
				}
				else {
					//spatny tah, preskakuju prazdne misto
				}
			}
			else {
				//spatny tah, man musi preskocit protihracovo piece
			}
		}
		else {
			if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col + first_position)) ) {
				if ((*fields) -> all_fields[cp_row + first_position][cp_col + first_position] -> piece == NULL) { 
					//spravny tah, poslat message se změnou
	
				}					
				else {
					//spatny tah, nelze jit na plne policko
				}
			}
			else if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col - first_position)) ) {
				if ((*fields) -> all_fields[cp_row + first_position][cp_col - first_position] -> piece == NULL) { 
					//spravny tah, poslat message se změnou	

				}					
				else {
					//spatny tah, nelze preskakovat vlastni piece
				}
			}
			else {
				//spatny tah, man se musi posouvat pouze dopredu
			}	
		}		
	}
}


//return: 0 - can't kill piece
//	  1 - can kill piece
int check_if_can_kill(fields *fields, int cp_row, int cp_col, char *color, char *type) {

	int first_position, second_position;

	if (strcmp(color, "white") == 0) {
		first_position = 1;
		second_position = 2;
	}
	else {
		first_position = -1;
		second_position = -2;
	}
	
	if (fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece != NULL) {
		if (strcmp(fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece -> color, color) == 1) {
			if (fields -> all_fields[cp_row - second_position][cp_col - second_position] -> piece == NULL) {				
				return 1;
			}
		}
	}		
	else if (fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece != NULL) {
		if (strcmp(fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece -> color, color) == 1) {
			if (fields -> all_fields[cp_row - second_position][cp_col + second_position] -> piece == NULL) {				
				return 1;
			}
		}
	}
	
	if (strcmp(type, "king") == 0) {
		if (fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece != NULL) {
			if (strcmp(fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece -> color, color) == 1) {
				if (fields -> all_fields[cp_row + second_position][cp_col - second_position] -> piece == NULL) {
					return 1;
				}
			}
		}		
		else if (fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece != NULL) {
			if (strcmp(fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece -> color, color) == 1) {
				if (fields -> all_fields[cp_row + second_position][cp_col + second_position] -> piece == NULL) {
					return 1;
				}
			}
		}	
	}
	
	return 0;				
}
