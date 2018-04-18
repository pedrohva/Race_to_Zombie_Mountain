#ifndef ZOMBIEMOUNTAIN_H
#define ZOMBIEMOUNTAIN_H

#include <stdlib.h>
#include <string.h>
#include "cab202_graphics.h"
#include "cab202_timers.h"
#include "cab202_sprites.h"

#include "imagemngr.h"
#include "hscore.h"

// Define the border character as a full stop (.)
#define BORDER_CHAR	46

// The minimum width of the dashboard
#define DASHBOARD_SIZE	20

// The width of the road
#define ROAD_WIDTH  	20

// The interval of the speed timer and loop timer
#define SPEED_INTERVAL	87
#define LOOP_INTERVAL	17

// The maximum speed the player can increase the car and thus the speed of updates in the game
#define MAX_SPEED			10
#define MAX_SPEED_OFFROAD	3

// Maximum amount of fuel the car has
#define MAX_FUEL			200

// The distance to the finish line (the number that appears in the distance stat is 1/5 of this one)
#define FINISH_LINE_DIST	500

// Input keys
#define INPUT_MOVE_LEFT		'a'
#define INPUT_MOVE_RIGHT	'd'
#define INPUT_ACCELERATE	'w'		
#define INPUT_DECELERATE	's'

// The delay (in distance) of when the next fuel station appears (randomness can be added to this value
// with the FUEL_STATION_VARIANCE constant)
#define FUEL_STATION_DELAY_DIST	30
// Determines the variance of where the fuel station appears above the screen (the smaller the value
// the more frequent a fuel station will appear)
#define FUEL_STATION_VARIANCE	15

// The sprite representing the player
sprite_id player;
// The speed of the player. This controls how long it takes for 
int speed;
// The current fuel available to the player
int fuel;

// The maximum number of terrain obstacles that can appear at once
int max_terrain_obs;
// An array which contains all of the terrain obstacles
sprite_id *terrain;

// The maximum number of hazards that can appear at once
int max_hazards;
// An array which contains all of the hazard obstacles
sprite_id *hazards;

// A timer that controls how fast the game updates (thus setting the speed)
timer_id speed_timer;
// Helps count how many ticks have passed from the speed_timer to help decide if we should update
int speed_ctr;

// How many units the road stretches from the bottom of the screen to the top
int road_length;
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

// The sprite representing the fuel station
sprite_id fuel_station;

// Represents whether the car is next to a fuel station stationary right now
bool refuelling;
// A timer that is used to verify if the car remained next to a fuel station long enough to refuel
timer_id refuel_timer;

// The condition of the car (represented as a percentage)
int car_condition;

// The sprite representing the finish line
sprite_id finish_line;

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

// Function declarations
void hazard_reset(int index);
void handle_input();
void draw_game_screen();

#endif