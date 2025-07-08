#include "chip8.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <SDL3/SDL.h>

void chip8::initialize() {
	pc = 0x200;  // Program counter starts at 0x200
	opcode = 0;      // Reset current opcode	
	I = 0;      // Reset index register
	sp = 0;      // Reset stack pointer

	std::fill_n(gfx, 64 * 32, 0);
	std::fill_n(stack, 16, 0);
	std::fill_n(V, 16, 0);
	std::fill_n(memory, 4096, 0);

	std::copy(chip8_fontset, chip8_fontset + 80, memory + 0x050);

	delay_timer = 0;
	sound_timer = 0;
}

void chip8::emulateCycle() {
	unsigned short opcode(memory[pc] << 8 | memory[pc + 1]);
	auto nibble1{ (opcode & 0x0F00) >> 8 };
	auto nibble2{ (opcode & 0x00F0) >> 4 };
	auto nibble3{ opcode & 0x000F };

	switch (opcode & 0xF000) {
	case 0x0000:
		switch (opcode & 0x000F) {
		case 0x0000: // 0x00E0: Clears the screen        
			std::fill_n(gfx, 64 * 32, 0);
			drawFlag = true;
			pc += 2;
			break;
		case 0x000E: // 0x00EE: Returns from subroutine          
			pc = stack[--sp];
			pc += 2;
			break;
		default:
			std::cout << "Unknown opcode [0x0000]: " << std::hex << opcode << std::dec << '\n';
		}
		break;
	case 0x1000: // 0x1NNN: Unconditional jump to NNN
		pc = opcode & 0x0FFF;
		break;
	case 0x2000: // 0x2NNN: Call subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;
	case 0x3000: // 0x3XNN: Conditionally skips the next instruction if Vx == NN
		if (V[nibble1] == ((nibble2 << 4) | nibble3))
			pc += 4;
		else
			pc += 2;
		break;
	case 0x4000: // 0x4XNN: Conditionally skips the next instruction if Vx != NN
		if (V[nibble1] != ((nibble2 << 4) | nibble3))
			pc += 4;
		else
			pc += 2;
		break;
	case 0x5000: // 0x5XY0: Conditionally skips the next instruction if Vx == Vy
		if (V[nibble1] == V[nibble2])
			pc += 4;
		else
			pc += 2;
		break;
	case 0x6000: // 0x6XNN: Sets Vx to NN (Vx = NN)
		V[nibble1] = (nibble2 << 4) | nibble3;
		pc += 2;
		break;
	case 0x7000: // 0x7XNN: Adds NN to Vx (Vx += NN)
		V[nibble1] += (nibble2 << 4) | nibble3;
		pc += 2;
		break;
	case 0x8000:
		switch (nibble3) {
		case 0x0000: // 8XY0: Sets Vx to the value of Vy (Vx = Vy)
			V[nibble1] = V[nibble2];
			pc += 2;
			break;
		case 0x0001: // 8XY1: Sets Vx to Vx or Vy (bitwise OR operation) (Vx |= Vy)
			V[nibble1] |= V[nibble2];
			pc += 2;
			break;
		case 0x0002: // 8XY2: Sets Vx to Vx and Vy (bitwise AND operation) (Vx &= Vy)
			V[nibble1] &= V[nibble2];
			pc += 2;
			break;
		case 0x0003: // 8XY3: Sets Vx to Vx xor Vy (Vx ^= Vy)
			V[nibble1] ^= V[nibble2];
			pc += 2;
			break;
		case 0x0004: // 8XY4: Adds Vy to Vx. VF is set to 1 when there's an overflow, and vice versa (Vx += Vy)
			if (V[nibble2] > (0xFF - V[nibble1]))
				V[0xF] = 1; // carry
			else
				V[0xF] = 0;
			V[nibble1] += V[nibble2];
			pc += 2;
			break;
		case 0x0005: // 8XY5: Vy is subtracted from Vx. VF is set to 0 when there's an underflow, and
			if (V[nibble1] < V[nibble2]) // vice versa (Vx -= Vy)
				V[0xF] = 0; // carry
			else
				V[0xF] = 1;
			V[nibble1] -= V[nibble2];
			pc += 2;
			break;
		case 0x0006: // 8XY6: Shifts Vx to the right by 1, then stores the least significant bit of
			V[0xF] = V[nibble1] & 1; // Vx prior to the shift into VF (Vx >>= 1)
			V[nibble1] >>= 1;
			pc += 2;
			break;
		case 0x0007: // 8XY7: Sets Vx to Vy minus Vx. VF is set to 0 when there's an underflow, and
			if (V[nibble2] < V[nibble1]) // vice versa (Vx = Vy - Vx)
				V[0xF] = 0; // carry
			else
				V[0xF] = 1;
			V[nibble1] = V[nibble2] - V[nibble1];
			pc += 2;
			break;
		/*
			8XYE: Shifts VX to the left by 1, then sets VF to 1 if the most significant bit
			of VX prior to that shift was set, and vice versa (Vx <<= 1)
		*/
		case 0x000E:
			V[0xF] = V[nibble1] & 0x80; // i.e. 0b10000000
			V[nibble1] <<= 1;
			pc += 2;
			break;
		default:
			std::cout << "Unknown opcode [0x8000]: " << std::hex << opcode << std::dec << '\n';
		}
		break;
	case 0x9000: // 9XY0: Skips the next instruction if Vx does not equal Vy
		if (V[nibble1] != V[nibble2])
			pc += 4;
		else
			pc += 2;
		break;
	case 0xA000: // ANNN: Sets I to the address NNN (I = NNN)
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	case 0xB000: // BNNN: Jumps to the address NNN plus V0
		pc = (opcode & 0x0FFF) + V[0];
		break;
	case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number and NN
		V[nibble1] = SDL_rand(256) & (opcode & 0x00FF);
		pc += 2;
		break;
	/*
		DXYN: Draws a sprite at coordinate (Vx, Vy) that has a width of 8 pixels and a height of N pixels.
		Each row of 8 pixels is read as bit-coded starting from memory location I;
		I value does not change after the execution of this instruction.
		VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
		and to 0 if that does not happen
	*/
	case 0xD000: {
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0) // 0x80 = 1000.0000 i.e. this line check individual bit
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						V[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
		break;
	case 0xE000:
		switch (opcode & 0x00FF)
		{
			// EX9E: Skips the next instruction 
			// if the key stored in Vx is pressed
		case 0x009E:
			if (key[V[nibble1]] != 0)
				pc += 4;
			else
				pc += 2;
			break;
		case 0x00A1: // EXA1: Skips the next instruction if the key stored in Vx is not pressed
			if (key[V[nibble1]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		default:
			std::cout << "Unknown opcode [0xE000]: " << std::hex << opcode << std::dec << '\n';
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007: // FX07: Sets Vx to the value of the delay timer (Vx = delay_timer)
			V[nibble1] = delay_timer;
			pc += 2;
			break;
		/*
			A key press is awaited, and then stored in VX (blocking operation, all instruction
			halted until next key event, delay and sound timers should continue processing)
		*/
		case 0x000A:
		{
			bool keyPress = false;

			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;
				}
			}

			// If we didn't received a keypress, skip this cycle and try again.
			if (!keyPress)
				return;

			pc += 2;
		}
			break;
		case 0x0015: // FX15: Sets the delay timer to Vx (delay_timer = Vx)
			delay_timer = V[nibble1];
			pc += 2;
			break;
		case 0x0018: // FX18: Sets the sound timer to Vx (sound_timer = Vx)
			sound_timer = V[nibble1];
			pc += 2;
			break;
		case 0x001E: // FX1E: Adds Vx to I. VF is not affected
			I += V[nibble1];
			pc += 2;
			break;
		case 0x0029: // FX29: Sets I to the location of the sprite for the character in Vx
			I = V[nibble1] * 5 + 0x50;
			pc += 2;
			break;
		/*
			FX33: Stores the binary-coded decimal representation of VX, with the hundreds digit
			in memory at location in I, the tens digit at location I+1, and the ones digit at
			location I+2
		*/
		case 0x0033:
			memory[I] = V[nibble1] / 100;
			memory[I + 1] = (V[nibble1] / 10) % 10;
			memory[I + 2] = (V[nibble1] % 100) % 10;
			pc += 2;
			break;
		case 0x0055: // FX55: copy V0 - Vx to memory from I. I is unmodified
			std::copy(V, V + nibble1 + 1, memory + I);
			pc += 2;
			break;
		case 0x0065: // FX65: copy V0 - VX to memory from I. I is unmodified
			std::copy(memory + I, memory + I + nibble1 + 1, V);
			pc += 2;
			break;
		}
		break;
	default:
		std::cout << "Unknown opcode: " << std::hex << opcode << std::dec << '\n';
	}
}

bool chip8::loadGame(std::string_view filename) {
	int pos{ 0x200 };
	for (std::ifstream f(filename.data(), std::ios::binary); f.good(); )
		memory[pos++ & 0xFFF] = f.get();
	return true;
}

void chip8::setKeys() {
	int numkeys{};
	SDL_PumpEvents();
	auto keyboard_state{ SDL_GetKeyboardState(&numkeys) };
	for (int i{ 4 }; i < numkeys; ++i) {
		switch (i) {
		case SDL_SCANCODE_1:
			key[0] = keyboard_state[i];
			break;
		case SDL_SCANCODE_2:
			key[1] = keyboard_state[i];
			break;
		case SDL_SCANCODE_3:
			key[2] = keyboard_state[i];
			break;
		case SDL_SCANCODE_4:
			key[3] = keyboard_state[i];
			break;
		case SDL_SCANCODE_Q:
			key[4] = keyboard_state[i];
			break;
		case SDL_SCANCODE_W:
			key[5] = keyboard_state[i];
			break;
		case SDL_SCANCODE_E:
			key[6] = keyboard_state[i];
			break;
		case SDL_SCANCODE_R:
			key[7] = keyboard_state[i];
			break;
		case SDL_SCANCODE_A:
			key[8] = keyboard_state[i];
			break;
		case SDL_SCANCODE_S:
			key[9] = keyboard_state[i];
			break;
		case SDL_SCANCODE_D:
			key[10] = keyboard_state[i];
			break;
		case SDL_SCANCODE_F:
			key[11] = keyboard_state[i];
			break;
		case SDL_SCANCODE_Z:
			key[12] = keyboard_state[i];
			break;
		case SDL_SCANCODE_X:
			key[13] = keyboard_state[i];
			break;
		case SDL_SCANCODE_C:
			key[14] = keyboard_state[i];
			break;
		case SDL_SCANCODE_V:
			key[15] = keyboard_state[i];
			break;
		}
	}
}