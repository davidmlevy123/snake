#pragma once
#include <windows.h>

// Get and set for high scores.
void set_high_score(const UINT high_score);
UINT get_high_score(void);

// Resets the high score.
void reset_high_score(void);