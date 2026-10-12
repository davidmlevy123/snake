#include "Snake.h"

Snake create_snake(const UINT length, const UINT cur_direction, const POINT* start_points, const UINT max) {
	Snake snake = { 0 };
	snake.length = length;
	// If we dont get a direction we set to right.
	if (cur_direction == 0) {
		snake.cur_direction = RIGHT;
	}
	else {
		snake.cur_direction = cur_direction;
	}

	// If we dont get specified start points we start at the default points that i chose.
	if (start_points == NULL) {
		POINT default_start_points[3];

		// We set the default points so we can pass it as the data for the queue.
		for (int i = 0; i < (int)snake.length; ++i) {
			default_start_points[i].x = (snake.length - 1 - i) + 1;
			default_start_points[i].y = 0;
		}

		// If we didnt get a max size we set to default (50).
		if (max == 0) {
			snake.snake = create_queue(default_start_points, sizeof(POINT), 50, snake.length, 0);
		}
		// If we got a max
		else {
			snake.snake = create_queue(default_start_points, sizeof(POINT), max, snake.length, 0);
		}
	}

	// If we got points to use.
	else {
		// Same idea with the max.
		if (max == 0) {
			snake.snake = create_queue(start_points, sizeof(POINT), 50, snake.length, 0);
		}
		else {
			snake.snake = create_queue(start_points, sizeof(POINT), max, snake.length, 0);
		}
	}

	// We return the snake we created
	return snake;
}

BOOL is_valid_turn(const UINT new_direction, const Snake* s) {
	// If its the same direction or oppisate directoin(distance of 2 loop around values).
	if (new_direction == s->cur_direction || new_direction == (s->cur_direction + 2) % 4) {
		return FALSE;
	}
	POINT* p = (POINT*)(front(&s->snake));
	if (new_direction == UP && (p->y <= 0)) {
		return FALSE;
	}
	if (new_direction == DOWN && (p->y >= GRID_H - 1)) {
		return FALSE;
	}
	if (new_direction == RIGHT && (p->x >= GRID_W - 1)) {
		return FALSE;
	}
	if (new_direction == LEFT && (p->x <= 0)) {
		return FALSE;
	}
	return TRUE;
}
BOOL point_on_snake(const POINT* p, const Snake* s, const UINT start_index) {
	// We go over the snake segments one by one checking if its the smae as p.
	for (int i = start_index; i < (int)s->length; ++i) {
		POINT* cur_snake_spot = (POINT*)(place(&s->snake, i));
		if (p->x == cur_snake_spot->x && p->y == cur_snake_spot->y) {
			return TRUE;
		}
	}
	return FALSE;
}
BOOL is_snake_head(const POINT* p, const Snake* s) {
	POINT* snake_head = (POINT*)front(&s->snake);
	return ((p->x == snake_head->x) && (p->y == snake_head->y));
}
