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

void process_move(games **all_games, clients *clients, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, char *color, char *type) {

	//1//You can't move opponents piece;
	//2//You can move only diagonally;
	//3//You did not select any your piece;
	//4//You can move your man only forward;
	//5//You can't destroy your piece;
	//6//You can't move by 2 fields when you do not hop over opponents piece;
	//7//You can't move to fields, because other piece is here;
	//8//You can move your man by only 1 field per move;
	//9//If you can destroy opponents piece, do it;

	int current_player_socket_ID = get_socket_ID_by_name(clients, (*all_games) -> games[game_ID] -> now_playing);
	char *current_player_color = get_color_by_name(clients, (*all_games) -> games[game_ID] -> now_playing);
	int second_player_socket_ID = -1;
	char *second_player_name;

	if (strcmp(color, "NA") == 0 || strcmp(type, "NA") == 0) {
		send_message(current_player_socket_ID, "wrong_move;3;\n");
		return;
	}
	
	if (strcmp(color, current_player_color) != 0) {
		send_message(current_player_socket_ID, "wrong_move;1;\n");
		return;
	}

	if ((cp_row == dp_row) || (cp_col == dp_col)) {
		send_message(current_player_socket_ID, "wrong_move;2;\n");
		return;
	}
	
	/*
	if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece == NULL) {
		send_message(current_player_socket_ID, "wrong_move;3;\n");
		return;
	}
	*/
	
	//int can_kill = check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type);	
	int i, j, tmp_can_kill = 0, can_kill = 0;
	printf("checking can_kill\n");
	for (i = 0; i < (*all_games) -> games[game_ID] -> fields -> size; i++) {
		for (j = 0; j < (*all_games) -> games[game_ID] -> fields -> size; j++) {
			if ((*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece != NULL) {
				if (strcmp( (*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece -> color, color) == 0 ) {
					printf("can_kill %d %d \n", i, j);					
					tmp_can_kill = check_if_can_kill((*all_games) -> games[game_ID] -> fields, i, j, color, type);	
					if (tmp_can_kill == 1) {
						can_kill = tmp_can_kill;
						break;
					}
				}
			}
		}
	}
	printf("can kill: %d\n", can_kill);

	int first_position, second_position;

	if (strcmp((*all_games) -> games[game_ID] -> now_playing, (*all_games) -> games[game_ID] -> name_1) == 0) {
		second_player_socket_ID = get_socket_ID_by_name(clients, (*all_games) -> games[game_ID] -> name_2);
		second_player_name = (*all_games) -> games[game_ID] -> name_2;
	}
	else {
		second_player_socket_ID = get_socket_ID_by_name(clients, (*all_games) -> games[game_ID] -> name_1);
		second_player_name = (*all_games) -> games[game_ID] -> name_1;
	}
	
	
	if (strcmp(color, "white") == 0) {
		first_position = 1;
		second_position = 2;
		if ((cp_row < dp_row) && (strcmp(type, "man") == 0)) {
			send_message(current_player_socket_ID, "wrong_move;4;\n");
			return;
		}
	}
	else {
		first_position = -1;
		second_position = -2;
		if ((cp_row > dp_row) && (strcmp(type, "man") == 0)) {
			send_message(current_player_socket_ID, "wrong_move;4;\n");
			return;
		}
	}
	
	//pokud zabiju hráče a není zde další piece který lze přeskočit, končí tah - udělat strom možností do hloubky 1 (bfs)
	//zkontrolovat všechny piece, zda mohou někoho v okolí sejmout (pokud ano, zjisti jeho souřadnice)
	//udělat promote mana, pokuď dojde na konec pole
	if (can_kill == 1) {
		if (dp_row == (cp_row - second_position)) {
			if (dp_col == (cp_col - second_position)) {
				if((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece != NULL) {
					if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece -> color, color) != 0) { 
							
						char correct_message[100];
						sprintf(correct_message, "correct_move;3;%d;%d;%d;%d;%d;%d;\n", cp_row, cp_col, cp_row - first_position, cp_col - first_position, dp_row, dp_col);

						send_message(current_player_socket_ID, correct_message);  					
						send_message(second_player_socket_ID, correct_message);
						if (check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type) != 0) {
							send_message(current_player_socket_ID, "end_move;\n");
							send_message(second_player_socket_ID, "play_next_player;\n");
							(*all_games) -> games[game_ID] -> now_playing = second_player_name;
						}
					
						(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
						(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
						(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece = NULL;
						return;
					}					
					else {
						send_message(current_player_socket_ID, "wrong_move;5;\n");
						return;
					}
				}
				else {
					send_message(current_player_socket_ID, "wrong_move;6;\n");
					return;
				}
			}
			else if (dp_col == (cp_col + second_position)) {
				if((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece != NULL) {
					if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece -> color, color) != 0) { 
					
						char correct_message[100];
						sprintf(correct_message, "correct_move;3;%d;%d;%d;%d;%d;%d;\n", cp_row, cp_col, cp_row - first_position, cp_col + first_position, dp_row, dp_col);

						send_message(current_player_socket_ID, correct_message);  					
						send_message(second_player_socket_ID, correct_message);

						if (check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type) != 0) {
							send_message(current_player_socket_ID, "end_move;\n");
							send_message(second_player_socket_ID, "play_next_player;\n");
							(*all_games) -> games[game_ID] -> now_playing = second_player_name;
						}					
						(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
						(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
						(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece = NULL;
						return;
					}					
					else {
						send_message(current_player_socket_ID, "wrong_move;5;\n");
						return;
					}
				}
				else {
					send_message(current_player_socket_ID, "wrong_move;6;\n");
					return;
				}
			}
			else if (strcmp(type, "king") == 0) {
				if (dp_row == (cp_row + second_position)) {
					if (dp_col == (cp_col + second_position)) {
						if((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece != NULL) {
							if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece -> color, color) != 0) { 
							
								char correct_message[100];
								sprintf(correct_message, "correct_move;3;%d;%d;%d;%d;%d;%d;\n", cp_row, cp_col, cp_row + first_position, cp_col + first_position, dp_row, dp_col);

								send_message(current_player_socket_ID, correct_message);  					
								send_message(second_player_socket_ID, correct_message);

								if (check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type) != 0) {
									send_message(current_player_socket_ID, "end_move;\n");
									send_message(second_player_socket_ID, "play_next_player;\n");
									(*all_games) -> games[game_ID] -> now_playing = second_player_name;
								}
								(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
								(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
								(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece = NULL;
								return;
							}					
							else {
								send_message(current_player_socket_ID, "wrong_move;5;\n");
								return;
							}
						}
						else {
							send_message(current_player_socket_ID, "wrong_move;6;\n");
							return;
						}
					}
					else if (dp_col == (cp_col - second_position)) {	
						if((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece != NULL) {
							if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece -> color, color) != 0) { 
							
								char correct_message[100];
								sprintf(correct_message, "correct_move;3;%d;%d;%d;%d;%d;%d;\n", cp_row, cp_col, cp_row + first_position, cp_col - first_position, dp_row, dp_col);

								send_message(current_player_socket_ID, correct_message);  					
								send_message(second_player_socket_ID, correct_message);

								if (check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type) != 0) {
									send_message(current_player_socket_ID, "end_move;\n");
									send_message(second_player_socket_ID, "play_next_player;\n");
									(*all_games) -> games[game_ID] -> now_playing = second_player_name;
								}
								(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
								(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
								(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece = NULL;
								return;
							}					
							else {
								send_message(current_player_socket_ID, "wrong_move;5;\n");
								return;
							}
						}
						else {
							send_message(current_player_socket_ID, "wrong_move;6;\n");
							return;
						}
					}
					else {
						send_message(current_player_socket_ID, "wrong_move;9;\n");
						return;
					}
				}
				else {
					send_message(current_player_socket_ID, "wrong_move;9;\n");
					return;
				}
			}
			else {
				send_message(current_player_socket_ID, "wrong_move;9;\n");
				return;
			}
		}
		else {
			send_message(current_player_socket_ID, "wrong_move;9;\n");
			return;
		}
	}
	else {
		if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col - first_position)) ) {
			if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece == NULL) { 
				
				char correct_message[100];
				sprintf(correct_message, "correct_move;2;%d;%d;%d;%d;\n", cp_row, cp_col, dp_row, dp_col);

				send_message(current_player_socket_ID, correct_message);  		
				send_message(second_player_socket_ID, correct_message);

				send_message(current_player_socket_ID, "end_move;\n");
				send_message(second_player_socket_ID, "play_next_player;\n");
				(*all_games) -> games[game_ID] -> now_playing = second_player_name;

				(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
				(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
				return;
			}					
			else {
				send_message(current_player_socket_ID, "wrong_move;7;\n");
				return;
			}
		}
		else if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col + first_position)) ) {
			if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece == NULL) { 

				char correct_message[100];
				sprintf(correct_message, "correct_move;2;%d;%d;%d;%d;\n", cp_row, cp_col, dp_row, dp_col);

				send_message(current_player_socket_ID, correct_message);  		
				send_message(second_player_socket_ID, correct_message);

				send_message(current_player_socket_ID, "end_move;\n");
				send_message(second_player_socket_ID, "play_next_player;\n");
				(*all_games) -> games[game_ID] -> now_playing = second_player_name;

				(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
				(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
				return;
			}					
			else {
				send_message(current_player_socket_ID, "wrong_move;5;\n");
				return;
			}
		}
		else if (strcmp(type, "king") == 0) {
			if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col + first_position)) ) {
				if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece == NULL) { 

					char correct_message[100];
					sprintf(correct_message, "correct_move;2;%d;%d;%d;%d;\n", cp_row, cp_col, dp_row, dp_col);

					send_message(current_player_socket_ID, correct_message);  		
					send_message(second_player_socket_ID, correct_message);

					send_message(current_player_socket_ID, "end_move;\n");
					send_message(second_player_socket_ID, "play_next_player;\n");
					(*all_games) -> games[game_ID] -> now_playing = second_player_name;

					(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
					(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
					return;
				}					
				else {
					send_message(current_player_socket_ID, "wrong_move;7;\n");
					return;
				}
			}
			else if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col - first_position)) ) {
				if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece == NULL) { 

					char correct_message[100];
					sprintf(correct_message, "correct_move;2;%d;%d;%d;%d;\n", cp_row, cp_col, dp_row, dp_col);

					send_message(current_player_socket_ID, correct_message);  		
					send_message(second_player_socket_ID, correct_message);

					send_message(current_player_socket_ID, "end_move;\n");
					send_message(second_player_socket_ID, "play_next_player;\n");
					(*all_games) -> games[game_ID] -> now_playing = second_player_name;

					(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
					(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
					return;
				}					
				else {
					send_message(current_player_socket_ID, "wrong_move;5;\n");
					return;
				}
			}
			else {
				send_message(current_player_socket_ID, "wrong_move;4;\n");
				return;
			}
		}
		else {
			send_message(current_player_socket_ID, "wrong_move;8;\n");
			return;
		}
	}	
}


//return: 0 - can't kill piece
//	  1 - can kill piece
//ošetřit hraniční situace - další podmínka zda cp_row - first_position >= 0 pro všechny 4 podmínky
int check_if_can_kill(fields *fields, int cp_row, int cp_col, char *color, char *type) {

	int first_position, second_position;

	first_position = 1;
	second_position = 2;
	
	if (strcmp(color, "white") == 0) {
		if ((cp_row - first_position) >= 0 && (cp_col - first_position) >= 0) {
			if (fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece != NULL) {
				if (strcmp(fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece -> color, color) != 0) {
					if((cp_row - second_position) >= 0 && (cp_col - second_position) >= 0) {
						if (fields -> all_fields[cp_row - second_position][cp_col - second_position] -> piece == NULL) {	
							return 1;
						}
					}
				}
			}	
		}	
		if ((cp_row - first_position) >= 0 && (cp_col + first_position) <= 9) {
			if (fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece != NULL) {
				if (strcmp(fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece -> color, color) != 0) {
					if((cp_row - second_position) >= 0 && (cp_col + second_position) <= 9) {	
						if (fields -> all_fields[cp_row - second_position][cp_col + second_position] -> piece == NULL) {				
							return 1;
						}
					}
				}
			}
		}
		
		if (strcmp(type, "king") == 0) {
			if ((cp_row + first_position) <= 9 && (cp_col - first_position) >= 0) {
				if (fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece != NULL) {
					if (strcmp(fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece -> color, color) != 0) {
						if((cp_row + second_position) <= 9 && (cp_col - second_position) >= 0) {	
							if (fields -> all_fields[cp_row + second_position][cp_col - second_position] -> piece == NULL) {
								return 1;
							}
						}
					}
				}
			}
			if ((cp_row + first_position) <= 9 && (cp_col + first_position) <= 9) {		
				if (fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece != NULL) {
					if (strcmp(fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece -> color, color) != 0) {
						if((cp_row + second_position) <= 9 && (cp_col + second_position) <= 9) {	
							if (fields -> all_fields[cp_row + second_position][cp_col + second_position] -> piece == NULL) {
								return 1;
							}
						}
					}
				}	
			}
		}
	}
	else {
		if ((cp_row + first_position) <= 9 && (cp_col + first_position) <= 9) {
			if (fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece != NULL) {
				if (strcmp(fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece -> color, color) != 0) {
					if((cp_row + second_position) <= 9 && (cp_col + second_position) <= 9) {
						if (fields -> all_fields[cp_row + second_position][cp_col + second_position] -> piece == NULL) {	
							return 1;
						}
					}
				}
			}	
		}	
		if ((cp_row + first_position) <= 9 && (cp_col - first_position) >= 0) {
			if (fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece != NULL) {
				if (strcmp(fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece -> color, color) != 0) {
					if((cp_row + second_position) <= 9 && (cp_col - second_position) >= 0) {	
						if (fields -> all_fields[cp_row + second_position][cp_col - second_position] -> piece == NULL) {				
							return 1;
						}
					}
				}
			}
		}
		
		if (strcmp(type, "king") == 0) {
			if ((cp_row - first_position) >= 0 && (cp_col + first_position) <= 9) {
				if (fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece != NULL) {
					if (strcmp(fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece -> color, color) != 0) {
						if((cp_row - second_position) >= 0 && (cp_col + second_position) <= 9) {	
							if (fields -> all_fields[cp_row - second_position][cp_col + second_position] -> piece == NULL) {
								return 1;
							}
						}
					}
				}
			}
			if ((cp_row - first_position) >= 0 && (cp_col - first_position) >= 0) {		
				if (fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece != NULL) {
					if (strcmp(fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece -> color, color) != 0) {
						if((cp_row - second_position) >= 0 && (cp_col - second_position) >= 0) {	
							if (fields -> all_fields[cp_row - second_position][cp_col - second_position] -> piece == NULL) {
								return 1;
							}
						}
					}
				}	
			}
		}
	}
	return 0;				
}
