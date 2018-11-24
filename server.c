#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "header.h"

int main (void)
{
	int max_players = 32;

	clients *array_clients;	
	create_clients(&array_clients);

	games *all_games;
	create_games(&all_games);

	wanna_play *wanna_plays;
	create_wanna_play(&wanna_plays);

	char state_not_playing[12] = "not_playing";
	char state_playing[8] = "playing";
	char state_wanna_play[12] = "wanna_play";

	int server_socket;
	int client_socket, fd;
	int return_value;
	int cbuf_size = 1024;
	char cbuf[cbuf_size];
	int len_addr;
	int a2read;
	struct sockaddr_in my_addr, peer_addr;
	fd_set client_socks, tests;
	
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	int param = 1;
        return_value = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&param, sizeof(int));
	
	if (return_value == -1)	printf("setsockopt ERR\n");

	memset(&my_addr, 0, sizeof(struct sockaddr_in));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(10000);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in));

	if (return_value == 0) printf("Bind - OK\n");
	else {
		printf("Bind - ERR\n");
		return -1;
	}

	return_value = listen(server_socket, 5);
	if (return_value == 0) {
		printf("Listen - OK\n");
	} else {
		printf("Listen - ERR\n");
	}

	FD_ZERO(&client_socks);
	FD_SET(server_socket, &client_socks);

	while(1) {
		tests = client_socks;

		return_value = select(FD_SETSIZE, &tests, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)0);

		if (return_value < 0) {
			printf("Select ERR\n");
			return -1;
		}

		for (fd = 3; fd < FD_SETSIZE; fd++) {
			if (FD_ISSET(fd, &tests)) {
				if (fd == server_socket) {
					client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &len_addr);
					FD_SET(client_socket, &client_socks);
				}
				else {
					ioctl(fd, FIONREAD, &a2read);
					if (a2read > 0) {
						int size_recv;

						if ((size_recv = recv(fd, &cbuf, cbuf_size*sizeof(char), 0) < 0)) {
						}

						char *tok = strtok(cbuf, ";");
						char *type_message = tok;

						if (strcmp(type_message, "login") == 0) {	

							login(&array_clients, all_games, tok, max_players, fd);
						}
						else if (strcmp(type_message, "play") == 0) {
							
							play(&array_clients, &wanna_plays, &all_games, fd);
						}
						else if (strcmp(type_message, "client_move") == 0) {

							client_move(&all_games, array_clients, tok);
						}
						else if (strcmp(type_message, "new_game_no") == 0) {
							//vymazat z pole klientů
							client_remove(&array_clients, &wanna_plays, fd);
						}
						else {
							//spatny prikaz
						}						
					}
					else {
						//pokud hraje hru, tak čekat, popřípadě hru rovnou vymazat

						close(fd);
						FD_CLR(fd, &client_socks);
						//client_remove(&array_clients, &wanna_plays, fd);
						set_state_by_socket_ID(&array_clients, fd, "disconnect");
					}
				}
			}
		}
	}	
	return 0;
}


//return: 0 - not contains name
//	  1 - contains name
int name_exists (clients *array_clients, char *name) {
	int i;
	for (i = 0; i < array_clients -> clients_count; i++) {
		if (strcmp(array_clients -> clients[i] -> name, name) == 0) {
			return 1;
		}
	}
	printf("\n");
	return 0;
}

void send_message(int client_socket, char *message) {	
	printf("Message: %s\n", message);
	send(client_socket, message, strlen(message) * sizeof(char), 0);
}


void login(clients **array_clients, games *all_games, char *tok, int max_players, int fd) {
	tok = strtok(NULL, ";");
	char *name = tok;
		
							
	if (name_exists((*array_clients), name) == 0) {								
		if (((*array_clients) -> clients_count) < (max_players)) {
			add_client(array_clients, name, fd);
			printf("Name: %s\n", (*array_clients) -> clients[(*array_clients) -> clients_count -1] -> name);
			send_message(fd, "login_ok;\n");					
		}
		else {
			send_message(fd, "login_false;1;\n");
		}		
	}
	else {
		char *state = get_state_by_name((*array_clients), name);
		if (strcmp(state, "disconnect") == 0) {
			reconnect(array_clients, all_games, name, fd);									
		}
		else {
			send_message(fd, "login_false;2;\n");
		}								
	} 
}

void reconnect(clients **array_clients, games *all_games, char *name, int fd) {
	set_socket_ID(array_clients, name, fd);
	set_state_by_name(array_clients, name, "playing"); 
								
	char board[1024];
	game *game = find_game_by_name(all_games, name);
								
	sprintf(board, "board;%s;%s;%d;%d;", name, get_color_by_socket_ID(*array_clients, fd), game -> game_ID, game -> fields -> count_pieces);
									
	int i,j;
	for (i = 0; i < game -> fields -> size; i++) {
		for(j = 0; j < game -> fields -> size; j++) {
			if (game -> fields -> all_fields[i][j] -> piece != NULL) {
				char x[3], y[3];

				sprintf(x, "%d;", i);
				sprintf(y, "%d;", j);
											
				strcat(board, x);
				strcat(board, y);
				strcat(board, game -> fields -> all_fields[i][j] -> piece -> color);
				strcat(board, ";");
				strcat(board, game -> fields -> all_fields[i][j] -> piece -> type);
				strcat(board, ";");							
			}
		}
	}
	strcat(board, "\n");
	send_message(fd, board);
}

void play(clients **array_clients, wanna_play **wanna_plays, games **all_games, int fd) {
	add_wanna_play(wanna_plays, fd);
	char *name = get_name_by_socket_ID(*array_clients, fd);
	set_state_by_name(array_clients, name, "wanna_play");
	if (((*wanna_plays) -> size) >= 2) {
		int socket_ID_1 = fd;
		int socket_ID_2;
		do {
			socket_ID_2 = (*wanna_plays) -> socket_IDs[rand() % ((*wanna_plays) -> size)];
		} 
		while(socket_ID_2 == socket_ID_1);								

		remove_wanna_play(wanna_plays, socket_ID_1);
		remove_wanna_play(wanna_plays, socket_ID_2);
								

		char message_1[100];
		char message_2[100];
		char *now_playing;								
							

		char *name_1 = get_name_by_socket_ID(*array_clients, socket_ID_1);
		char *name_2 = get_name_by_socket_ID(*array_clients, socket_ID_2);

		int r = rand() % 2;
		if (r == 0) {
			sprintf(message_1, "start_game;black;%d;\n", (*all_games) -> games_count);
			sprintf(message_2, "start_game;white;%d;\n", (*all_games) -> games_count);

			set_color(array_clients, socket_ID_1, "black");
			set_color(array_clients, socket_ID_2, "white");
	
			now_playing = name_2; 		
		}
		else {
			sprintf(message_1, "start_game;white;%d;\n", (*all_games) -> games_count);
			sprintf(message_2, "start_game;black;%d;\n", (*all_games) -> games_count);

			set_color(array_clients, socket_ID_1, "white");
			set_color(array_clients, socket_ID_2, "black");		

			now_playing = name_1;
		}
							

		set_state_by_name(array_clients, name_1, "playing");
		set_state_by_name(array_clients, name_2, "playing");

		add_game(all_games, name_1, name_2, now_playing);

		send_message(socket_ID_1, message_1);
		send_message(socket_ID_2, message_2);
	}
}

void client_move(games **all_games, clients *array_clients, char *tok) {
	int i = 0;
	char *array[10];
						
	tok = strtok(NULL, ";");

	while(tok != NULL) {
		array[i++] = tok;
		tok = strtok(NULL, ";");
	}

	int game_ID = atoi(array[0]);
	int cp_row = atoi(array[1]);
	int cp_col = atoi(array[2]);
	int dp_row = atoi(array[3]);
	int dp_col = atoi(array[4]);
	char *color = array[5];
	char *piece = array[6];
							
	printf("Move: GID=%d CP=%d,%d DP=%d,%d COLOR=%s TYPE=%s\n", game_ID, cp_row, cp_col, dp_row, dp_col, color, piece);
							
	process_move(all_games, array_clients, game_ID, cp_row, cp_col, dp_row, dp_col, color, piece); 
}

