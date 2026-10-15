#include "high_score.h"

// Gets the path to the file of the high score. We only declare here and static because we only need it here.
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
void set_high_score(const UINT high_score) {
	// We get the path and make sure it was found.
	wchar_t path[MAX_PATH];

	// If we failed ro get the file path.
	if (!get_high_score_file_path(path)) {
		MessageBox(NULL, L"Failed To Get File Path", L"ERROR", MB_OK | MB_ICONERROR);
		return;
	}

	// Same as get_high score but we open for writing instead of reading.
	HANDLE h_file =
		CreateFile(path, // Path to the file we are creating/opening
			GENERIC_WRITE, // We open the file for writing.
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
	else if (num_of_bytes_writen != sizeof(high_score)) {
		MessageBox(NULL, L"Failed To Write High Score.", L"ERROR", MB_OK | MB_ICONERROR);
	}

	// Realse the mem.
	CloseHandle(h_file);
}

// A function that gets the high score from the file and returns it.
UINT get_high_score() {
	// We get the path and make sure it was found.
	wchar_t path[MAX_PATH];
	UINT high_score = 0;

	// If we failed to get the file path.
	if (!get_high_score_file_path(path)) {
		MessageBox(NULL, L"Failed To Get File Path", L"ERROR", MB_OK | MB_ICONERROR);
		return 0;
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
		return 0;
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

	// We return the high score that we got(0 if an error occured).
	return high_score;
}

// We reset the high score to 0 using set high score with the value of 0.
void reset_high_score(void) {
	set_high_score(0);
}
