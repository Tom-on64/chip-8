#include <SDL2/SDL.h>
#include <math.h>
#include "libsdl.h"

struct audiodata {
	float tonePos;
	float toneInc;
};

/*
 * This array represents the keypad for the emulator.
 * They are the hex numbers 0-F in order.
 */
static char keys[] = {
	SDL_SCANCODE_X,
	SDL_SCANCODE_1,
	SDL_SCANCODE_2,
	SDL_SCANCODE_3,
	SDL_SCANCODE_Q,
	SDL_SCANCODE_W,
	SDL_SCANCODE_E,
	SDL_SCANCODE_A,
	SDL_SCANCODE_S,
	SDL_SCANCODE_D,
	SDL_SCANCODE_Z,
	SDL_SCANCODE_C,
	SDL_SCANCODE_4,
	SDL_SCANCODE_R,
	SDL_SCANCODE_F,
	SDL_SCANCODE_V,
};

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* texture;
static SDL_AudioDeviceID device;
static SDL_AudioSpec* spec;

static void feed(void* udata, uint8_t* stream, int len) {
	struct audiodata* audio = (struct audiodata*)udata;
	for (int i = 0; i < len; i++) {
		stream[i] = sinf(audio->tonePos) + 127;
		audio->tonePos += audio->toneInc;
	}
}

static SDL_AudioSpec* initAudioSpec(void) {
	struct audiodata* audio = malloc(sizeof(*audio));
	audio->tonePos = 0;
	audio->toneInc = 2 * 3.14159 * 1000 / AUDIO_FREQ;

	spec = malloc(sizeof(*spec));
	spec->freq = AUDIO_FREQ;
	spec->format = AUDIO_U8;
	spec->channels = 1;
	spec->samples = 4096;
	spec->callback = *feed;
	spec->userdata = audio;
	return spec;
}

static void cleanup(void) {
	if (device != 0) {
		SDL_CloseAudioDevice(device);
		device = 0;
	}

	if (spec != NULL) {
		free(spec->userdata);
		free(spec);
		spec = NULL;
	}

	if (texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}

	if (renderer != NULL) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}

	if (window != NULL) {
		SDL_DestroyWindow(window);
		window = NULL;
	}

	SDL_Quit();
}

static void expandScreen(uint8_t* from, uint32_t* to, int useHDPI) {
	if (useHDPI) { 
		for (int i = 0; i < SCREEN_SW * SCREEN_SH; i++) to[i] = from[i] ? -1 : 0;
		return;
	}
		
	int x = 0;
	int y = 0;

	for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
		uint32_t val = from[i] ? -1 : 0;
		to[TEXTURE_PX(2 * x + 0, 2 * y + 0)] = val;
		to[TEXTURE_PX(2 * x + 1, 2 * y + 0)] = val;
		to[TEXTURE_PX(2 * x + 0, 2 * y + 1)] = val;
		to[TEXTURE_PX(2 * x + 1, 2 * y + 1)] = val;
		if (++x == SCREEN_W) {
			x = 0;
			y++;
		}
	}
}

int initContext(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return 1;

	window = SDL_CreateWindow(
		WIN_TITLE, 
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WIN_WIDTH, 
		WIN_HEIGHT,
		SDL_WINDOW_SHOWN
	);
	if (window == NULL) {
		cleanup();
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		cleanup();
		return 1;
	}

	texture = SDL_CreateTexture(
		renderer, 
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		128,
		64
	);
	if (texture == NULL) {
		cleanup();
		return 1;
	}

	return 0;
}

void destroyContext(void) { cleanup(); }

int initAudio(void) {
	spec = initAudioSpec();
	device = SDL_OpenAudioDevice(NULL, 0, spec, NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	return (device == 0);
}

void drawDisplay(struct cpu* cpu) {
	void* pixels;
	int pitch;

	SDL_LockTexture(texture, NULL, &pixels, &pitch);
	expandScreen(cpu->screen, (uint32_t*)pixels, cpu->esm);
	SDL_UnlockTexture(texture);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

int isKeyDown(uint8_t key) {
	const uint8_t* sdlkeys;
	uint8_t realkey;
	if (key > 15) return 0;
	sdlkeys = SDL_GetKeyboardState(NULL);
	realkey = keys[(int)key];
	return sdlkeys[realkey];
}

int isQuitRequest() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		if (ev.type == SDL_QUIT) return 1;
	}
	return 0;
}

void updateSpeaker(int enabled) {
	if (enabled) SDL_PauseAudioDevice(device, 0);
	else SDL_PauseAudioDevice(device, 1);
}

