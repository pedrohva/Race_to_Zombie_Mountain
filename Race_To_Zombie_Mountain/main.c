#include <stdlib.h>
#include <string.h>
#include "cab202_graphics.h"
#include "cab202_timers.h"
#include "cab202_sprites.h"

// Define the border character as a full stop (.)
#define BORDER_CHAR	46

// The minimum width of the dashboard
#define DASHBOARD_SIZE	20

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

// Input keys
#define INPUT_MOVE_LEFT		'a'
#define INPUT_MOVE_RIGHT	'd'
#define INPUT_ACCELERATE	'w'		
#define INPUT_DECELERATE	's'

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

// The system time in the user's computer that the game started (game screen not start menu)
double game_start_time;
// A tick counter that decides if enough ground has been travelled to cover 1 meter
int distance_counter;
// The distance in meters travelled since the start of the game
int distance_travelled;

// The x-coordinate of the border of the dashboard
int dashboard_x;
char dashboard_border_char = '/';

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
 * Decides on the width and length of the road and adds the proper graphics to the array which
 * holds the road information
 **/
void setup_road() {
	road_length = screen_height() - 2;	// There should be borders at the top and bottom of the screen
	road_width = (PLAYER_WIDTH * 2) + (PLAYER_WIDTH * 0.5);
	int road_x = ((screen_width() - road_width - 1 + DASHBOARD_SIZE) * 0.5);
	even_stripe = true;
	road = malloc(road_length * sizeof(int));
	road_x_coords = malloc(road_length * sizeof(int));

	// Build the individual road sections
	for(int i=0; i<road_length; i++) {
		add_road_section(road_x, i + 1, ROAD_STRAIGHT);
	}
}

/**
 * Resets and setups the dashboard to display in a variety of window sizes
 */
void setup_dashboard() {
	dashboard_x = DASHBOARD_SIZE;
}

/**
 * Starts the game
 **/
void setup_game_state() {
	setup_dashboard();
	setup_road();

	// Setup the car at the bottom of the screen, middle of road
	int y = screen_height() - PLAYER_HEIGHT - 2;
	int x = (road_width / 2) + road_x_coords[y] - (PLAYER_WIDTH/2) + 1;

	player = sprite_create(x, y, PLAYER_WIDTH, PLAYER_HEIGHT, car_image);

	// Initialise the speed settings
	speed = 0;
	speed_ctr = 0;

	game_start_time = get_current_time();
	distance_counter = 0;
	distance_travelled = 0;
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
 * Updates the position of the car depending on what movement key was pressed
 **/
void handle_movement_input(int key) {
	int dx = 0;

	switch(key) {
		case INPUT_MOVE_LEFT:
			dx--;
			break;
		case INPUT_MOVE_RIGHT:
			dx++;
			break;
		default:
			break;
	}

	sprite_move(player, dx, 0);
}

/**
 * Updates the speed of the car depending on the speed change key that was pressed
 **/
void handle_speed_input(int key) {
	// The change in speed
	int dv = 0;

	switch(key) {
		case INPUT_ACCELERATE:
			dv++;
			break;
		case INPUT_DECELERATE:
			dv--;
			break;
	}

	if(((speed + dv) <= MAX_SPEED) && (speed + dv) >= 0) {
		speed += dv;
	}
}

/**
 * Gets a character from the input buffer and checks if it is valid. If it is, the appropriate 
 * function will be called to handle that input.
 **/ 
void handle_input() {
	int key = get_char();

	// Check if there's any input waiting to be processed
	if(key >= 0) {
		switch(key) {
			case INPUT_MOVE_LEFT:
			case INPUT_MOVE_RIGHT:
				handle_movement_input(key);
				break;
			case INPUT_ACCELERATE:
			case INPUT_DECELERATE:
				handle_speed_input(key);
				break;
		}
	}
}

/**
 * Code that updates the logic of the game relevant to the Start state
 **/
void update_start_screen() {
	// If the user presses any key, start the game
	if(get_char() >= 0) {
		change_state(GAME_SCREEN);
	}
}

/**
 * Code that updates the logic of the game relevant to the Game state
 **/
void update_game_screen() {
	handle_input();

	// Decides when to update the game (if enough time has speed depending on the speed)
	if(MAX_SPEED - speed + 2 < speed_ctr) {
		even_stripe = !even_stripe;

		distance_counter++;
		// Updates the distance if enough ticks have passed to cover 1 meter
		if(distance_counter > 5) {
			distance_travelled++;
			distance_counter = 0;
		}
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
 * Find the x coordinate needed to center a text in the terminal and then print it
 **/
void draw_center_text(char * text, int y) {
	int text_size = strlen(text);
	int x = (screen_width()/2) - (text_size/2) - 1;

	draw_string(x,y,text);
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
	draw_center_text("Race to Zombie Mountain", 3);
	draw_center_text("Pedro Alves - n9424342", screen_height() - 2);
}

/**
 * Draw the borders and all information that we want displayed on the dashboard
 **/
void draw_dashboard() {
	 // Draw the borders
	 for(int y=1; y<screen_width()-1; y++) {
		 draw_char(dashboard_x, y, dashboard_border_char);
	 }

	 // Draw the speed stat
	draw_string(2, 4, "Speed");
	draw_int(11, 4, speed);

	// Draw the time elapsed since game started
	draw_string(2, 3, "Time");
	draw_double(11, 3, get_current_time() - game_start_time);

	// The distance travelled since the start of the game
	draw_string(2, 2, "Distance");
	draw_int(11, 2, distance_travelled);
}

/**
 * Draws the road by using the graphic information from the 'road' array. Also decides where to draw
 * the center stripes
 **/
void draw_road() {
	for(int i=0; i<road_length; i++) {
		draw_char(road_x_coords[i], i + 1, get_road_image(road[i]));
		draw_char(road_x_coords[i] + road_length, i + 1, get_road_image(road[i]));
		if(((i+1) % 2 == 0) && (even_stripe)) {
			draw_char(road_x_coords[i] + (road_length * 0.5), i + 1, get_road_image(road[i]));
		} else if(((i+1) % 2 != 0) && !even_stripe) {
			draw_char(road_x_coords[i] + (road_length * 0.5), i + 1, get_road_image(road[i]));
		}
	}
}

/**
 * Draw the game screen
 **/
void draw_game_screen() {
	draw_dashboard();
	draw_road();

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