#include <windows.h>
#include <tchar.h>

const wchar_t class_name[] = L"Snake";
const wchar_t window_name[] = L"Main Window";
LRESULT CALLBACK WindowProc(HWND key_of_window, UINT code_of_msg, WPARAM key_pressed, LPARAM extra_msg_info);

int WINAPI wWinMain(HINSTANCE handle_of_instance, HINSTANCE non_needed, PWSTR command_line, int flag_min_max_normal) {//PWSTR=wchar_t*.
	WNDCLASSEX window_blueprint = { 0 };
	window_blueprint.cbSize = sizeof(WNDCLASSEX);
	window_blueprint.style = 0;
	window_blueprint.lpfnWndProc = WindowProc;//long pointer to function. 

	//we dont reserve any extra space.
	window_blueprint.cbClsExtra = 0;
	window_blueprint.cbWndExtra = 0;
	window_blueprint.hInstance = handle_of_instance;

	//IDI is the ID for Icon and APPLICATION is the standered windows icon. We send null because all built in icons/cursors get NULL.
	window_blueprint.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	//IDC is the ID for the Cursor and ARROW is the standered one(UPARROW is also used).
	window_blueprint.hCursor = LoadCursor(NULL, IDC_ARROW);

	//we set the background to the default.
	window_blueprint.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	//we set it to NULL because we dont have a menu built yet.
	window_blueprint.lpszMenuName = NULL;
	window_blueprint.lpszClassName = class_name;
	window_blueprint.hIconSm = LoadIcon(NULL, IDI_ASTERISK);

	//Is the class creation failed: (the function gets a pointer to a WNDCLASS so we send the address)
	if (!RegisterClassEx(&window_blueprint)) {
		//we send a message box that has the error. We send NULL in the first one because we use the default message box window and not one we created. the second spot is the message in the box and the third is the title of the box. In the last spot we put MB_OK which means that we just show the test box. Its the code for what type we want. MB_ICONEXCLAMATION is to make the error message loom better
		MessageBox(NULL, TEXT("Failed To Create Class."), TEXT("ERROR."), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}

	HWND cur_win = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW, // a type of window style. (in)
		class_name, // the windows class name. (in, optional) 
		window_name, // the windows name. (in, optional)
		WS_OVERLAPPEDWINDOW, // the window style(old vertion). (in)
		CW_USEDEFAULT, CW_USEDEFAULT, // default x y coordinates. (in)
		500, 500, // width and height of the window. (in)
		NULL, // this is not a child so we dont have a Parent window. (in, optional)
		NULL, // also NULL for the same reason. (in, optional)
		handle_of_instance, // handle of the instance. (in, optional)
		NULL  //this is LPVOID we dont have any extra stuff so its NULL. (in, optional)
	);

	if (cur_win == NULL) { // if we failed to open the current window: 
		//we print the ERROR message.
		MessageBox(NULL, TEXT("Failed To Create Window."), TEXT("ERROR."), MB_OK | MB_ICONEXCLAMATION);
		return 2;
	}

	// gets the windows spot ready to be painted in and puts painting the inside in the waiting queue. The flag is for tellimg what way to prepare the window(minimized, full screen, normal and so on)
	ShowWindow(cur_win, flag_min_max_normal);
	// forcibly paints the window in now.
	UpdateWindow(cur_win);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {

	}
	return -1;
}

LRESULT CALLBACK WindowProc(HWND key_of_window, UINT code_of_msg, WPARAM key_pressed, LPARAM extra_msg_info) {
	switch (code_of_msg) {
		case WM_DESTROY:
		{
			PostQuitMessage(0);//if the case is to leave we end the program.
			return 0;
		}
		
	}

	//use default for the rest of the codes.
	return DefWindowProc(key_of_window, code_of_msg, key_pressed, extra_msg_info);
}
