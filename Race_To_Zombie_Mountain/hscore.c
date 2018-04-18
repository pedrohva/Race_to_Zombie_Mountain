#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cab202_graphics.h"
#include "hscore.h"

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