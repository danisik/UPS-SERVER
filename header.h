typedef struct the_client client;
typedef struct the_clients clients;
typedef struct the_piece piece;
typedef struct the_field field;
typedef struct the_fields fields;
typedef struct the_wanna_play wanna_play;
typedef struct the_game game;
typedef struct the_games games;

struct the_client {
	char *name;
	int socket_ID;
	char *color;
	char *state;
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
	int size; //10
	field ***all_fields;
};

struct the_wanna_play {
	int size;
	int *socket_IDs;
};

struct the_game {
	char *name_1;
	char *name_2;
	char *now_playing;
	fields *fields;
};


struct the_games {
	int games_count;
	game **games;
};


//server.c
int name_exists (clients *array_clients, char *name);
void send_message(int client_socket, char *message);

//client.c
void create_clients(clients **array_clients);
void create_client(client **cl, char *name, int socket_ID);
void add_client(clients **array_clients, char *name, int socket_ID);
void client_remove(clients **array_clients, wanna_play **wanna_plays, int socket_ID);
void set_color(clients **array_clients, int socket_ID, char *color);
void set_state(clients **array_clients, char *name, char *state);
char *get_name_by_socket_ID(clients *array_clients, int socket_ID);
int get_socket_ID_by_name(clients *array_clients, char *name);

//game.c
void create_wanna_play(wanna_play **wanna_plays);
void add_wanna_play(wanna_play **wanna_plays, int socket_ID);
void remove_wanna_play(wanna_play **wanna_plays, int socket_ID);

void create_games(games **all_games);
void inicialize_pieces(fields **fields, char *color, int row, int col);
void create_fields(game **gm);
void create_game(game **gm, char *name_1, char *name_2, char *now_playing);
void add_game(games **all_games, char *name_1, char *name_2, char *now_playing);
void remove_game();
void restore_game();

void process_move(games **all_games, clients *clients, int game_ID, int cp_row, int cp_col, int dp_row, int dp_col, char *color, char *type);
int check_if_can_kill(fields *fields, int cp_row, int cp_col, char *color, char *type);

//free.c
void free_client();
void free_clients();
void free_field();
void free_fields();
void free_wanna_play();
void free_game();
void free_games();

