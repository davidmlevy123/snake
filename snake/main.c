#include <windows.h>
#include <tchar.h>
#include "additions.h" // The additions i created.

const wchar_t class_name[] = L"Main Window.";
const wchar_t window_name[] = L"Snake.";

// We have a global hInstance so we can use it in the function and not just main.
HINSTANCE global_hInstance;
HWND global_cur_win;

// The bool to know if we should draw the smiley face in the WM_PAINT.
static BOOL draw_face = FALSE;

LRESULT CALLBACK WindowProc(HWND key_of_window, UINT code_of_msg, WPARAM key_pressed, LPARAM extra_msg_info);
INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Resize(HWND hwnd, UINT code_of_message, int width, int height);
void SetHatchBrushBackground(HDC hdc, BOOL transparent);
void SetWindowBackground(HDC hdc, PAINTSTRUCT pt);
void words_for_window(HWND hwnd, HDC hdc);
void Eyes(HDC hdc);
void Head(HDC hdc);
void Mouth(HDC hdc);
void smiley_face(HDC hdc);

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

	// IDC is the ID for the Cursor and ARROW is the standered one(UPARROW is also used).
	window_blueprint.hCursor = LoadCursor(NULL, IDC_ARROW);

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

		// We need to translate the message because pressing a key can have a few meanings depending on shift, keyboared language and more.
		// It returns true if it was translated else false. If we press a key without any other stuff(shift,...) it doesnt translate.
		TranslateMessage(&msg);
		// Tells the window to run the message.
		DispatchMessage(&msg);
	}

	// We return that because wPram gets the number from PostQuitMessage. 
	return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND key_of_window, UINT code_of_msg, WPARAM wParam, LPARAM lParam) {
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
					DialogBox(
						GetModuleHandle(NULL),           // We get the hinstance of the current process.
						MAKEINTRESOURCE(IDD_BASIC_DIALOG), // The ID of the dialog in LWCTSTR.
						key_of_window,                   // The main window.
						AboutDialogProc                  // The function that controls the dialog(its lower dowm).
					);
					//I havent made any of the game logic yet so for now it does nothing.
					MessageBox(key_of_window, L"Game Not Created yet", L"ERROR:", MB_OK);// We can use TEXT(""), _T("") or L"".
					draw_face = TRUE;
					InvalidateRect(key_of_window, NULL, TRUE);
					break;
				}
			}
		
		// Automatically called when we create the window. We can also manualy call.
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT pt;

			// We get the window ready to be painted.
			hdc = BeginPaint(key_of_window, &pt);

			// Prints the words for the window.
			words_for_window(key_of_window, hdc);

			// Calls the function that draws the rectangle with vertical lines.
			// SetHatchBrushBackground(hdc, TRUE);
			// Not used in smiley face.

			if (draw_face) {
				// We call the function to create the whole smiley face.
				smiley_face(hdc);
				// We set the draw_face to false so next time we dont draw unless we want too.
				draw_face = FALSE;
				// We pause everthing for 5 seconds and then delete the face. After the sleep 
				Sleep(1000);
				InvalidateRect(key_of_window, NULL, TRUE);
			}

			// We must end the painting.
			EndPaint(key_of_window, &pt);
			break;
		}

		// If the user is done resizing.
		case WM_EXITSIZEMOVE:
		{
			// In this case wParam is the flag that indicates if the window is minimized maximized or normal.
			// The lParam has the new height and width in it. The first 16 bits are the hieght and the last 16 are width.
			int height = HIWORD(lParam);
			int width = LOWORD(lParam);

			Resize(key_of_window, (UINT)wParam, width, height);
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
				}

				// If the key is 'P'(the spot of P on the keyboard). 
				case  'P': {
					// We create a pointer to the file path we make it the max size a path can be(260).
					LPWSTR path[MAX_PATH];
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
				}

				default: {
					// Use the default.
					return DefWindowProc(key_of_window, code_of_msg, wParam, lParam);
				}
			}
		}

		// If the user closes the widnow with the x at the top right.
		case WM_CLOSE:
		{
			DestroyWindow(key_of_window);
		}

		// Automatically called after WM_CLOSE.
		case WM_DESTROY:
		{
			// If the case is to leave we end the program and window. We need the PostQuitMessage becasue if we dont have it the function will return 0 without closing the window.
			PostQuitMessage(0);
			return 0;
		}

		// Any other case.
		default: {
			// Use default for the rest of the codes.
			return DefWindowProc(key_of_window, code_of_msg, wParam, lParam);

		}
	}
}

INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		// Called the moment before the dialog appears on screen
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		// Handles buttons clicked inside the dialog
		case WM_COMMAND:
			// If they click our OK button, or press the red X (IDCANCEL)
			if (LOWORD(wParam) == ID_OK_BUTTON) {
				// This destroys the dialog and unfreezes the main game window
				EndDialog(hwndDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
		break;
	}

	// Return FALSE if we didn't handle the message
	return (INT_PTR)FALSE;
}
void Resize(HWND hwnd, UINT code_of_message, int width, int height) {
	// If the window was resized we get here(After we are done resizing).
	MessageBox(hwnd, L"Resize Dected.", L"Info.", MB_OK);
	// Direct call to paint. We send NULL because we have no specific spot to repaint. We semd true because we want to repaint the background.
	InvalidateRect(hwnd, NULL, TRUE);
}

void SetHatchBrushBackground(HDC hdc, BOOL transparent) {
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

void SetWindowBackground(HDC hdc, PAINTSTRUCT pt) {
	// We use the FillRect function to fill in the rectangle in black. The rectangle is the window in this case.
	FillRect(hdc, &pt.rcPaint, CreateSolidBrush(RGB(0, 0, 0)));
}

// Function to add main words to the window. If child: "This Is A Child Window." , If its has no parent: "Welcome To My Window.".
void words_for_window(HWND hwnd, HDC hdc) {
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
		// We print the text with our colours. 
		TextOut(hdc, 5, 5, text, _tcslen(text));
	}
}

// We create the eyes for the similey face.
void Eyes(HDC hdc) {
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
void Head(HDC hdc) {
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
void Mouth(HDC hdc) {
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
void smiley_face(HDC hdc) {
	Head(hdc);
	Eyes(hdc);
	Mouth(hdc);
}
