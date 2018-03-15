#include <string.h>
#include "cab202_graphics.h"
#include "cab202_timers.h"
#include "cab202_sprites.h"

// Define the border character as a full stop (.)
#define BORDER_CHAR	46

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
 * Changes the current screen and the current game state. Will also initialise all that is required to make 
 * that state run properly
 **/
void change_state(int new_state) {
	purge_input_buffer();
	game_state = new_state;
}

void update_start_screen() {
	// If the user presses any key, start the game
	if(get_char() > 0) {
		change_state(GAME_SCREEN);
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

	// Start the main game loop
	while(game_state != EXIT_SCREEN) {
		update();
		draw();
	}

	cleanup_screen();
	return 0;
}