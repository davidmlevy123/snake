#include "dialogs.h"

INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	// Handles buttons clicked inside the dialog
	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {

				// If the start game button was pressed. Equivalent to WM_CREATE for a window. The controls are already created by this point.
			case  ID_START_GAME_BUTTON: {	

				EndDialog(hwndDlg, LOWORD(wParam));
			
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
INT_PTR CALLBACK SettingsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
