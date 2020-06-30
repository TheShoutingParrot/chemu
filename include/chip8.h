#ifndef _CHIP8_H
#define _CHIP8_h

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define WINDOW_NAME		"chip8 emulator"
#define WINDOW_WIDTH		640
#define WINDOW_HEIGHT		480

#define STACK_SIZE		16

#define EMULATOR_SCREEN_WIDTH	64
#define EMULATOR_SCREEN_HEIGHT	32

#define FONTCHAR_SIZE		5

#define TIMER_SPEED		60
#define TIMER_TICKS_PER_CYCLE	1000 / TIMER_SPEED

#define USAGE			"usage: chemu rom.ch8"

uint16_t opcode,
	 I,
	 pc,
	 stack[STACK_SIZE],
	 sp;

uint8_t memory[4096],
	V[16],
	screen[EMULATOR_SCREEN_HEIGHT][EMULATOR_SCREEN_WIDTH],
	delayTimer,
	soundTimer,
	key[16];

uint8_t chip8Fontset[80] = {
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

bool drawFlag,
     cpuIsRunning;

SDL_Window *gWindow;
SDL_Renderer *gRenderer;

SDL_Event gEvent;

SDL_SpinLock gTimerLock;

bool initializeGraphics(void);
bool initializeEmulator(void);

bool loadProgram(const char *path);

int runTimers(void *data);

void emulateCycle(void);

void drawInstruction(uint8_t x, uint8_t y, uint8_t height);

void waitForKey(void);

void updateScreen(void);
void drawScreen(void);

uint8_t convertKeyToHex(SDL_Keycode n);

void printOutMemory(uint16_t start, uint16_t end);

void cleanup(void);

void unknownOpcode(void);

void die(const char *fmt, ...);

#endif // #ifndef _CHIP8_H
