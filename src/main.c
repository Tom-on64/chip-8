#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "libsdl.h"
#include "cpu.h"

static char* err_exename = NULL;
#define ERROR(_s) \
	do {\
		fprintf(stderr, "%s: \x1b[31m%s\x1b[0m\n", err_exename, (_s));\
		exit(1);\
	} while (0);

void usage(void);
void loadRaw(struct cpu* cpu, char* path);
void loadRom(struct cpu* cpu, char* path);

int main(int argc, char** argv) {
	err_exename = argv[0];

	char* romname = NULL;
	int speed = 16;
	int raw = 0;

	struct cpu* cpu = malloc(sizeof(*cpu));
	if (cpu == NULL) ERROR("not enough memory.");
	memset(cpu, 0x00, sizeof(*cpu));

	for (int i = 1; i < argc; i++) {
		char* arg = argv[i];
		if (arg[0] != '-') {
			romname = arg;
			continue;
		}

		if (arg[1] == '\0') ERROR("invalid argument.");

		switch (arg[1]) {
		case 'e': cpu->esm = 1; break;
		case 'h': usage(); exit(0);
		case 'd': cpu->dbg = 1; break;
		case 'r': raw = 1; break;
		case 's': {
			i++;
			speed = atoi(argv[i]);
			if (speed < 0) ERROR("speed cannot be negative");
		}
		}
	}

	// Load ROM
	if (romname == NULL) ERROR("no ROM given.");
	if (raw) loadRaw(cpu, romname);
	else loadRom(cpu, romname);

	// Init SDL
	if (initContext() != 0) ERROR("failed to init SDL.");

	// Init audio
	cpu->speaker = &updateSpeaker;
	if (initAudio() != 0) {
		fprintf(stderr, "%s: no audio.\n", err_exename);
		cpu->speaker = NULL;
	}

	// Other setup
	srand(time(NULL));
	if (initEmu(cpu) != 0) ERROR("failed to init emulator.");

	int lastTicks = SDL_GetTicks();
	int lastDelta = 0;
	while (!isQuitRequest()) {
		lastDelta = lastTicks - SDL_GetTicks();
		lastTicks = SDL_GetTicks();

		updateTime(cpu, lastDelta);
		for (int i = 0; i < speed; i++) stepEmu(cpu);

		drawDisplay(cpu);

		// Render at 60 Hz (dt = 16.77)
		int renderTime = SDL_GetTicks() - lastTicks;
		if (renderTime < 16) SDL_Delay(16 - renderTime); // Sleep
		else SDL_Delay(1); // Make the CPU not blow up
	}

	destroyContext();
	return 0;
}

void usage(void) {
	printf("Usage: %s [-ehr] [-s <speed>] <filename>\n", err_exename);
	printf("  -d        Use debug mode\n");
	printf("  -e        Use extended screen mode\n");
	printf("  -h        Display this help message\n");
	printf("  -r        Use a raw image\n");
	printf("  -s        Set emulator speed\n");
}

void loadRaw(struct cpu* cpu, char* path) {
	FILE* fp;
	if ((fp = fopen(path, "rb")) == NULL) ERROR("failed to open file.");

	fseek(fp, 0, SEEK_END);
	size_t fsize = ftell(fp);
	rewind(fp);

	if (fsize > MEMORY_LEN) ERROR("file too big.");
	if (fread(cpu->memory, fsize, 1, fp) != 1) ERROR("failed to read file.");

	fclose(fp);
}

void loadRom(struct cpu* cpu, char* path) {
	FILE* fp;
	if ((fp = fopen(path, "rb")) == NULL) ERROR("failed to open file.");

	fseek(fp, 0, SEEK_END);
	size_t fsize = ftell(fp);
	rewind(fp);

	if (fsize > MEMORY_LEN - 0x200) ERROR("file too big.");
	if (fread(&cpu->memory[0x200], fsize, 1, fp) != 1) ERROR("failed to read file.");

	fclose(fp);
}

