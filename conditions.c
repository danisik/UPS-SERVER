//
//	DRAUGHTS
//	VERSION 1.0.0
//
//	Copyright (c) 2010-2018 Dept. of Computer Science & Engineering,
//	Faculty of Applied Sciences, University of West Bohemia in Plzeň.
//	All rights reserved.
//
//	Code written by:	Vojtěch Danišík
//	Last update on:		21-12-2018
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "header.h"

/*
 * Check if player can kill opponents piece
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param color - color set to player
 * @param type - type of piece
 * @return 0 if player cant kill any opponents piece, 1 if can
 */	
int check_can_kill(games **all_games, int game_ID, char *color, char *type) {
	int i, j, tmp_can_kill = 0;
	for (i = 0; i < (*all_games) -> games[game_ID] -> fields -> size; i++) {
		for (j = 0; j < (*all_games) -> games[game_ID] -> fields -> size; j++) {
			if ((*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece != NULL) {
				if (strcmp( (*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece -> color, color) == 0 ) {			
					tmp_can_kill = check_if_can_kill((*all_games) -> games[game_ID] -> fields, i, j, color, (*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece -> type);	
					if (tmp_can_kill == 1) {
						return tmp_can_kill;
					}
				}
			}
			
		}
	}
	return 0;
}

/*
 * Check if piece can kill opponents piece
 * @param fields - fields 
 * @param cp_row - x position (row) of piece
 * @param cp_col - y position (column) of piece
 * @param color - color set to player
 * @param type - type of piece
 * @return 0 if player cant kill any opponents piece, 1 if can
 */	
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

/*
 * Send message to players about correct move
 * @param array_clients - array of logged clients
 * @param all_games - array of all games
 * @param info - structures to save log info
 * @param game_ID - ID of game
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param curr_pl_socket_ID - socket ID of current player
 * @param sec_pl_socket_ID - socket ID of second player
 * @param sec_pl_name - name of second player
 */
void send_all_no_kill(clients **all_clients, games **all_games, log_info **info, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name) {
	char correct_message[100];
	sprintf(correct_message, "correct_move;2;%d;%d;%d;%d;\n", cp_row, cp_col, dp_row, dp_col);

	send_message(curr_pl_socket_ID, correct_message, info);  		
	send_message(sec_pl_socket_ID, correct_message, info);

	client *client_1 = get_client_by_socket_ID(*all_clients, curr_pl_socket_ID);
	client *client_2 = get_client_by_socket_ID(*all_clients, sec_pl_socket_ID);

	set_state(&client_1, 4);
	set_state(&client_2, 3);

	send_message(curr_pl_socket_ID, "end_move;\n", info);
	send_message(sec_pl_socket_ID, "play_next_player;\n", info);

	(*all_games) -> games[game_ID] -> now_playing = sec_pl_name;

	(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
	(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
	return;
}

/*
 * Send message to players about correct move (included killing piece)
 * @param array_clients - array of logged clients
 * @param all_games - array of all games
 * @param info - structures to save log info
 * @param game_ID - ID of game
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param middle_row - x position (row) of killed piece
 * @param middle_col - y position (column) of killed piece
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param curr_pl_socket_ID - socket ID of current player
 * @param sec_pl_socket_ID - socket ID of second player
 * @param sec_pl_name - name of second player
 * @param color - color set to player
 * @param type - type of piece
 */
void send_all_kill(clients **all_clients, games **all_games, log_info **info, int game_ID, int cp_row, int cp_col, int middle_row, int middle_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type) {
	char correct_message[100];
	sprintf(correct_message, "correct_move;3;%d;%d;%d;%d;%d;%d;\n", cp_row, cp_col, middle_row, middle_col, dp_row, dp_col);

	send_message(curr_pl_socket_ID, correct_message, info);  					
	send_message(sec_pl_socket_ID, correct_message, info);

	client *client_1 = get_client_by_socket_ID(*all_clients, curr_pl_socket_ID);
	client *client_2 = get_client_by_socket_ID(*all_clients, sec_pl_socket_ID);				

	(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
	(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
	(*all_games) -> games[game_ID] -> fields -> all_fields[middle_row][middle_col] -> piece = NULL;
	(*all_games) -> games[game_ID] -> fields -> count_pieces--;

	char opponent_color[10];
	if (strcmp(color, "white") == 0) strcpy(opponent_color, "black");
	else strcpy(opponent_color, "white");
	
	int val = check_if_opponent_have_pieces(all_games, game_ID, opponent_color);
	if (val == 0) {
		end_game(all_clients, all_games, game_ID, 1, 0, curr_pl_socket_ID, sec_pl_socket_ID, info);
	}

	int can_kill_next = check_can_kill(all_games, game_ID, color, type);
	if(can_kill_next == 0) {
		set_state(&client_1, 4);
		set_state(&client_2, 3);
		send_message(curr_pl_socket_ID, "end_move;\n", info);
		send_message(sec_pl_socket_ID, "play_next_player;\n", info);
		(*all_games) -> games[game_ID] -> now_playing = sec_pl_name;
	}

	return;
}

/*
 * check first field next to piece to move
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param color - color set to player
 * @return 0 if there is something bad, 27 is ID of error and 1 is move
 */
int all_first_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color) {

	if ((cp_row - first_position < 0) || (cp_row - first_position > 9)) {
		return 0;
	}
	if ((cp_col - first_position < 0) || (cp_col - first_position > 9)) {
		return 0;
	}

	if (color != NULL) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece != NULL) {
			if (strcmp(color, (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece -> color) != 0) {
				return 0;
			} 
		}
		else {
			return 0;
		}
	}

	if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col - first_position)) ) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece == NULL) { 
			return 1;
		}					
		else {
			return 27;
		}
	}
	else {
		return 0;
	}
}

/*
 * check second field next to piece to move
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param color - color set to player
 * @return 0 if there is something bad, 25 is ID of error and 1 is move
 */
int all_second_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color) {
	if ((cp_row - first_position < 0) || (cp_row - first_position > 9)) {
		return 0;
	}
	if ((cp_col + first_position < 0) || (cp_col + first_position > 9)) {
		return 0;
	}

	if (color != NULL) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece != NULL) {
			if (strcmp(color, (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece -> color) != 0) {
				return 0;
			} 
		}
		else {
			return 0;	
		}
	}

	if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col + first_position)) ) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece == NULL) { 
			return 1;
		}					
		else {
			return 25;
		}
	}
	else {
		return 0;
	}
}

/*
 * check third field next to piece to move (only for kings)
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param color - color set to player
 * @return 0 if there is something bad, 27 is ID of error and 1 is move
 */
int king_first_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color) {
	if ((cp_row + first_position < 0) || (cp_row + first_position > 9)) {
		return 0;
	}
	if ((cp_col + first_position < 0) || (cp_col + first_position > 9)) {
		return 0;
	}

	if (color != NULL) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece != NULL) {
			if (strcmp(color, (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece -> color) != 0) {
				return 0;
			} 
		}
		else {
			return 0;
		}
	}

	if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col + first_position)) ) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece == NULL) { 
			return 1;
		}					
		else {
			return 27;
		}
	}
	else  {
		return 0;
	}
}

/*
 * check fourth field next to piece to move (only for kings)
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param color - color set to player
 * @return 0 if there is something bad, 25 is ID of error and 1 is move
 */
int king_second_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color) {
	if ((cp_row + first_position < 0) || (cp_row + first_position > 9)) {
		return 0;
	}
	if ((cp_col - first_position < 0) || (cp_col - first_position > 9)) {
		return 0;
	}

	if (color != NULL) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece != NULL) {
			if (strcmp(color, (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece -> color) != 0) {
				return 0;
			} 
		}
		else {
			return 0;	
		}
	}

	if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col - first_position)) ) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece == NULL) { 
			return 1;
		}					
		else {
			return 25;
		}
	}
	else {
		return 0;
	}
}

/*
 * check first field next to piece to move and kill opponents piece
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param second_position - constant with value 2 (-2 for player with black piece)
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param color - color set to player
 * @return 0 if there is something bad, 25/26 is ID of error and 100 is move
 */
int all_first_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color) {
	if ((cp_row - first_position < 0) || (cp_row - first_position > 9)) {
		return 0;
	}
	if ((cp_col - first_position < 0) || (cp_col - first_position > 9)) {
		return 0;
	}

	if (dp_col == (cp_col - second_position)) {
		if((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece != NULL) {
			if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece -> color, color) != 0) { 
				return 100;
			}					
			else {
				return 25;
			}						
		}
		else {
			return 26;
		}
	}
	else {
		return 0;
	}
}

/*
 * check second field next to piece to move and kill opponents piece
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param second_position - constant with value 2 (-2 for player with black piece)
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param color - color set to player
 * @return 0 if there is something bad, 25/26 is ID of error and 101 is move
 */
int all_second_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color) {
	if ((cp_row - first_position < 0) || (cp_row - first_position > 9)) {
		return 0;
	}
	if ((cp_col + first_position < 0) || (cp_col + first_position > 9)) {
		return 0;
	}

	if (dp_col == (cp_col + second_position)) {
		if((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece != NULL) {
			if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece -> color, color) != 0) { 
				return 101;
			}					
			else {
				return 25;
			}
		}
		else {
			return 26;
		}
	}
	else {
		return 0;
	}
}

/*
 * check third field next to piece to move and kill opponents piece (only for kings)
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param second_position - constant with value 2 (-2 for player with black piece)
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param color - color set to player
 * @return 0 if there is something bad, 25/26 is ID of error and 111 is move
 */
int king_first_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color) {
	if ((cp_row + first_position < 0) || (cp_row + first_position > 9)) {
		return 0;
	}
	if ((cp_col + first_position < 0) || (cp_col + first_position > 9)) {
		return 0;
	}

	if (dp_col == (cp_col + second_position)) {
		if((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece != NULL) {
			if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece -> color, color) != 0) { 
				return 111;
			}					
			else {
				return 25;
			}
		}
		else {
			return 26;
		}
	}
	else {
		return 0;
	}
}

/*
 * check fourth field next to piece to move and kill opponents piece (only for kings)
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param second_position - constant with value 2 (-2 for player with black piece)
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param color - color set to player
 * @return 0 if there is something bad, 25/26 is ID of error and 110 is move
 */
int king_second_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color) {
	if ((cp_row + first_position < 0) || (cp_row + first_position > 9)) {
		return 0;
	}
	if ((cp_col - first_position < 0) || (cp_col - first_position > 9)) {
		return 0;	
	}

	if (dp_col == (cp_col - second_position)) {	
		if((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece != NULL) {
			if (strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece -> color, color) != 0) { 
				return 110;			
			}					
			else {
				return 25;
			}
		}
		else {
			return 26;
		}
	}
	else {
		return 0;	
	}
}

/*
 * check if moved piece can be promoted to king
 * @param all_games - array of all games
 * @param info - structures to save log info
 * @param game_ID - ID of game
 * @param dp_row - x position (row) of piece after move
 * @param dp_col - y position (column) of piece after move
 * @param curr_pl_socket_ID - socket ID of current player
 * @param sec_pl_socket_ID - socket ID of second player
 * @param sec_pl_name - name of second player
 * @param color - color set to player
 * @param type - type of piece
 * @param cp_row - x position (row) of piece before move
 * @param cp_col - y position (column) of piece before move
 */
void check_if_can_promote(games **all_games, log_info **info, int game_ID, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *color, char *type, int cp_row, int cp_col) {
	if (strcmp(type, "man") == 0) {
		char message_promote[100];
		if (!((*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece == NULL)) {
			if ((*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece -> type == NULL) return;
		}
		else return;

		if (strcmp(color, "white") == 0) {
			if (dp_row == 0) {
				(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece -> type = "king";
				sprintf(message_promote, "promote;%d;%d;\n", dp_row, dp_col);
				send_message(curr_pl_socket_ID, message_promote, info);
				send_message(sec_pl_socket_ID, message_promote, info);
			}
		}
		else {
			if (dp_row == 9) {
				(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece -> type = "king";
				sprintf(message_promote, "promote;%d;%d;\n", dp_row, dp_col);
				send_message(curr_pl_socket_ID, message_promote, info);
				send_message(sec_pl_socket_ID, message_promote, info);
			}
		}
	}
}

/*
 * check if opponent have pieces
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param color - color set to player
 * @return 0 if opponent dont have pieces and 1 if have at least 1 piece
 */
int check_if_opponent_have_pieces(games **all_games, int game_ID, char *color) {
	int i,j;
	for (i = 0; i < (*all_games) -> games[game_ID] -> fields -> size; i++) {
		for (j = 0; j < (*all_games) -> games[game_ID] -> fields -> size; j++) {
			if((*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece != NULL) {
				if(strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece -> color, color) == 0) {
					return 1;
				}			
			}
		}
	}
	return 0;
}

/*
 * check if player can move or not
 * @param all_games - array of all games
 * @param game_ID - ID of game
 * @param first_position - constant with value 1 (-1 for player with black piece)
 * @param second_position - constant with value 2 (-2 for player with black piece)
 * @param color - color set to player
 * @param type - type of piece
 * @return 0 if opponent cant move, 1 if player can move
 */
int check_if_can_move(games **all_games, int game_ID, int first_position, int second_position, char *color, char *type) {
	
	int i, j, value;

	for (i = 0; i < (*all_games) -> games[game_ID] -> fields -> size; i++) {
		for (j = 0; j < (*all_games) -> games[game_ID] -> fields -> size; j++) {
			if((*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece != NULL) {
				if(strcmp((*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece -> color, color) == 0) {
					if (strcmp(color, "white") == 0) {
						value = all_first_move_no_kill(all_games, game_ID, first_position, i, j, i-1, j-1, color);
						if (value == 1) return 1;
	
						value = all_first_move_kill(all_games, game_ID, first_position, second_position, i, j, i-2, j-2, color);	
						if (value >= 100) return 1;
			
						value = all_second_move_no_kill(all_games, game_ID, first_position, i, j, i-1, j+1, color);
						if (value == 1) return 1;
	
						value = all_second_move_kill(all_games, game_ID, first_position, second_position, i, j, i-2, j+2, color);	
						if (value >= 100) return 1;
	
						if (strcmp(type, "king") == 0) {
							value = king_first_move_no_kill(all_games, game_ID, first_position, i, j, i+1, j+1, color);
							if (value == 1) return 1;
	
							value = king_first_move_kill(all_games, game_ID, first_position, second_position, i, j, i+2, j+2, color);	
							if (value >= 100) return 1;
	
							value = king_second_move_no_kill(all_games, game_ID, first_position, i, j, i+1, j-1, color);
							if (value == 1) return 1;
	
							value = king_second_move_kill(all_games, game_ID, first_position, second_position, i, j, i+2, j-2, color);	
							if (value >= 100) return 1;
						}
					}
					else {
						value = all_first_move_no_kill(all_games, game_ID, first_position, i, j, i+1, j+1, color);
						if (value == 1) return 1;
	
						value = all_first_move_kill(all_games, game_ID, first_position, second_position, i, j, i+2, j+2, color);	
						if (value >= 100) return 1;
			
						value = all_second_move_no_kill(all_games, game_ID, first_position, i, j, i+1, j-1, color);
						if (value == 1) return 1;
	
						value = all_second_move_kill(all_games, game_ID, first_position, second_position, i, j, i+2, j-2, color);	
						if (value >= 100) return 1;
	
						if (strcmp(type, "king") == 0) {
							value = king_first_move_no_kill(all_games, game_ID, first_position, i, j, i-1, j-1, color);
							if (value == 1) return 1;
	
							value = king_first_move_kill(all_games, game_ID, first_position, second_position, i, j, i-2, j-2, color);	
							if (value >= 100) return 1;
	
							value = king_second_move_no_kill(all_games, game_ID, first_position, i, j, i-1, j+1, color);
							if (value == 1) return 1;
	
							value = king_second_move_kill(all_games, game_ID, first_position, second_position, i, j, i-2, j+2, color);	
							if (value >= 100) return 1;
						}
					}
				}
			}
		}
	}
	return 0;
}
