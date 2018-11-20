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
	printf("socket ID %d removed from queue\n", socket_ID);
}

void create_games(games **all_games) {
	int max_games = 16;
	(*all_games) = calloc(1, sizeof(games));
	(*all_games) -> games_count = 0;
	(*all_games) -> games = calloc(1, max_games * sizeof(game));	
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
	(*gm) -> fields = fields; 	
}

void create_game(game **gm, char *name_1, char *name_2, char *now_playing) {
	(*gm) = calloc(1, sizeof(game));
	(*gm) -> name_1 = name_1;
	(*gm) -> name_2 = name_2;
	(*gm) -> now_playing = now_playing;
	create_fields(gm);
}

void add_game(games **all_games, char *name_1, char *name_2, char *now_playing) {
	(*all_games) -> games_count++;
	printf("Games count: %d\n", (*all_games) -> games_count);
	(*all_games) -> games = realloc((*all_games) -> games, (*all_games) -> games_count * sizeof(game));
	game *game = NULL;
	create_game(&game, name_1, name_2, now_playing);
	(*all_games) -> games[((*all_games) -> games_count) - 1] = game;
}

void remove_game(clients **clients, games **all_games, int game_ID) {
	int i;
	int count = (*all_games) -> games_count;
	int index;
	for (i = 0; i < count; i++) {
		if (i == game_ID) {
			(*all_games) -> games_count--;			
			if (i < (count - 1)) {
				free((*all_games) -> games[i]);
				(*all_games) -> games[i] = (*all_games) -> games[((*all_games) -> games_count)];								
			}
			(*all_games) -> games[((*all_games) -> games_count)] = NULL;			
			(*all_games) -> games = realloc((*all_games) -> games, (*all_games) -> games_count * sizeof(game));
			index = i;			
			return;
		}
	}
	char *message;
	sprintf(message, "update_game_ID;%d;\n", index);
	
	send_message(get_socket_ID_by_name(*clients, (*all_games) -> games[index] -> name_1), message);
	send_message(get_socket_ID_by_name(*clients, (*all_games) -> games[index] -> name_2), message);
}

//král všemi směry, ale pouze o krok - HOTOVO
//pokud mám krále a muže, můžu hýbat kým chci - NEŘEŠIT
//skákání je povinné - NEŘEŠIT
//pokud lze vzít protihráči figurku (za figurkou je volné místo), musí jí přeskočit a přejít na volné políčko - HOTOVO
//pokud hráč nemůže hrát, prohrál (je zablokovaný, nemá šutry)
void process_move(games **all_games, clients *clients, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, char *color, char *type) {

	int current_player_socket_ID = get_socket_ID_by_name(clients, (*all_games) -> games[game_ID] -> now_playing);
	char *current_player_color = get_color_by_name(clients, (*all_games) -> games[game_ID] -> now_playing);
	int second_player_socket_ID = -1;

	//kontrola zda určitý hráč může hýbat alespoň s jednou figurkou

	if (strcmp(color, "NA") == 0 || strcmp(type, "NA") == 0) {
		send_message(current_player_socket_ID, "wrong_move;Color or type is NA;\n");
		return;
	}
	
	if (strcmp(color, current_player_color) != 0) {
		send_message(current_player_socket_ID, "wrong_move;You can't move opponents piece");
		return;
	}

	if ((cp_row == dp_row) || (cp_col == dp_col)) {
		send_message(current_player_socket_ID, "wrong_move;You can move only diagonally;\n");
		return;
	}
	
	if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece == NULL) {
		send_message(current_player_socket_ID, "wrong_move;You did not select any your piece;\n");
		return;
	}

	
	int can_kill = check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type);		
	

	int first_position, second_position;

	if (strcmp((*all_games) -> games[game_ID] -> now_playing, (*all_games) -> games[game_ID] -> name_1) == 0) {
		second_player_socket_ID = get_socket_ID_by_name(clients, (*all_games) -> games[game_ID] -> name_2);
	}
	else {
		second_player_socket_ID = get_socket_ID_by_name(clients, (*all_games) -> games[game_ID] -> name_1);
	}
	
	
	if (strcmp(color, "white") == 0) {
		first_position = 1;
		second_position = 2;
		if ((cp_row < dp_row) && (strcmp(type, "man") == 0)) {
			send_message(current_player_socket_ID, "wrong_move;You can move your man only forward;\n");
		}
	}
	else {
		first_position = -1;
		second_position = -2;
		if ((cp_row > dp_row) && (strcmp(type, "man") == 0)) {
			send_message(current_player_socket_ID, "wrong_move;You can move your man only forward;\n");
			return;
		}
	}
	
	//pokud zabiju hráče a není zde další piece který lze přeskočit, končí tah - udělat strom možností do hloubky 1 (bfs)
	if (can_kill == 1) {
		if (dp_row == (cp_row - second_position)) {
			if ((dp_col == (cp_col - second_position)) && ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece != NULL)) {
				if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece -> color, color) == 1) { 
					send_message(current_player_socket_ID, "correct_move;\n"); //zpráva ve tvaru correct_move;2;0;1;1;2; - pouze posun o 1 políčko
												   //nebo zpráva     correct_move;3;0;1;1;2;2;3; - přeskok piece  					
					send_message(second_player_socket_ID, "correct_move;\n");
					if (check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type) == 1) {
						send_message(current_player_socket_ID, "end_move;\n");
						send_message(second_player_socket_ID, "play_next_player;\n");
					}
					return;

				}					
				else {
					send_message(current_player_socket_ID, "wrong_move;You can't destroy your piece;\n");
					return;
				}
			}
			else if ((dp_col == (cp_col + second_position)) && ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece != NULL)) {
				if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece -> color, color) == 1) { 
					send_message(current_player_socket_ID, "correct_move;\n");
					send_message(second_player_socket_ID, "correct_move;\n");
					if (check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type) == 1) {
						send_message(current_player_socket_ID, "end_move;\n");
						send_message(second_player_socket_ID, "play_next_player;\n");
					}					
					return;
				}					
				else {
					send_message(current_player_socket_ID, "wrong_move;You can't destroy your piece;\n");
					return;
				}

			}
			//elseif piece je king a porovnat zbylé dva možné kroky na opačnou stranu
			else {
				send_message(current_player_socket_ID, "wrong_move;You can't move by 2 fields when you do not hop over opponents piece;\n");
				return;
			}
		}
		else {
			send_message(current_player_socket_ID, "wrong_move;You can't move by 2 fields when you do not hop over opponents piece;\n");
			return;
		}
	}
	else {
		if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col - first_position)) ) {
			if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece == NULL) { 
				send_message(current_player_socket_ID, "correct_move;\n");
				//send_message(second_player_socket_ID, "correct_move;\n");
				//send message aby hrál druhý hráč
				return;

			}					
			else {
				send_message(current_player_socket_ID, "wrong_move;You can't move to fields, because other piece is here;\n");
				return;
			}
		}
		else if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col + first_position)) ) {
			if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece == NULL) { 
				send_message(current_player_socket_ID, "correct_move;\n");
				//send_message(second_player_socket_ID, "correct_move;\n");
				//send message aby hrál druhý hráč
				return;
			}					
			else {
				send_message(current_player_socket_ID, "wrong_move;You can't destroy your piece;\n");
				return;
			}
		}
		//elseif piece je king a porovnat zbylé dva možné kroky na opačnou stranu
		else {
			send_message(current_player_socket_ID, "wrong_move;You can move your man by only 1 field per move;\n");
			return;
		}
	}	

	if (strcmp(type, "king") == 0) {
		if (can_kill == 1) {
			if (dp_row == (cp_row + second_position)) {
				if ((dp_col == (cp_col + second_position)) && ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece != NULL)) {
					if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece -> color, color) == 1) { 
						send_message(current_player_socket_ID, "correct_move;\n");
						//send_message(second_player_socket_ID, "correct_move;\n");	
						return;
					}					
					else {
						send_message(current_player_socket_ID, "wrong_move;You can't destroy your piece;\n");
						return;
					}
				}
				else if ((dp_col == (cp_col - second_position)) && ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece != NULL)) {
					if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece -> color, color) == 1) { 
						send_message(current_player_socket_ID, "correct_move;\n");
						//send_message(second_player_socket_ID, "correct_move;\n");
						return;
					}					
					else {
						send_message(current_player_socket_ID, "wrong_move;You can't destroy your piece;\n");
						return;
					}
	
				}
				else {
					send_message(current_player_socket_ID, "wrong_move;You can't move by 2 fields when you do not hop over opponents piece;\n");
					return;
				}
			}
			else {
				send_message(current_player_socket_ID, "wrong_move;You can't move by 2 fields when you do not hop over opponents piece;\n");
				return;
			}
		}
		else {
			if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col + first_position)) ) {
				if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece == NULL) { 
					send_message(current_player_socket_ID, "correct_move;\n");
					//send_message(second_player_socket_ID, "correct_move;\n");
					//send message aby hrál druhý hráč
					return;
				}					
				else {
					send_message(current_player_socket_ID, "wrong_move;You can't move to fields, because other piece is here;\n");
					return;
				}
			}
			else if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col - first_position)) ) {
				if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece == NULL) { 
					send_message(current_player_socket_ID, "correct_move;\n");
					//send_message(second_player_socket_ID, "correct_move;\n");
					//send message aby hrál druhý hráč
					return;
				}					
				else {
					send_message(current_player_socket_ID, "wrong_move;You can't destroy your piece;\n");
					return;
				}
			}
			else {
				send_message(current_player_socket_ID, "wrong_move;You can move your man only forward;\n");
				return;
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
