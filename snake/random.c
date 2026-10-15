#include "random.h"
// An int that holds the state of our random numbers. We declare it static here because only this file uses it.
static unsigned long long random_seed = 1;

// A void that sets up getting random numbers. We call it once at the beggining of the game.
void set_up_rand(void) {
	// Gets the number of milliseconds since pc has started. It will be different for every run.
	random_seed = GetTickCount64();
}

// Return a random number in the range 0->2^15-1.
int full_range_rand_pos(void) {
	// This is the formula used by the Microsoft C compiler. It multiplies the seed by a prime number and adds an offset.
	random_seed = (random_seed * 214013 + 2531011);

	// We use that becasue only 16 bits end up random so we shift it, we do &7FFF to get rid of negetive numbers.
	// If we want a number than 32767(2^15-1) we use the first digits as one random number and the next 16 as another and so on.
	return (random_seed >> 16) & 0x7FFF;
}

int rand_32_bits_pos(void) {
	int small_bits = full_range_rand_pos(); // bits 16-30
	int large_bits = full_range_rand_pos(); // bits 1-15
	int first_bit = full_range_rand_pos(); // We use for bit 0

	// We shift it so we can get the random numbers in the lower bits(16-30 are the bits for the 1,2,4,8...).
	small_bits <<= 16;
	// We shift 1 for spots 1-15.
	large_bits <<= 1;

	// We dont touch bit 31(the last one), making it stay at 0, meaning it will always be positive. We only get the first bit of the first_bit becasue its spot 0 so we only need 1 bit.
	return (small_bits | large_bits | (first_bit & 1));
}

int my_rand(const int a, const int b) {
	// If b-a is bigger than the max for the default rand function i created we use the 31 bit rand function.
	if (b - a > FULL_RANGE_POS_MAX_RAND) {
		// If its also too big for that we return -1 to signal an error.
		if (b - a > RAN_32_POS_MAX) {
			return -1;
		}
		// We use the 32 bit function.
		else {
			return a + (rand_32_bits_pos() % (b - a + 1));
		}
	}

	// We use the default if we can.
	return a + (full_range_rand_pos() % (b - a + 1));
}
