#pragma once
#include "additions.h"
#include <windows.h>
// Passing a void means we dont have any arguments. If we dont pass anything the compiler wont give us warning when we pass stuff over.
void set_up_rand(void);
// Gets a random number with only 16 bits
int  full_range_rand_pos(void);
// Gets a random nuber with 32 bits.
int rand_32_bits_pos(void);

int  my_rand(const int a, const int b); // Gets a random number in between a and b.

