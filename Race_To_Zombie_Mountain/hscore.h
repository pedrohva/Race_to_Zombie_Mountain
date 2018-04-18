#ifndef HSCORE_H
#define HSCORE_H

// The maximum number of highscores we'll display
#define MAX_SCORES      100
// The maximum size of names
#define MAX_NAME_SIZE   10

// The score the player has achieved
int score;

// The names of the top 100 highscore players
char hscore_names[MAX_SCORES][MAX_NAME_SIZE+1];
// The score of the top 100 highscore players
int hscore_scores[MAX_SCORES];

/**
 * Draws the highscore screen that will be visible to the player
 **/
void draw_highscore_screen();

/**
 * Updates the highscore screen by allowing the player to either play the game again or quit
 **/
void update_highscore_screen();

/**
 * Gets all recorded highscores in our highscore file and writes them to the proper arrays
 **/
void get_hscores();

/**
 * Sorts the scores by placing the highest score at the top and the lowest at the bottom
 **/
void sort_scores();

/**
 * Adds the current score to the highscore arrays to a free space if there is any or by 
 * replacing the lowest score currently in the highscore table
 **/
void process_hscore(char *name);

/**
 * Check if the current score is a new highscore
 **/
bool check_new_hscore();

/**
 * Saves all of the high score data from the arrays to a highscore file
 **/
void save_scores();

#endif