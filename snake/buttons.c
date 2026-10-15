#include "buttons.h"

// Gets the start button ready to be shown.
static BOOL start_button_set_up(HWND parent_window, HWND* button) {
	RECT client_area;
	GetClientRect(parent_window, &client_area);
	int width = (client_area.right - client_area.left) / 7;
	int hight = (client_area.bottom - client_area.top) / 7;
	int x = (client_area.right) / 2 - 3 * width / 4;
	int y = 2 * (client_area.bottom - hight) / 3;

	// We create a big start game button.
	*button = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", L"start game", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x, y, width, hight, parent_window, (HMENU)ID_START_BUTTON_MAIN, GetModuleHandle(NULL), NULL);

	// If it failed to create we return false.
	if (!*button) {
		MessageBox(parent_window, L"Failed To Set Up Start Button", L"ERROR", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

// Gets the standered button setup for the start of the game.
static BOOL game_modes_button_set_up(window_buttons* buttons) {
	RECT client_area;
	GetClientRect(buttons->main_window, &client_area);
	int width = (client_area.right - client_area.left) / 7;
	int hight = (client_area.bottom - client_area.top) / 15;
	int x = (client_area.right) / 2 - 3 * width / 4;
	int y = (client_area.bottom - hight) / 2;

	buttons->move_game_mode_left = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", L"<", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x - width / 4, 5 * y / 6, width / 4, hight, buttons->main_window, (HMENU)ID_SWITCH_GAME_MODE_LEFT, GetModuleHandle(NULL), NULL);
	buttons->move_game_mode_right = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", L">", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x + width, 5 * y / 6, width / 4, hight, buttons->main_window, (HMENU)ID_SWITCH_GAME_MODE_RIGHT, GetModuleHandle(NULL), NULL);

	buttons->cur_game_mode = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", L"classic", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x, 3 * y / 4, width, 2 * hight, buttons->main_window, (HMENU)ID_GAME_MODE_BUTTON, GetModuleHandle(NULL), NULL);

	if (!buttons->cur_game_mode || !buttons->move_game_mode_left || !buttons->move_game_mode_right) {
		MessageBox(buttons->main_window, L"Failed To Set Up Button For Game Mode", L"Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	// We allocate the room in the heap for the array that has all of our game modes that are set up. HEAP_ZERO_MEMORY initializes all spots to NULL.
	buttons->game_mode_windows = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HWND) * (ID_THREE_APPLES_WALLS_DONT_KILL - ID_CLASSIC + 1));
	if (buttons->game_mode_windows != NULL) {
		buttons->game_mode_windows[0] = buttons->cur_game_mode;
	}
	else {
		MessageBox(buttons->main_window, L"Failed To Allocate Memory For Game Modes.", L"ERROR", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

// Gets all the stuff in the window_buttons struct ready for set up.
BOOL create_window_buttons(window_buttons* buttons, HWND parent_window) {
	buttons->main_window = parent_window;
	buttons->cur_game_mode_int = ID_CLASSIC;
	// if we failed to set up the start button.
	if (!start_button_set_up(buttons->main_window, &buttons->start_button)) {
		return FALSE;
	}
	// If we failed to set up game modes.
	if (!game_modes_button_set_up(buttons)) {
		return FALSE;
	}
	return TRUE;
}

// Converts UINT value into the correct game mode to print
static wchar_t* button_int_to_words(const UINT val) {
	switch (val) {
	case ID_CLASSIC: {
		return L"classic";
		break;
	}
	case ID_WALSS_DONT_KILL: {
		return L"easy";
		break;
	}
	case ID_DISAPEARING_APPLE: {
		return L"fast";
		break;
	}
	case ID_DISAPEARING_APPLE_WALLS_DONT_KILL: {
		return L"easy fast";
		break;
	}
	case ID_THREE_APPLES: {
		return L"multiple apples";
		break;
	}
	case ID_THREE_APPLES_WALLS_DONT_KILL: {
		return L"easy multiple apples";
		break;
	}
	default: {
		return NULL;
	}
	}
}

// Function that changes the current button that is shown baides on request.
BOOL change_game_mode(window_buttons* buttons, const int right_or_left) {

	// We get the new button we are moving too.
	int new_spot = buttons->cur_game_mode_int + right_or_left;
	int new_spot_index = new_spot - ID_CLASSIC;

	// If the index is negative it means we went over while moving left so we take the last ID.
	if (new_spot_index < 0) {
		new_spot = ID_THREE_APPLES_WALLS_DONT_KILL;
		new_spot_index = new_spot - ID_CLASSIC;
	}

	// If we make it passed the end, meaning we moved right over the limit we need to loop back to the first ID.
	else if (new_spot > ID_THREE_APPLES_WALLS_DONT_KILL) {
		new_spot = ID_CLASSIC;
		new_spot_index = 0;
	}

	// If we have already been on this game mode, meaning it has already been opened in the past, we can just unhide it. We remove ID_CLASSIC every time we index to get the exact value for an index.
	if (buttons->game_mode_windows[new_spot_index]) {
		ShowWindow(buttons->cur_game_mode, SW_HIDE);
		ShowWindow(buttons->game_mode_windows[new_spot_index], SW_SHOW);
		buttons->cur_game_mode = buttons->game_mode_windows[new_spot_index];
		buttons->cur_game_mode_int = new_spot;
		// We are done with this function because we changed the window.
		return TRUE;
	}

	wchar_t* game_mode = button_int_to_words(new_spot);
	// If we failed to get the words we print an error and leave.
	if (game_mode == NULL) {
		MessageBox(buttons->main_window, L"Failed To Get Game Mode Name", L"ERROR", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// We get the spot we will show this button at.
	RECT client_area;
	GetClientRect(buttons->main_window, &client_area);
	int width = (client_area.right - client_area.left) / 7;
	int hight = (client_area.bottom - client_area.top) / 15;
	int x = (client_area.right) / 2 - 3 * width / 4;
	int y = (client_area.bottom - hight) / 2;

	// We create the new button.
	HWND new_button = NULL;
	new_button = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", game_mode, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x, 3 * y / 4, width, 2 * hight, buttons->main_window, (HMENU)ID_GAME_MODE_BUTTON, GetModuleHandle(NULL), NULL);
	if (new_button == NULL) {
		MessageBox(buttons->main_window, L"Failed To Create New Game Mode Button", L"ERROR", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// We save the button as created in the array and update our current game mode.
	ShowWindow(buttons->cur_game_mode, SW_HIDE);
	ShowWindow(new_button, SW_SHOW);
	buttons->cur_game_mode_int = new_spot;
	buttons->game_mode_windows[new_spot_index] = new_button;
	buttons->cur_game_mode = new_button;
	// Changing the button worked so we return true.
	return TRUE;
}

// Destroyes the array that has the handles to all the buttons that are open.
void destroy_window_buttons(window_buttons* buttons) {
	// If its alreadt Null we can just return.
	if (buttons->game_mode_windows == NULL) {
		DestroyWindow(buttons->move_game_mode_left);
		DestroyWindow(buttons->move_game_mode_right);
		DestroyWindow(buttons->start_button);
		return;
	}

	// We go over the whole array.
	for (int i = 0; i <= ID_THREE_APPLES_WALLS_DONT_KILL - ID_CLASSIC; ++i) {
		// If a window does exist in that position we destroy it and put it to NULL.
		if (buttons->game_mode_windows[i]) {
			DestroyWindow(buttons->game_mode_windows[i]);
			buttons->game_mode_windows[i] = NULL;
		}
	}

	// We free the memory that we allocated from the heap and set the pointer to NULL.
	HeapFree(GetProcessHeap(), 0, buttons->game_mode_windows);
	buttons->game_mode_windows = NULL;

	// Destroy the windows of the other buttons.
	DestroyWindow(buttons->move_game_mode_left);
	DestroyWindow(buttons->move_game_mode_right);
	DestroyWindow(buttons->start_button);
}
