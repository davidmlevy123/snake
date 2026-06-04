#include <windows.h>
#include <tchar.h>

const wchar_t class_name[] = L"Snake";
const wchar_t window_name[] = L"Main Window";

// We have a global hInstance so we can use it in the function and not just main.
HINSTANCE global_hInstance;
HWND global_cur_win;

LRESULT CALLBACK WindowProc(HWND key_of_window, UINT code_of_msg, WPARAM key_pressed, LPARAM extra_msg_info);
void Resize(HWND hwnd, UINT code_of_message, int width, int height);
void SetHatchBrushBackground(HDC hdc, BOOL transparent);
void SetWindowBackground(HDC hdc, PAINTSTRUCT pt);
void Eyes(HDC hdc);

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

	// IDI is the ID for Icon and APPLICATION is the standered windows icon. We send null because all built in icons/cursors get NULL.
	window_blueprint.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	// IDC is the ID for the Cursor and ARROW is the standered one(UPARROW is also used).
	window_blueprint.hCursor = LoadCursor(NULL, IDC_ARROW);

	// We set the background to the default.
	window_blueprint.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	// We set it to NULL because we dont have a menu built yet.
	window_blueprint.lpszMenuName = NULL;
	window_blueprint.lpszClassName = class_name;
	window_blueprint.hIconSm = LoadIcon(NULL, IDI_ASTERISK);

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
	global_cur_win = cur_win;
	if (cur_win == NULL) { // If we failed to open the current window: 
		// We print the ERROR message.
		MessageBox(NULL, TEXT("Failed To Create Window."), TEXT("ERROR."), MB_OK | MB_ICONEXCLAMATION);
		return 2;
	}

	// Gets the windows spot ready to be painted in and puts painting the inside in the waiting queue. The flag is for tellimg what way to prepare the window(minimized, full screen, normal and so on)
	ShowWindow(cur_win, flag_min_max_normal);
	// Forcibly paints the window in now.
	UpdateWindow(cur_win);

	MSG msg;
	// BOOL for the value of the message. As long as its not 0 or -1 we don't care what it is
	BOOL get_message_val;
	// While the value is not 0. If its 0 than we got a termination message so we stop getting messages.

	while ((get_message_val = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		// If the message is -1 that means that the hWnd is invalid so we give an error.
		if (get_message_val == -1) {
			int ans = MessageBox(NULL, TEXT("Invalid HWND - Window Does Not Exist.\n Do You Want To Try Again?"), TEXT("ERROR."), MB_YESNO | MB_ICONQUESTION);
			// If the answer was yes, the user wants to continue we continue
			if (ans == IDYES) {
				continue;
			}
			// If the answer is no, we end the program automatically closing the window. This only works because we are in the main.
			if (ans == IDNO) {
				return 0;
			}
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
		case WM_DESTROY:
		{
			// If the case is to leave we end the program and window. We need the PostQuitMessage becasue if we dont have it the function will return 0 without closing the window.
			PostQuitMessage(0);
			return 0;
		}

		// Automatically called when we create the window. We can also manualy call.
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT pt;

			// We get the window ready to be painted.
			hdc = BeginPaint(key_of_window, &pt);

			// We set the background of text and text colours.
			// TCHAR text[] = L"Welcome To My Window.";
			// SetBkColor(hdc, RGB(255, 255, 255));
			// SetTextColor(hdc, RGB(0, 255, 0));
			// TextOut(hdc, 5, 5, text, _tcslen(text));
			// We print the text with our colours.
			// Not used in the smiley face.
		
			// We set the background of the window to black using the function.
			SetWindowBackground(hdc, pt);

			// Calls the function that draws the rectangle with vertical lines.
			// SetHatchBrushBackground(hdc, TRUE);
			// Also not used in smiley face

			// We call the function to create the eyes.
			Eyes(hdc);

			// We must end the painting.
			EndPaint(key_of_window, &pt);
			return 0;
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
			// If that key was escape.
			if (wParam == VK_ESCAPE) {
				// We close the window.
				if (MessageBox(key_of_window, L"Are You Sure You Want To Quit?", L"Quit Menu", MB_YESNO | MB_ICONQUESTION) == IDYES) {
					DestroyWindow(key_of_window);
				}
				else {
					return 0;
				}
			}
			else {
				// Use the default.
				return DefWindowProc(key_of_window, code_of_msg, wParam, lParam);
			}
		}

		// If the user closes the widnow with the x at the top right.
		case WM_CLOSE:
		{
			if (MessageBox(key_of_window, L"Are You Sure You Want To Quit?", L"Quit Menu", MB_YESNO | MB_ICONQUESTION) == IDYES) {
				DestroyWindow(key_of_window);
			}
			else {
				return 0;
			}
		}
	}
	// Use default for the rest of the codes.
	return DefWindowProc(key_of_window, code_of_msg, wParam, lParam);
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

void Eyes(HDC hdc) {
	// We create the white part of the eye and put it in hdc.
	HBRUSH outer_eye = CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH default_brush = (HBRUSH)SelectObject(hdc, outer_eye);
	//Create each eye
	Ellipse(hdc, 600, 150, 650, 200);
	Ellipse(hdc, 850, 150, 900, 200);

	// We create the inner eye.
	HBRUSH inner_eye = CreateSolidBrush(RGB(0, 0, 0));
	SelectObject(hdc, inner_eye);
	Ellipse(hdc, 620, 173, 632, 185);
	Ellipse(hdc, 870, 173, 882, 185);

	// We reput the default brush so we can delete the ones we created.
	SelectObject(hdc, default_brush);

	// We delete the brushes we created.
	DeleteObject(outer_eye);
	DeleteObject(inner_eye);
}

