#ifndef ZOMBIEMOUNTAIN_H
#define ZOMBIEMOUNTAIN_H

#include <stdlib.h>
#include <string.h>
#include "cab202_graphics.h"
#include "cab202_timers.h"
#include "cab202_sprites.h"

#include "hscore.h"
#include "obstacles.h"

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

// Function declarations
void handle_input();
void draw_game_screen();

#endif