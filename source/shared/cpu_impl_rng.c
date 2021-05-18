#include <stdint.h>

static uint8_t random_seed;

uint8_t BatariBasicRNG();


// Random number, 0-256
uint8_t Chip8_RandomNumber()
{
	return BatariBasicRNG();
}

void Chip8_SeedRNG()
{
	// Seed the random number generator
	random_seed = 42;
}

uint8_t BatariBasicRNG()
{
	if (random_seed & 1)
	{
		random_seed >>= 1;
		random_seed ^= 0xB4;
	}
	else
	{
		random_seed >>= 1;
	}

	return random_seed;
}