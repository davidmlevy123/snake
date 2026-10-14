#include <windows.h>
#include <tchar.h>
#include "additions.h" // The additions i created
#include "resource.h"
#include "queue.h" 
#include "Snake.h"

const wchar_t class_name[] = L"Main Window.";
const wchar_t window_name[] = L"Snake";


// We create our actual snake, in c we cant give default values so we need to give the values when we define it.
Snake my_snake = { .length = 3 , .cur_direction = RIGHT };
static void set_snake_start_points(){
	// Distroy the snake if one still exists and has not been destroyed.
	destroy_queue(&my_snake.snake);
	// We create the snake with the default points (passing NULL).
	my_snake = create_snake(3, RIGHT, NULL, 50);
}

// A queue that has the buttons pressed so we can do them on their time. If we dont have it its possible to do 2 moves per tick breaking the game.
queue buttons_pressed;
static void set_button_queue() {
	// Free mem if a prev game was played so we have a new one without leaking mem.
	destroy_queue(&buttons_pressed);
	buttons_pressed = create_queue(NULL, sizeof(int), 3, 0, 0);
}

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

// A handle to the button taht starts the game.
HWND start_button;
HWND move_game_mode_right;
HWND move_game_mode_left;
HWND cur_game_mode;
HWND* game_mode_windows;

// A global UINT that 
UINT cur_game_mode_int = ID_CLASSIC;

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

// We get the font for drawing the game score and high score.
HFONT font_for_drawing = NULL;

// A brush that has the snake head bitmap to use loded in it.
HBRUSH snake_head_brush = NULL;
// The bitmap with the snake head in it.
HBITMAP snake_head_bitmap = NULL;
// hight and width of the snake head bitmap. 
int snake_head_bitmap_h = 0;
int snake_head_bitmap_w = 0;

// A int that holds the state of our random numbers.
static unsigned long long random_seed = 1;

static LRESULT CALLBACK WindowProc(HWND key_of_window, UINT code_of_msg, WPARAM key_pressed, LPARAM extra_msg_info);
static INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK SettingsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void SetHatchBrushBackground(HDC hdc, BOOL transparent);
static void SetWindowBackground(HDC hdc, PAINTSTRUCT pt, INT n); // n is an option to chose the bk colour, -1 means continue loop.
static void words_for_window(HWND hwnd, HDC hdc);
static void Eyes(HDC hdc);
static void Head(HDC hdc);
static void Mouth(HDC hdc);
static void smiley_face(HDC hdc);
static void queue_direction_if_valid(const UINT new_direction);
static void set_up_rand();
static int  full_range_ran();
static int  my_rand(const int a, const int b); // Gets a random number in between a and b.
static void start_game(const HWND key_of_window);
static BOOL create_my_font(HFONT* to_create, HWND hwnd);
static void print_game_score(HDC hdc, const int cell_size, HFONT my_font);
static BOOL get_high_score_file_path(wchar_t* path);
static void set_high_score();
static void get_high_score();
static int  get_cell_size(HWND key_of_window);
static void invalidate_cell(HWND hwnd, const POINT* cell, const int cell_size);
static void start_button_set_up(HWND parent_window, HWND* button);
static void draw_snake_head(HDC hdc, const RECT* cell);
static void game_modes_button_set_up(HWND parent_window, HWND* left_button, HWND* main_button, HWND* right_button);
static void change_game_mode(HWND parent_window, HWND* main_button, UINT new_game_mode);
static wchar_t* button_int_to_words(UINT val);
static void destroy_window_array();

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
							ShowWindow(start_button, SW_HIDE);
							ShowWindow(cur_game_mode, SW_HIDE);
							ShowWindow(move_game_mode_left, SW_HIDE);
							ShowWindow(move_game_mode_right, SW_HIDE);

							InvalidateRect(key_of_window, NULL, TRUE);
							start_game(key_of_window);
						}
					}
					break;
				}

				// If the < button was pressed to switch the game mode. 
				case ID_SWITCH_GAME_MODE_LEFT: {
					// If we made it to the first ID we loop back because we are moving backwords.
					if (cur_game_mode_int == ID_CLASSIC) {
						cur_game_mode_int = ID_THREE_APPLES_WALLS_DONT_KILL;
					}
					else {
						cur_game_mode_int--;
					}

					change_game_mode(key_of_window, &cur_game_mode, cur_game_mode_int);
					break;
				}

				// If the > button was pressed to switch the game mode.
				case ID_SWITCH_GAME_MODE_RIGHT: {
					// If we make it to the end while going right we circle back.
					if (cur_game_mode_int == ID_THREE_APPLES_WALLS_DONT_KILL) {
						cur_game_mode_int = ID_CLASSIC;
					}
					else {
						cur_game_mode_int++;
					}

					change_game_mode(key_of_window, &cur_game_mode, cur_game_mode_int);
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
			SetWindowBackground(hdc, pt, -1);

			// Prints the words for the window.
			words_for_window(key_of_window, hdc);

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
					draw_snake_head(hdc, &head_to_colour);

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
					draw_snake_head(hdc, &head_to_colour);

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
				print_game_score(hdc, cell_size, font_for_drawing);
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
				// We create the font we use to write the game score and high score. We do it here so we dont end up creating it a lot of times. Deleted in WM_DESTROY.
				create_my_font(&font_for_drawing, key_of_window);
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

			// We maximise our window now making button placement consistent.
			ShowWindow(key_of_window, SW_SHOWMAXIMIZED);

			// We get the high score of the game.It is only needed once per run and is cleaner here than in the main.
			get_high_score();

			// We get the start button up and running to sart the game. We only do it for the main window and not the child.
			start_button_set_up(key_of_window, &start_button);
			game_modes_button_set_up(key_of_window, &move_game_mode_left, &cur_game_mode, &move_game_mode_right);

			// We create a handle to the bitmap with the snake head image.
			snake_head_bitmap = LoadBitmap(global_hInstance, MAKEINTRESOURCE(IDB_SNAKE_HEAD));
			// If it failed to create we send the error message and leave.
			if (snake_head_bitmap == NULL) {
				MessageBox(key_of_window, L"Failed To Load Snake Head Bitmap", L"ERROR", MB_OK | MB_ICONERROR);
				break;
			}
			BITMAP snake_head_info;
			// Gets the info about the snake_head bitmap. 
			GetObject(snake_head_bitmap, sizeof(BITMAP), &snake_head_info);
			// We get the hight and width for the snake head bitmap.
			snake_head_bitmap_h = snake_head_info.bmHeight;
			snake_head_bitmap_w = snake_head_info.bmWidth;

			// We create a brush with that bitmap as its pattern.
			snake_head_brush = CreatePatternBrush(snake_head_bitmap);
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

			// We delete the font we created for writing the current and high score.
			DeleteObject(font_for_drawing);

			// We destroy the button for starting the game.
			DestroyWindow(start_button);

			// We delete the brush we used for drawing the snake head in the end.
			DeleteObject(snake_head_brush);
			// We delete the handle to the bitmap with the snake head in it
			DeleteObject(snake_head_bitmap);

			// We destroy the array with all the pointers to the windows that have each game mode button.
			destroy_window_array();

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
						ShowWindow(start_button, SW_SHOW);
						ShowWindow(cur_game_mode, SW_SHOW);
						ShowWindow(move_game_mode_left, SW_SHOW);
						ShowWindow(move_game_mode_right, SW_SHOW);

						InvalidateRect(key_of_window, NULL, TRUE);
					}

					// If the score we got this game is the high score (high score gets the number from the file at the beggining) we set it here and not in WM_DESTROY because if the code crashes we might lose the high score so its better here.
					if (score > high_score) {
						high_score = score;
						set_high_score();
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

static INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		// Called the moment before the dialog appears on screen
		case WM_INITDIALOG:
		{
			// If the cancel was pressed we 
			if (cancel_pressed != 0) {
				WCHAR final_text[32];
				// We add the number to the end of the final text. We use %u because cancel_pressed is a UINT.
				swprintf_s(final_text, 32, L"Cancel: %u", cancel_pressed);
				// We set the new dialogs text in the cancel box to "Cancel: x". x is the amount of cancels since last game.
				SetDlgItemText(hwndDlg, ID_CANCEL_DIALOG, final_text);
			}

			return (INT_PTR)TRUE;
		}

		// Handles buttons clicked inside the dialog
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {

				// If the start game button was pressed. Equivalent to WM_CREATE for a window. The controls are already created by this point.
				case  ID_START_GAME_BUTTON: {
					//// We get the length of the text in the cancel button in the dialog. Doesnt include null terminator.
					//int len = GetWindowTextLength(GetDlgItem(hwndDlg, ID_CANCEL_DIALOG));
					//if (len > 0) {
					//	// We create the buffer. We use len+1 becasue we have to include the null terminator.
					//	LPWSTR buffer = (LPWSTR)calloc(len + 1, sizeof(wchar_t));
					//	// We get the actual text. It returns an int value for the amount of data we read not including the null terminator.
					//	GetDlgItemText(hwndDlg, ID_CANCEL_DIALOG, buffer, len + 1);

					//	// We clode the dialog
					//	EndDialog(hwndDlg, LOWORD(wParam));

					//	if (len == 7) {
					//		MessageBox(NULL, L"No Cancels", L"Cancels", MB_OK | MB_ICONINFORMATION);
					//	}
					//	else 
					//	{
					//		MessageBox(NULL, buffer, L"Cancels", MB_OK | MB_ICONINFORMATION);
					//	}
					//	free(buffer);
					//}

					//else {
					// This destroys the dialog and unfreezes the main game window. It is needed because a dialog is something we created, like a window.

					EndDialog(hwndDlg, LOWORD(wParam));
					//}					
					return (INT_PTR)TRUE;
				}

				// If the cancel button was pressed.
				case ID_CANCEL_DIALOG: {
					EndDialog(hwndDlg, LOWORD(wParam));
					return (INT_PTR)TRUE;
				}
				
				// If the x was pressed.
				case IDCANCEL:
				{
					EndDialog(hwndDlg, LOWORD(wParam));
					return (INT_PTR)TRUE;
				}
			}
			return (INT_PTR)FALSE;
			break;
		}

		default: {
			// Return FALSE if we didn't handle the message.
			return (INT_PTR)FALSE;
		}
	}
}

// Function for modeless dialog.
static INT_PTR CALLBACK SettingsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG: {
			return (INT_PTR)TRUE;
			break;
		}
		
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				// If the hide button was pressed
				case ID_HIDE_DIALOG: {
					// We hide the dialog(still open just not in view.
					ShowWindow(hwndDlg, SW_HIDE);

					// If it is stile set to SW_HIDE or it got closed.
					if (IsWindowVisible(handle_to_settings) || !IsWindow(handle_to_settings)) {
						MessageBox(handle_to_settings, L"Failed To Hide Window", L"ERROR", MB_OK | MB_ICONERROR);
					}
					return (INT_PTR)TRUE;
					break;
				}

				// If the x at the top was pressed.
				case IDCANCEL:
				{
					// We use destroy DestroyWindow because that is what is used for a task that did not freeze everything else(a normal dialog does).
					DestroyWindow(hwndDlg);
					handle_to_settings = NULL;
					return (INT_PTR)TRUE;
					break;
				}
			}
			break;
		}

		// If we use a system command.
		case WM_SYSCOMMAND: {
			// If we try to move the dialog.
			if ((wParam & 0xFFF0) == SC_MOVE) {// We only look at the first 16 digs because SC is 16 bits
				// We dont move it because we want ot lock it in place.
				return (INT_PTR)TRUE;
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}

static void SetHatchBrushBackground(HDC hdc, BOOL transparent) {
	// We create a brush that is Blue. 
	HBRUSH hSolidBrush = CreateSolidBrush(RGB(0, 0, 255));
	// It fills the empty spaces between the hatch lines.
	SetBkColor(hdc, RGB(255, 0, 0));

	// If we want it to be transparend type we set it to it else opaque. (types of background settings)
	if (transparent == TRUE) {
		SetBkMode(hdc, TRANSPARENT);
	}
	else {
		SetBkMode(hdc, OPAQUE);
	}

	// We get the default brush hdc had so we can put it back in at the end to allow us to delete the other brushes. We also equip hSolidBrush to hdc.
	HBRUSH default_brush = (HBRUSH)SelectObject(hdc, hSolidBrush);

	// If we failed to create hSolidBrush.
	if (hSolidBrush == NULL) {
		// We dont have to delete anything because we failed to create the first brush.
		MessageBox(NULL, L"Failed To Create Hatch Brush.", _T("ERROR."), MB_OK);
		return;
	}
	// Create a Rectangle with those dimentions and use hdc on it meaning use the brush. (0,0) is top left corner.
	Rectangle(hdc, 50, 40, 400, 500);
	// We create a hatch brush that is vertical lines in black.
	HBRUSH hHatchBrush = CreateHatchBrush(HS_VERTICAL, RGB(0, 0, 0));

	// If we failed to create the brush:
	if (hHatchBrush == NULL) {
		// Give hdc the default brush back so we can delete the brush(hSolidBrush because he was already created without a problem).
		SelectObject(hdc, default_brush);
		DeleteObject(hSolidBrush);
		MessageBox(NULL, L"Failed To Create Hatch Brush.", _T("ERROR."), MB_OK);
		return;
	}

	// We bunch the hdc with the new brush, replacing the old one.
	SelectObject(hdc, hHatchBrush);
	// We create a new rectangle in hdc with our new brush(it sits on top of the old one).
	Rectangle(hdc, 50, 40, 400, 500);

	// We put the default brush back in because we cant delete a brush that is in hdc 
	SelectObject(hdc, default_brush);
	// We manually delete both brushes because they dont get deleted automatically (exept if they are default brushes).
	DeleteObject(hSolidBrush);
	DeleteObject(hHatchBrush);
}

// Function to set the colour of the background. 
static void SetWindowBackground(HDC hdc, PAINTSTRUCT pt, INT n) {
	// We use the FillRect function to fill in the rectangle in black. The rectangle is the window in this case.
	int save_bk_colour = bk_colour;
	// If we got a specified value we use it.
	if (n != -1) {
		bk_colour = n;
	}
	switch (bk_colour) {
		case BLACKBK: {
			HBRUSH colour = CreateSolidBrush(RGB(0, 0, 0));
			HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
			FillRect(hdc, &pt.rcPaint, colour);
			SelectObject(hdc, default_brush);
			DeleteObject(colour);
			break;
		}
		case REDBK: {
			HBRUSH colour = CreateSolidBrush(RGB(255, 0, 0));
			HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
			FillRect(hdc, &pt.rcPaint, colour);
			SelectObject(hdc, default_brush);
			DeleteObject(colour);
			break;
		}
		case GREENBK: {
			HBRUSH colour = CreateSolidBrush(RGB(0, 255, 0));
			HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
			FillRect(hdc, &pt.rcPaint, colour);
			SelectObject(hdc, default_brush);
			DeleteObject(colour);
			break;
		}
		case BLUEBK: {
			HBRUSH colour = CreateSolidBrush(RGB(0, 0, 255));
			HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
			FillRect(hdc, &pt.rcPaint, colour);
			SelectObject(hdc, default_brush);
			DeleteObject(colour);
			break;
		}
		case WHITEBK: {
			HBRUSH colour = CreateSolidBrush(RGB(255, 255, 255));
			HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
			FillRect(hdc, &pt.rcPaint, colour);
			SelectObject(hdc, default_brush);
			DeleteObject(colour);
			break;
		}
		case LIGHTGREENBK: {
			HBRUSH colour = CreateSolidBrush(RGB(144, 238, 144));
			HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
			FillRect(hdc, &pt.rcPaint, colour);
			SelectObject(hdc, default_brush);
			DeleteObject(colour);
			break;
		}
		case LIGHTBLUEBK: {
			HBRUSH colour = CreateSolidBrush(RGB(144, 213, 255));
			HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
			FillRect(hdc, &pt.rcPaint, colour);
			SelectObject(hdc, default_brush);
			DeleteObject(colour);
			break;
		}
		case VIOLETBK: {
			HBRUSH colour = CreateSolidBrush(RGB(151, 89, 154));
			HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
			FillRect(hdc, &pt.rcPaint, colour);
			SelectObject(hdc, default_brush);
			DeleteObject(colour);
			break;
		}
		default: {
			int ans = MessageBox(global_cur_win, L"Invalid Background Colour.\nSet To Default?", L"ERROR", MB_YESNO | MB_ICONERROR);
			// Set to defaule, Black
			if (ans == IDYES) {
				HBRUSH colour = CreateSolidBrush(RGB(0, 0, 0));
				HBRUSH default_brush = (HBRUSH)SelectObject(hdc, colour);
				FillRect(hdc, &pt.rcPaint, colour);
				SelectObject(hdc, default_brush);
				DeleteObject(colour);
			}
			// Leave and dont change
			else {
				return;
			}
		}
	}
	// If we ended up using n we want to go back to the samne spot in cycle.
	bk_colour = save_bk_colour;
}

// Function to add main words to the window. If child: "This Is A Child Window." , If its has no parent: "Welcome To My Window.".
static void words_for_window(HWND hwnd, HDC hdc) {
	LONG_PTR style;

	// If the coordinates of the window are relitive to a parent that means it must be a child. 
	if ((style = GetWindowLongPtr(hwnd, GWL_STYLE)) & WS_CHILD)
	{
		TCHAR text[] = L"This Is A Child Window.";
		SetTextColor(hdc, RGB(0, 255, 0));
		TextOut(hdc, 5, 5, text, (int)_tcslen(text));
	}
	else {
		// We set the background of text and text colours.
		TCHAR text[] = L"Welcome To My Window.";
		SetBkColor(hdc, RGB(255, 255, 255));
		SetTextColor(hdc, RGB(0, 255, 0));
		if (game_state == NOT_STARTED) {
			// We print the text with our colours. We do that only if we havent started the game so we dont block the game with the words. 
			TextOut(hdc, 5, 5, text, (int)_tcslen(text));
		}
	}
}

// We create the eyes for the similey face.
static void Eyes(HDC hdc) {
	// We set the background mode to TRANSPARENT mode.
	SetBkMode(hdc, TRANSPARENT);

	// We create the white part of the eye and put it in hdc.
	HBRUSH outer_eye_brush = CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH default_brush = (HBRUSH)SelectObject(hdc, outer_eye_brush);

	//Create each eye
	Ellipse(hdc, 625, 220, 675, 270);
	Ellipse(hdc, 825, 220, 875, 270);

	// We create the inner eye.
	HBRUSH inner_eye_brush = CreateSolidBrush(RGB(0, 0, 0));
	SelectObject(hdc, inner_eye_brush);
	Ellipse(hdc, 645, 243, 657, 255);
	Ellipse(hdc, 845, 243, 857, 255);

	// We reput the default brush so we can delete the ones we created.
	SelectObject(hdc, default_brush);

	// We delete the custom brushes we created.
	DeleteObject(outer_eye_brush);
	DeleteObject(inner_eye_brush);
}

// We create the head for the similey face.
static void Head(HDC hdc) {
	// We create a blue brush for the head and put it in hdc.
	HBRUSH head = CreateSolidBrush(RGB(0, 0, 255));
	HBRUSH default_brush = (HBRUSH)SelectObject(hdc, head);

	// We create the head with the brush.
	Ellipse(hdc, 500, 100, 1000, 600);

	// We reput the default brush so we can delete the ones we created.
	SelectObject(hdc, default_brush);

	// We detele the custom brush we created.
	DeleteObject(head);
}

// Function to create the mouth.
static void Mouth(HDC hdc) {
	// We create a green brush for the mouth and put it in hdc
	HBRUSH mouth_brush = CreateSolidBrush(RGB(255, 0, 0));
	HBRUSH default_brush = SelectObject(hdc, mouth_brush);

	// A function to create an arc. It has Left, Top, Right, Bottom, StartX, StartY, EndX, EndY.
	Chord(hdc, 600, 425, 900, 545, 600, 485, 900, 485);

	// Same logic as always.
	SelectObject(hdc, default_brush);
	DeleteObject(mouth_brush);
}

// Funtion to create the smiley face.
static void smiley_face(HDC hdc) {
	Head(hdc);
	Eyes(hdc);
	Mouth(hdc);
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

// A void that sets up getting random numbers. We call it once at the beggining of the game.
static void set_up_rand() {
	// Gets the number of milliseconds since pc has started. It will be different for every run.
	random_seed = GetTickCount64();
}

// Return a random number in the range 0->2^15-1.
static int full_range_ran() {
	// This is the formula used by the Microsoft C compiler. It multiplies the seed by a prime number and adds an offset.
	random_seed = (random_seed * 214013 + 2531011);

	// We use that becasue only 16 bits end up random so we shift it, we do &7FFF to get rid of negetive numbers.
	// If we want a number than 32767(2^15-1) we use the first digits as one random number and the next 16 as another and so on.
	return (random_seed >> 16) & 0x7FFF;
}

static int my_rand(const int a, const int b) {
	return a + (full_range_ran() % (b - a + 1));
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

static BOOL create_my_font(HFONT* to_create, HWND hwnd) {
	int cell_size = get_cell_size(hwnd);
	int font_size = -(cell_size * 3 / 4);

	// If we have already created it and we want to recreate beacuase the widow size was changed.
	if (*to_create) {
		DeleteObject(*to_create);
		*to_create = NULL;
	}

	// We create the actual font.
	*to_create =
		CreateFont(font_size, // cHeight - the height of the character if negative. If positive the height of the cell that the char is in so char size would be a bit smaller and for 0 the default for that font.
			0, 0, 0,
			FW_ULTRABOLD, // Font type (i.e. normal, bold, light, super bold...)
			FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, // The char's we have in our font HEBREW_CHARSET for a hebrew font.
			OUT_DEFAULT_PRECIS, // Lets windows find the font that best matches my request. I can use other values to tell it my priorities.
			CLIP_DEFAULT_PRECIS, // Same idea but with clipping (clipping - where it is allowed to draw the chars. Different values say dont darw at all if its not in ad others).  
			CLEARTYPE_QUALITY, // Simular idea.
			FF_MODERN, // The family of fonts we choose from (i.e. modern, roman, old english...)
			L"font of score" /*name of font*/); // for more info .https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta.

	// If the font failed to create we send a message and leave the function.
	if (!(*to_create)) {
		MessageBox(global_cur_win, L"Failed To Create Font.", L"ERROR", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

// Prints the current score of game onto the screen.
static void print_game_score(HDC hdc, const int cell_size, HFONT my_font) {
	// We select the new font on to the hdc. We save the old font so we can put it back together with hdc.
	HFONT old_font = (HFONT)SelectObject(hdc, my_font);

	// We create the text that we will be printing on the screen.
	wchar_t text[256];
	swprintf_s(text, sizeof(text) / sizeof(wchar_t), L"current score: %u", score);

	// We print out the text we want to in its spot.
	TextOut(hdc, cell_size * (GRID_W + 2), cell_size, text, (int)wcslen(text));
	
	// We print the bigger one between our current score and the high score.
	UINT high_score_print;
	if (high_score < score) {
		high_score_print = score;
	}
	else {
		high_score_print = high_score;
	}

	// Same thing but we print the high score under.
	wchar_t high_score_text[256];
	swprintf_s(high_score_text, sizeof(high_score_text) / sizeof(wchar_t), L"high score: %u", high_score_print);
	TextOut(hdc, cell_size * (GRID_W + 2), cell_size * 2, high_score_text, (int)wcslen(high_score_text));

	// We repair the old font with hdc and now we can delete the one we created.
	SelectObject(hdc, old_font);
}

// Gets the path to the file of the high score.
static BOOL get_high_score_file_path(wchar_t* path) {
	// We get the file path of the .exe into path. If failed (code 0) return false.
	if (GetModuleFileName(NULL, path, MAX_PATH) == 0) {
		return FALSE;
	}

	// We go back to the begining of the .exe file that is runing, earase it and replace with the path of the highsocre file.
	int i = (int)wcslen(path);
	// While we arent at the begginig of the path and we didnt make it to \\ yet, meaning we didnt make it to the beggining of the .exe.
	while (i >= 0 && path[i] != L'\\') {
		i--;
	}
	// We cut off the string after the .exe ends so we get rid of that part of the path.
	path[i + 1] = L'\0';
	// We use the .dat ending so we cant just edit the file(if .txt) to give a big number as the max which is just a lie.
	wcscat_s(path, MAX_PATH, L"high_score.dat");

	return TRUE;
}

// Sets the high score to the value we get.
static void set_high_score() {
	// We get the path and make sure it was found.
	wchar_t path[MAX_PATH];

	// If we failed ro get the file path.
	if (!get_high_score_file_path(path)) {
		MessageBox(NULL, L"Failed To Get File Path", L"ERROR", MB_OK | MB_ICONERROR);
	}
	
	// Same as get_high score but we open for writing instead of reading.
	HANDLE h_file =
		CreateFile(path, // Path to the file we are creating/opening
			GENERIC_WRITE, // We open the file for reading.
			0, // This deals with sharing which we dont have.
			NULL, // Security stuff for child processes, its optional and we dont need it.
			CREATE_ALWAYS, // We open the file if it exists if not we create it.
			FILE_ATTRIBUTE_HIDDEN, // Hides the file so the user doesnt tamper with by accident. It can be viewed if see hidden files is on.
			NULL); // Again has to do with security and encription which we dont need.

	// If the file failed to open so we can save the high score we send an error message.
	if (h_file == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, L"Failed To Save High Score.", L"ERROR", MB_OK | MB_ICONERROR);
		return;
	}

	// Same logic as get_high_score but here we write.
	DWORD num_of_bytes_writen = 0;
	if (!WriteFile(h_file, &high_score, sizeof(high_score), &num_of_bytes_writen, NULL)) {
		MessageBox(NULL, L"Failed To Write High Score.", L"ERROR", MB_OK | MB_ICONERROR);
	}
	if (num_of_bytes_writen != sizeof(high_score)) {
		MessageBox(NULL, L"Failed To Write High Score.", L"ERROR", MB_OK | MB_ICONERROR);
	}

	// Realse the mem.
	CloseHandle(h_file);
}

// A function that gets the high score from the file and puts it into the global var high_score.
static void get_high_score() {
	// We get the path and make sure it was found.
	wchar_t path[MAX_PATH];

	// If we failed to get the file path.
	if (!get_high_score_file_path(path)) {
		MessageBox(NULL, L"Failed To Get File Path", L"ERROR", MB_OK | MB_ICONERROR);
	}

	// We create the file if it didnt exist and if it did we open it.
	HANDLE h_file =
		CreateFile(path, // Path to the file we are creating/opening
			GENERIC_READ, // We open the file for reading.
			0, // This deals with sharing which we dont have.
			NULL, // Security stuff for child processes, its optional and we dont need it.
			OPEN_ALWAYS, // We open the file if it exists if not we create it.
			FILE_ATTRIBUTE_HIDDEN, // Hides the file so the user doesnt tamper with by accident. It can be viewed if see hidden files is on.
			NULL); // Again has to do with security and encription which we dont need.

	// If the file failed to open so we can get the high score we send an error message.
	if (h_file == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, L"Failed To Get High Score.", L"ERROR", MB_OK | MB_ICONERROR);
		return;
	}

	DWORD num_of_bytes_read = 0;
	// If the read failed.
	if (!ReadFile(h_file, &high_score, sizeof(high_score), &num_of_bytes_read, NULL)) {
		MessageBox(NULL, L"Failed To Read High Score.", L"ERROR", MB_OK | MB_ICONERROR);
		high_score = 0;
	}

	// If the number of bytes read is 0 that means we just created the file now(first run ever) so we set high socre to 0 with no error.
	if (num_of_bytes_read == 0) {
		high_score = 0;
	}
	// If the number of bytes we read was not the amount that should have been put in meaning the data was currupted.
	else if (num_of_bytes_read != sizeof(high_score)) {
		MessageBox(NULL, L"Failed To Read High Score.", L"ERROR", MB_OK | MB_ICONERROR);
		high_score = 0;
	}

	// We realese the mem we used.
	CloseHandle(h_file);
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

// Gets the start button ready to be shown.
static void start_button_set_up(HWND parent_window, HWND* button) {
	RECT client_area;
	GetClientRect(parent_window, &client_area);
	int width = (client_area.right - client_area.left) / 7;
	int hight = (client_area.bottom - client_area.top) / 7;
	int x = (client_area.right) / 2 - 3 * width / 4;
	int y = 2 * (client_area.bottom - hight) / 3;

	// We create a big start game button.
	*button = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", L"start game", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x, y, width, hight, parent_window, (HMENU)ID_START_BUTTON_MAIN, global_hInstance, NULL);

}

// Function that draws the snake head with our bitmap. 
static void draw_snake_head(HDC hdc, const RECT* cell) {
	// If the snake heads bitmap failed to load its null so we just return.
	if (snake_head_bitmap == NULL) {
		return;
	}
	// We create a memory DC that is compatable with our screen DC.
	HDC mem_dc = CreateCompatibleDC(hdc);

	// We put the bitmap into the memory DC and save the old one so we can put it back.
	HBITMAP save_old = (HBITMAP)SelectObject(mem_dc, snake_head_bitmap);

	// Makes the bitmap look good when shrinking.
	SetStretchBltMode(hdc, HALFTONE);
	// We reset the brush origion.
	SetBrushOrgEx(hdc, 0, 0, NULL);

	// We copy the bitmap into the cell taht is the head.
	StretchBlt(hdc,
		cell->left, cell->top, cell->right - cell->left, cell->bottom - cell->top, // Destanation x, y, width and hight. 
		mem_dc, 0, 0,
		snake_head_bitmap_w, snake_head_bitmap_h, // We load the whole bitmap, its whole width and whole hight.
		SRCCOPY /*copy the pixles normally*/);

	// Reput the old default back in.
	SelectObject(mem_dc, save_old);

	// We delete the memory DC we created.
	DeleteDC(mem_dc);
}

// Gets the standered button setup for the start of the game.
static void game_modes_button_set_up(HWND parent_window, HWND* left_button, HWND* main_button, HWND* right_button) {
	RECT client_area;
	GetClientRect(parent_window, &client_area);
	int width = (client_area.right - client_area.left) / 7;
	int hight = (client_area.bottom - client_area.top) / 15;
	int x = (client_area.right) / 2 - 3 * width / 4;
	int y = (client_area.bottom - hight) / 2;

	*left_button = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", L"<", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x - width / 4, 5 * y / 6, width / 4, hight, parent_window, (HMENU)ID_SWITCH_GAME_MODE_LEFT, global_hInstance, NULL);
	*right_button = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", L">", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x + width, 5*y/6, width / 4, hight, parent_window, (HMENU)ID_SWITCH_GAME_MODE_RIGHT, global_hInstance, NULL);

	*main_button = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", L"classic", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x, 3 * y / 4, width, 2 * hight, parent_window, (HMENU)ID_GAME_MODE_BUTTON, global_hInstance, NULL);

	// We allocate the room in the heap for the array that has all of our game modes that are set up. HEAP_ZERO_MEMORY initializes all spots to NULL.
	game_mode_windows = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HWND) * (ID_THREE_APPLES_WALLS_DONT_KILL - ID_CLASSIC + 1));
	if (game_mode_windows != NULL) {
		game_mode_windows[0] = *main_button;
	}
	else {
		MessageBox(parent_window, L"Failed To Allocate Memory For Game Modes.", L"ERROR", MB_OK | MB_ICONERROR);
	}
}

// Converts UINT value into the correct game mode to print
static wchar_t* button_int_to_words(UINT val) {
	// We undo the removing of 3000 in change_game_mode() that helped us with going to spots in arrays.
	val += ID_CLASSIC;
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
static void change_game_mode(HWND parent_window, HWND* main_button, UINT new_mode) {
	if (new_mode > 3005 || new_mode < 3000) {
		MessageBox(parent_window, L"Invalid Game Mode", L"ERROR", MB_OK | MB_ICONERROR);
		return;
	}
	// We hide the window that had the previous button.
	ShowWindow(*main_button, SW_HIDE);

	// If we have already been on this game mode, meaning it has already been opened in the past, we can just unhide it. We remove ID_CLASSIC every time we index to get the exact value for an index.
	if (game_mode_windows[new_mode - ID_CLASSIC]) {
		ShowWindow(game_mode_windows[new_mode - ID_CLASSIC], SW_SHOW);
		cur_game_mode = game_mode_windows[new_mode - ID_CLASSIC];
		// We are done with this function because we changed the window.
		return;
	}

	wchar_t* game_mode = button_int_to_words(new_mode);
	// If we failed to get the words we print an error and leave.
	if (game_mode == NULL) {
		MessageBox(parent_window, L"Failed To Get Game Mode Name", L"ERROR", MB_OK | MB_ICONERROR);
		return;
	}

	// We get the spot we will show this button at.
	RECT client_area;
	GetClientRect(parent_window, &client_area);
	int width = (client_area.right - client_area.left) / 7;
	int hight = (client_area.bottom - client_area.top) / 15;
	int x = (client_area.right) / 2 - 3 * width / 4;
	int y = (client_area.bottom - hight) / 2;

	// We create the new button.
	HWND new_button = NULL;
	new_button = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"BUTTON", game_mode, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x, 3 * y / 4, width, 2 * hight, parent_window, (HMENU)(UINT_PTR)new_mode, global_hInstance, NULL);
	// We save the button as created in the array and update our current game mode.
	game_mode_windows[new_mode - ID_CLASSIC] = new_button;
	cur_game_mode = new_button;
}

static void destroy_window_array() {
	// If its alreadt Null we can just return.
	if (game_mode_windows == NULL) {
		return;
	}
	
	// We go over the whole array.
	for (int i = 0; i <= ID_THREE_APPLES_WALLS_DONT_KILL - ID_CLASSIC; ++i) {
		// If a window does exist in that position we destroy it and put it to NULL.
		if (game_mode_windows[i]) {
			DestroyWindow(game_mode_windows[i]);
			game_mode_windows[i] = NULL;
		}
	}

	// We free the memory that we allocated from the heap and set the pointer to NULL.
	HeapFree(GetProcessHeap(), 0, game_mode_windows);
	game_mode_windows = NULL;
}
