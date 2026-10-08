#include <windows.h>
#include <tchar.h>
#include "additions.h" // The additions i created.
#include "queue.h" 

const wchar_t class_name[] = L"Main Window.";
const wchar_t window_name[] = L"Snake.";

// Create a struck that has the snake. It is a global struct
typedef struct
{
	// We creata an queue of points that has a length of 3(we will have the snake size at 3 for now).
	queue snake;
	UINT length;
	UINT cur_direction; // The current direction is UP=0,RIGHT=1,DOWN=2,LEFT=3.
}Snake;
// We create our actual snake, in c we cant give default values so we need to give the values when we define it.
Snake my_snake = { .length = 3 , .cur_direction = RIGHT };
static void set_snake_start_points(){
	// Distroy the snake if one still exists and has not been destroyed.
	destroy_queue(&my_snake.snake);

	POINT start_points[3];
	// We have to do this becasue my_snake.length is a UINT meaning it cant have negitive.
	for (int i = 0; i < (int)my_snake.length; ++i){
		start_points[i].x = (my_snake.length - 1 - i) + 1;
		start_points[i].y = 0;
	}
	my_snake.snake = create_queue(start_points, sizeof(POINT), 50, my_snake.length, 0);
}

// A queue that has the buttons pressed so we can do them on their time. If we dont have it its possible to do 2 moves per tick breaking the game.
queue buttons_pressed;
static void set_button_queue() {
	// Free mem if a prev game was played so we have a new one without leaking mem.
	destroy_queue(&buttons_pressed);
	buttons_pressed = create_queue(NULL, sizeof(int), 3, 0, 0);
}

// A bool to check if the game has started yet.
BOOL game_started = FALSE;

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

// Ints for timers
UINT_PTR timer_1 = 0;
UINT_PTR timer_for_apple = 0;

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
static BOOL is_valid_turn(UINT new_direction, const Snake* s);
static void queue_direction_if_valid(UINT new_direction);


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
						//// I havent made any of the game logic yet so for now it does nothing.
						//MessageBox(key_of_window, L"Game Not Created yet", L"ERROR:", MB_OK);// We can use TEXT(""), _T("") or L"".
						//draw_face = TRUE;

						// we set the flag that says the game has started to true.
						game_started = TRUE;

						// Setting timer to move the snake.
						timer_1 = SetTimer(key_of_window, IDT_TIMER1, 100, NULL);
						timer_for_apple = SetTimer(key_of_window, IDT_TIMER_FOR_APPLE, NULL);
						set_snake_start_points();
						// Get the queue of the buttons pressed that need to be executed ready.
						set_button_queue();
						InvalidateRect(key_of_window, NULL, TRUE);
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
					// If it is light blue, which is the last colour we circle back to the first, black.
					if (bk_colour == LIGHTBLUEBK) {
						bk_colour = BLACKBK;
					}
					// If we are not at the end we go to the next colour.
					else {
						bk_colour++;
					}
					// We want to call the function to change the background in WM_PAINT.
					InvalidateRect(key_of_window, NULL, TRUE);
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

			if (draw_face) {
				// We call the function to create the whole smiley face.
				smiley_face(hdc);
				// We set the draw_face to false so next time we dont draw unless we want too.
				draw_face = FALSE;
				// We pause everthing for 5 seconds and then delete the face. After the sleep 
				Sleep(1000);
				InvalidateRect(key_of_window, NULL, TRUE);
			}

			// If the used dicided to start the game.
			if (game_started) {
				
				// We get the game dimentions.
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

				// Creating the snake and giving him a colour.
				if (bk_colour == GREENBK || bk_colour == LIGHTGREENBK) {
					HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
					for (int i = 0; i < my_snake.length; ++i) {
						RECT block;
						block.left = (((POINT*)place(&my_snake.snake, i))->x) * cell_size;
						block.top = (((POINT*)place(&my_snake.snake, i))->y) * cell_size;
						block.right = block.left + cell_size;
						block.bottom = block.top + cell_size;
						FillRect(hdc, &block, black);
					}
					DeleteObject(black);
				}
				else {
					HBRUSH green = CreateSolidBrush(RGB(0, 255, 0));
					for (int i = 0; i < my_snake.length; ++i) {
						RECT block;
						block.left = (((POINT*)place(&my_snake.snake, i))->x) * cell_size;
						block.top = (((POINT*)place(&my_snake.snake, i))->y) * cell_size;
						block.right = block.left + cell_size;
						block.bottom = block.top + cell_size;
						FillRect(hdc, &block, green);
					}
					DeleteObject(green);
				}
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

		// Automatically called after WM_CLOSE.
		case WM_DESTROY:
		{
			// We need to kill the timer before we end the code so it doesnt continue going off.
			KillTimer(key_of_window, IDT_TIMER1);
			KillTimer(key_of_window, IDT_TIMER_FOR_APPLE);
			destroy_queue(&my_snake.snake);
			destroy_queue(&buttons_pressed);

			// If the case is to leave we end the program and window. We need the PostQuitMessage becasue if we dont have it the function will return 0 without closing the window.
			PostQuitMessage(0);
			return 0;
		}

		// If we have a timer go off.
		case WM_TIMER: {
			switch (wParam) {
				// The timer for moving the snake went off.
				case IDT_TIMER1:{

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

					// We move each part of the snake from tail to the spot infront of it(exept the head).
					for (int i = my_snake.length - 1; i > 0; --i) {
						POINT* cur = ((POINT*)place(&my_snake.snake, i));
						POINT* prev = ((POINT*)place(&my_snake.snake, i - 1));
						*cur = *prev;
					}
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
					InvalidateRect(key_of_window, NULL, TRUE);
					break;
				}// case IDT_TIMER1
				default: {
					
				}
			} // switch(wPram)
			break;
		} // Case WM_TIMER

		// Any other case.
		default: {
			// Use default for the rest of the codes.
			return DefWindowProc(key_of_window, code_of_msg, wParam, lParam);
			break;
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
		default: {
			int ans = MessageBox(GetModuleHandle(NULL), L"Invalid Background Colour.\nSet To Default?", L"ERROR", MB_YESNO | MB_ICONERROR);
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
		TextOut(hdc, 5, 5, text, _tcslen(text));
	}
	else {
		// We set the background of text and text colours.
		TCHAR text[] = L"Welcome To My Window.";
		SetBkColor(hdc, RGB(255, 255, 255));
		SetTextColor(hdc, RGB(0, 255, 0));
		if (!game_started) {
			// We print the text with our colours. We do that only if we havent started the game so we dont block the game with the words. 
			TextOut(hdc, 5, 5, text, _tcslen(text));
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

static BOOL is_valid_turn(UINT new_direction, const Snake* s) {
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

// Puts a direction in the queue if it is going to be a valid direction when its turn comes up.
static void queue_direction_if_valid(UINT new_direction) {
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
