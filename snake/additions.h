#pragma once

// All of these are instead of using ENUM.

// Colours
#define BLACKBK 0
#define REDBK 1
#define GREENBK 2
#define BLUEBK 3
#define WHITEBK 4
#define LIGHTGREENBK 5
#define LIGHTBLUEBK 6
#define VIOLETBK 7

// Directions 
#define UP 4
#define RIGHT 1
#define DOWN 2
#define LEFT 3

// Grid dimentions for game.
#define GRID_W 30
#define GRID_H 20

// Defualt values for queue.
#define DEFAULT_MAX_Q 100

// Game states
#define NOT_STARTED 0
#define GAME_STARTED 1
#define GAME_OVER 2

// Timers
#define IDT_TIMER1 1000
#define IDT_TIMER_FOR_APPLE 1001
#define IDT_TIMER_GAME_OVER 1002
#define IDT_TIMER_TICK_SPEED 1003

// ID's for game modes
#define ID_CLASSIC 3000
#define ID_WALSS_DONT_KILL 3001
#define ID_DISAPEARING_APPLE 3002
#define ID_DISAPEARING_APPLE_WALLS_DONT_KILL 3003
#define ID_THREE_APPLES 3004
#define ID_THREE_APPLES_WALLS_DONT_KILL 3005
