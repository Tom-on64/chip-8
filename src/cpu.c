#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "cpu.h"

// NOTE: Assumes that 'cpu' is in scope 
#define LOG(_fmt, ...) if (cpu->dbg) {\
	fprintf(stderr, "[DBG] ");\
	fprintf(stderr, _fmt, __VA_ARGS__);\
}

static int delta;

static uint8_t font[] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Screen?
static void nib_0(struct cpu* cpu, uint16_t ins) {
	if (ins == 0x00e0) memset(cpu->screen, 0, sizeof_array(cpu->screen));
}

static void nib_1(struct cpu* cpu, uint16_t ins) {
	cpu->pc = DEC_NNN(ins);
}

static void nib_2(struct cpu* cpu, uint16_t ins) {}
static void nib_3(struct cpu* cpu, uint16_t ins) {}
static void nib_4(struct cpu* cpu, uint16_t ins) {}
static void nib_5(struct cpu* cpu, uint16_t ins) {}

static void nib_6(struct cpu* cpu, uint16_t ins) {
	cpu->regs[DEC_X(ins)] = DEC_NN(ins);
}

static void nib_7(struct cpu* cpu, uint16_t ins) {
	cpu->regs[DEC_X(ins)] += DEC_NN(ins);
}

static void nib_8(struct cpu* cpu, uint16_t ins) {}
static void nib_9(struct cpu* cpu, uint16_t ins) {}

static void nib_a(struct cpu* cpu, uint16_t ins) {
	cpu->idx = DEC_NNN(ins);
}

static void nib_b(struct cpu* cpu, uint16_t ins) {}
static void nib_c(struct cpu* cpu, uint16_t ins) {}

static void nib_d(struct cpu* cpu, uint16_t ins) {
	uint8_t x = cpu->regs[DEC_X(ins)];
	uint8_t y = cpu->regs[DEC_Y(ins)];
	
	if (cpu->esm && DEC_N(ins) == 0) {
		for (int j = 0; j < 16; j++) {
			uint8_t hi = cpu->memory[(cpu->idx + 2 * j) & ADDRESS_MASK];
			uint8_t lo = cpu->memory[(cpu->idx + 2 * j + 1) & ADDRESS_MASK];
			uint16_t sprite = (hi << 8) | lo;
			for (int i = 0; i < 16; i++) {
				int px = (x + i) & (SCREEN_SW - 1);
				int py = (y + j) & (SCREEN_SH - 1);
				int pos = SCREEN_SW * py + px;

				int pix = (sprite & (1 << (15 - i))) != 0;
				cpu->regs[0xf] |= (cpu->screen[pos] & pix);
				cpu->screen[pos] ^= pix;
			}
		}
		return;
	}

	for (int j = 0; j < DEC_N(ins); j++) {
		uint8_t sprite = cpu->memory[(cpu->idx + j) & ADDRESS_MASK];
		for (int i = 0; i < 8; i++) {
			int px = (x + i) & (cpu->esm ? (SCREEN_SW - 1) : (SCREEN_W - 1));
			int py = (y + j) & (cpu->esm ? (SCREEN_SH - 1) : (SCREEN_H - 1));
			int pos = (cpu->esm ? SCREEN_SW : SCREEN_W) * py + px;
			
			int pix = (sprite & (1 << (7 - i))) != 0;
			cpu->regs[0xf] |= (cpu->screen[pos] & pix);
			cpu->screen[pos] ^= pix;
		}
	}
}

static void nib_e(struct cpu* cpu, uint16_t ins) {}
static void nib_f(struct cpu* cpu, uint16_t ins) {}

void stepEmu(struct cpu* cpu) {
	// Fetch
	uint8_t hib = cpu->memory[cpu->pc++];
	uint8_t lob = cpu->memory[cpu->pc++];
	uint16_t ins = (hib << 8) | lob;

	LOG("[%03x] = %04x %x\n", cpu->pc-2, ins, ins >> 12);

	// Decode & Execute
	switch (ins >> 12) {
	case 0x0: nib_0(cpu, ins); break;
	case 0x1: nib_1(cpu, ins); break;
	case 0x2: nib_2(cpu, ins); break;
	case 0x3: nib_3(cpu, ins); break;
	case 0x4: nib_4(cpu, ins); break;
	case 0x5: nib_5(cpu, ins); break;
	case 0x6: nib_6(cpu, ins); break;
	case 0x7: nib_7(cpu, ins); break;
	case 0x8: nib_8(cpu, ins); break;
	case 0x9: nib_9(cpu, ins); break;
	case 0xa: nib_a(cpu, ins); break;
	case 0xb: nib_b(cpu, ins); break;
	case 0xc: nib_c(cpu, ins); break;
	case 0xd: nib_d(cpu, ins); break;
	case 0xe: nib_e(cpu, ins); break;
	case 0xf: nib_f(cpu, ins); break;
	}
}

void updateTime(struct cpu* cpu, int dt) {
	delta += dt;
	while (dt > (1000/60)) {
		delta -= (1000/60);
		if (cpu->dtimer != 0) cpu->dtimer--;
		if (cpu->stimer != 0) {
			if (--cpu->stimer == 0 && cpu->speaker) cpu->speaker(0);
			else if (cpu->speaker) cpu->speaker(1);
		}
	}
}

int initEmu(struct cpu* cpu) {
	memcpy(&cpu->memory[0x50], font, sizeof_array(font));
	cpu->pc = 0x200;
	cpu->waitkey = -1;
	delta = 0;
	return 0;
}

