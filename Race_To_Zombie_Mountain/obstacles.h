#ifndef OBSTACLES_H
#define OBSTACLES_H

#include "imagemngr.h"

/**
 *  Obstacles are defined as a game entity that causes the player to experience game effect
 * when colliding with it.
 *  The current obstacles in the game are
 *      - Road
 *      - Terrain (Obstacles that only appear offroad)
 *      - Hazards (Obstacles that only appear on the road)
 *      - Fuel Stations 
 **/

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

// The condition of the car (represented as a percentage)
int car_condition;

/**
 * Allocate the required memory and create arrays which will hold all of our obstacles
 **/
void init_obs();

/**
 * Draw the obstacles that appear in the game screen
 **/
void draw_obs();

/**
 * Step through all obstacles, making sure to wrap any that go 
 * out of bounds to the top of the screen
 **/
void update_obs();

/**
 * Initializes all of the required arrays and create the terrain, road hazards, 
 * fuel station and road
 **/
void setup_obs();

/**
 * Checks if there is any terrain, hazard or fuel station colliding with the sprite.
 **/
bool check_collision(sprite_id sprite);

/**
 * Checks if the two sprites collide with each other
 **/
bool check_sprite_collided(sprite_id sprite1, sprite_id sprite2);

/**
 * Moves a hazard to the top of the screen and changes the hazard type
 **/
void hazard_reset(int index);

/**
 * Moves a terrain to the top of the screen and changes the terrain type
 **/
void terrain_reset(int index);

#endif