#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include "header.h"

void sigint_handler(int sig);
log_info *info; 
char *filename = "log.txt";

struct timeval server_start;
struct timeval server_end;

int main (void)
{
	int max_players = 32;

	remove(filename);

	clients *array_clients;	
	create_clients(&array_clients);

	games *all_games;
	create_games(&all_games);

	wanna_play *wanna_plays;
	create_wanna_play(&wanna_plays);

	char state_not_playing[12] = "not_playing";
	char state_playing[8] = "playing";
	char state_wanna_play[12] = "wanna_play";

	struct timeval client_timeout;
	client_timeout.tv_sec = 300; //5 min

	info = calloc(1, sizeof(info));

	int count_bytes = 0;
	int count_messages = 0;
	int count_connections = 0;
	int count_bad_transmissions = 0;

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
	
	gettimeofday(&server_start, NULL);	

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

	signal(SIGINT, sigint_handler);
	while(1) {	
		tests = client_socks;

		return_value = select(FD_SETSIZE, &tests, (fd_set*)NULL, (fd_set*)NULL, &client_timeout);

		if (return_value < 0) {
			printf("Select ERR\n");
			return -1;
		}
		else if (return_value == 0) {
			delete_connection(&array_clients, &wanna_plays, &client_socks, fd);
			printf("Client with socket id %d timeouted\n", fd);
		}

		for (fd = 3; fd < FD_SETSIZE; fd++) {
			if (FD_ISSET(fd, &tests)) {
				if (fd == server_socket) {
					client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &len_addr);
					FD_SET(client_socket, &client_socks);
					info -> count_connections++;
				}
				else {
					int int_ioctl = ioctl(fd, FIONREAD, &a2read);
					if (int_ioctl >= 0) {	
						if (a2read > 0) {
							int int_recv = recv(fd, &cbuf, cbuf_size*sizeof(char), 0);
	
							if (int_recv == 0) {
								delete_connection(&array_clients, &wanna_plays, &client_socks, fd);
								printf("Client with socket id %d closed connection\n", fd);							
								//connection closed - vym
							}
							else if (int_recv < 0) {
								//recv failed - nastavit timeout
								set_state_by_socket_ID(&array_clients, fd, "disconnect");
								printf("Client with socket id %d disconnected\n", fd);
							}
							else {
								char *tok = strtok(cbuf, ";");
								char *type_message = tok;
		
								if (strcmp(type_message, "login") == 0) {	
		
									login(&array_clients, all_games, &info, tok, max_players, fd);
								}
								else if (strcmp(type_message, "play") == 0) {
									
									play(&array_clients, &wanna_plays, &all_games, &info, fd);
								}
								else if (strcmp(type_message, "client_move") == 0) {
		
									client_move(&all_games, array_clients, &info, tok);
								}
								else if (strcmp(type_message, "new_game_no") == 0) {
									client_remove(&array_clients, &wanna_plays, fd);
								}
								else {
									//spatny prikaz
								}
							}						
						}
						else {
							//pokud hraje hru, tak čekat, popřípadě hru rovnou vymazat pokud oba dva hráči z jedné hry vyskočí
							delete_connection(&array_clients, &wanna_plays, &client_socks, fd);
						}
					}
					else {
						printf("Ioctl failed and returned: %s\n",strerror(errno));
					}
				}
			}
		}
	}	
	return 0;
}

void sigint_handler(int sig) {
	gettimeofday(&server_end, NULL);

	server_running(server_start, server_end, &info);
	log_all(filename, info);
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

void send_message(int client_socket, char *message, log_info **info) {	
	printf("Message: %s\n", message);
	send(client_socket, message, strlen(message) * sizeof(char), 0);
	
	(*info) -> count_bytes += strlen(message) + 1;
	(*info) -> count_messages++;
}


void login(clients **array_clients, games *all_games, log_info **info, char *tok, int max_players, int fd) {
	tok = strtok(NULL, ";");
	char *name = tok;
		
							
	if (name_exists((*array_clients), name) == 0) {								
		if (((*array_clients) -> clients_count) < (max_players)) {
			add_client(array_clients, name, fd);
	
			printf("Name: %s\n", (*array_clients) -> clients[(*array_clients) -> clients_count -1] -> name);
			char *message = "login_ok;\n";
			send_message(fd, message, info);					
		}
		else {
			char *message = "login_false;1;\n";
			send_message(fd, message, info);
		}		
	}
	else {
		char *state = get_state_by_name((*array_clients), name);
		if (strcmp(state, "disconnect") == 0) {
			reconnect(array_clients, all_games, info, name, fd);									
		}
		else {
			char *message = "login_false;2;\n";
			send_message(fd, message, info);
		}								
	} 
}

void reconnect(clients **array_clients, games *all_games, log_info **info, char *name, int fd) {
	set_socket_ID(array_clients, name, fd);
	set_state_by_name(array_clients, name, "playing"); 
								
	char board[1024];
	game *game = find_game_by_name(all_games, name);
								
	sprintf(board, "board;%s;%s;%d;%d;", name, get_color_by_socket_ID(*array_clients, fd), game -> game_ID, game -> fields -> count_pieces);;					
					
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
	send_message(fd, board, info);
}

void play(clients **array_clients, wanna_play **wanna_plays, games **all_games, log_info **info, int fd) {
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

		send_message(socket_ID_1, message_1, info);
		send_message(socket_ID_2, message_2, info);
	}
}

void client_move(games **all_games, clients *array_clients, log_info **info, char *tok) {
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
							
	process_move(all_games, array_clients, info, game_ID, cp_row, cp_col, dp_row, dp_col, color, piece); 
}

void delete_connection(clients **array_clients, wanna_play **wanna_plays, fd_set *client_socks, int fd) {
	close(fd);
	FD_CLR(fd, client_socks);
	client_remove(array_clients, wanna_plays, fd);
}

//Každý program bude doplněn o zpracování statistických údajů: 
//	1.přenesený počet bytů
//	2.přenesený počet zpráv
//	3.počet navázaných spojení
//	4.počet přenosů zrušených pro chybu
//	5.doba běhu 
//	6.apod.

//	1.strlen(send_message) 
//	2.počet send
//	3.úspěšný connect
//	4.počet bitů, které se přenesli a nevyužili jsme je
//	5.nazačátku si uložit čas, při ukončení čas, z toho zjistit jak dlouho běžel server

void log_all(char *filename, log_info *info) {

	char message_count_bytes[100];
	char message_count_messages[100];
	char message_count_connections[100];
	char message_count_bad_transmissions[100];	
	char message_server_running[100];
		
	sprintf(message_count_bytes, "Bytes sent: %d\n", info -> count_bytes);
	sprintf(message_count_messages, "Messages sent: %d\n", info -> count_messages);
	sprintf(message_count_connections, "Clients connected: %d\n", info -> count_connections);
	sprintf(message_count_bad_transmissions, "Unused bytes sent: %d\n", info -> count_bad_transmissions);
	sprintf(message_server_running, "Server ran for %d minutes\n", info -> server_running_minutes);

	FILE *fptr;
	fptr = fopen(filename, "r+");
	if(fptr == NULL) 
	{
	    fptr = fopen(filename, "wb");
	}

	fprintf(fptr, message_count_bytes);
	fprintf(fptr, message_count_messages);
	fprintf(fptr, message_count_connections);
	fprintf(fptr, message_count_bad_transmissions);
	fprintf(fptr, message_server_running);

	fclose(fptr);	
}

void server_running(struct timeval start, struct timeval end, log_info **info) {

	(*info) -> server_running_minutes = (end.tv_sec - start.tv_sec) / 60;
}
