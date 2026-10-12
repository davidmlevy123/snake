#pragma once
#include <tchar.h>
#include "queue.h"

// Create a struck that has the snake. It is a global struct
typedef struct {
	// We creata an queue of points that has a length of 3(we will have the snake size at 3 for now).
	queue snake;
	UINT length;
	UINT cur_direction; // The current direction is UP=0,RIGHT=1,DOWN=2,LEFT=3.
}Snake;

// Function to set up snake
// length - the length of the snake in cell's
// cur_direction - the current direction the snake is moving in when creating send 0 for default value.
// start_points - the start points we get as the start points for the snaek. If NULL use default { (1,0) , (2,0) , (3,0) }. 
// max - the max size the snake can get. It is a field of the queue in it.
Snake create_snake(const UINT length, const UINT cur_direction, const POINT* start_points, const UINT max);

// If the turn we make is a valid turn.
BOOL is_valid_turn(const UINT new_direction, const Snake* s); 
// If the point we got is a point on our snake.
BOOL point_on_snake(const POINT* p, const Snake* s, const UINT start_index);
// If the point we get is the snakes head.
BOOL is_snake_head(const POINT* p, const Snake* s);

