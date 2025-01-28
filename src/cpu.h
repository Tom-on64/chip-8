#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define sizeof_array(_a) (sizeof(_a) / sizeof(_a[0]))

#define MEMORY_LEN	4096
#define STACK_LEN	32
#define TIMER_FREQ	60
#define ADDRESS_MASK	0x0fff

#define SCREEN_W	64
#define SCREEN_H	32
#define SCREEN_SW	128
#define SCREEN_SH	64

// Decoding macros
#define DEC_X(_i)	(((_i) >> 8) & 0xf)
#define DEC_Y(_i)	(((_i) >> 4) & 0xf)
#define DEC_N(_i)	((_i) & 0xf)
#define DEC_NN(_i)	((_i) & 0xff)
#define DEC_NNN(_i)	((_i) & 0xfff)

typedef int (*kbdpoller_t)(char);
typedef void(*spkpoller_t)(int);

struct cpu {
	// Memory
	uint8_t memory[MEMORY_LEN];
	uint16_t pc; // Program counter

	// Stack
	uint16_t stack[STACK_LEN];
	uint8_t sp; // Stack pointer

	// Registers
	uint8_t regs[16];
	uint16_t idx;

	// Timer regs
	uint8_t dtimer;
	uint8_t stimer;

	// Pollers
	kbdpoller_t keydown;
	spkpoller_t speaker;

	// Screen
	uint8_t screen[SCREEN_SW * SCREEN_SH]; // Allocate enough for extended screen
	int esm; // Extended screen mode

	int hlt; // Halt flag
	int dbg; // Debug flag
	int waitkey; // Key to wait for
};

int initEmu(struct cpu* cpu);
void updateTime(struct cpu* cpu, int dt);
void stepEmu(struct cpu* cpu);

#endif
