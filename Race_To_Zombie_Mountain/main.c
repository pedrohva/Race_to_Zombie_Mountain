#include <stdlib.h>
#include <string.h>
#include "cab202_graphics.h"
#include "cab202_timers.h"
#include "cab202_sprites.h"

// Define the border character as a full stop (.)
#define BORDER_CHAR	46

// Define the player car's details
#define PLAYER_WIDTH	8
#define PLAYER_HEIGHT	5

// Define the direction the road is turning.
#define ROAD_STRAIGHT	1
#define ROAD_RIGHT		2
#define ROAD_LEFT		3

// The interval of the speed timer and loop timer
#define SPEED_INTERVAL	87
#define LOOP_INTERVAL	17

// The maximum speed the player can increase the car and thus the speed of updates in the game
#define MAX_SPEED			10
#define MAX_SPEED_OFFROAD	3

// Define the image representing the player's car as well as the sprite
char * car_image =
	"   /\\   "
	"[]-||-[]"
	"   ||   "
	"[]-||-[]"
	"  ----  ";

sprite_id player;

// The speed of the player. This controls how long it takes for 
int speed;
// A timer that controls how fast the game updates (thus setting the speed)
timer_id speed_timer;
// Helps count how many ticks have passed from the speed_timer to help decide if we should update
int speed_ctr;

// How many units the road stretches from the bottom of the screen to the top
int road_length;
// How many units the road stretches horizontally from its left to the its right boundary
int road_width;
// Decides if the odd or even on the y coord road sections will contain a middle stripe
bool even_stripe;
// The array which will hold all of the road sections
int *road;
// The array which will hold the x coordinate of the road sections
int *road_x_coords;

/**
 * Holds information regarding what screen the player should be seeing right now. The state should
 * only be changed through the function change_state()
 **/
enum GameScreens {
	START_SCREEN,
	GAME_SCREEN,
	GAME_OVER_SCREEN,
	EXIT_SCREEN
} game_state;

/**
 * Removes all keyboard input sitting in the buffer in order to prevent unexpected commands when switching states
 **/
void purge_input_buffer() {
	timer_pause(500);
	while ( get_char() >= 0 ) { /* loop until no chars buffered */ }
}

/**
 * Get the right image for the road depending on its type
 **/
char get_road_image(int type) {
	char image = ' ';

	switch(type) {
		case ROAD_STRAIGHT:
			image = '|';
			break;
		default:
			break;
	}

	return image;
}

/**
 * Create a road section of a certain type at the specified location
 **/
void add_road_section(int x, int y, int type) {
	road[y - 1] = type;
	road_x_coords[y - 1] = x;
}

/**
 * Starts the gma
 **/
void setup_game_state() {
	// Set up the road
	road_length = screen_height() - 2;	// There should be borders at the top and bottom of the screen
	road_width = (PLAYER_WIDTH * 2) + (PLAYER_WIDTH * 0.5);
	int road_x = (screen_width() - road_width - 1) * 0.5;
	even_stripe = true;
	road = malloc(road_length * sizeof(int));
	road_x_coords = malloc(road_length * sizeof(int));

	// Build the individual road sections
	for(int i=0; i<road_length; i++) {
		add_road_section(road_x, i + 1, ROAD_STRAIGHT);
	}

	// Setup the car at the bottom of the screen, middle of road
	// TODO change the car location to be in middle of road instead of screen
	int x = (screen_width() - PLAYER_WIDTH) / 2;
	int y = screen_height() - PLAYER_HEIGHT - 2;

	player = sprite_create(x, y, PLAYER_WIDTH, PLAYER_HEIGHT, car_image);

	// Initialise the speed settings
	speed = 1;
	speed_ctr = 0;
}

/**
 * Changes the current screen and the current game state. Will also initialise all that is required to make 
 * that state run properly
 **/
void change_state(int new_state) {
	purge_input_buffer();

	// Decide if we need to initiate the state before switching to it
	switch(new_state) {
		case GAME_SCREEN:
			setup_game_state();
			break;
		default:
			break;
	}

	game_state = new_state;
}

/**
 * Code that updates the logic of the game relevant to the Start state
 **/
void update_start_screen() {
	// If the user presses any key, start the game
	if(get_char() > 0) {
		change_state(GAME_SCREEN);
	}
}

/**
 * Code that updates the logic of the game relevant to the Game state
 **/
void update_game_screen() {
	if(MAX_SPEED - speed + 2 < speed_ctr) {
		even_stripe = !even_stripe;
		speed_ctr = 0;
	}
}

/**
 * Will step through one tick of the game logic in accordance with the game state and take into account any 
 * input from the user.
 **/
void update() {
	switch(game_state) {
		case START_SCREEN:
			update_start_screen();
			break;
		case GAME_SCREEN:
			update_game_screen();
			break;
		default:
			break;
	}
}

/**
 * Find the x coordinate needed to center a text in the terminal
 **/
int center_text_x_coord(char * text) {
	int text_size = strlen(text);
	int x = (screen_width()/2) - (text_size/2) - 1;

	return x;
}

/**
 * Draw a set of lines around the terminal screen
 **/
void draw_borders() {
	draw_line(0,0,screen_width()-1,0, BORDER_CHAR);
	draw_line(0,screen_height()-1,screen_width()-1,screen_height()-1, BORDER_CHAR);
	draw_line(0,0,0,screen_height()-1,BORDER_CHAR);
	draw_line(screen_width()-1,0,screen_width()-1,screen_height()-1,BORDER_CHAR);
}

/**
 * Draw the start screen
 **/
void draw_start_screen() {
	char * title = "Race to Zombie Mountain";
	int title_x = center_text_x_coord(title);
	int title_y = 3;
	draw_string(title_x,title_y,title);

	char * author = "Pedro Alves - n9424342";
	int author_x = center_text_x_coord(author);
	int author_y = screen_height() - 2;
	draw_string(author_x,author_y,author);
}

/**
 * Draw the game screen
 **/
void draw_game_screen() {
	// Draw the road
	for(int i=0; i<road_length; i++) {
		draw_char(road_x_coords[i], i + 1, get_road_image(road[i]));
		draw_char(road_x_coords[i] + road_length, i + 1, get_road_image(road[i]));
		if(((i+1) % 2 == 0) && (even_stripe)) {
			draw_char(road_x_coords[i] + (road_length * 0.5), i + 1, get_road_image(road[i]));
		} else if(((i+1) % 2 != 0) && !even_stripe) {
			draw_char(road_x_coords[i] + (road_length * 0.5), i + 1, get_road_image(road[i]));
		}
	}

	sprite_draw(player);
}

/**
 * Draw every visible entity of the game and all UI elements belonging to the current screen
 **/
void draw() {
	clear_screen();

	draw_borders();
	// Draw the current screen
	switch(game_state) {
		case START_SCREEN:
			draw_start_screen();
			break;
		case GAME_SCREEN:
			draw_game_screen();
			break;
		default:
			break;
	}

	show_screen();
}

/**
 * The entry point to the program
 **/
int main( void ) {
	setup_screen();

	change_state(START_SCREEN);

	// The timer to check how long it takes to execute one iteration of game loop. 
	// Set the interval to 17 so the game runs at ~60 fps
	timer_id loop_timer = create_timer(LOOP_INTERVAL);

	// Start the speed timer
	speed_timer = create_timer(SPEED_INTERVAL);

	// Start the main game loop
	while(game_state != EXIT_SCREEN) {
		update();
		draw();

		// Update the speed counter. We also check if it would update while we pause the execution with
		// the loop timer. Added 10 to the loop interval in case update and draw takes 17 ms to execute
		if(speed_timer->milliseconds > (SPEED_INTERVAL - LOOP_INTERVAL - 10)) {
			speed_ctr++;
		}

		// Wait for a bit in order to not go above 20fps
		while(!timer_expired(loop_timer)) { }	
	}

	cleanup_screen();
	return 0;
}