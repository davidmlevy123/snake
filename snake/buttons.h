#pragma once
#include <windows.h>
#include "additions.h"
#include "resource.h"

typedef struct {
	HWND main_window;
	// A pointer to an array that has all the game mode buttons that are open.
	HWND* game_mode_windows;
	// Our current game mode.
	HWND cur_game_mode;
	// The start game button
	HWND start_button;
	// Move right/left buttons for game modes.
	HWND move_game_mode_right;
	HWND move_game_mode_left;
	// An int that has the game mode number in it.
	UINT cur_game_mode_int;
}window_buttons;

BOOL create_window_buttons(window_buttons* buttons, HWND parent_window);
// We use 1 to move 1 right and -1 to move 1 left.
BOOL change_game_mode(window_buttons* buttons, const int right_or_left);
void destroy_window_buttons(window_buttons* buttons);