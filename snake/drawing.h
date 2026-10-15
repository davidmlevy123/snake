#pragma once
#include <windows.h>
#include "additions.h"
#include <tchar.h>
typedef struct {
	// Handle to the process that is currently running. Used for the bitmap creation and more.
	HINSTANCE cur_process;
	// Font of of this drawing resource.
	HFONT main_font;
	// Brush that we get for painting stuff.
	HBRUSH main_brush;
	// Bitmap and its info
	HBITMAP main_bitmap;
	UINT bitmap_id;
	int bitmap_h;
	int bitmap_w;
}drawing_resources;

// A function that creates the font we are going to have in our resources for drawing.
BOOL create_my_font(HFONT* to_create, const int font_size);

// If we dont want font pass font size as 0.
// If we dont want bitmap pass bitmap_id as 0.
// If we dont want a brush we pass as 0 (will be created as black but just dont use).
BOOL create_drawing_resources(drawing_resources* to_create, HINSTANCE cur_process, const int font_size, const UINT bitmap_id, const COLORREF rgb);
void destroy_drawing_resources(drawing_resources* to_destroy);

void fill_square_with_bitmap(HDC hdc, const RECT* cell, drawing_resources* tool);
void fill_area_with_brush(HDC hdc, const RECT* cell, drawing_resources* tool);

// Drawing function that dont need drawing_recources.
void SetHatchBrushBackground(HDC hdc, BOOL transparent);
void SetWindowBackground(HDC hdc, PAINTSTRUCT pt, int n, UINT* bk_colour); // n is an option to chose the bk colour, -1 means continue loop.
void words_for_window(HWND hwnd, HDC hdc, UINT game_state);
void smiley_face(HDC hdc);
void print_game_score(HDC hdc, const int cell_size, drawing_resources* font, const UINT score, const UINT high_score);