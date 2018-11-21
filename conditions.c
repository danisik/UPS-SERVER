#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "header.h"

int check_can_kill(games **all_games, int game_ID, char *color, char *type) {
	int i, j, tmp_can_kill = 0;
	for (i = 0; i < (*all_games) -> games[game_ID] -> fields -> size; i++) {
		for (j = 0; j < (*all_games) -> games[game_ID] -> fields -> size; j++) {
			if ((*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece != NULL) {
				if (strcmp( (*all_games) -> games[game_ID] -> fields -> all_fields[i][j] -> piece -> color, color) == 0 ) {			
					tmp_can_kill = check_if_can_kill((*all_games) -> games[game_ID] -> fields, i, j, color, type);	
					if (tmp_can_kill == 1) {
						return tmp_can_kill;
					}
				}
			}
		}
	}
}

//return: 0 - can't kill piece
//	  1 - can kill piece
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

//
void send_all_no_kill(games **all_games, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name) {
	char correct_message[100];
	sprintf(correct_message, "correct_move;2;%d;%d;%d;%d;\n", cp_row, cp_col, dp_row, dp_col);

	send_message(curr_pl_socket_ID, correct_message);  		
	send_message(sec_pl_socket_ID, correct_message);

	send_message(curr_pl_socket_ID, "end_move;\n");
	send_message(sec_pl_socket_ID, "play_next_player;\n");
	(*all_games) -> games[game_ID] -> now_playing = sec_pl_name;

	(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
	(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
}

//
void send_all_kill(games **all_games, int game_ID, int cp_row, int cp_col, int middle_row, int middle_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type) {
	char correct_message[100];
	sprintf(correct_message, "correct_move;3;%d;%d;%d;%d;%d;%d;\n", cp_row, cp_col, middle_row, middle_col, dp_row, dp_col);

	send_message(curr_pl_socket_ID, correct_message);  					
	send_message(sec_pl_socket_ID, correct_message);
	if (check_if_can_kill((*all_games) -> games[game_ID] -> fields, cp_row, cp_col, color, type) != 0) {
		send_message(curr_pl_socket_ID, "end_move;\n");
		send_message(sec_pl_socket_ID, "play_next_player;\n");
		(*all_games) -> games[game_ID] -> now_playing = sec_pl_name;
	}
					
	(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece = (*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece;
	(*all_games) -> games[game_ID] -> fields -> all_fields[cp_row][cp_col] -> piece = NULL;
	(*all_games) -> games[game_ID] -> fields -> all_fields[middle_row][middle_col] -> piece = NULL;
}

//0 - nothing
//1 - correct_move
//2 - wrong move
int all_first_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name) {
	if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col - first_position)) ) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col - first_position] -> piece == NULL) { 
			return 1;
		}					
		else {
			return 27;
		}
	}
	else return 0;
}

//
int all_second_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name) {
	if ( (dp_row == (cp_row - first_position)) && (dp_col == (cp_col + first_position)) ) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row - first_position][cp_col + first_position] -> piece == NULL) { 
			return 1;
		}					
		else {
			return 25;
		}
	}
	else return 0;
}

//
int king_first_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name) {
	if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col + first_position)) ) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col + first_position] -> piece == NULL) { 
			return 1;
		}					
		else {
			return 27;
		}
	}
	else return 0;
}

//
int king_second_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name) {
	if ( (dp_row == (cp_row + first_position)) && (dp_col == (cp_col - first_position)) ) {
		if ((*all_games) -> games[game_ID] -> fields -> all_fields[cp_row + first_position][cp_col - first_position] -> piece == NULL) { 
			return 1;
		}					
		else {
			return 25;
		}
	}
	else return 0;
}

//
int all_first_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type) {
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
	else return 0;
}

//
int all_second_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type) {
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
	else return 0;
}

//
int king_first_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type) {
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
	else return 0;
}

//
int king_second_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type) {
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
	else return 0;
}
