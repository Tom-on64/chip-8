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

// Font
#define FONT_0 { 0xF0, 0x90, 0x90, 0x90, 0xF0 } 
#define FONT_1 { 0x20, 0x60, 0x20, 0x20, 0x70 }
#define FONT_2 { 0xF0, 0x10, 0xF0, 0x80, 0xF0 }
#define FONT_3 { 0xF0, 0x10, 0xF0, 0x10, 0xF0 }
#define FONT_4 { 0x90, 0x90, 0xF0, 0x10, 0x10 }
#define FONT_5 { 0xF0, 0x80, 0xF0, 0x10, 0xF0 }
#define FONT_6 { 0xF0, 0x80, 0xF0, 0x90, 0xF0 }
#define FONT_7 { 0xF0, 0x10, 0x20, 0x40, 0x40 }
#define FONT_8 { 0xF0, 0x90, 0xF0, 0x90, 0xF0 }
#define FONT_9 { 0xF0, 0x90, 0xF0, 0x10, 0xF0 }
#define FONT_A { 0xF0, 0x90, 0xF0, 0x90, 0x90 }
#define FONT_B { 0xE0, 0x90, 0xE0, 0x90, 0xE0 }
#define FONT_C { 0xF0, 0x80, 0x80, 0x80, 0xF0 }
#define FONT_D { 0xE0, 0x90, 0x90, 0x90, 0xE0 }
#define FONT_E { 0xF0, 0x80, 0xF0, 0x80, 0xF0 }
#define FONT_F { 0xF0, 0x80, 0xF0, 0x80, 0x80 }

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
	uint8_t dt;
	uint8_t st;

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
