#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
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
	
	int can_kill = check_can_kill(all_games, game_ID, color, type);

	/*
	int can_move = 0;
	if (can_kill == 0) {
		for (i = 0; i < (*all_games) -> games[game_ID] -> fields -> size; i++) {
			for (j = 0; j < (*all_games) -> games[game_ID] -> fields -> size; j++) {
				
			}
		}
	}
	

	if (can_move == 0) {
		//hráč už nemůže hrát, zkontrolovat i druhého hráče, pokud taky nebude moct, je to remíza, jinak vyhrál on
		int can_move_opponent = 0;

		if (can_move_opponent == 0) {
			//remiza
		}
		else {
			//prohra aktualniho hrace
		}
	}
	*/

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
	
	//kontrola, jestli hráč může hrát alespoň jednou figurkou
	if (can_kill == 1) {
		if (dp_row == (cp_row - second_position)) {
			int first_move_kill = all_first_move_kill(all_games, game_ID, first_position, second_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, 					second_player_name, color, type);

			int if_return = switch_kill(first_move_kill, all_games, game_ID, first_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, 					second_player_name, color, type);
			if (if_return == 1) return;

			int second_move_kill = all_second_move_kill(all_games, game_ID, first_position, second_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, 					second_player_name, color, type);
			
			if_return = switch_kill(second_move_kill, all_games, game_ID, first_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name, color, type);
			if (if_return == 1) return;
			
		}
		else if (strcmp(type, "king") == 0) {
			if (dp_row == (cp_row + second_position)) {
				int first_move_kill = king_first_move_kill(all_games, game_ID, first_position, second_position, cp_row, cp_col, dp_row, dp_col, 							current_player_socket_ID, second_player_socket_ID, second_player_name, color, type);
				
				int if_return = switch_kill(first_move_kill, all_games, game_ID, first_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, 						second_player_name, color, type);
				if (if_return == 1) return;

				int second_move_kill = king_second_move_kill(all_games, game_ID, first_position, second_position, cp_row, cp_col, dp_row, dp_col, 							current_player_socket_ID, second_player_socket_ID, second_player_name, color, type);

				if_return = switch_kill(second_move_kill, all_games, game_ID, first_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, 						second_player_name, color, type);
				if (if_return == 1) return;

				send_message(current_player_socket_ID, "wrong_move;9;\n");
				return;
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
		int first_move_no_kill = all_first_move_no_kill(all_games, game_ID, first_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name);
			
		int if_return = switch_no_kill(first_move_no_kill, all_games, game_ID, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name);
		if (if_return == 1) return;

		int second_move_no_kill = all_second_move_no_kill(all_games, game_ID, first_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name);
				
		if_return = switch_no_kill(second_move_no_kill, all_games, game_ID, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name);
		if (if_return == 1) return;

		if (strcmp(type, "king") == 0) {
			first_move_no_kill = king_first_move_no_kill(all_games, game_ID, first_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name);
			
			if_return = switch_no_kill(first_move_no_kill, all_games, game_ID, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name);
			if (if_return == 1) return;

			second_move_no_kill = king_second_move_no_kill(all_games, game_ID, first_position, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name);
			
			if_return = switch_no_kill(second_move_no_kill, all_games, game_ID, cp_row, cp_col, dp_row, dp_col, current_player_socket_ID, second_player_socket_ID, second_player_name);
			if (if_return == 1) return;

			send_message(current_player_socket_ID, "wrong_move;4;\n");
			return;
		}
		else {
			send_message(current_player_socket_ID, "wrong_move;8;\n");
			return;
		}
	}	
	
	if (strcmp(type, "man") == 0) {
		char message_promote[100];
		
		if (strcmp(color, "white") == 0) {
			if (dp_row == 0) {
				(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece -> type = "king";
				sprintf(message_promote, "promote;%d;%d;\n", dp_row, dp_col);
				send_message(current_player_socket_ID, message_promote);
				send_message(second_player_socket_ID, message_promote);
			}
		}
		else {
			if (dp_row == 9) {
				(*all_games) -> games[game_ID] -> fields -> all_fields[dp_row][dp_col] -> piece -> type = "king";
				sprintf(message_promote, "promote;%d;%d;\n", dp_row, dp_col);
				send_message(current_player_socket_ID, message_promote);
				send_message(second_player_socket_ID, message_promote);
			}
		}
	}
}

int switch_no_kill(int value, games **all_games, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name) {
	switch (value) {
		case 1:
			send_all_no_kill(all_games, game_ID, cp_row, cp_col, dp_row, dp_col, curr_pl_socket_ID, sec_pl_socket_ID, sec_pl_name);
			return 1;
		case 25:
			send_message(curr_pl_socket_ID, "wrong_move;5;\n");
			return 1;
		case 27:
			send_message(curr_pl_socket_ID, "wrong_move;7;\n");
			return 1;
		defaut:
			return 0;
	}
}

int switch_kill(int value, games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type) {
	switch (value) {
		case 100:
			send_all_kill(all_games, game_ID, cp_row, cp_col, cp_row - first_position, cp_col - first_position, dp_row, dp_col, curr_pl_socket_ID, sec_pl_socket_ID, sec_pl_name, color, type);
			return 1;
		case 101:
			send_all_kill(all_games, game_ID, cp_row, cp_col, cp_row - first_position, cp_col + first_position, dp_row, dp_col, curr_pl_socket_ID, sec_pl_socket_ID, sec_pl_name, color, type);
			return 1;
		case 111:
			send_all_kill(all_games, game_ID, cp_row, cp_col, cp_row + first_position, cp_col + first_position, dp_row, dp_col, curr_pl_socket_ID, sec_pl_socket_ID, sec_pl_name, color, type);
			return 1;
		case 110:
			send_all_kill(all_games, game_ID, cp_row, cp_col, cp_row + first_position, cp_col - first_position, dp_row, dp_col, curr_pl_socket_ID, sec_pl_socket_ID, sec_pl_name, color, type);
			return 1;
		case 25:
			send_message(curr_pl_socket_ID, "wrong_move;5;\n");
			return 1;

		case 26:
			send_message(curr_pl_socket_ID, "wrong_move;6;\n");
			return 1;
		defaut:
			return 0;
	}
}
