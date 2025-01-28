#include <stdio.h>
#include "libsdl.h"
#include "cpu.h"

static char* err_exename = NULL;
#define ERROR(_s) \
	do {\
		fprintf(stderr, "%s: \x1b[31m%s\x1b[0m\n", err_exename, (_s));\
		exit(1);\
	} while (0);

int main(int argc, char** argv) {
	err_exename = argv[0];
	char* filename = NULL;

	struct cpu* cpu = malloc(sizeof(*cpu));
	memset(cpu, 0, sizeof(*cpu));

	for (int i = 1; i < argc; i++) {
		char* arg = argv[i];
		if (arg[0] != '-') {
			filename = arg;
			continue;
		}

		if (arg[1] == '\0') ERROR("invalid argument.");

		switch (arg[1]) {
		case 'e': cpu->esm = 1; break;
		case 'h': printf("%s [-eh] <file>\n", argv[0]); exit(0);
		case 'd': cpu->dbg = 1; break;
		}
	}
	if (filename == NULL) ERROR("no ROM given.");

	if (cpu == NULL) ERROR("not enough memory.");
	if (initContext() != 0) ERROR("failed to init SDL.");
	if (initAudio() != 0) ERROR("failed to init audio.");
	
	int i = 0;
	while (!isQuitRequest()) {
		cpu->screen[i] = ~cpu->screen[i];
		i++;
		if (i > SCREEN_W * SCREEN_H) i = 0;

		drawDisplay(cpu);
	}

	destroyContext();
	return 0;
}

