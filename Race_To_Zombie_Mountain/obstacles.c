#include <stdlib.h>
#include "cab202_graphics.h"
#include "cab202_sprites.h"

#include "obstacles.h"

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

    // Create the sprite of the terrain
    terrain[index] = sprite_create(x, y, width, height, image);
    sprite_turn_to(terrain[index], 0, 1);
}

/**
 * Creates a piece of hazard anywhere on the road
 **/
void hazard_create(int index) {
	// Choose the type of hazard to place
	int id = rand() % NUM_HAZARD_TYPES;
	int width = 0;
	int height = 0;
	char* image = get_image(id, HAZARD, &width, &height);

    // Randomly get a x-y coordinate 
	int y = (rand()%((screen_height()/2)-height)) + 2;
    int min_x = road_x_coords[y] + 1;
    int max_x = road_x_coords[y] + ROAD_WIDTH - 1 - width;
    int x = rand() % (max_x + 1 - min_x) + min_x;

    // Create the hazard sprite
    hazards[index] = sprite_create(x, y, width, height, image);
    sprite_turn_to(hazards[index], 0, 1);
}

/**
 * Creates all terrain that will appear on the game screen
 **/
void setup_terrain() {
    // Set all the sprite_ids to null
    for(int i=0; i<max_terrain_obs; i++) {
        terrain[i] = NULL;
    }

    // Create all of the terrain obstacles 
	for(int i=0; i<max_terrain_obs; i++) {
		terrain_create(i);
	}

    // Reset all of the terrain obstacles so that they don't collide
    for(int i=0; i<max_terrain_obs; i++) {
		terrain_reset(i);
	}
}

/**
 * Creates all hazards that will appear on the game screen
 **/
void setup_hazards() {
    // Set all the sprite_ids to null
    for(int i=0; i<max_hazards; i++) {
        hazards[i] = NULL;
    }

	for(int i=0; i<max_hazards; i++) {
		hazard_create(i);
	}

    for(int i=0; i<max_hazards; i++) {
		hazard_reset(i);
	}
}

/**
 * Create the fuel station sprite. Will choose a random side of the road and make it appear a 
 * random distance above the screen
 **/
void setup_fuel_station() {
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

	// Set a certain distance above the screen
	int y = 0 - FINISH_LINE_DIST;

	finish_line = sprite_create(road_x_coords[0], y, width, 1, image);
	sprite_turn_to(finish_line, 0, 1);
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
    even_stripe = true;
    int road_x = ((screen_width() - ROAD_WIDTH - 1 + DASHBOARD_SIZE) * 0.5);

	// Build the individual road sections
	for(int i=0; i<road_length; i++) {
		add_road_section(road_x, i + 1, ROAD_STRAIGHT);
	}
}

/**
 * Initializes all of the required arrays and create the terrain, road hazards, 
 * fuel station and road
 **/
void setup_obs() {
    setup_road();
	setup_fuel_station();
	setup_terrain();
	setup_hazards();
    setup_finish_line();
}

/**
 * Allocate the required memory and create arrays which will hold all of our obstacles
 **/
void init_obs() {
    // Init Hazards
    // Decide on maximum number of terrain obstacles that can appear
	max_hazards = 2;
	hazards = malloc(max_hazards * sizeof(sprite_id));

    // Init Terrain
    // Decide on maximum number of terrain obstacles that can appear
	max_terrain_obs = 6;
	terrain = malloc(max_terrain_obs * sizeof(sprite_id));

    // Init road
    road_length = screen_height() - 2;	// There should be borders at the top and bottom of the screen
	road = malloc(road_length * sizeof(int));
	road_x_coords = malloc(road_length * sizeof(int));
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
 * Step through all terrain and road hazards, making sure to wrap any that go 
 * out of bounds to the top of the screen
 **/
void update_obs() {
    update_fuel_station();
    sprite_step(finish_line);
    update_terrain();
    update_hazards();
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
 * Draw the terrain and road hazards that appear in the game screen
 **/
void draw_obs() {
    draw_road();
	sprite_draw(finish_line);
    draw_terrain();
    draw_hazards();
    sprite_draw(fuel_station);
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
		}
	}

	return collided;
}

/**
 * Checks if the player has collided with the fuel station. This is separate from the 
 * check_collision() funtion as a collision with the fuel station is lethal
 **/
bool check_fstation_collision(sprite_id sprite) {
    bool collided = false;

	// The coordinates of the sprite (used to improve readability as the coords should not change in this function)
	int x = sprite_x(sprite);
	int y = sprite_y(sprite);
	int width = sprite_width(sprite);
	int height = sprite_height(sprite);

    // Check if there is any collision with the fuel station
	if(!((x + width <= sprite_x(fuel_station)) || (x >= sprite_x(fuel_station) + sprite_width(fuel_station)))) {
		// Check if there is collision in the y-axis
		if(!((y + height < sprite_y(fuel_station)) || (y > sprite_y(fuel_station) + sprite_height(fuel_station)))) {
			collided = true;
		}
	}

    return collided;
}