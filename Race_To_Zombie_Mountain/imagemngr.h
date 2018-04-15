#ifndef IMAGEMNGR_H
#define IMAGEMNGR_H

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

// Setup all of the images and properties to the arrays
void imagemngr_init();
char* get_image(int id, int type, int* width, int* height);

#endif