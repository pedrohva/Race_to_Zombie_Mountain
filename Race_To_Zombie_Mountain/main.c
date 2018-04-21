/** ---------------------------- INCLUDES ----------------------------- **/
#include <stdlib.h>
#include <string.h>
#include "cab202_graphics.h"
#include "cab202_timers.h"
#include "cab202_sprites.h"

/** ----------------------------- GLOBALS ----------------------------- **/
// Define the border character as a full stop (.)
#define BORDER_CHAR	46
// Define the border character for the dashboard as a slash (/)
#define DASHBOARD_BORDER_CHAR	47

// The interval of the speed timer and loop timer
#define SPEED_INTERVAL	87
#define LOOP_INTERVAL	17

// The maximum speed the player can increase the car and thus the speed of updates in the game
#define MAX_SPEED			10
#define MAX_SPEED_OFFROAD	3

// Maximum amount of fuel the car has
#define MAX_FUEL			100

// Input keys
#define INPUT_MOVE_LEFT		'a'
#define INPUT_MOVE_RIGHT	'd'
#define INPUT_ACCELERATE	'w'		
#define INPUT_DECELERATE	's'

// Define the player car's details
#define PLAYER_WIDTH	8
#define PLAYER_HEIGHT	5

// Define the type of obstacles (terrain is offroad, hazards only on the road)
#define TERRAIN     0
#define HAZARD      1

// The types of terrain
#define NUM_TERRAIN_TYPES	3
#define TERRAIN_BOULDER		0
#define TERRAIN_TREE 		1
#define TERRAIN_GRAVE		2

// The types of hazards
#define NUM_HAZARD_TYPES	2
#define HAZARD_SPIKES		0
#define HAZARD_TRIANGLE		1

// Define the direction the road is turning.
#define ROAD_STRAIGHT	1

// The minimum width of the dashboard
#define DASHBOARD_SIZE	20

// The width of the road
#define ROAD_WIDTH  	20

// The delay (in distance) of when the next fuel station appears (randomness can be added to this value
// with the FUEL_STATION_VARIANCE constant)
#define FUEL_STATION_DELAY_DIST	30
// Determines the variance of where the fuel station appears above the screen (the smaller the value
// the more frequent a fuel station will appear)
#define FUEL_STATION_VARIANCE	15

// The distance to the finish line (the number that appears in the distance stat is 1/5 of this one)
#define FINISH_LINE_DIST	500

// The maximum number of highscores we'll display
#define MAX_SCORES      100
// The maximum size of names
#define MAX_NAME_SIZE   12

// The speed of the player. This controls how long it takes for 
int speed;
// The current fuel available to the player
int fuel;

// The sprite representing the player
sprite_id player;

// A timer that controls how fast the game updates (thus setting the speed)
timer_id speed_timer;
// Helps count how many ticks have passed from the speed_timer to help decide if we should update
int speed_ctr;

// The system time in the user's computer that the game started (game screen not start menu)
double game_start_time;
// A tick counter that decides if enough ground has been travelled to cover 1 meter
int distance_counter;
// The distance in meters travelled since the start of the game
int distance_travelled;

// Represents whether the car is next to a fuel station stationary right now
bool refuelling;
// A timer that is used to verify if the car remained next to a fuel station long enough to refuel
timer_id refuel_timer;

// Checks if the game was over because of a loss instead of a win
bool game_over_loss;

/**
 * Holds information regarding what screen the player should be seeing right now. The state should
 * only be changed through the function change_state()
 **/
enum GameScreens {
	START_SCREEN,
	GAME_SCREEN,
	GAME_OVER_SCREEN,
	HIGHSCORE_SCREEN,
	EXIT_SCREEN
} game_state;

// The x-coordinate of the border of the dashboard
int dashboard_x;

// The sprite representing the fuel station
sprite_id fuel_station;

// The sprite representing the finish line
sprite_id finish_line;

// How many units the road stretches from the bottom of the screen to the top
int road_length;
// Decides if the odd or even on the y coord road sections will contain a middle stripe
bool even_stripe;
// The array which will hold all of the road sections
int *road;
// The array which will hold the x coordinate of the road sections
int *road_x_coords;

// The maximum number of terrain obstacles that can appear at once
int max_terrain_obs;
// An array which contains all of the terrain obstacles
sprite_id *terrain;

// The maximum number of hazards that can appear at once
int max_hazards;
// An array which contains all of the hazard obstacles
sprite_id *hazards;

// Hold the properties of terrain
char* terrain_image[NUM_TERRAIN_TYPES];
int terrain_width[NUM_TERRAIN_TYPES];
int terrain_height[NUM_TERRAIN_TYPES];

// Hold the properties of hazards
char* hazards_image[NUM_HAZARD_TYPES];
int hazards_width[NUM_HAZARD_TYPES];
int hazards_height[NUM_HAZARD_TYPES];

// The condition of the car (represented as a percentage)
int car_condition;

// The score the player has achieved
int score;

// The names of the top 100 highscore players
char hscore_names[MAX_SCORES][MAX_NAME_SIZE+1];
// The score of the top 100 highscore players
int hscore_scores[MAX_SCORES];

/** ------------------------ FUNCTION VARIABLES ------------------------ **/

/**
 * Checks if there is any terrain, hazard or fuel station colliding with the sprite.
 **/
bool check_collision(sprite_id sprite);

/**
 * Checks if the two sprites collide with each other
 **/
bool check_sprite_collided(sprite_id sprite1, sprite_id sprite2);

/**
 * Sorts the scores by placing the highest score at the top and the lowest at the bottom
 **/
void sort_scores();

/** ------------------------- IMAGE MANAGER --------------------------- **/
/**
 * Add to the arrays specified by the type the image and properties of a type of obstacle
 **/
void add_image(int id, char* image, int width, int height, int type) {
    if(type == TERRAIN) {
        terrain_image[id] = image;
        terrain_width[id] = width;
        terrain_height[id] = height;
    } else if(type == HAZARD) {
        hazards_image[id] = image;
        hazards_width[id] = width;
        hazards_height[id] = height;
    }
}

/**
 * Add all of the images to the terrain array
 **/
void terrain_init() {
    // Boulders
    char *terrain_boulder_image =
        " o00o "
        "o0000o"
        " o00o ";
    add_image(TERRAIN_BOULDER, terrain_boulder_image, 6, 3, TERRAIN);

    // Tree
    char *terrain_tree_image =
        "  ,,,  "
        " ,,,,, "
        ",,,,,,,"
        "  | |  "
        "  | |  ";
    add_image(TERRAIN_TREE, terrain_tree_image, 7, 5, TERRAIN);

    // Grave
    char *terrain_grave_image =
        " ___ "
        "|RIP|"
        "|   |"
        "-----"; 
    add_image(TERRAIN_GRAVE, terrain_grave_image, 5, 4, TERRAIN);
}

/**
 * Add all of the images to the hazards array
 **/
void hazards_init() {
    // Spikes
    char *hazard_spikes_image =
        "|-----|"
        "|-----|";
    add_image(HAZARD_SPIKES, hazard_spikes_image, 7, 2, HAZARD);

    // Triangle
    char *hazard_triangle_image =
        " . "
        "/!\\"
        "---";
    add_image(HAZARD_TRIANGLE, hazard_triangle_image, 3, 3, HAZARD);
}

/**
 * Setup all of the images and properties to the arrays
 **/
void imagemngr_init() {
    terrain_init();
    hazards_init();
}

/**
 * Get the image and its properties
 **/
char* get_image(int id, int type, int* width, int* height) {
    if(type == TERRAIN) {
        *width = terrain_width[id];
        *height= terrain_height[id];
        return terrain_image[id];
    } else if(type == HAZARD) {
        *width = hazards_width[id];
        *height= hazards_height[id];
        return hazards_image[id];
    }

    return "";
}

/**
 * Get the image representing the car
 **/
char* get_car_image() {
    // Define the image representing the player's car
    char* car_image =
        "   /\\   "
        "[]-||-[]"
        "   ||   "
        "[]-||-[]"
        "  ----  ";

    return car_image;
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
 * Return the image and properties of the fuel station image. 
 **/
char* get_fuel_station_image(int* width, int* height) {
    char* image = 
        "--------"
        "|      |"
        "| FUEL |"
        "|      |"
        "--------";
    
    *width = 8;
    *height = 5;

    return image;
}

/**
 * Get the image representing the finish line.
 * IMPORTANT: Must be the same width as the road + 1
 **/
char* get_finish_line_image() {
    char* image = 
        "!///////////////////!";

    return image;
}

/** --------------------------- OBSTACLES ----------------------------- **/
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

	// Place the terrain above the screen a random amount
	int y = 0 - height - (rand() % 60);

	// Create a temporary sprite of the new terrain to check for collision
	sprite_id temp_sprite = sprite_create(x, y, width, height, image);
	// We won't reset the terrain unless there will be no collision
	if(!check_collision(temp_sprite)) {
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
	int y = 0 - height - (rand() % 30);

	// Choose a x coordinate between the road limits
	int min_x = road_x_coords[0] + 1;
	int max_x = road_x_coords[0] + ROAD_WIDTH - 1 - width;
	int x = rand() % (max_x + 1 - min_x) + min_x;

	// Create a temporary sprite of the new hazard to check for collision
	sprite_id temp_sprite = sprite_create(x, y, width, height, image);
	if(!check_collision(temp_sprite)) {
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
	
	int min_y = 0 - (rand()%screen_height());
	int max_y = screen_height() - height - 2;
	int y = (rand()%(max_y + 1 - min_y)) + min_y;

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
    // Create all of the terrain obstacles 
	for(int i=0; i<max_terrain_obs; i++) {
		terrain_create(i);
	}

    // Reset all of the terrain obstacles so that they don't collide
    bool collided = true;
	while(collided) {
		collided = false;
		for(int i=0; i<max_terrain_obs; i++) {
			if(check_collision(terrain[i])) {
				collided = true;
				terrain_reset(i);
			}
		}
	}
}

/**
 * Creates all hazards that will appear on the game screen
 **/
void setup_hazards() {
	for(int i=0; i<max_hazards; i++) {
		hazard_create(i);
	}

	// Reset all of the hazard obstacles so that they don't collide
    bool collided = true;
	while(collided) {
		collided = false;
		for(int i=0; i<max_hazards; i++) {
			if(check_collision(hazards[i])) {
				collided = true;
				hazard_reset(i);
			}
		}
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
	max_hazards = 3;
	hazards = malloc(max_hazards * sizeof(sprite_id));

    // Init Terrain
    // Decide on maximum number of terrain obstacles that can appear
	max_terrain_obs = 14 + ((screen_width()-80)/5) + ((screen_height()-24)/5);
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

		// Move the fuel station to the new location
		sprite_move_to(fuel_station, x, y);

		// Reset any terrain that might be on the way
		for(int i=0; i<max_terrain_obs; i++) {
			if(check_sprite_collided(fuel_station, terrain[i])) {
				terrain_reset(i);
			}
		}
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
 * Checks if the two sprites collide with each other
 **/
bool check_sprite_collided(sprite_id sprite1, sprite_id sprite2) {
	// Check if both sprites are valid
	if((sprite1 != NULL) && (sprite2 != NULL)) {
		// Check if there is colllision in the x-axis
		if(!((sprite_x(sprite1) + sprite_width(sprite1) <= sprite_x(sprite2)) || (sprite_x(sprite1) >= sprite_x(sprite2) + sprite_width(sprite2)))) {
			// Check if there is collision in the y-axis
			if(!((sprite_y(sprite1) + sprite_height(sprite1) < sprite_y(sprite2)) || (sprite_y(sprite1) > sprite_y(sprite2) + sprite_height(sprite2)))) {
				return true;
			}
		}
	}

	return false;
}

/**
 * Checks if there is any terrain, hazard or fuel station colliding with the sprite.
 **/
bool check_collision(sprite_id sprite) {
	// Iterate through the terrain to see if there was a collision
	for(int i=0; i<max_terrain_obs; i++) {
		// We don't want to check if it is colliding with itself
		if(!sprites_equal(sprite, terrain[i])) {
			if(check_sprite_collided(sprite,terrain[i])) {
				return true;
			}
		}
	}

	// Iterate through the hazards to see if there was a collision
	for(int i=0; i<max_hazards; i++) {
		// We don't want to check if it is colliding with itself
		if(!sprites_equal(sprite, hazards[i])) {
			if(check_sprite_collided(sprite,hazards[i])) {
				return true;
			}
		}
	}

	// Check if collides with fuel station
	if(check_sprite_collided(sprite,fuel_station)) {
		return true;
	}

	return false;
}

/** -------------------------- HIGH SCORE ----------------------------- **/
/**
 * Gets all recorded highscores in our highscore file and writes them to the proper arrays
 * Reference used: https://www.cs.bu.edu/teaching/c/file-io/intro/
 **/
void get_hscores() {
    FILE *hscore_file = fopen("highscores", "r");

    // Fill the whole scores table with 0s
    for(int i=0; i<MAX_SCORES; i++) {
        hscore_scores[i] = 0;
    }

    // We only want the top 100 scores, so don't record anything else if we're above that number
    int counter = 0;

    // Get the highscore data if the file exists
    if(hscore_file != NULL) {
        // The data we want from each line
        char* name = malloc(sizeof(char) * (MAX_NAME_SIZE+1));
        int score;

        // Read the score and names from the file. Stop reading if reached EOF or found data that doesn't 
        // match the "name score" format
        while(fscanf(hscore_file, "%s %d", name, &score) == 2) {
            if(counter < MAX_SCORES) {
                strcpy(hscore_names[counter], name);
                hscore_scores[counter] = score;
                counter++;
            }
        }

        free(name);
    } else {
        // Add the game master as the score to beat
        hscore_scores[counter] = 1000;
        strcpy(hscore_names[counter], "GameMaster");
        counter++;
    }

    fclose(hscore_file);
    sort_scores();
}

/**
 * Saves all of the high score data from the arrays to a highscore file
 **/
void save_scores() {
    FILE *hscore_file = fopen("highscores", "w");

    for(int i=0; i<MAX_SCORES; i++) {
        if((hscore_names[i] != NULL) && (hscore_scores[i] > 0)) {
            fprintf(hscore_file, "%s %d\n", hscore_names[i], hscore_scores[i]);
        }
    }

    fclose(hscore_file);
}

/**
 * Reads the arrays containing the highscores and prints the information to the user
 **/
void draw_hscores() {
    // Decide how many highscores we can actually draw
    int min_y = 4;
    int max_y = screen_height() - 3;
    int num_scores = max_y - min_y;
    
    // Keep the number of scores to 100 max
    if(num_scores > MAX_SCORES) {
        num_scores = MAX_SCORES;
    }

    // The current y position where we will be drawing
    int y = min_y;
    // Decide on how much space we'll use to pad the table on the sides
    // The hscore number will never be above 100, the names should not be above 11 and the score 
    // caps at 999999 (6 digits) and we add 3 for column padding (making total be 13 + MAX_NAME_SIZE)
    int space = (screen_width()/2) - ((13 + MAX_NAME_SIZE)/2) - 1;
    // Get the highscores from the arrays and draw them to the screen
    for(int i=0; i<num_scores; i++) {
        // Draw the highscore number
        draw_int(space + 2, y, i+1);
        // Draw the name if it exists
        if(hscore_names[i] != NULL) {
            char *name = (char *)hscore_names[i];
            draw_string(space + 6, y, name);
        }
        // Draw the score if it exists
        if(hscore_scores[i] > 0) {
            draw_int(space + 19, y, hscore_scores[i]);
        }
        // Move down a line
        y++;
    }
}

/**
 * Check if the current score is a new highscore
 **/
bool check_new_hscore() {
    int index = 0;
    int old_score = 0;

    // Transverse the score array and get the lowest score (the one at the bottom)
    for(int i=0; i<MAX_SCORES; i++) {
        if(hscore_scores[i] > 0) {
            old_score = hscore_scores[i];
            index = i;
        }
    }

    // If score is 0, we don't have a high score
    if(score == 0) {
        return false;
    }

    // Check if we have a new highscore
    if((score > old_score) || (index < (MAX_SCORES-1))) {
        return true;
    } else {
        return false;
    }
}

/**
 * Sorts the scores by placing the highest score at the top and the lowest at the bottom
 **/
void sort_scores() {
    // Sort the scores if we have more than 1 entry
    for(int i=0; i<MAX_SCORES; i++) {
        for(int j=i+1; j<MAX_SCORES; j++) {
            if((hscore_names[i] != NULL) && (hscore_names[j] != NULL) && (hscore_scores[i] != 0)) {
                if(hscore_scores[i] < hscore_scores[j]) {  
                    // Swap the scores
                    int tmp = hscore_scores[i];
                    hscore_scores[i] = hscore_scores[j];
                    hscore_scores[j] = tmp;
                    // Swap the names
                    char temp[11] = "";
                    strcpy(temp, hscore_names[i]);
                    strcpy(hscore_names[i],hscore_names[j]);
                    strcpy(hscore_names[j],temp);
                }
            }
        }
    }
}

/**
 * Adds the current score to the highscore arrays to a free space if there is any or by 
 * replacing the lowest score currently in the highscore table
 **/
void process_hscore(char *name) {
    // Sort the scores just in case (can be removed if slowing down too much)
    sort_scores();

    int index = 0;

    // Transverse the score array and get the lowest score (the one at the bottom)
    for(int i=0; i<MAX_SCORES; i++) {
        if(hscore_scores[i] > 0) {
            index = i;
        }
    }

    // Add to the end of the table if there is space
    if(index < (MAX_SCORES-1)) {
        hscore_scores[index+1] = score;
        strcpy(hscore_names[index+1], name);
    } else {
        // Replace the lowest score if there is no space
        hscore_scores[index] = score;
        strcpy(hscore_names[index], name);
    }
}

/** -------------------------- MAIN GAME ------------------------------ **/
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