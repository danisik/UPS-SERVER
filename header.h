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

typedef enum { 
	IN_LOBBY, 		//0
	WANNA_PLAY,		//1
	DISCONNECT,		//2
	YOU_PLAYING,		//3
	OPPONENT_PLAYING	//4
} STATES;

typedef struct the_client client;
typedef struct the_clients clients;
typedef struct the_piece piece;
typedef struct the_field field;
typedef struct the_fields fields;
typedef struct the_wanna_play wanna_play;
typedef struct the_game game;
typedef struct the_games games;
typedef struct the_log_info log_info;

struct the_client {
	char *name;
	int socket_ID;
	char color[20];
	STATES state;
	pthread_t client_thread;
	int connected; //0 false, 1 true
	int disconnected_time;
};

struct the_clients {
	int clients_count;
	client **clients;
};

struct the_piece {
	char *color;
	char *type;
};

struct the_field {
	int row;
	int col;
	char *color;
	piece *piece;
};

struct the_fields {
	int size;
	field ***all_fields;
	int count_pieces;
};

struct the_wanna_play {
	int size;
	int *socket_IDs;
};

struct the_game {
	char *name_1;
	char *name_2;
	char *now_playing;
	int game_ID;
	fields *fields;
};


struct the_games {
	int games_count;
	game **games;
};

struct the_log_info {
	int count_bytes; 
	int count_messages; 
	int count_connections;
	int count_bad_transmissions;
	int server_running_minutes;
};


//server.c
int name_exists (clients *array_clients, char *name);
void send_message(int client_socket, char *message, log_info **info);
void send_message_no_info(int client_socket, char *message);

void login(clients **array_clients, games *all_games, log_info **info, char *tok, int max_players, int fd);
void reconnect(clients **array_clients, games *all_games, log_info **info, char *name, int fd, char *tok, int max_players);
void play(clients **array_clients, wanna_play **wanna_plays, games **all_games, log_info **info, int fd);
void client_move(games **all_games, clients **array_clients, log_info **info, char *tok, int fd);
void delete_connection(clients **array_clients, wanna_play **wanna_plays, fd_set *client_socks, int fd);
void log_all(char *filename, log_info *info);
void server_running(struct timeval start, struct timeval end, log_info **info);
void disconnect(clients **array_clients, log_info **info, games *all_games, int fd);
void delete(clients **array_clients, wanna_play **wanna_plays, fd_set *client_socks, games **all_games, log_info **info, int fd, int err_ID);
void game_info();
int check_if_contains_semicolon(char *cbuf);
void *check_connectivity(void *args);

//client.c
void create_clients(clients **array_clients);
void create_client(client **cl, char *name, int socket_ID);
void add_client(clients **array_clients, char *name, int socket_ID);
void client_remove(clients **array_clients, wanna_play **wanna_plays, int socket_ID);
client *get_client_by_socket_ID(clients *array_clients, int socket_ID);
client *get_client_by_name(clients *array_clients, char *name);
void set_state(client **client, int state);
void set_socket_ID(client **cl, int socket_ID);
void set_color(client **client, char *color);
void set_connected(client **cl, int connected);
void set_disconnected_time(client **cl, int disconnected_time);

//conditions.c
int check_can_kill(games **all_games, int game_ID, char *color, char *type);
int check_if_can_kill(fields *fields, int cp_row, int cp_col, char *color, char *type);
void send_all_no_kill(clients **all_clients, games **all_games, log_info **info, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name);
void send_all_kill(clients **all_clients, games **all_games, log_info **info, int game_ID, int cp_row, int cp_col, int middle_row, int middle_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type);

int all_first_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color);
int all_second_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color);
int king_first_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color);
int king_second_move_no_kill(games **all_games, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color);

int all_first_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color);
int all_second_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color);
int king_first_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color);
int king_second_move_kill(games **all_games, int game_ID, int first_position, int second_position, int cp_row, int cp_col, int dp_row, int dp_col, char *color);

void check_if_can_promote(games **all_games, log_info **info, int game_ID, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *color, char *type, int cp_row, int cp_col);
int check_if_opponent_have_pieces(games **all_games, int game_ID, char *color);
int check_if_can_move(games **all_games, int game_ID, int first_position, int second_position, char *color, char *type);

//game.c
void create_wanna_play(wanna_play **wanna_plays);
void add_wanna_play(wanna_play **wanna_plays, int socket_ID);
void remove_wanna_play(wanna_play **wanna_plays, int socket_ID);
int player_wanna_play(wanna_play *wanna_plays, client *cl);

void create_games(games **all_games);
void inicialize_pieces(fields **fields, char *color, int row, int col);
void create_fields(game **gm);
void create_game(game **gm, char *name_1, char *name_2, char *now_playing);
void add_game(games **all_games, char *name_1, char *name_2, char *now_playing);
void remove_game(clients **cls, games **all_games, log_info **info, int game_ID);

void process_move(games **all_games, clients *clients, log_info **info, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, char *color, char *type);
int switch_no_kill(clients **all_clients, int value, games **all_games, log_info **info, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name);
int switch_kill(clients **all_clients, int value, games **all_games, log_info **info, int game_ID, int first_position, int cp_row, int cp_col, int dp_row, int dp_col, int curr_pl_socket_ID, int sec_pl_socket_ID, char *sec_pl_name, char *color, char *type);
game *find_game_by_name(games *all_games, char *name);

void end_game(clients **cls, games **all_games, int game_ID, int status, int status_opponent, int current_player_socket_ID, int second_player_socket_ID, log_info **info);
void check_can_move(clients **all_clients, games **all_games, log_info **info, int game_ID, int cp_row, int dp_row, char *color, char *type);
