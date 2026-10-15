#include "drawing.h"

BOOL create_my_font(HFONT* to_create, const int font_size) {
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
		MessageBox(NULL, L"Failed To Create Font.", L"ERROR", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

BOOL create_drawing_resources(drawing_resources* to_create, HINSTANCE cur_process, const int font_size, const UINT bitmap_id, const COLORREF rgb) {
	// We use this to set all values to NULL or 0 so we dontget garbage data.
	*to_create = (drawing_resources){ 0 };

	to_create->cur_process = cur_process;

	// If the id is not 0, meaning we want a value.
	if (!(bitmap_id == 0))
	{
		to_create->bitmap_id = bitmap_id;
		to_create->main_bitmap = LoadBitmap(to_create->cur_process, MAKEINTRESOURCE(bitmap_id));
		if (to_create->main_bitmap == NULL) {
			MessageBox(NULL, L"Failed To Create Bitmap For A Drawing Resource", L"ERROR", MB_OK | MB_ICONERROR);
			// We destroy everything that has been created so far so we dont have a mem leak.
			destroy_drawing_resources(to_create);
			return FALSE;
		}
		BITMAP snake_head_info;
		// Gets the info about the snake_head bitmap. 
		GetObject(to_create->main_bitmap, sizeof(BITMAP), &snake_head_info);
		// We get the hight and width for the snake head bitmap.
		to_create->bitmap_h = snake_head_info.bmHeight;
		to_create->bitmap_w = snake_head_info.bmWidth;
	}

	// We create the brush and make sure it got created. Even if 0 was passed becasause we need a way to paint balck.
	to_create->main_brush = CreateSolidBrush(rgb);
	if (!to_create->main_brush) {
		MessageBox(NULL, L"Failed To Create Brush For A Drawing Resource", L"ERROR", MB_OK | MB_ICONERROR);
		// We destroy everything that has been created so far so we dont have a mem leak.
		destroy_drawing_resources(to_create);
		return FALSE;
	}

	// Same idea as bitmap, if we haev a font size of 0 that means we dont want ot create it.
	if (!(font_size == 0)) {
		// If we failed to create the font.
		if (!create_my_font(&to_create->main_font, font_size)) {
			MessageBox(NULL, L"Failed To Create Font For A Drawing Resource", L"ERROR", MB_OK | MB_ICONERROR);
			// We destroy everything that has been created so far so we dont have a mem leak.
			destroy_drawing_resources(to_create);
			return FALSE;
		}
	}

	// If all the creations worked we retun true.
	return TRUE;
}

void destroy_drawing_resources(drawing_resources* to_destroy) {
	// If we have a each item created, we destroy it.
	if (to_destroy->main_bitmap) {
		DeleteObject(to_destroy->main_bitmap);
		to_destroy->main_bitmap = NULL;
	}
	if (to_destroy->main_brush) {
		DeleteObject(to_destroy->main_brush);
		to_destroy->main_brush = NULL;
	}
	if (to_destroy->main_font) {
		DeleteObject(to_destroy->main_font);
		to_destroy->main_font = NULL;
	}
}

// Function that draws our section with the bitmap we have.
void fill_square_with_bitmap(HDC hdc, const RECT* cell, drawing_resources* tool) {
	// If the snake heads bitmap failed to load its null so we just return.
	if (tool->main_bitmap == NULL) {
		return;
	}
	// We create a memory DC that is compatable with our screen DC.
	HDC mem_dc = CreateCompatibleDC(hdc);

	// We put the bitmap into the memory DC and save the old one so we can put it back.
	HBITMAP save_old = (HBITMAP)SelectObject(mem_dc, tool->main_bitmap);

	// Makes the bitmap look good when shrinking.
	SetStretchBltMode(hdc, HALFTONE);
	// We reset the brush origion.
	SetBrushOrgEx(hdc, 0, 0, NULL);

	// We copy the bitmap into the cell taht is the head.
	StretchBlt(hdc,
		cell->left, cell->top, cell->right - cell->left, cell->bottom - cell->top, // Destanation x, y, width and hight. 
		mem_dc, 0, 0,
		tool->bitmap_w, tool->bitmap_h, // We load the whole bitmap, its whole width and whole hight.
		SRCCOPY /*copy the pixles normally*/);

	// Reput the old default back in.
	SelectObject(mem_dc, save_old);

	// We delete the memory DC we created.
	DeleteDC(mem_dc);
}

// fills an area with the brush colour we want.
void fill_area_with_brush(HDC hdc, const RECT* cell, drawing_resources* tool) {
	// If we dont have a brush created we leave the function.
	if (tool->main_brush == NULL) {
		return;
	}

	// We fill the spot with our brush.
	FillRect(hdc, cell, tool->main_brush);
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

// Function to set the colour of the background. 
void SetWindowBackground(HDC hdc, PAINTSTRUCT pt, int n, UINT* bk_colour) {
	// We use the FillRect function to fill in the rectangle in black. The rectangle is the window in this case.
	int save_bk_colour = *bk_colour;
	// If we got a specified value we use it.
	if (n != -1) {
		*bk_colour = n;
	}
	switch (*bk_colour) {
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
		int ans = MessageBox(NULL, L"Invalid Background Colour.\nSet To Default?", L"ERROR", MB_YESNO | MB_ICONERROR);
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
	*bk_colour = save_bk_colour;
}

// Function to add main words to the window. If child: "This Is A Child Window." , If its has no parent: "Welcome To My Window.".
void words_for_window(HWND hwnd, HDC hdc, UINT game_state) {
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

// We dont declare eyes head and mouth in the header becasue they can be private.

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
void smiley_face(HDC hdc) {
	Head(hdc);
	Eyes(hdc);
	Mouth(hdc);
}

// Prints the current score of game onto the screen.
void print_game_score(HDC hdc, const int cell_size, drawing_resources* font, const UINT score, const UINT high_score) {
	// We select the new font on to the hdc. We save the old font so we can put it back together with hdc.
	HFONT old_font = (HFONT)SelectObject(hdc, font->main_font);

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
