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
 * Checks if there is any terrain colliding with the sprite. If invulnerable, clear the hazard on the road so that the car can
 * spawn
 **/
bool check_collision(sprite_id sprite, bool invulnerable) {
	bool collided = false;

	// The coordinates of the sprite (used to improve readability as the coords should not change in this function)
	int x = sprite_x(sprite);
	int y = sprite_y(sprite);
	int width = sprite_width(sprite);
	int height = sprite_height(sprite);

	// Iterate through the terrain to see if there was a collision
	for(int i=0; i<max_terrain_obs; i++) {
		// Check if the terrain has already been created (needed in order to check for collision when first creating 
		// the terrain objects, see create_terrain())
		if(terrain[i] != NULL) {
			int terrain_x = sprite_x(terrain[i]);
			int terrain_y = sprite_y(terrain[i]);
			int terrain_width = sprite_width(terrain[i]);
			int terrain_height = sprite_height(terrain[i]);

			// Check if there is colllision in the x-axis
			if(!((x + width <= terrain_x) || (x >= terrain_x + terrain_width))) {
				// Check if there is collision in the y-axis
				if(!((y + height < terrain_y) || (y > terrain_y + terrain_height))) {
					collided = true;
				}
			}
		}
	}

	// Iterate through the hazards to see if there was a collision
	for(int i=0; i<max_hazards; i++) {
		// Check if the terrain has already been created (needed in order to check for collision when first creating 
		// the terrain objects, see create_terrain())
		if(hazards[i] != NULL) {
			int hazard_x = sprite_x(hazards[i]);
			int hazard_y = sprite_y(hazards[i]);
			int hazard_width = sprite_width(hazards[i]);
			int hazard_height = sprite_height(hazards[i]);

			// Check if there is colllision in the x-axis
			if(!((x + width <= hazard_x) || (x >= hazard_x + hazard_width))) {
				// Check if there is collision in the y-axis
				if(!((y + height < hazard_y) || (y > hazard_y + hazard_height))) {
					collided = true;
					if(invulnerable) {
						hazard_reset(i);
					}
				}
			}
		}
	}

	// Check if there is any collision with the fuel station
	if(!((x + width <= sprite_x(fuel_station)) || (x >= sprite_x(fuel_station) + sprite_width(fuel_station)))) {
		// Check if there is collision in the y-axis
		if(!((y + height < sprite_y(fuel_station)) || (y > sprite_y(fuel_station) + sprite_height(fuel_station)))) {
			collided = true;
			// Destroy the player
			if(sprites_equal(sprite, player)) {
				car_condition = 0;
			}
		}
	}

	return collided;
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

	// Check for collision and clear the hazard on the road if there is any
	check_collision(player, true);
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
	int road_x = ((screen_width() - ROAD_WIDTH - 1 + DASHBOARD_SIZE) * 0.5);
	even_stripe = true;
	road = malloc(road_length * sizeof(int));
	road_x_coords = malloc(road_length * sizeof(int));

	// Build the individual road sections
	for(int i=0; i<road_length; i++) {
		add_road_section(road_x, i + 1, ROAD_STRAIGHT);
	}
}

/**
 * Moves a terrain to the top of the screen and changes the terrain type
 **/
void terrain_reset(int index) {
	// Choose a new terrain
	int id = rand() % NUM_TERRAIN_TYPES;
	int width = 0;
	int height = 0;
	char* image = get_image(id, TERRAIN, &width, &height);

	// Check if we'll place the terrain on the left or right side of the road
	bool left = rand() % 2;
	int x = -1;
	if(left) {
		int min_x = DASHBOARD_SIZE + 1;
		int max_x = road_x_coords[0] - width - 1;
		x = rand() % (max_x + 1 - min_x) + min_x;
	} else {
		int min_x = road_x_coords[0] + ROAD_WIDTH + 1;
		int max_x = screen_width() - 2 - width;
		x = rand() % (max_x + 1 - min_x) + min_x;
	}

	// Place the terrain just above the screen so it scrolls as into view as soon as reset
	int y = 0 - height;

	// Create a temporary sprite of the new terrain to check for collision
	sprite_id temp_sprite = sprite_create(x, y, width, height, image);
	// We won't reset the terrain unless there will be no collision (try again next tick)
	if(!check_collision(temp_sprite,false)) {
		// Reset the terrain
		terrain[index]->x = x;
		terrain[index]->y = y;
		terrain[index]->width = width;
		terrain[index]->height = height;
		sprite_set_image(terrain[index], image);
		sprite_turn_to(terrain[index],0,1);
	}
	sprite_destroy(temp_sprite);
}

/**
 * Moves a hazard to the top of the screen and changes the hazard type
 **/
void hazard_reset(int index) {
	// Choose the type of hazard to place
	int id = rand() % NUM_HAZARD_TYPES;
	int width = 0;
	int height = 0;
	char* image = get_image(id, HAZARD, &width, &height);

	// Move the hazard above the screen a random amount
	int y = 0 - height - rand() % 25;

	// Choose a x coordinate between the road limits
	int min_x = road_x_coords[0] + 1;
	int max_x = road_x_coords[0] + ROAD_WIDTH - 1 - width;
	int x = rand() % (max_x + 1 - min_x) + min_x;

	// Create a temporary sprite of the new hazard to check for collision
	sprite_id temp_sprite = sprite_create(x, y, width, height, image);
	if(!check_collision(temp_sprite,false)) {
		// Reset the hazard
		hazards[index]->x = x;
		hazards[index]->y = y;
		hazards[index]->width = width;
		hazards[index]->height = height;
		sprite_set_image(hazards[index], image);
		sprite_turn_to(hazards[index], 0, 1);
	}
	sprite_destroy(temp_sprite);
}

/**
 * Creates a piece of terrain at a valid location (anywhere outside the road)
 **/
void terrain_create(int index) {
	// Choose the type of terrain to place
	int id = rand() % NUM_TERRAIN_TYPES;
	int width = 0;
	int height = 0;
	char* image = get_image(id, TERRAIN, &width, &height);
	
	// Keep searching for new coordinates until there is no collision with other terrain
	bool valid_location = false;
	while(!valid_location) {
		int y = (rand()%(screen_height()-2-height)) + 2;

		// Check if we'll place the terrain on the left or right side of the road
		bool left = rand() % 2;
		int x = -1;
		if(left) {
			int min_x = DASHBOARD_SIZE + 1;
			int max_x = road_x_coords[y] - width - 1;
			x = rand() % (max_x + 1 - min_x) + min_x;
		} else {
			int min_x = road_x_coords[y] + ROAD_WIDTH + 1;
			int max_x = screen_width() - 2 - width;
			x = rand() % (max_x + 1 - min_x) + min_x;
		}

		// Create a temporary sprite of the new terrain to check for collision
		sprite_id temp_sprite = sprite_create(x, y, width, height, image);
		if(!check_collision(temp_sprite,false)) {
			// Create the sprite of the terrain
			terrain[index] = sprite_create(x, y, width, height, image);
			sprite_turn_to(terrain[index], 0, 1);
			valid_location = true;
		}
		sprite_destroy(temp_sprite);
	}
}

/**
 * Creates a piece of hazard at a valid location (anywhere in the road)
 **/
void hazard_create(int index) {
	// Choose the type of hazard to place
	int id = rand() % NUM_HAZARD_TYPES;
	int width = 0;
	int height = 0;
	char* image = get_image(id, HAZARD, &width, &height);

	// Keep searching for new coordinates until there is no collision with other hazards
	bool valid_location = false;
	while(!valid_location) {
		int y = (rand()%((screen_height()/2)-height)) + 2;
		int min_x = road_x_coords[y] + 1;
		int max_x = road_x_coords[y] + ROAD_WIDTH - 1 - width;
		int x = rand() % (max_x + 1 - min_x) + min_x;

		// Create a temporary sprite of the new hazard to check for collision
		sprite_id temp_sprite = sprite_create(x, y, width, height, image);
		if(!check_collision(temp_sprite,false)) {
			// Create the sprite of the terrain
			hazards[index] = sprite_create(x, y, width, height, image);
			sprite_turn_to(hazards[index], 0, 1);
			valid_location = true;
		}
		sprite_destroy(temp_sprite);
	}
}

/**
 * Creates all terrain that will appear on the game screen
 **/
void setup_terrain() {
	// Decide on maximum number of terrain obstacles that can appear
	max_terrain_obs = 6;
	terrain = malloc(max_terrain_obs * sizeof(sprite_id));

	for(int i=0; i<max_terrain_obs; i++) {
		terrain_create(i);
	}
}

/**
 * Creates all hazards that will appear on the game screen
 **/
void setup_hazards() {
	// Decide on maximum number of terrain obstacles that can appear
	max_hazards = 2;
	hazards = malloc(max_hazards * sizeof(sprite_id));

	for(int i=0; i<max_hazards; i++) {
		hazard_create(i);
	}
}

/**
 * Create the fuel station sprite. Will choose a random side of the road and make it appear a 
 * random distance above the screen
 **/
void setup_fuel_station() {
	fuel = MAX_FUEL;
	refuelling = false;

	int station_width = 0;
	int station_height = 0;
	char* station_image = get_fuel_station_image(&station_width, &station_height);

	// Put the fuel station a random distance above the screen
	int y = 0 - station_height - FUEL_STATION_DELAY_DIST - (rand() % FUEL_STATION_VARIANCE);

	// Choose the side of the road
	int x;
	bool left = rand() % 2;
	if(left) {
		x = road_x_coords[0] - station_width;
	} else {
		x = road_x_coords[0] + ROAD_WIDTH + 1;
	}

	fuel_station = sprite_create(x, y, station_width, station_height, station_image);
	sprite_turn_to(fuel_station, 0, 1);
}

/**
 * Create a finish line a certain distance above the screen
 **/
void setup_finish_line() {
	int width = ROAD_WIDTH + 1;
	char* image = get_finish_line_image();

	// Set a certain distance above the car
	int y = sprite_y(player) - FINISH_LINE_DIST;

	finish_line = sprite_create(road_x_coords[0], y, width, 1, image);
	sprite_turn_to(finish_line, 0, 1);
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
	setup_fuel_station();
	setup_terrain();
	setup_hazards();

	// Get all current highscores from the highscore file
	get_hscores();
	
	// Setup the car at the bottom of the screen, middle of road
	int y = screen_height() - PLAYER_HEIGHT - 2;
	int x = (ROAD_WIDTH / 2) + road_x_coords[y] - (PLAYER_WIDTH/2) + 1;
	player = sprite_create(x, y, PLAYER_WIDTH, PLAYER_HEIGHT, get_car_image());
	car_condition = 100;

	setup_finish_line();

	// Initialise the speed settings
	speed = 0;
	speed_ctr = 0;

	score = 0;

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
	if(check_collision(temp_sprite,false)) {
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
 * Step the terrain so that it scrolls and create new terrain when the old one goes out of bounds
 **/
void update_terrain() {
	for(int i=0; i<max_terrain_obs; i++) {
		sprite_step(terrain[i]);

		// Check if any terrain went out of bounds
		if(sprite_y(terrain[i]) > screen_height()) {
			// Create a new terrain 
			terrain_reset(i);
		}
	}
}

/**
 * Step the hazards so that it scrolls and create new hazards when the old one goes out of bounds
 **/
void update_hazards() {
	for(int i=0; i<max_hazards; i++) {
		sprite_step(hazards[i]);

		// Check if any hazard went out of bounds
		if(sprite_y(hazards[i]) > screen_height()) {
			// Create a new hazard
			hazard_reset(i);
		}
	}
}

/**
 * Check if the fuel station has gone off limits and resets it to the appropritate location 
 **/
void update_fuel_station() {
	sprite_step(fuel_station);

	// Check if the fuel station went out of bounds
	if(sprite_y(fuel_station) > screen_height()) {
		// Reset the fuel station to a location above the screen
		int y = 0 - sprite_height(fuel_station) - FUEL_STATION_DELAY_DIST - (rand() % FUEL_STATION_VARIANCE);

		// Choose the side of the road
		int x;
		bool left = rand() % 2;
		if(left) {
			x = road_x_coords[0] - sprite_width(fuel_station);
		} else {
			x = road_x_coords[0] + ROAD_WIDTH + 1;
		}

		sprite_move_to(fuel_station, x, y);
	}
}

/**
 * Code that updates the logic of the game relevant to the Game state
 **/
void update_game_screen() {
	handle_input();

	// Update the speed counter. We also check if it would update while we pause the execution with
	// the loop timer. Added 10 to the loop interval in case update and draw takes 17 ms to execute
	if(speed_timer->milliseconds > (SPEED_INTERVAL - LOOP_INTERVAL - 10)) {
		speed_ctr++;
	}

	// How fast the screen scrolls (can be negative). Higher value the faster
	int speed_rate = 2;

	// Decides when to update the game (if enough time has speed depending on the speed)
	if((MAX_SPEED - speed - speed_rate < speed_ctr) && (speed > 0) && (fuel > 0)) {
		even_stripe = !even_stripe;

		fuel--;

		distance_counter++;
		// Updates the distance if enough ticks have passed to cover 1 meter
		if(distance_counter > 5) {
			distance_travelled++;
			distance_counter = 0;
		}

		// Check if the car has collided
		if(check_collision(player,false)) {
			speed = 0;
			fuel = MAX_FUEL;
			car_condition -= 10;
			if(car_condition <= 0) {
				change_state(GAME_OVER_SCREEN);
			}
			reset_player_location();
		}

		update_fuel_station();
		update_terrain();
		update_hazards();
		sprite_step(finish_line);

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
		change_state(GAME_OVER_SCREEN);
	}

	// Update the score
	score = ((distance_travelled * 8) - (car_condition)) - (get_current_time() - game_start_time) + 50;
	if(score < 0) {
		score = 1;
	} else if(score > 999999) {
		score = 999999;
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

	// The current score of the player
	draw_string(2, 2, "Score");
	draw_int(12, 2, score);

	// The distance travelled since the start of the game
	draw_string(2, 3, "Distance");
	draw_int(12, 3, distance_travelled);

	// Draw the time elapsed since game started
	draw_string(2, 4, "Time");
	draw_double(12, 4, get_current_time() - game_start_time);

	// Draw the speed stat
	draw_string(2, 5, "Speed");
	draw_int(12, 5, speed);

	// Draw the fuel stat
	draw_string(2, 6, "Fuel");
	draw_int(12, 6, fuel);

	// Draw the condition stat
	draw_string(2,7,"Condition");
	draw_int(12,7,car_condition);

	// Draw warning stating that the car is offroad
	if(car_offroad()) {
		draw_string(2, 9, "OFFROAD");
	}

	// Draw warning saying we're refuelling
	if(refuelling) {
		draw_string(2, 10, "REFUELLING");
	}
}

/**
 * Draws the road by using the graphic information from the 'road' array. Also decides where to draw
 * the center stripes
 **/
void draw_road() {
	for(int i=0; i<road_length; i++) {
		draw_char(road_x_coords[i], i + 1, get_road_image(road[i]));
		draw_char(road_x_coords[i] + ROAD_WIDTH, i + 1, get_road_image(road[i]));
		if(((i+1) % 2 == 0) && (even_stripe)) {
			draw_char(road_x_coords[i] + (ROAD_WIDTH * 0.5), i + 1, get_road_image(road[i]));
		} else if(((i+1) % 2 != 0) && !even_stripe) {
			draw_char(road_x_coords[i] + (ROAD_WIDTH * 0.5), i + 1, get_road_image(road[i]));
		}
	}
}

/**
 * Draw all of the terrain sprites
 **/
void draw_terrain() {
	for(int i=0; i<max_terrain_obs; i++) {
		sprite_draw(terrain[i]);
	}
}

/**
 * Draw all of the hazard sprites
 **/
void draw_hazards() {
	for(int i=0; i<max_hazards; i++) {
		sprite_draw(hazards[i]);
	}
}

/**
 * Draw the game screen
 **/
void draw_game_screen() {
	draw_dashboard();
	draw_road();
	sprite_draw(finish_line);
	draw_terrain();
	draw_hazards();

	sprite_draw(fuel_station);
	sprite_draw(player);
}

/**
 * Draw the game over screen
 **/
void draw_game_over_screen() {
	draw_center_text("GAME OVER", screen_height() / 2);
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
	// Setup all of the images to be used on the sprites
	imagemngr_init();

	setup_screen();

	change_state(START_SCREEN);

	// Seed our random number generator 
	srand(100);

	// The timer to check how long it takes to execute one iteration of game loop. 
	// Set the interval to 17 so the game runs at ~60 fps
	timer_id loop_timer = create_timer(LOOP_INTERVAL);

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
