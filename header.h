typedef struct the_client client;
typedef struct the_clients clients;
typedef struct the_field field;
typedef struct the_wanna_play wanna_play;
typedef struct the_game game;
typedef struct the_games games;

struct the_client {
	char *name;
	int socket_ID;
	char *color;
};

struct the_clients {
	int clients_count;
	client **clients;
};

struct the_field {
	int row;
	int col;
	char *color;
};

struct the_wanna_play {
	int size;
	int *socket_IDs;
};

struct the_game {
	int size;
	int first_player_socket_ID;
	int second_player_socket_ID;
	field **fields;
};


struct the_games {
	int games_count;
	game **games;
};

//client.c
void create_clients(clients **array_clients);
void create_client(client **cl, char *name, int socket_ID);
void add_client(clients **array_clients, char *name, int socket_ID);
void client_remove(clients **array_clients, wanna_play **wanna_plays, int socket_ID);
void set_color(clients **array_clients, int socket_ID, char *color);

//game.c
void create_wanna_play(wanna_play **wanna_plays);
void add_wanna_play(wanna_play **wanna_plays, int socket_ID);
void remove_wanna_play(wanna_play **wanna_plays, int socket_ID);

void create_games(games **all_games);
void create_game(game **gm, int first_player_socket_ID, int second_player_socket_ID);
void add_game(games **all_games, int first_player_socket_ID, int second_player_socket_ID);
void remove_game();
void restore_game();

//server.c
int name_exists (clients *array_clients, char *name);
void send_message(int client_socket, char *message);

//free.c
void free_client();
void free_clients();
void free_field();
void free_fields();
void free_wanna_play();
void free_game();
void free_games();

