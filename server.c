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
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "header.h"

#define MAX_TIME_DISCONNECT 120


void sigint_handler(int sig);
log_info *info; 
char *filename = "log.txt";
	
games *all_games;
wanna_play *wanna_plays;
clients *array_clients;	

struct timeval server_start;
struct timeval server_end;

fd_set client_socks, tests;

int main(int argc, char *argv[])
{	
	int port = 10000;
	game_info();

	struct sockaddr_in my_addr, peer_addr;

	memset(&my_addr, 0, sizeof(struct sockaddr_in));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	switch(argc) {
		case 3:
			if (strcmp(argv[1], "-address") == 0 || strcmp(argv[1], "-a") == 0) {
				printf("Address set to %s\n", argv[2]);
				my_addr.sin_addr.s_addr = inet_addr(argv[2]);
			}
			else if (strcmp(argv[1], "-port") == 0 || strcmp(argv[1], "-p") == 0) {
				printf("Port set to %d\n", atoi(argv[2]));
				my_addr.sin_port = htons(atoi(argv[2]));
			}
			break;
		case 5:
			if (strcmp(argv[1], "-address") == 0 || strcmp(argv[1], "-a") == 0) {
				printf("Address set to %s\n", argv[2]);
				my_addr.sin_addr.s_addr = inet_addr(argv[2]);
			}
			if (strcmp(argv[3], "-port") == 0 || strcmp(argv[3], "-p") == 0) {
				printf("Port set to %d\n", atoi(argv[4]));
				my_addr.sin_port = htons(atoi(argv[4]));
			}
			break;
		case 1:
			break;
		default:
			printf("Wrong parameters set\n");
			break;
	}
	int max_players = 32;
	remove(filename);

	create_clients(&array_clients);
	create_games(&all_games);
	create_wanna_play(&wanna_plays);

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
	
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	int param = 1;
        return_value = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&param, sizeof(int));

	gettimeofday(&server_start, NULL);	

	if (return_value == -1)	printf("setsockopt ERR\n");

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

	for (;;) {
	
		tests = client_socks;
		return_value = select(FD_SETSIZE, &tests, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)0);
		if (return_value == 0) {
			delete(&array_clients, &wanna_plays, &client_socks, &all_games, &info, fd, 0);
		}
		if (return_value < 0){
			printf("Select ERR\n");
			return -1;
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
							if (int_recv <= 0) {
								disconnect(&array_clients, &info, all_games, fd);
								continue;
							}
				
							int contains_semicolon = -1;
							contains_semicolon = check_if_contains_semicolon(cbuf);
							
							if (contains_semicolon == 0) continue;
							

							char *tok = strtok(cbuf, ";");
							char *type_message = tok;
							if (strcmp(type_message, "login") == 0) {
								login(&array_clients, all_games, &info, tok, max_players, fd);
							}
							else if (strcmp(type_message, "play") == 0) {
								play(&array_clients, &wanna_plays, &all_games, &info, fd);
							}
							else if (strcmp(type_message, "client_move") == 0) {
								client_move(&all_games, &array_clients, &info, tok, fd);							
							}
							else if (strcmp(type_message, "new_game_no") == 0) {
								client *cl = get_client_by_socket_ID(array_clients, fd);
								if (cl == NULL) continue;

								if (cl -> state == 3 || cl -> state == 4) {
									client_remove(&array_clients, &wanna_plays, fd);
								}
							}
							else if (strcmp(type_message, "app_end") == 0) {
								delete(&array_clients, &wanna_plays, &client_socks, &all_games, &info, fd, 1);
							}
							else if (strcmp(type_message, "is_connected") == 0) {
								client *cl = get_client_by_socket_ID(array_clients, fd);
								set_connected(&cl, 1);
								printf("Received message: is_connected\n");
							}
							else {
								printf("wrong type\n");
							}	
												
						}
						else {
							FD_CLR(fd, &client_socks);
							delete(&array_clients, &wanna_plays, &client_socks, &all_games, &info, fd, 1);
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


/*
 * method called when server closing
 */
void sigint_handler(int sig) {
	gettimeofday(&server_end, NULL);
	server_running(server_start, server_end, &info);
	log_all(filename, info);
	_exit(1);
	return;
}

/*
 * Check if name exists in array of clients
 * @param array_clients - array of logged clients
 * @param name - selected name of actually loging client
 * @return 0 if name does not exist, 1 if exist
 */
int name_exists (clients *array_clients, char *name) {
	int i;
	for (i = 0; i < array_clients -> clients_count; i++) {
		if (strcmp(array_clients -> clients[i] -> name, name) == 0) {	
			return 1;
		}
	}
	return 0;
}

/*
 * Process login for client
 * @param client_socket - socket of logged client
 * @param message - message to send to client
 * @param info - structures to save log info
 */
void send_message(int client_socket, char *message, log_info **info) {	
	printf("Writed message: %s", message);
	send(client_socket, message, strlen(message) * sizeof(char), 0);
	
	(*info) -> count_bytes += strlen(message) + 1;
	(*info) -> count_messages++;
	return;
}

/*
 * Process login for client
 * @param client_socket - socket of logged client
 * @param message - message to send to client
 */
void send_message_no_info(int client_socket, char *message) {	
	printf("Writed message: %s", message);
	send(client_socket, message, strlen(message) * sizeof(char), 0);
	
	info -> count_bytes += strlen(message) + 1;
	info -> count_messages++;
	return;
}

/*
 * Process login for client
 * @param array_clients - array of logged clients
 * @param all_games - array of all games
 * @param info - structures to save log info
 * @param tok - string send by client containing login info
 * @param max_players - number saying how many client can be logged at one time
 * @param fd - client socket
 */
void login(clients **array_clients, games *all_games, log_info **info, char *tok, int max_players, int fd) {
	client *cl = get_client_by_socket_ID(*array_clients, fd);
	tok = strtok(NULL, ";");
	char *name = tok;
	if (name_exists((*array_clients), name) == 0) {								
		if (((*array_clients) -> clients_count) < (max_players)) {			
			if (cl == NULL) {
				add_client(array_clients, name, fd);
				cl = (*array_clients) -> clients[(*array_clients) -> clients_count - 1];
				pthread_create(&(cl -> client_thread), NULL, &check_connectivity, &cl); 
				printf("Name: %s\n", (*array_clients) -> clients[(*array_clients) -> clients_count -1] -> name);
				char *message = "login_ok;\n";
				send_message(fd, message, info);	
				return;				
			}
			else {
				//already logged
			}
		}
		else {
			char *message = "login_false;1;\n";
			send_message(fd, message, info);
		}		
	}
	else {
		cl = get_client_by_name(*array_clients, name);
		if (cl -> state == 2) {
			reconnect(array_clients, all_games, info, name, fd, tok, max_players);									
		}
		else {
			char *message = "login_false;2;\n";
			send_message(fd, message, info);
		}								
	} 
	return;
}
	
/*
 * Client reconnected, sending board of game
 * @param array_clients - array of logged clients
 * @param all_games - array of all games
 * @param info - structures to save log info
 * @param name - name of client
 * @param fd - client socket
 * @param tok - string send by client containing login info
 * @param max_players - number saying how many client can be logged at one time
 */
void reconnect(clients **array_clients, games *all_games, log_info **info, char *name, int fd, char *tok, int max_players) {
	client *cl = get_client_by_name(*array_clients, name);
								
	char board[1024];
	game *game = find_game_by_name(all_games, name);

	if(game == NULL) {
		set_state(&cl, 0);

		if (player_wanna_play(wanna_plays, cl)) {
			set_state(&cl, 1);
			char *message = "already_wanna_play;\n";
			send_message(fd, message, info);
		}

		char *message = "login_ok;\n";
		send_message(fd, message, info);

		set_socket_ID(&cl, fd);
		pthread_create(&(cl -> client_thread), NULL, &check_connectivity, &cl); 
		return;
	}
	set_socket_ID(&cl, fd);
	pthread_create(&(cl -> client_thread), NULL, &check_connectivity, &cl); 

	char now_playing[10];
	if (strcmp(game -> now_playing, name) == 0) { 
		set_state(&cl, 3);
		strcat(now_playing, "you");  
	}
	else {
		set_state(&cl, 4);
		strcat(now_playing, "opponent");
	}
	
	sprintf(board, "board;%s;%s;%d;%d;%s;", name, cl -> color, game -> game_ID, game -> fields -> count_pieces, now_playing);					
					
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

	if (strcmp(game -> name_1, name) == 0) { 
		send_message(get_client_by_name(*array_clients, game -> name_2) -> socket_ID, "connection_restored;\n", info);
	}
	else {
		send_message(get_client_by_name(*array_clients, game -> name_1) -> socket_ID, "connection_restored;\n", info);	
	}
	return;
}


/*
 * Client want play a game
 * @param array_clients - array of logged clients
 * @param wanna_play - array of clients who wants to play a game
 * @param all_games - array of all games
 * @param info - structures to save log info
 * @param fd - client socket
 */
void play(clients **array_clients, wanna_play **wanna_plays, games **all_games, log_info **info, int fd) {

	client *cl = get_client_by_socket_ID(*array_clients, fd);
	if (cl == NULL) return;
	if (cl -> state != 0) return;

	client *second_client = NULL;
	add_wanna_play(wanna_plays, fd);
	set_state(&cl, 1);
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

		second_client = get_client_by_socket_ID(*array_clients, socket_ID_2);

		int r = rand() % 2;
		if (r == 0) {
			sprintf(message_1, "start_game;black;%d;\n", (*all_games) -> games_count);
			sprintf(message_2, "start_game;white;%d;\n", (*all_games) -> games_count);

			set_state(&cl, 4);
			set_state(&second_client, 3);

			set_color(&cl, "black");
			set_color(&second_client, "white");

			now_playing = second_client -> name; 		
		}
		else {
			sprintf(message_1, "start_game;white;%d;\n", (*all_games) -> games_count);
			sprintf(message_2, "start_game;black;%d;\n", (*all_games) -> games_count);

			set_state(&cl, 3);
			set_state(&second_client, 4);

			set_color(&cl, "white");
			set_color(&second_client, "black");			

			now_playing = cl -> name;
		}

		add_game(all_games, cl -> name, second_client -> name, now_playing);

		send_message(socket_ID_1, message_1, info);
		send_message(socket_ID_2, message_2, info);
	}
	return;
}


/*
 * Move method, process move client just realized in client
 * @param all_games - array of all games
 * @param array_clients - array of logged clients
 * @param info - structures to save log info
 * @param tok - text with info about processed move in client
 * @param fd - client socket
 */
void client_move(games **all_games, clients **array_clients, log_info **info, char *tok, int fd) {
	client *cl = get_client_by_socket_ID(*array_clients, fd);
	if (cl == NULL) return;
	if (cl -> state != 3) return;
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
	char *type = array[6];	
			
	printf("Move: GID=%d CP=%d,%d DP=%d,%d COLOR=%s TYPE=%s\n", game_ID, cp_row, cp_col, dp_row, dp_col, color, type);
							
	process_move(all_games, *array_clients, info, game_ID, cp_row, cp_col, dp_row, dp_col, color, type); 
	check_can_move(array_clients, all_games, info, game_ID, cp_row, dp_row, color, type);

	return;
}


/*
 * Delete connection of client
 * @param array_clients - array of logged clients
 * @param wanna_play - array of clients who wants to play a game
 * @param client_socks - array of client sockets
 * @param fd - client socket
 */
void delete_connection(clients **array_clients, wanna_play **wanna_plays, fd_set *client_socks, int fd) {
	close(fd);
	FD_CLR(fd, client_socks);
	client_remove(array_clients, wanna_plays, fd);
	return;
}

/*
 * Disconnect method, when client disconnect but not timeout-ed
 * @param array_clients - array of logged clients
 * @param info - structures to save log info
 * @param all_games - array of all games
 * @param fd - client socket
 */
void disconnect(clients **array_clients, log_info **info, games *all_games, int fd) {
	client *cl = get_client_by_socket_ID(*array_clients, fd);

	if (cl == NULL) return;
	if (cl -> state == 2) return;
	
	set_state(&cl, 2);

	char *second_client_name;
	game *game = find_game_by_name(all_games, cl -> name);
	if (game == NULL) return;
		
	if (strcmp(game -> name_1, cl -> name) == 0) {
		second_client_name = game -> name_2;
	} 
	else {
		second_client_name = game -> name_1;
	}
	
	printf("Opponent with socket ID %d disconnect\n", fd);
	send_message(get_client_by_name(*array_clients, second_client_name) -> socket_ID, "opponent_connection_lost;\n", info);
		
	return;
}

/*
 * Delete method, when client got timeout or left (in game/not in game)
 * @param array_clients - array of logged clients
 * @param wanna_play - array of clients who wants to play a game
 * @param client_socks - array of client sockets
 * @param all_games - array of all games
 * @param info - structures to save log info
 * @param fd - client socket
 * @param err_ID - error ID
 */
void delete(clients **array_clients, wanna_play **wanna_plays, fd_set *client_socks, games **all_games, log_info **info, int fd, int err_ID) {	
	client *cl = get_client_by_socket_ID(*array_clients, fd);
	if (cl == NULL) {
		return;
	}
	printf("name: %s, id: %d\n", cl -> name, cl -> socket_ID);	
	game *gm = find_game_by_name(*all_games, cl -> name);	
	char message[30];

	switch (err_ID) {
		case 0:
			printf("Client with socket %d timeout\n", fd);
			break;
		case 1:
			printf("Client with socket %d left\n", fd);
			break;
	}
	if (gm == NULL) {
		printf("game null\n");
		delete_connection(array_clients, wanna_plays, client_socks, fd);
		return;
	}
	printf("Game id %d\n", gm -> game_ID);
	char *second_client_name;

	if (strcmp(gm -> name_1, cl -> name) == 0) {
		second_client_name = gm -> name_2;
	} 
	else {
		second_client_name = gm -> name_1;
	}
	client *sec_cl = get_client_by_name(*array_clients, second_client_name);
	set_state(&sec_cl, 0);
	switch (err_ID) {
		case 0:
			sprintf(message, "end_game_timeout;11;\n");
			printf("Game with id %d removed due to client timeout\n", gm -> game_ID);
			break;
		case 1:
			sprintf(message, "end_game_left;10;\n");
			printf("Game with id %d removed due to client left\n", gm -> game_ID);
			break;
	}	
	send_message(sec_cl -> socket_ID, message, info);
	remove_game(array_clients, all_games, info, gm -> game_ID);
	delete_connection(array_clients, wanna_plays, client_socks, fd);

	return;
}

/*
 * Log info into file
 * @param filename - name of file
 * @param info - structures to save log info
 */
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
	return;
}


/*
 * Calculate in minutes, how long server ran (in minutes)
 * @param start - start time of server
 * @param end - end time of server
 * @param info - structures to save log info
 */
void server_running(struct timeval start, struct timeval end, log_info **info) {
	(*info) -> server_running_minutes = (end.tv_sec - start.tv_sec) / 60;
	return;
}


/*
 * Print info into console how to write parameters for server
 */
void game_info(){
	printf("Info about draughts\n");
	printf("-address || -a [IPv4] : set listening address (valid IPv4), default INADDR_ANY\n");
	printf("-port || -p [number] : set listening port (1024-65535), default 10000\n");
	printf("Example: ./server.exe -a 127.0.0.1 -p 10000\n\n");
}


/*
 * Check if input string contains semicolon between index 1 - 20
 * @param cbuf - input string to be checked
 * @return 0 if not contains, 1 if contains
 */
int check_if_contains_semicolon(char *cbuf) {
	char *e;
	int index;
	e = strchr(cbuf, ';');
	if (e == NULL) return 0;
	index = (int)(e - cbuf);
	if (index > 20 || index <= 0) return 0;
	else return 1;
}

void *check_connectivity(void *args) {
	client **cli = (client**) args;	
	int socket_ID = (*cli) -> socket_ID;
	char name[30];
	strcpy((*cli) -> name, name);
	client *cl = NULL;
	int disconnected_time = 0;
	while(1) {
		cl = get_client_by_socket_ID(array_clients, socket_ID);
		printf("Socket ID: %d\n", socket_ID);
		if (cl == NULL) {
			printf("Client with socket ID %d is null, deleting thread\n", socket_ID);	
			break;
		}
		if (strcmp(name, cl -> name) != 0) break;

		printf("Client with socket ID %d connected: %d\n", socket_ID, cl -> connected);
		if (cl -> connected == 1) {
			set_connected(&cl, 0);
			disconnected_time += 0;
			set_disconnected_time(&cl, disconnected_time);
		}
		else {
			disconnect(&array_clients, &info, all_games, socket_ID);
			
			if (cl -> disconnected_time == MAX_TIME_DISCONNECT) {
				delete(&array_clients, &wanna_plays, &client_socks, &all_games, &info, socket_ID, 0);
				break;
			}
			
			disconnected_time += 15;
			set_disconnected_time(&cl, disconnected_time);
			
		}
		send_message_no_info(cl -> socket_ID, "is_connected;\n");
		
		sleep(15);
	}
	pthread_exit(0);
}
