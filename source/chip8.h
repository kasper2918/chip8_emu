#ifndef CHIP8_H
#define CHIP8_H

#include <string_view>
#include <SDL3/SDL.h>

class chip8 {
public:
	// Flag that determines whether to redraw a screen
	bool drawFlag{};

	/*
		Initializes chip8.
		pc (program counter) is set to 0x200,
		memory addresses [0x050, 0x0A0) are set to (preloaded) fontset.
		Anything else is set to/filled with zero.
	*/
	void initialize();

	/*
		Emulates executing one opcode.
	*/
	void emulateCycle();

	bool loadGame(std::string_view filename);

	/*
		Scans keyboard and saves key states in chip8's key member array
	*/
	void setKeys();

	void decreaseTimers() { --delay_timer; --sound_timer; }

	friend int main(int argc, char* argv[]);
	friend SDL_AppResult SDL_AppIterate(void* appstate);

private:
	// Chip8 has fixed-length 2 byte instructions (opcodes)
	unsigned short opcode{};

	unsigned char memory[4096]{};
	unsigned char chip8_fontset[80] =
	{
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

	// Registers array
	unsigned char V[16]{};

	// Pointer to a memory address
	unsigned short I{};
	// Program counter
	unsigned short pc{};

	// Chip8 has 64x32 display with white-black graphics
	unsigned char gfx[64 * 32]{};

	unsigned char delay_timer{};
	unsigned char sound_timer{};

	// Stack with depth of 16 and stack pointer (sp)
	unsigned short stack[16]{};
	unsigned short sp{};

	// An array to store key states
	unsigned char key[16]{};
};

#endif // !CHIP8_H
