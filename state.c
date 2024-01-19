#include "state.h"

struct State state = {
	.water = -1,
	.day = 1,
	.location = "Missing"
};

struct Settings settings = {
	.save_path = "test.dat"
};

void saveState(const char* path)
{
	FILE* f = fopen(path, "wb");
	fwrite(&state, 1, sizeof(state), f);
	fclose(f);
}

void loadState(const char* path)
{
	FILE* f = fopen(path, "rb");
	fread(&state, 1, sizeof(state), f);
	fclose(f);
}