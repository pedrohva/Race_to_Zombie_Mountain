#include "zombiemountain.h"

/**
 * Removes all keyboard input sitting in the buffer in order to prevent unexpected commands when switching states
 **/
void purge_input_buffer() {
	timer_pause(500);
	while ( get_char() >= 0 ) { /* loop until no chars buffered */ }
}

/**
 * Deallocate memory assigned to some of our globals
 **/
void free_memory() {
	free(terrain);
	free(hazards);
	free(road);
	free(road_x_coords);
}

/**
 * Checks if the given coordinates are offroad. Takes width into account
 **/
bool offroad(int x, int y, int width) {
	// Check if to the left of the road
	if((x+width-1) < road_x_coords[y]) {
		return true;
	}

	// Check if to the right of the road
	if(x > (road_x_coords[y]+ROAD_WIDTH-1)) {
		return true;
	}

	return false;
}

/**
 * Check if the car is offroad
 **/
bool car_offroad() {
	if(sprite_x(player) < road_x_coords[(int)sprite_y(player)]) {
		return true;
	}

	if((sprite_x(player) + PLAYER_WIDTH - 1) > (road_x_coords[(int)sprite_y(player)] + ROAD_WIDTH)) {
		return true;
	}

	return false;
}

/**
 * Checks if the car is next to a fuel station while travelling below the specified speed. 
 **/
void check_refuel() {
	bool valid_location = false;
	// Check if the player is to the left of the fuel station
	if((sprite_x(player) + sprite_width(player)) == sprite_x(fuel_station) && (sprite_y(fuel_station) == sprite_y(player))) {
		valid_location = true;
	}
	// Check if the player is to the right of the fuel station
	if((sprite_x(fuel_station) + sprite_width(fuel_station)) == sprite_x(player) && (sprite_y(fuel_station) == sprite_y(player))) {
		valid_location = true;
	}

	bool ready_to_refuel = false;
	// Check if the player is travelling below a speed of 2 in order to begin refuelling
	if(valid_location) {
		if(speed < 3) {
			ready_to_refuel = true;
		}
	}

	if(ready_to_refuel) {
		refuelling = true;
		refuel_timer = create_timer(3000);
		speed = 0;
	}
}

/**
 * Refuels the car if possible making sure that the player has remained stationary for 3 seconds
 **/
void refuel() {
	if(!refuelling) {
		check_refuel();
	} else {
		// Cancel refuelling if the car starts moving again
		if(speed > 0) {
			refuelling = false;
		}

		// Check if player has remained still for 3 seconds
		if(timer_expired(refuel_timer)) {
			refuelling = false;
			fuel = MAX_FUEL;
			speed = 1;
			destroy_timer(refuel_timer);
		}
	}
}

/**
 * Checks if the coordinates given are in bounds of the game area (excludes the dashboard from the area)
 **/
bool in_bounds(int x, int y) {
	if((x <= DASHBOARD_SIZE) || (x >= screen_width()-2)) {
		return false;
	}

	if((y <= 1) || (y >= screen_height()-2)) {
		return false;
	}

	return true;
}

/**
 * Add the player to the middle of the road again
 **/
void reset_player_location() {
	// Setup the car at the bottom of the screen, middle of road
	int y = screen_height() - PLAYER_HEIGHT - 2;
	int x = (ROAD_WIDTH / 2) + road_x_coords[y] - (PLAYER_WIDTH/2) + 1;

	player->x = x;
	player->y = y;
}

/**
 * Place the player's car sprite in the middle of the road and gives it full health
 **/
void setup_player_car() {
	int y = screen_height() - PLAYER_HEIGHT - 2;
	int x = (ROAD_WIDTH / 2) + road_x_coords[y] - (PLAYER_WIDTH/2) + 1;
	player = sprite_create(x, y, PLAYER_WIDTH, PLAYER_HEIGHT, get_car_image());
	car_condition = 100;

	// Setup fuel settings 
	fuel = MAX_FUEL;
	refuelling = false;
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
	setup_obs();

	// Get all current highscores from the highscore file
	get_hscores();
	
	// Setup the car at the bottom of the screen
	setup_player_car();

	// Initialise the speed settings
	speed = 0;
	speed_ctr = 0;

	score = 0;
	game_over_loss = false;

	game_start_time = get_current_time();
	distance_counter = 0;
	distance_travelled = 0;

	// Start the speed timer
	speed_timer = create_timer(SPEED_INTERVAL);
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

	// Check if the car will be in bounds
	// Check bounded by left border
	int newX = sprite_x(player) + dx;
	if(!in_bounds(newX, sprite_y(player))) {
		dx = 0;
	}
	// Check bounded by right border
	newX += PLAYER_WIDTH/2 + 2;
	if(!in_bounds(newX, sprite_y(player))) {
		dx = 0;
	}

	// Create a temporary sprite to check if car will collide
	sprite_id temp_sprite = sprite_create(sprite_x(player)+dx, sprite_y(player), PLAYER_WIDTH, PLAYER_HEIGHT, get_car_image());
	if(check_collision(temp_sprite)) {
		dx = 0;
	}
	sprite_destroy(temp_sprite);

	// Check if car is stationary and should be allowed to move
	if(speed > 0) {
		sprite_move(player, dx, 0);
	}
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

	// Decide on the max speed the car can reach
	int max_speed = MAX_SPEED;
	if(car_offroad()) {
		max_speed = MAX_SPEED_OFFROAD;
	}

	// Check if the speed will fall in bounds
	if(((speed + dv) <= max_speed) && (speed + dv) >= 0) {
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
 * Updates the speed_ctr if the timer has passed a certain limit. 
 * Will return true if it is time to update the game logic
 **/
bool update_speed_ctr() {
	// Update the speed counter. We also check if it would update while we pause the execution with
	// the loop timer. Added 10 to the loop interval in case update and draw takes 17 ms to execute
	if(speed_timer->milliseconds > (SPEED_INTERVAL - LOOP_INTERVAL - 10)) {
		speed_ctr++;
	}

	// How fast the screen scrolls (can be negative). Higher value the faster
	int speed_rate = -1;

	// Check if we can update the game logic
	if((MAX_SPEED - speed - speed_rate < speed_ctr) && (speed > 0) && (fuel > 0)) {
		return true;
	}

	return false;
}

/**
 * Changes the speed to zero, reduces car condition and resets the player to the middle of the road
 **/
void handle_collision() {
	speed = 0;
	fuel = MAX_FUEL;
	car_condition -= 20;
	if(car_condition <= 0) {
		game_over_loss = true;
		change_state(GAME_OVER_SCREEN);
	}
	reset_player_location();
	// Remove any hazards in the way 
	for(int i=0; i<max_hazards; i++) {
		if(check_sprite_collided(player, hazards[i])) {
			hazard_reset(i);
		}
	}
}

/**
 * Updates the distance travelled
 **/
void update_distance() {
	distance_counter++;
	// Updates the distance if enough ticks have passed to cover 1 meter
	if(distance_counter > 5) {
		distance_travelled++;
		distance_counter = 0;
		fuel -= 2;
	}
}

/**
 * Updates the score based on the distance travelled, car condition and time
 **/
void update_score() {
	// Update the score
	score = ((distance_travelled * 10) - (car_condition)) - (get_current_time() - game_start_time) + 90;
	if(score <= 0) {
		score = 1;
	} else if(score > 999999) {
		score = 999999;
	}
}

/**
 * Code that updates the logic of the game relevant to the Game state
 **/
void update_game_screen() {
	handle_input();

	// Decides when to update the game (if enough time has speed depending on the speed)
	if(update_speed_ctr()) {
		even_stripe = !even_stripe;
		update_distance();
		// Check if the car has collided with an obstacle
		if(check_collision(player)) {
			// Check if the car has collided with a fuel station
			if(check_sprite_collided(player,fuel_station)) {
				game_over_loss = true;
				change_state(GAME_OVER_SCREEN);
			} else {
				handle_collision();
			}
		}
		update_obs();
		speed_ctr = 0;
	}

	// Refuel the car if all criteria are met
	refuel();

	// If the car is offroad, set its speed to the maximum offroad speed
	if(car_offroad() && (speed > MAX_SPEED_OFFROAD)) {
		speed = MAX_SPEED_OFFROAD;
	}
	
	// Check if the player has won the game
	if((sprite_y(player) + sprite_height(player)) < sprite_y(finish_line)) {
		game_over_loss = false;
		change_state(GAME_OVER_SCREEN);
	}

	update_score();

	// Check if the player has run out of fuel
	if(fuel <= 0) {
		game_over_loss = true;
		change_state(GAME_OVER_SCREEN);
	}
}

/**
 * Updates the game over screen. Will pretty much just wait till the user presses a button to move to the Highscore 
 * screen
 **/
void update_game_over_screen() {
	// Check if the user has achieved a new highscore
	if(check_new_hscore()) {
        // Get the user's name
        char name[MAX_NAME_SIZE] = "";

        // The current letter we're in
        int index = 0;
        // Get the characters from the user for the name
        bool done = false;
        while((index < (MAX_NAME_SIZE-1)) && (!done)) {
            char letter = get_char();
            // Stop if the user presses ENTER
            if(letter == 10) {
				// If the user didn't type anything
				if(index == 0) {
					strcpy(name, "Anonymous");
				}
                done = true;
            } else {
                if((letter > 32) && (letter < 127)) {
                    // Append the letter to the current name
                    name[index] = letter;
                    index++;
                }
            }
        }

        process_hscore(name);
		sort_scores();
		save_scores();
		change_state(HIGHSCORE_SCREEN);
    } else {
		// Wait for the user to press a key if no game over was announced
		if(get_char() >= 0) {
			change_state(HIGHSCORE_SCREEN);
		}
	}
}

/**
 * Updates the highscore screen by allowing the player to either play the game again or quit
 **/
void update_highscore_screen() {
    char key = get_char();

	switch(key) {
		case 'p':
		case 'P':
			change_state(GAME_SCREEN);
			break;
		case 's':
		case 'S':
			change_state(START_SCREEN);
			break;
		case 'q':
		case 'Q':
			change_state(EXIT_SCREEN);
			break;
		default:
			break;
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
		case GAME_OVER_SCREEN:
			update_game_over_screen();
			break;
		case HIGHSCORE_SCREEN:
			update_highscore_screen();
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
	
	// The y coordinate of where we'll print the how to play text
	int y = (screen_height() / 2) - 3;
	// The x coordinate of where we'll print the controls
	int x = screen_width() - 32;
	draw_string(3, y, "INSTRUCTIONS");
	draw_string(x, y, "CONTROLS");
	y++;
	draw_string(3, y, "Reach the finish line");
	draw_string(x, y, "a/d : Move Left/Right");
	y++;
	draw_string(3, y, "Collisions reduce car condition");
	draw_string(x, y, "w/s : Accelerate/Decelerate");
	y++;
	draw_string(3, y++, "Game over if car condition is 0, ");
	draw_string(3, y++, "collides with fuel station or ");
	draw_string(3, y++, "runs out of fuel");
	draw_string(3, y++, "Drive with low speed next to fuel station to refuel");
	draw_center_text("Press any key to play...", screen_height() - 5);
	draw_center_text("Pedro Alves - n9424342", screen_height() - 2);
}

/**
 * Find how much time there is left until the car finishes refuelling
 **/
double refuel_time_left() {
	// The time left to refuel
	double time_left = 3.0 - (get_current_time() - refuel_timer->reset_time);
	if(time_left > 3.0) {
		time_left = 3.0;
	}
	return time_left;
}

/**
 * Draw the borders and all information that we want displayed on the dashboard
 **/
void draw_dashboard() {
	// Draw the borders
	for(int y=1; y<screen_width()-1; y++) {
		draw_char(dashboard_x, y, DASHBOARD_BORDER_CHAR);
	}

	 draw_string(2, 2, "Telemetry");
	// Draw the speed stat
	draw_string(2, 3, "Speed");
	draw_int(12, 3, speed);
	// Draw the fuel stat
	draw_string(2, 4, "Fuel");
	draw_int(12, 4, fuel);
	// Draw the condition stat
	draw_string(2,5,"Condition");
	draw_int(12,5,car_condition);

	draw_string(2, 7, "Stats");
	// The current score of the player
	draw_string(2, 8, "Score");
	draw_int(12, 8, score);
	// The distance travelled since the start of the game
	draw_string(2, 9, "Distance");
	draw_int(12, 9, distance_travelled);
	// Draw the time elapsed since game started
	draw_string(2, 10, "Time");
	draw_double(12, 10, get_current_time() - game_start_time);

	// Draw warning stating that the car is offroad
	if(car_offroad()) {
		draw_string(2, 12, "OFFROAD");
	}

	// Draw warning saying we're refuelling
	if(refuelling) {
		draw_string(2, 13, "REFUELLING");
		draw_double(2, 14, refuel_time_left());
	} else if(fuel < (MAX_FUEL/4)) {
		draw_string(2, 13, "LOW FUEL");
	}
}

/**
 * Draw the game screen
 **/
void draw_game_screen() {
	draw_dashboard();
	draw_obs();

	sprite_draw(player);
}

/**
 * Draw the game over screen
 **/
void draw_game_over_screen() {
	if(game_over_loss) {
		draw_center_text("GAME OVER", screen_height() / 2);
	} else {
		draw_center_text("YOU WIN!", screen_height() / 2);
	}
	char score_text[50];
	sprintf(score_text, "Your score was: %d", score);
	draw_center_text(score_text, (screen_height() / 2) + 1);
	if(check_new_hscore()) {
		draw_center_text("High Score!!", (screen_height() / 2) + 4);
		draw_center_text("Type your name and press Enter", (screen_height() / 2) + 5);
	} else {
		draw_center_text("Press any key to continue", screen_height()-2);
	}
}

/**
 * Draws the highscore screen that will be visible to the player
 **/
void draw_highscore_screen() {
    draw_center_text("HIGHSCORES", 2);
    draw_hscores();
	draw_center_text("(P)lay again , (S)tart screen, (Q)uit", screen_height() - 2);
}

/**
 * Draw every visible entity of the game and all UI elements belonging to the current screen
 **/
void draw() {
	clear_screen();

	// Draw the current screen
	switch(game_state) {
		case START_SCREEN:
			draw_start_screen();
			break;
		case GAME_SCREEN:
			draw_game_screen();
			break;
		case GAME_OVER_SCREEN:
			draw_game_over_screen();
			break;
		case HIGHSCORE_SCREEN:
			draw_highscore_screen();
			break;
		default:
			break;
	}

	draw_borders();
	show_screen();
}

/**
 * The entry point to the program
 **/
int main( void ) {
	// Setup the ZDK screen. Alwas do this first
	setup_screen();

	// Setup all of the images to be used on the sprites
	imagemngr_init();

	change_state(START_SCREEN);

	// Seed our random number generator 
	srand(get_current_time());

	// The timer to check how long it takes to execute one iteration of game loop. 
	// Set the interval to 17 so the game runs at ~60 fps
	timer_id loop_timer = create_timer(LOOP_INTERVAL);

	// Setup the obstacle arrays 
	init_obs();

	// Start the main game loop
	while(game_state != EXIT_SCREEN) {
		update();
		draw();

		// Wait for a bit in order to not go above 20fps
		while(!timer_expired(loop_timer)) { }	
	}

	cleanup_screen();
	free_memory();
	return 0;
}
