#include "imagemngr.h"

// Hold the properties of terrain
char* terrain_image[NUM_TERRAIN_TYPES];
int terrain_width[NUM_TERRAIN_TYPES];
int terrain_height[NUM_TERRAIN_TYPES];

// Hold the properties of hazards
char* hazards_image[NUM_HAZARD_TYPES];
int hazards_width[NUM_HAZARD_TYPES];
int hazards_height[NUM_HAZARD_TYPES];

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

    dashboard_border_char = '/';
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
 * Return the image and properties of the fuel station image
 **/
char* get_fuel_station_image(int* width, int* height) {
    char* image = 
        "--------"
        "|      |"
        "|      |"
        "| FUEL |"
        "|      |"
        "|      |"
        "--------";
    
    *width = 8;
    *height = 7;

    return image;
}