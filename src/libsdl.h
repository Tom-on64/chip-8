#ifndef LIBSDL_H
#define LIBSDL_H

#include <SDL2/SDL.h>
#include "cpu.h"

#define TEXTURE_PX(_x, _y) (128 * (_y) + (_x))

#define AUDIO_FREQ	44100
#define WIN_TITLE	"CHIP-8 Emulator"
#define WIN_WIDTH	640
#define WIN_HEIGHT	320

int initContext();
int initAudio();
void destroyContext();
void drawDisplay(struct cpu* cpu);
int isKeyDown(uint8_t key);
int isQuitRequest();
void updateSpeaker(int enabled);

#endif
