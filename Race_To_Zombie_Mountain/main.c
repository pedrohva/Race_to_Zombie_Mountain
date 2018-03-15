#include "cab202_graphics.h"

int main( void ) {
	setup_screen();
	draw_string( 0, screen_height() - 3, "Press any key to finish..." );
    show_screen();
	wait_char();
	cleanup_screen();
	return 0;
}