#include <windows.h>
#include <tchar.h>
#include "additions.h" // The additions i created
#include "resource.h"
#include "queue.h" 
#include "Snake.h"
#include "random.h"
#include "high_score.h"
#include "drawing.h"
#include "buttons.h"
#include "dialogs.h"

const wchar_t class_name[] = L"Main Window.";
const wchar_t window_name[] = L"Snake";

// We create our actual snake, in c we cant give default values so we need to give the values when we define it.
Snake my_snake = { .length = 3 , .cur_direction = RIGHT };
static void set_snake_start_points(){
	// Distroy the snake if one still exists and has not been destroyed.
	destroy_queue(&my_snake.snake);
	// We create the snake with the default points (passing NULL).
	my_snake = create_snake(3, RIGHT, NULL, MAX_SNAKE_SIZE);
}

// A queue that has the buttons pressed so we can do them on their time. If we dont have it its possible to do 2 moves per tick breaking the game.
queue buttons_pressed;
static void set_button_queue() {
	// Free mem if a prev game was played so we have a new one without leaking mem.
	destroy_queue(&buttons_pressed);
	buttons_pressed = create_queue(NULL, sizeof(int), 3, 0, 0);
}

// Our drawing default resources. (Used when i want the default stuff that i created).
drawing_resources default_my_colouring_stuff;

// Our buttons we have for the start window.
window_buttons start_buttons;

typedef struct {
	POINT apple;
	BOOL apple_draw;
}Apple;
// We create an apple and we dont start by drawing.
Apple apple = { .apple_draw = FALSE };

// A bool to check if the game has started yet.
UINT game_state = NOT_STARTED;

// We have a global hInstance so we can use it in the function and not just main.
HINSTANCE global_hInstance;
HWND global_cur_win;
// We make it global so we can save if the dialog is up.
HWND handle_to_settings = NULL;

// The bool to know if we should draw the smiley face in the WM_PAINT.
BOOL draw_face = FALSE;

// A int to tell the computer the background colour we want.
UINT bk_colour = BLACKBK;
UINT cancel_pressed = 0;

// The score we got in the game.
UINT score = 0;
UINT high_score = 0;

// The time of a tick in the game, meaning the speed of the snake.
UINT cur_timer_speed = 100;

static LRESULT CALLBACK WindowProc(HWND key_of_window, UINT code_of_msg, WPARAM key_pressed, LPARAM extra_msg_info);

static void queue_direction_if_valid(const UINT new_direction);
static void start_game(const HWND key_of_window);
static int  get_cell_size(HWND key_of_window);
static void invalidate_cell(HWND hwnd, const POINT* cell, const int cell_size);

int WINAPI wWinMain(HINSTANCE handle_of_instance, HINSTANCE not_needed, PWSTR command_line, int flag_min_max_normal) {//PWSTR=wchar_t*.
	global_hInstance = handle_of_instance;
	WNDCLASSEX window_blueprint = { 0 };
	window_blueprint.cbSize = sizeof(WNDCLASSEX);
	window_blueprint.style = 0;
	window_blueprint.lpfnWndProc = WindowProc;// Long pointer to function. 

	// We dont reserve any extra space.
	window_blueprint.cbClsExtra = 0;
	window_blueprint.cbWndExtra = 0;
	window_blueprint.hInstance = handle_of_instance;

	// We load the icon i created. We use MAKEINTRESOURCE to convert from int to lpcstr
	window_blueprint.hIcon = LoadIcon(handle_of_instance, MAKEINTRESOURCE(IDI_MAIN_SNAKE_LOGO));

	// We load the cursor that we put in(its a snake it looks funny).
	window_blueprint.hCursor = LoadCursor(handle_of_instance, MAKEINTRESOURCE(IDC_SNAKE_CURSOR));

	// We set the background to black. For the default (HBRUSH)(COLOR_WINDOW + 1).
	window_blueprint.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

	// We set it to the menu we built in addition.h and in additions.rc.
	window_blueprint.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
	window_blueprint.lpszClassName = class_name;
	window_blueprint.hIconSm = LoadIcon(handle_of_instance, MAKEINTRESOURCE(IDI_SNAKE_LOGO));

	// Is the class creation failed: (the function gets a pointer to a WNDCLASS so we send the address)
	if (!RegisterClassEx(&window_blueprint)) {
		// We send a message box that has the error. We send NULL in the first one because we use the default message box window and not one we created. the second spot is the message in the box and the third is the title of the box. In the last spot we put MB_OK which means that we just show the test box. Its the code for what type we want. MB_ICONEXCLAMATION is to make the error message loom better
		MessageBox(NULL, TEXT("Failed To Create Class."), TEXT("ERROR."), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}

	HWND cur_win = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW, // a type of window style. (in)
		class_name, // the windows class name. (in, optional) 
		window_name, // the windows name. (in, optional)
		WS_OVERLAPPEDWINDOW, // the window style(old vertion). (in)
		0, 0, // for default x y coordinates CW_USEDEFAULT. (in)
		2500, 800, // width and height of the window. (in)
		NULL, // this is not a child so we dont have a Parent window. (in, optional)
		NULL, // also NULL for the same reason. (in, optional)
		handle_of_instance, // handle of the instance. (in, optional)
		NULL  //this is LPVOID we dont have any extra stuff so its NULL. (in, optional)
	);
	
	if (cur_win == NULL) { // If we failed to open the current window: 
		// We print the ERROR message.
		MessageBox(NULL, TEXT("Failed To Create Window."), TEXT("ERROR."), MB_OK | MB_ICONEXCLAMATION);
		return 2;
	}
	global_cur_win = cur_win;
	// The blueprint for the child is very simular to the main_window_blueprint we just change the icons and cursor.
	WNDCLASSEX child_window_blueprint = { 0 }; 
	child_window_blueprint.cbSize = sizeof(WNDCLASSEX);
	child_window_blueprint.style = CS_HREDRAW | CS_VREDRAW; 
	child_window_blueprint.lpfnWndProc = WindowProc;
	child_window_blueprint.cbClsExtra = 0;
	child_window_blueprint.cbWndExtra = 0;
	child_window_blueprint.hInstance = handle_of_instance;
	// IDI is the ID for Icon and APPLICATION is the standered windows icon. We send null because all built in icons/cursors get NULL.
	child_window_blueprint.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	child_window_blueprint.hCursor = LoadCursor(NULL, IDC_HAND);
	child_window_blueprint.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	child_window_blueprint.lpszMenuName = TEXT("Menu.");
	child_window_blueprint.lpszClassName = TEXT("Child Window.");
	child_window_blueprint.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&child_window_blueprint)) {
		MessageBox(NULL, TEXT("Failed To Create Child Class."), TEXT("ERROR."), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
	
	HWND child_win = CreateWindowEx( // We create the child window.
		WS_EX_OVERLAPPEDWINDOW, // The style of the window.
		TEXT("Child Window."), // The window class name.
		TEXT("Menu."), // The windows name.
		WS_CHILD | WS_VISIBLE, // Setting it up as a child.
		1371, 488, 160, 300, // X, Y, Width, Height.
		cur_win, // Parent handle. 
		(HMENU)1, // We give the window the child_id 1 and cast it into a HMENU type.
		handle_of_instance, NULL); // Handle for the .exe file and extra stuff for the window(NULL because we don't have).

	if (child_win == NULL) { // If we failed to open the child window: 
		// We print the ERROR message.
		MessageBox(NULL, TEXT("Failed To Create Window."), TEXT("ERROR."), MB_OK | MB_ICONEXCLAMATION);
		return 2;
	}

	// Gets the windows spot ready to be painted in and puts painting the inside in the waiting queue. We create it maximized.
	ShowWindow(cur_win, SW_SHOWMAXIMIZED);
	// Forcibly paints the window in now.
	UpdateWindow(cur_win);

	// Gets the child window's spot ready. The flag is for tellimg what way to prepare the window(minimized, full screen, normal and so on)
	ShowWindow(child_win, flag_min_max_normal);
	// Forcibly paints the window in now.
	UpdateWindow(child_win);

	MSG msg;
	// BOOL for the value of the message. As long as its not 0 or -1 we don't care what it is
	BOOL get_message_val;

	// While the value is not 0. If its 0 than we got a termination message so we stop getting messages.
	while ((get_message_val = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		// If the message is -1 that means that the hWnd is invalid so we give an error.
		if (get_message_val == -1) {
			MessageBox(NULL, TEXT("Message Error."), TEXT("ERROR."), MB_OK | MB_ICONERROR);
			return -1;
		}

		// If the message is not a message for our modeless dialog.
		if (handle_to_settings == NULL || !IsDialogMessage(handle_to_settings, &msg)) {
			// We need to translate the message because pressing a key can have a few meanings depending on shift, keyboared language and more.
			// It returns true if it was translated else false. If we press a key without any other stuff(shift,...) it doesnt translate.
			TranslateMessage(&msg);
			// Tells the window to run the message.
			DispatchMessage(&msg);
		}
	}

	// We return that because wPram gets the number from PostQuitMessage. 
	return (int)msg.wParam;
}

static LRESULT CALLBACK WindowProc(HWND key_of_window, UINT code_of_msg, WPARAM wParam, LPARAM lParam) {
	switch (code_of_msg) {

		// Called when a menu item is selected or control sends a message or an accelerator key is pressed.
		case WM_COMMAND: {
			// LOWORD retruns the last 16 bits. Simular use to EXIRSIZEMOVE.
			switch (LOWORD(wParam)) {
				// If the exit was pressed in the menu.
				case ID_FILE_EXIT: {
					// Same as PostMessage but here we go to the front of the queue. 
					SendMessage(key_of_window, WM_CLOSE, 0, 0);
					break;
				}

				// If the New Game was pressed in the menu
				case ID_FILE_NEW_GAME:
				{
					INT_PTR ans = DialogBox(
						GetModuleHandle(NULL),           // We get the hinstance of the current process.
						MAKEINTRESOURCE(IDD_BASIC_DIALOG), // The ID of the dialog in LWCTSTR.
						key_of_window,                   // The main window.
						AboutDialogProc                  // The function that controls the dialog(its lower dowm).
					);

					// If the start game button was pressed in the dialog box
					if (ans == ID_START_GAME_BUTTON)
					{
						//// We have started a game since last cancel.
						//cancel_pressed = 0;
						start_game(key_of_window);
						break;
					}

					// If the cancel button was pressed.
					else if (ans == ID_CANCEL_DIALOG || ans == IDCANCEL) {
						// We add one to the amount of canceld pressed. We cant change the text here because the dialog is about to be destroyed so it wont save text changes.
						cancel_pressed++;
						break;
					}

					// If there was an error (ans == -1)
					else {
						MessageBox(key_of_window, L"Dialog failed.", L"ERROR", MB_OK | MB_ICONERROR);
						break;
					}
				}

				// If the help was klicked in the setting menu.
				case ID_SETTINGS_HELP_OPEN:
				{

					// If it has not been created yet.
					if (!IsWindow(handle_to_settings)) {
						handle_to_settings = CreateDialog(
							GetModuleHandle(NULL),
							MAKEINTRESOURCE(IDD_MODELESS_DIALOG),
							key_of_window,
							SettingsDialogProc // We use a different function because we need to handle the case differently when we use a modeless diaglog.
						);

						// If it was created without an error.
						if (handle_to_settings != NULL) {
							// We pop up the dialog. (SW_SHOW a mode)
							ShowWindow(handle_to_settings, SW_SHOW);
						}

						// If there was an error.
						else {
							MessageBox(key_of_window, L"Failed To Create Modeless Dialog.", L"ERROR", MB_OK | MB_ICONERROR);
						}
					}

					// If the dialog is hidden.
					else if(!IsWindowVisible(handle_to_settings)){
						// We bring the dialog back into view.
						ShowWindow(handle_to_settings, SW_SHOW);
					}

					// If it is visable.
					else {
						// We hide the window.
						ShowWindow(handle_to_settings, SW_HIDE);

						// If it is stile set to SW_HIDE or it got closed.
						if (IsWindowVisible(handle_to_settings) || !IsWindow(handle_to_settings)) {
							MessageBox(handle_to_settings, L"Failed To Hide Window", L"ERROR", MB_OK | MB_ICONERROR);
						}
					}

					break;
				}
				
				// If the background colour button was pressed in the settings menu.
				case ID_SETTINGS_BK_COLOUR: {
					// If it is violet, which is the last colour we circle back to the first, black.
					if (bk_colour == VIOLETBK) {
						bk_colour = BLACKBK;
					}
					// If we are not at the end we go to the next colour.
					else {
						bk_colour++;
					}
					// We want to call the function to change the background in WM_PAINT.
					InvalidateRect(key_of_window, NULL, TRUE);
					break;
				}

				// If the start button was pressed.
				case ID_START_BUTTON_MAIN: {
					switch (HIWORD(wParam)) {
						// If the button was pressed.
						case BN_CLICKED: {
							// We hide the button. We only destroy in WM_DESTROY because we need to have it there if the button wasnt pressed, so we just out it only there to make it simpler.
							ShowWindow(start_buttons.start_button, SW_HIDE);
							ShowWindow(start_buttons.cur_game_mode, SW_HIDE);
							ShowWindow(start_buttons.move_game_mode_left, SW_HIDE);
							ShowWindow(start_buttons.move_game_mode_right, SW_HIDE);

							InvalidateRect(key_of_window, NULL, TRUE);
							start_game(key_of_window);
						}
					}
					break;
				}

				// If the < button was pressed to switch the game mode. 
				case ID_SWITCH_GAME_MODE_LEFT: {
					change_game_mode(&start_buttons, -1);
					break;
				}

				// If the > button was pressed to switch the game mode.
				case ID_SWITCH_GAME_MODE_RIGHT: {
					change_game_mode(&start_buttons, 1);
					break;
				}
			}
			break;
		}
		
		// Automatically called when we create the window. We can also manualy call.
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT pt;

			// We get the window ready to be painted.
			hdc = BeginPaint(key_of_window, &pt);

			// We set the background colour if the flag is true.
			SetWindowBackground(hdc, pt, -1, &bk_colour);

			// Prints the words for the window.
			words_for_window(key_of_window, hdc, game_state);

			if (game_state == GAME_OVER) {
				// We call the function to create the whole smiley face.
				smiley_face(hdc);
			}

			// If the used dicided to start the game.
			else if (game_state == GAME_STARTED) {
				
				// We get the game dimentions.
				int cell_size = get_cell_size(key_of_window);

				// Setting an outline for the game area.
				if (bk_colour == BLACKBK) {
					HBRUSH white_outline = CreateSolidBrush(RGB(255, 255, 255));
					RECT rect = { .right = GRID_W * cell_size,.left = 0,.bottom = GRID_H * cell_size,.top = 0 };
					FrameRect(hdc, &rect, white_outline);
					DeleteObject(white_outline);
				}
				else {
					HBRUSH black_outline = CreateSolidBrush(RGB(0, 0, 0));
					RECT rect = { .right = GRID_W * cell_size,.left = 0,.bottom = GRID_H * cell_size,.top = 0 };
					FrameRect(hdc, &rect, black_outline);
					DeleteObject(black_outline);
				}

				// Creating the snake and giving him a colour. If the background is green we make him black so we can see him, if its red the apples are green so we agian make the snake black.
				if (bk_colour == GREENBK || bk_colour == LIGHTGREENBK || bk_colour == REDBK) {
					HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
					// We start at the second spot so we can make the head a different type.
					for (int i = 1; i < (int)my_snake.length; ++i) {
						RECT block;
						POINT* cur_place = ((POINT*)place(&my_snake.snake, i));
						block.left = (cur_place->x) * cell_size + 1;
						block.top = (cur_place->y) * cell_size + 1;
						block.right = block.left + cell_size - 2;
						block.bottom = block.top + cell_size - 2;
						FillRect(hdc, &block, black);
					}

					// Same idea as in the else.
					RECT head_to_colour;
					POINT* head_point = (POINT*)front(&my_snake.snake);
					head_to_colour.left = head_point->x * cell_size + 1;
					head_to_colour.top = head_point->y * cell_size + 1;
					head_to_colour.right = head_to_colour.left + cell_size - 2;
					head_to_colour.bottom = head_to_colour.top + cell_size - 2;
					// We call the functionn that draws the head with the snake bitmap.
					fill_square_with_bitmap(hdc, &head_to_colour, &default_my_colouring_stuff);

					// We delete the HBRUSH we created.
					DeleteObject(black);
				}
				else {
					HBRUSH green = CreateSolidBrush(RGB(0, 255, 0));
					// Same idea as the if 
					for (int i =1; i < (int)my_snake.length; ++i) {
						RECT block;
						block.left = (((POINT*)place(&my_snake.snake, i))->x) * cell_size + 1;
						block.top = (((POINT*)place(&my_snake.snake, i))->y) * cell_size + 1;
						block.right = block.left + cell_size - 2;
						block.bottom = block.top + cell_size - 2;
						FillRect(hdc, &block, green);
					}

					RECT head_to_colour;
					POINT* head_point = (POINT*)front(&my_snake.snake);
					head_to_colour.left = head_point->x * cell_size + 1;
					head_to_colour.top = head_point->y * cell_size + 1;
					head_to_colour.right = head_to_colour.left + cell_size - 2;
					head_to_colour.bottom = head_to_colour.top + cell_size - 2;
					fill_square_with_bitmap(hdc, &head_to_colour, &default_my_colouring_stuff);

					DeleteObject(green);
				}

				// Drawing apple if flag is TRUE
				if (apple.apple_draw) {
					RECT to_fill;
					// We put the apple at the random spots we got and make it 1 cell by 1 cell
					to_fill.left = apple.apple.x * cell_size;
					to_fill.right = to_fill.left + cell_size;
					to_fill.top = apple.apple.y * cell_size;
					to_fill.bottom = to_fill.top + cell_size;

					// If the background is red we donr want to make a red apple
					if (bk_colour == REDBK) {
						HBRUSH green = CreateSolidBrush(RGB(0, 255, 0));
						FillRect(hdc, &to_fill, green);
						DeleteObject(green);
					}
					else {
						HBRUSH red = CreateSolidBrush(RGB(255, 0, 0));
						FillRect(hdc, &to_fill, red);
						DeleteObject(red);
					}
				}
				
				// We call the function that prints our game score.
				print_game_score(hdc, cell_size, &default_my_colouring_stuff, score, high_score);
			}

			// We must end the painting.
			EndPaint(key_of_window, &pt);
			break;
		}

		// Called whenever we get a fillrect with the change background as true.
		case WM_ERASEBKGND: {
			// We tell it that we will deal with the background changing.
			return 1;
		}

		// If the user is done resizing.
		case WM_SIZE:
		{
			LONG_PTR style;
			// We only create for the parent window not child.
			if (!((style = GetWindowLongPtr(key_of_window, GWL_STYLE)) & WS_CHILD))
			{
				// We create the font we use to write the game score and high score. We do it here so we dont end up creating it a lot of times. Deleted in WM_DESTROY. It should already be created but we need to resize.
				int font_size = -(get_cell_size(key_of_window) * 3 / 4);
				create_my_font(&default_my_colouring_stuff.main_font, font_size);
			}

			InvalidateRect(key_of_window, NULL, TRUE);
			break;
		}

		// If a key was pressed.
		case WM_KEYDOWN:
		{
			switch(wParam)
			{
				// If that key was escape.
				case VK_ESCAPE: {
					// We close the window.
					if (MessageBox(key_of_window, L"Are You Sure You Want To Quit?", L"Quit Menu", MB_YESNO | MB_ICONQUESTION) == IDYES) {
						DestroyWindow(key_of_window);
					}
					else {
						return 0;
					}
					break;
				}

				// If the key is 'P'(the spot of P on the keyboard). 
				case  'P': {
					// We create a char* of file path we make it the max size a path can be(260).
					WCHAR path[MAX_PATH];
					// We call the function to get the file name. The name goes into the path and we send MAX_PATH as the size of it. We use the GetModuleHandle function to get the HINSTANCE to tell the program which .exe file we want the path 2. We send NULL so we get the current files HINSTANCE.
					GetModuleFileName(GetModuleHandle(NULL), path, MAX_PATH);
					// We create the new message the size of MAX_PATH+32 because the length of the addition is 14, but its better to put a power of 2.
					WCHAR msg[MAX_PATH + 32];

					// We copy the "Close File: " to the begining of message. 
					lstrcpyW(msg, L"Close File: ");
					// We copy the path into it at the end.
					lstrcatW(msg, path);
					// We copy the "?" to the end of the msg.
					lstrcatW(msg, L"?");

					int ans = MessageBox(NULL, msg, L"Closing File: ", MB_YESNO | MB_ICONQUESTION);
					if (ans == IDNO) {
						return DefWindowProc(key_of_window, code_of_msg, wParam, lParam);
					}
					else {
						DestroyWindow(key_of_window);
					}
					break;
				}

				// We dont put a break in case of the left arrow we want to have the same as W so the code will just continue to there and we will put a break at the end of it.
				case VK_LEFT: 
				case 'A': {
					// If we can set next direction as left we do so.
					queue_direction_if_valid(LEFT);
					break;
				}

				// Same logic here
				case VK_RIGHT:
				case 'D': {
					queue_direction_if_valid(RIGHT);
					break;
				}

				case VK_UP:
				case 'W': {
					queue_direction_if_valid(UP);
					break;
				}

				case VK_DOWN:
				case 'S': {
					queue_direction_if_valid(DOWN);
					break;
				}

				default: {
					// Use the default.
					return DefWindowProc(key_of_window, code_of_msg, wParam, lParam);
				}
			}
			break;
		}

		// If the user closes the widnow with the x at the top right.
		case WM_CLOSE:
		{
			DestroyWindow(key_of_window);
			break;
		}

		// Automatically caleed when window is created. 
		case WM_CREATE: {
			// For WM_CREATE the lParam hold the info about the window mainly(what we need) the style.
			CREATESTRUCT* create_info = (CREATESTRUCT*)lParam;
			// If the window we are at now is a child, we dont do any of the other stuff so we break.
			if ((create_info->style & WS_CHILD)) {
				break;
			}

			// good font size.
			int cell_size = get_cell_size(key_of_window);
			int font_size = -(cell_size * 3 / 4);
			// If we failed to create 
			if (!create_drawing_resources(&default_my_colouring_stuff, global_hInstance, font_size, IDB_SNAKE_HEAD, 0)) {
				MessageBox(key_of_window, L"Closing Window, Drawing Resources Failed.", L"ERROR", MB_OK | MB_ICONERROR);
				return -1;
			}

			// We maximise our window now making button placement consistent.
			ShowWindow(key_of_window, SW_SHOWMAXIMIZED);

			// We get the high score of the game.It is only needed once per run and is cleaner here than in the main.
			high_score = get_high_score();

			// We get the start button up and running to sart the game. We only do it for the main window and not the child. If it failed we leave the creation of the window.
			if (!create_window_buttons(&start_buttons, key_of_window)) {
				MessageBox(key_of_window, L"Closing Window, Window Start Buttons Failed.", L"ERROR", MB_OK | MB_ICONERROR);
				return -1;
			}

			return 0;
		}

		// Automatically called after WM_CLOSE.
		case WM_DESTROY:
		{
			// We need to kill the timer before we end the code so it doesnt continue going off.
			KillTimer(key_of_window, IDT_TIMER1);
			KillTimer(key_of_window, IDT_TIMER_FOR_APPLE);
			KillTimer(key_of_window, IDT_TIMER_TICK_SPEED);
			destroy_queue(&my_snake.snake);
			destroy_queue(&buttons_pressed);

			// We destroy the array with all the pointers to the windows that have each game mode button.
			destroy_window_buttons(&start_buttons);

			// We destroy my drawing resource.
			destroy_drawing_resources(&default_my_colouring_stuff);

			// If the case is to leave we end the program and window. We need the PostQuitMessage becasue if we dont have it the function will return 0 without closing the window.
			PostQuitMessage(0);
			return 0;
		}

		// If we have a timer go off.
		case WM_TIMER: {
			switch (wParam) {
				// The timer for moving the snake went off.
				case IDT_TIMER1: {
					// We get the cell size for updating the areas of screen that were updated (score, head cell, tail cell and maybe high score).
					int cell_size = get_cell_size(key_of_window);

					// We set the snake to move in next direction.
					int next_direction = my_snake.cur_direction;

					// If we had an element to pop we put it as the next direction. We pop until we get a key that is valid.
					while(pop(&buttons_pressed, &next_direction)) {
						if (is_valid_turn(next_direction, &my_snake)) {
							my_snake.cur_direction = next_direction;
							break;
						}
					}

					POINT* cur_spot = (POINT*)(front(&my_snake.snake));
					// If we ate an apple we add one at the end meaning we can just move everything one forward but move the tail one back. We copy by value so we put that value at the end not a pointer becasue then it moves with the for loop.
					POINT to_add;
					copy(tail(&my_snake.snake), &to_add, sizeof(POINT));

					// We move each part of the snake from tail to the spot infront of it(exept the head).
					for (int i = my_snake.length - 1; i > 0; --i) {
						POINT* cur = ((POINT*)place(&my_snake.snake, i));
						POINT* prev = ((POINT*)place(&my_snake.snake, i - 1));
						*cur = *prev;
					}

					// Moves head to correct spot.
					switch (my_snake.cur_direction) {
						case UP: {
							// If we made it to the top of the screen.
							if (cur_spot->y <= 0) {
								// If we are at the right edge.
								if (cur_spot->x >= GRID_W - 1) {
									my_snake.cur_direction = LEFT;
									cur_spot->x--;
								}
								else {
									my_snake.cur_direction = RIGHT;
									cur_spot->x++;
								}
							}
							// If we had  a valid spot.
							else {
								// We do y-- because y=0 is the top (y grows downwards).
								cur_spot->y--;
							}
							break;
						}
						case DOWN: {
							// Same logic just for bottom and not top
							if (cur_spot->y >= GRID_H - 1) {
								if (cur_spot->x >= GRID_W - 1) {
									my_snake.cur_direction = LEFT;
									cur_spot->x--;
								}
								else {
									my_snake.cur_direction = RIGHT;
									cur_spot->x++;
								}
							}
							else
							{
								cur_spot->y++;
							}
							break;
						}
						case RIGHT: {
							// If we make it to right barier we go up.
							if (cur_spot->x >= GRID_W - 1) {
								if (cur_spot->y <= 0) {
									my_snake.cur_direction = DOWN;
									cur_spot->y++;
								}
								else {
									my_snake.cur_direction = UP;
									cur_spot->y--;
								}
							}
							else 
							{
								cur_spot->x++;
							}
							break;
						}
						case LEFT: {
							if (cur_spot->x <= 0) {
								if (cur_spot->y <= 0) {
									my_snake.cur_direction = DOWN;
									cur_spot->y++;
								}
								else {
									my_snake.cur_direction = UP;
									cur_spot->y--;
								}
							}
							else
							{
								cur_spot->x--;
							}
							break;
						}
						default: {
							// If the dirction was bad we set it to UP.
							my_snake.cur_direction = UP;
						}// default
					}// switch (my_snake.cur_direction)
					
					// If the snake is in the same spot as the apple.
					if (apple.apple_draw && is_snake_head(&apple.apple, &my_snake)) {
						if (push(&my_snake.snake, &to_add)) {
							my_snake.length++;
						}
						apple.apple_draw = FALSE;
						score++;
						
						// Update the score and high score box after eating the apple. (If score>=high score we update it too).
						RECT score_spot;
						score_spot.left = cell_size * (GRID_W + 2);
						// We go 15 cells from where the score started because the score is more than one cell and depends on the number there so we do 15 just in case.
						score_spot.right = score_spot.left + cell_size * 15;
						score_spot.top = cell_size;
						// We go down 2 cells to update the score and the high score (might not change but again ust in case score>high_score).
						score_spot.bottom = cell_size * 3;
						InvalidateRect(key_of_window, &score_spot, FALSE);
					}

					// After we moved the head we check if it hits any of the other points on the snake ending the game. We start at 1 becasue 0 is the head.
					if (point_on_snake(cur_spot, &my_snake, 1)) {
						// We kill the timers so the game doesnt run in the background just that w edont see it.
						KillTimer(key_of_window, IDT_TIMER1);
						KillTimer(key_of_window, IDT_TIMER_FOR_APPLE);
						
						// Game is over we lost.
						game_state = GAME_OVER;

						// Redraw the screen getting rid of the game.
						InvalidateRect(key_of_window, NULL, TRUE);

						// We set a timer for 1s so the face stays on screen for 2s
						SetTimer(key_of_window, IDT_TIMER_GAME_OVER, 2000, NULL);

						// Leave the case.
						break;
					}

					// We reset the timer with the cur tick speed it gets changed in the timer for tick speed.
					SetTimer(key_of_window, IDT_TIMER1, cur_timer_speed, NULL);

					invalidate_cell(key_of_window, cur_spot, cell_size);
					// We invalidate the cell behind teh head because the head has a different style.
					invalidate_cell(key_of_window, (POINT*)place(&my_snake.snake, 1), cell_size);
					invalidate_cell(key_of_window, &to_add, cell_size);

					break;
				}// case IDT_TIMER1

				// If the apple timer went off.
				case IDT_TIMER_FOR_APPLE: {
					// We get the cell size so we can update the cells that the apple ent into and remove previous one if needed.
					int cell_size = get_cell_size(key_of_window);

					// If we have an apple on screen now we update the spot to remove the apple.
					if (apple.apple_draw == TRUE) {
						apple.apple_draw = FALSE;
						invalidate_cell(key_of_window, &apple.apple, cell_size);
					}

					// We get random numbers and if they are on the snake we get new ones becasue we dont want the apple on the snake.
					do {
						apple.apple.x = my_rand(0, GRID_W - 1);
						apple.apple.y = my_rand(0, GRID_H - 1);
					} while (point_on_snake(&apple.apple, &my_snake, 0));

					// We update the spot the new apple is in.
					invalidate_cell(key_of_window, &apple.apple, cell_size);

					apple.apple_draw = TRUE;

					// We reset the timer with a new random time for the apple.
					SetTimer(key_of_window, IDT_TIMER_FOR_APPLE, my_rand(1000, 5000), NULL);
					break;
				}

				// If the timer we set for the face staying on screen goes off.
				case IDT_TIMER_GAME_OVER: {
					// We kill the timer because we only want it to run once.
					KillTimer(key_of_window, IDT_TIMER_GAME_OVER);

					// We kill the timer that lowers the tick speed during the game.
					KillTimer(key_of_window, IDT_TIMER_TICK_SPEED);

					// set back to false so we dont draw face again.
					draw_face = FALSE;

					// We redraw the screen getting rid of smiley face.
					InvalidateRect(key_of_window, NULL, TRUE);

					// We create a text a that has in it what we need fot the print in the MessageBox that says the score and the replay?.
					wchar_t text[256];
					swprintf(text, sizeof(text) / sizeof(wchar_t), L"Score: %u\nReplay?", score);

					int ans = MessageBox(key_of_window, text, L"GAME OVER", MB_ICONINFORMATION | MB_YESNO);

					// If the user choose to start the game agian we restart 
					if (ans == IDYES) {
						start_game(key_of_window);
					}
					// If he choose not to we go back to main screen.
					else {
						game_state = NOT_STARTED;
						// We bring back the start button whenwe are in the main scree.
						ShowWindow(start_buttons.start_button, SW_SHOW);
						ShowWindow(start_buttons.cur_game_mode, SW_SHOW);
						ShowWindow(start_buttons.move_game_mode_left, SW_SHOW);
						ShowWindow(start_buttons.move_game_mode_right, SW_SHOW);

						InvalidateRect(key_of_window, NULL, TRUE);
					}

					// If the score we got this game is the high score (high score gets the number from the file at the beggining) we set it here and not in WM_DESTROY because if the code crashes we might lose the high score so its better here.
					if (score > high_score) {
						high_score = score;
						set_high_score(high_score);
					}
					break;
				}

				// If the timer that tells us to lower the tick speed goes off.
				case IDT_TIMER_TICK_SPEED: {
					// If the current tick speed is 10ms we stop lowering because then it gets too fast.
					if (cur_timer_speed > 10) {
						cur_timer_speed--;
					}
					break;
				}

				default: {
					
				}
			} // switch(wPram)
			break;
		}// Case WM_TIMER

		// Any other case.
		default: {
			// Use default for the rest of the codes.
			return DefWindowProc(key_of_window, code_of_msg, wParam, lParam);
		}
	}
	return 0;
}

// Puts a direction in the queue if it is going to be a valid direction when its turn comes up.
static void queue_direction_if_valid(const UINT new_direction) {
	UINT cur = my_snake.cur_direction;
	int* last_in_queue = (int*)tail(&buttons_pressed);
	// We put the value that will come up right before the new key that was pressed.
	if (last_in_queue != NULL) {
		cur = *last_in_queue;
	}
	// If same direction/opposite direction we dont add to queue and leave function.
	if (cur == new_direction || cur == ((new_direction + 2) % 4)) {
		return;
	}
	int to_push = new_direction;
	push(&buttons_pressed, &to_push);
}

// Gets the game ready for starting.
static void start_game(const HWND key_of_window) {
	// we set the flag that says the game has started to true.
	game_state = GAME_STARTED;

	// We get the random numers ready to be used.
	set_up_rand();

	// Reset the tick time from previous game.
	cur_timer_speed = 100;

	// We set the timer that lowers the tick speed every 4 secs.
	SetTimer(key_of_window, IDT_TIMER_TICK_SPEED, 4000, NULL);
	// Setting timer to move the snake.
	SetTimer(key_of_window, IDT_TIMER1, cur_timer_speed, NULL);
	// We give the apple a random amount of time to spawn in. We make teh timer again in WM_TIMER becasuse we want a new random number for each apple.
	SetTimer(key_of_window, IDT_TIMER_FOR_APPLE, my_rand(1000, 5000), NULL);

	// Get the queue of the buttons pressed that need to be executed ready.
	set_button_queue();

	// We reset so the apple and snake so they dont carry over into the new game.
	apple.apple_draw = FALSE;
	set_snake_start_points();

	// We reset the game score for starting a new game 
	score = 0;

	// We repaint the window with the changes.
	InvalidateRect(key_of_window, NULL, TRUE);
}

// A function to get the cell size for our current window size.
static int get_cell_size(HWND key_of_window) {
	RECT screen_size;
	GetClientRect(key_of_window, &screen_size);
	int cell_w = screen_size.right / GRID_W;
	int cell_h = screen_size.bottom / GRID_H;
	int cell_size;
	if (cell_w > cell_h) {
		cell_size = cell_h;
	}
	else {
		cell_size = cell_w;
	}
	return cell_size;
}

// Updates one cell on the window instead of a whole window.
static void invalidate_cell(HWND hwnd, const POINT* cell, const int cell_size) {
	RECT to_fill;
	// We create the rect that we fill in.
	to_fill.left = cell->x * cell_size;
	to_fill.top = cell->y * cell_size;
	to_fill.right = to_fill.left + cell_size;
	to_fill.bottom = to_fill.top + cell_size;

	// We send it to be filled in.
	InvalidateRect(hwnd, &to_fill, FALSE);
}
