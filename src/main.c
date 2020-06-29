#include "../include/chip8.h"

int main(int argc, char *args[]) {
	SDL_Thread *timerThread;
	
	if(argc < 2)
		die(USAGE);

	if(!initializeGraphics())
		die("emulator initialization has failed!");
	if(!initializeEmulator())
		die("emulator initialization has failed!");

	
	loadProgram(args[argc - 1]);

	cpuIsRunning = true;

	timerThread = SDL_CreateThread(runTimers, "TimerThread", NULL);
	
	for(;;) {
		while(SDL_PollEvent(&gEvent)) {
			switch(gEvent.type) {
				case SDL_APP_TERMINATING:
				case SDL_QUIT:
					SDL_AtomicLock(&gTimerLock);

					cpuIsRunning = false;

					SDL_AtomicUnlock(&gTimerLock);

					goto endLoop;
					break;

				case SDL_WINDOWEVENT:
					updateScreen();
					break;

				case SDL_KEYDOWN:
					key[convertKeyToHex(gEvent.key.keysym.sym)] = 1;

					break;
			}
		}

		emulateCycle();

		if(drawFlag) {
			updateScreen();

			drawFlag = false;
		}


		SDL_Delay(1);
	}

endLoop:

	SDL_WaitThread(timerThread, NULL);

	cleanup();

	return EXIT_SUCCESS;
}

bool initializeGraphics(void) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
                fprintf(stderr, "Failed to initialize SDL. %s", SDL_GetError());
                return false;
        }

	gWindow = SDL_CreateWindow(WINDOW_NAME,
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        WINDOW_WIDTH, WINDOW_HEIGHT,
                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if(gWindow == NULL) {
                fprintf(stderr, "Failed to create the window: %s", SDL_GetError());
                return false;
        }

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);

        if(gRenderer == NULL) {
                fprintf(stderr, "Failed to create the renderer: %s", SDL_GetError());
                return false;
        }

	SDL_RenderSetLogicalSize(gRenderer, EMULATOR_SCREEN_WIDTH, EMULATOR_SCREEN_HEIGHT);

	return true;
}

bool initializeEmulator(void) {
	int i;

	gTimerLock = 0;

	pc 	= 0x200;
	opcode	= 0;
	I	= 0;
	sp	= 0;

	drawFlag = false;

	memset(&memory, 0x0, sizeof(memory));
	memset(&screen, 0x0, sizeof(screen));
	memset(&stack, 0x0, sizeof(stack));
	memset(&V, 0x0, sizeof(V));

	delayTimer = 0;

	for(i = 0; i < 80; i++) {
		memory[i] = chip8Fontset[i];
	}

	return true;
}



bool loadProgram(const char *path) {
	FILE *programFile;
	uint8_t buffer[4096-0x200];
	size_t i;

	programFile = fopen(path, "rb");

	fread(&buffer, 4096-0x200, 1, programFile);


	for(i = 0; i < 4096-0x200; i++) {
		memory[i + 0x200] = buffer[i];
	}

	fclose(programFile);
}

int runTimers(void *data) {
	int capTimer;

	for(;;) {
		capTimer = SDL_GetTicks();

		SDL_AtomicLock(&gTimerLock);

		if(delayTimer > 0)
			delayTimer--;
		if(soundTimer > 0) {
			puts("BEEP!");
			soundTimer--;
		}

		if(!cpuIsRunning)
			break;

		SDL_AtomicUnlock(&gTimerLock);

		if((capTimer - SDL_GetTicks()) < TIMER_TICKS_PER_CYCLE)
			SDL_Delay(TIMER_TICKS_PER_CYCLE - (capTimer - SDL_GetTicks()));
	}

	return 0;
}

void emulateCycle(void) {
	uint8_t i;


	opcode = memory[pc] << 8 | memory[pc + 1];

	// Almost all of the following descriptions of the instructions are
	// based on Cowdog's Chip-8 technical reference (http://devernay.free.fr/hacks/chip8/C8TECH10.HTM).
	// The descriptions and mnemonics are there just to help to find
	// an instructions and to quickly understand what a specific instruction
	// does. These comments can't replace a technical manual (such as Cowdog's).
	
	switch(opcode & 0xF000) {
		case 0x0000:
			switch(opcode & 0x00FF) {
				// CLS: clears the screen
				case 0x00E0:
					memset(&screen, 0, sizeof(screen));

					drawFlag = true;

					pc+=2;

					break;

				// RET: Returns back from where subroutine was called
				case 0x00EE:
					pc = stack[--sp];

					break;

				// SYS addr: this instruction is ignored by modern interpreters
				default:
					//pc += 2;
					break;
			}

			break;

		// JP addr: jumps to address addr
		case 0x1000:
			pc = opcode & 0x0FFF;

			break;

		// CALL addr: Call subroutine at addr
		case 0x2000:
			stack[sp++] = (pc+=2);

			pc = opcode & 0x0FFF;

			break;

		// SE Vx, byte: Skips the next instruction if the register Vx is = the byte
		case 0x3000:
			if((V[(opcode & 0x0F00) >> 8]) == (opcode & 0x00FF)) {
				pc += 2;
			}

			pc += 2;

			break;

		// SNE Vx, byte: skips the next instruction if Vx != byte
		case 0x4000:
			if((V[(opcode & 0x0F00) >> 8]) != (opcode & 0x00FF))
				pc += 2;

			pc += 2;

			break;

		// SE Vx, Vy: skips the next instruction if Vx = Vy
		case 0x5000:
			if((V[(opcode & 0x0F00) >> 8]) == V[(opcode & 0x00F0) >> 4])
				pc += 2;

			pc += 2;

			break;

		// LD Vx, byte: changes the value of Vx to byte
		case 0x6000:
			V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			pc+=2;

			break;

		// ADD Vx, byte: Adds byte to Vx and sum is stored in Vx
		case 0x7000:
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;

			pc += 2;

			break;

		case 0x8000:
			switch(opcode & 0x000F) {
				// LD Vx, Vy: stores the value of refister Vy into register Vx
				case 0x0000:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];

					pc += 2;

					break;

				// OR Vx, Vy: ORs the values of Vx and Vy and after that stores the value in Vx
				case 0x0001:
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];

					pc += 2;

					break;

				// AND Vx, Vy: ANDs the values of Vx and Vy and after that stores the value in Vx
				case 0x0002:
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];

					pc += 2;

					break;

				// XOR Vx, Vy: XORs the values of Vx and Vy and after that stores the value in Vx
				case 0x0003:
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];

					pc += 2;

					break;

				// ADD Vx, Vy: Adds the values of registers Vx and Vy together and the sum is stored in
				// the Vx register. If the sum of the 2 registers are greater than 8 bits then the
				// VF register's value is set to 1
				case 0x0004:
					if(((uint16_t)V[(opcode & 0x0F00) >> 8] + (uint16_t)V[(opcode & 0x00F0) >> 4] > 255))
						V[0xF] = 1;
					else
						V[0xF] = 0;

					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4]; 

					pc += 2;

					break;

				// SUB Vx, Vy: The value of register Vy is subtracted from the value of
				// register Vx and the result is stored in Vx
				case 0x0005:
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
						V[0xF] = 1;
					else
						V[0xF] = 0;

					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];

					pc += 2;

					break;

				// SHR Vx {, Vy}: Vx is divided by 2 and stores the result in Vx
				case 0x0006:
					if(V[(opcode & 0x0F00) >> 8] & 1)
						V[0xF] = 1;
					else
						V[0xF] = 0;

					V[(opcode & 0x0F00) >> 8] >>= 1;

					pc += 2;

					break;

				// SUBN Vx, Vy: Value of Vx is subtracted from Vy and the result is stored in Vx  
				case 0x0007:
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
						V[0xF] = 1;
					else
						V[0xF] = 0;

					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];

					pc += 2;

					break;

				// SHL Vx {, Vy}: Vx is multiplied by 2
				case 0x000E:
					if(V[(opcode & 0x0F00) >> 8] & (1 << 7))
						V[0xF] = 1;
					else
						V[0xF] = 0;

					V[(opcode & 0x0F00) >> 8] <<= 1;

					pc += 2;

					break;
			}

			break;

		// SNE Vx, Vy: Skips next instruction if Vx != Vy
		case 0x9000:
			if((V[(opcode & 0x0F00) >> 8]) != V[(opcode & 0x00F0) >> 4])
				pc += 2;

			pc += 2;

			break;

		// LD I, addr: changes the value of the I register to address addr 
		case 0xA000:
			I = opcode & 0x0FFF;
			
			pc+=2;

			break;

		// JP V0, addr: Jump to location addr + V0 (register can't be changed is always V0 i.e. 
		// "JP V4, addr" is illegal)
		case 0xB000:
			pc = (opcode & 0x0FFF) + V[0x0];

			break;

		// RND Vx, byte: generates a random byte and ANDs it with byte and stores it in register Vx
		case 0xC000:
			V[(opcode & 0x0F00) >> 8] = genRand() & (opcode & 0x00FF);

			pc += 2;

			break;

		// DRW, Vx, Vy, nibble: Draws n-byte sprite at memory location I at Vx, Vy
		case 0xD000:
			drawInstruction(V[(opcode & 0x0F00) >> 8], V[(opcode & 0x00F0) >> 4], 
					opcode & 0x000F);
			pc += 2;
	
			break;

		case 0xE000:
			switch(opcode & 0x00FF) {
				// SKP Vx: Skips next instruction if key with the value of register Vx is pressed.
				case 0x009E:
					if(key[V[(opcode & 0x0F00) >> 8]]) {
						key[V[(opcode & 0x0F00) >> 8]] = 0;
						pc += 2;
					}

					pc += 2;
					break;
				// SKNP Vx: Skips next instruction if key with the value of register Vx isn't pressed
				case 0x00A1:
					if(!(key[V[(opcode & 0x0F00) >> 8]]))
						pc += 2;
					else
						key[V[(opcode & 0x0F00) >> 8]] = 0;


					pc += 2;
					break;
			}

			break;

		case 0xF000:
			switch(opcode & 0x00FF) {
				// LD Vx, DT: Delay Timer = Vx
				case 0x0007:
					SDL_AtomicLock(&gTimerLock);
					V[(opcode & 0x0F00) >> 8] = delayTimer;
					SDL_AtomicUnlock(&gTimerLock);

					pc += 2;

					break;

				// LD Vx, K: Waits for a key press then stores in Vx
				case 0x000A:
					waitForKey();

					pc += 2;

					break;

				// LD DT, Vx: Vx = Delay Timer
				case 0x0015:
					SDL_AtomicLock(&gTimerLock);
					delayTimer = V[(opcode & 0x0F00) >> 8];
					SDL_AtomicUnlock(&gTimerLock);

					pc += 2;

					break;

				// LD ST, Vx: Sound Timer = Vx
				case 0x0018:
					SDL_AtomicLock(&gTimerLock);
					soundTimer = V[(opcode & 0x0F00) >> 8];
					SDL_AtomicUnlock(&gTimerLock);
					
					pc += 2;

					break;
				// ADD I, Vx: I = I + Vx
				case 0x001E:
					I += V[(opcode & 0x0F00) >> 8];

					pc += 2;

					break;

				// LD F, Vx: Sets I to the location of sprite for digit Vx
				case 0x0029:
					I = FONTCHAR_SIZE * V[(opcode & 0x0F00) >> 8];

					pc += 2;

					break;

				// LD B, Vx: Store BCD representation of Vx in memory locations I, I+1 and I+2
				case 0x0033:
					memory[I]     = (V[(opcode & 0x0F00) >> 8] % 1000) / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] % 100) / 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 10);

					pc += 2;

					break;

				// LD [I], Vx: Stores registers V0 through Vx in memory starting at location I
				case 0x0055:
					for(i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
						memory[I + i] = V[i];
					}

					pc += 2;

					break;

				// LD Vx, [i]: Read registers V0 throught Vx from memory starting at memory location I
				case 0x0065:
					for(i = 0; i <= (opcode & 0x0F00) >> 8; i++) {
						V[i] = memory[I+i];
					}

					pc += 2;

					break;
			}
	}
}

void drawInstruction(uint8_t x, uint8_t y, uint8_t height) {
	uint16_t pixel;
	int i, j;
	
	for(i = 0; i < height; i++) {
		pixel = memory[I + i];
		for(j = 0; j < 8; j++) {
			if((pixel & (0x80 >> j)) != 0) {
				if(screen[y + i][x + j] == 1)
					V[0xF] = 1;
				screen[y + i][x + j] ^= 1;
			}
		}
	}


	drawFlag = true;
}

void waitForKey(void) {
	for(;;) {
		while(SDL_PollEvent(&gEvent)) {
			switch(gEvent.type) {
				case SDL_APP_TERMINATING:
				case SDL_QUIT:
					cleanup();

					exit(0);

					break;

				case SDL_KEYDOWN:
					V[(opcode & 0x0F00) >> 8] = convertKeyToHex(gEvent.key.keysym.sym);

					return;
			}
		}

		SDL_Delay(1);
	}
}

void updateScreen(void) {
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(gRenderer);
	drawScreen();
	SDL_RenderPresent(gRenderer);
}

void drawScreen(void) {
	int x, y;

	SDL_SetRenderDrawColor(gRenderer, 0x00, 0xff, 0xff, 0xff);
	for(y = 0; y < EMULATOR_SCREEN_HEIGHT; y++) 
		for(x = 0; x < EMULATOR_SCREEN_WIDTH; x++)
			if(screen[y][x])
				SDL_RenderDrawPoint(gRenderer, x, y);
}

uint8_t genRand(void) {
	return 173; // TODO: Create a random number generator
}

uint8_t convertKeyToHex(SDL_Keycode key) {
	switch(key) {
		case SDLK_0:
			return 0x0;
		case SDLK_1:
			return 0x1;
		case SDLK_2:
			return 0x2;
		case SDLK_3:
			return 0x3;
		case SDLK_4:
			return 0x4;
		case SDLK_5:
			return 0x5;
		case SDLK_6:
			return 0x6;
		case SDLK_7:
			return 0x7;
		case SDLK_8:
			return 0x8;
		case SDLK_9:
			return 0x9;
		case SDLK_a:
			return 0xA;
		case SDLK_b:
			return 0xB;
		case SDLK_c:
			return 0xC;
		case SDLK_d:
			return 0xD;
		case SDLK_e:
			return 0xE;
		case SDLK_f:
			return 0xF;
	}

	return 0x0;
}

void cleanup(void) {
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);

	SDL_Quit();
}

void unknownOpcode(void) {
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unknown opcode %X...", opcode);
}

void die(const char *fmt, ...) {
        va_list vargs;

        va_start(vargs, fmt);
        SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, fmt, vargs);
        va_end(vargs);

        cleanup();

        exit(EXIT_FAILURE);
}
