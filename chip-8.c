// TODO: more elegant way of showing/hiding debug output

// C LIBRARIES
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// SDL
#include <SDL3/SDL.h>

// lookup table for reversing a bit string - useful in draw instruction
	static const uint8_t reverse_table[256] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };

// config
	struct color {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha;
	};
	
	struct color fg_color;
	struct color bg_color;
	
	uint8_t    SCALE;
	bool   debug;

// sdl tools
	SDL_Window*   window   = NULL;
	SDL_Renderer* renderer = NULL;

// array for display
	bool screen[32][64] = {0};	// each pixel's value

// emulator state
	bool running = false;
	
// memory - 4kb
	uint8_t mem[4096] = {0};
	
// size of the rom
	long size = 0;
	
// current instruction
	uint16_t instr 	= 0;
	uint8_t  first 	= 0;  
	uint8_t  second = 0;
	uint8_t  third  = 0;
	uint8_t  fourth = 0;
	uint8_t  last_2 = 0;
	uint16_t last_3 = 0;

// 16 bit registers - program index, memory index
	uint16_t pc = 0x200;
	uint16_t i  = 0x0;
	
// general purpose 8-bit registers
	uint8_t v[16] = {0};

// stack - holds 12 addresses
	uint16_t stack[48] = {0};
	short    stack_addr = -1;

// 8 bit timers - delay, sound
	uint8_t delay = 0;
	uint8_t sound = 0;
	
// fontset - goes into memory at the start
	static unsigned char chip8_fontset[80] =
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

// HOUSEKEEPING FUNCTIONS
// copies fontset to memory
void copy_fonts() {
	// 80 is the size of the fontset
	for (int a = 0; a < 80; a++) {
		mem[a] = chip8_fontset[a];
	}
}

// opens the rom to play
// if no rom is specified, just open roms/ibm_logo.ch8
bool open_file(char* name) {
	// open the file
	FILE* rom = NULL;
	
	if (name == NULL) {
		printf("no file name given! opening roms/ibm_logo.ch8\n\n");
		rom = fopen("roms/ibm_logo.ch8", "rb");
	} else {
		printf("opening %s\n\n", name);
		rom = fopen(name, "rb");
	}
	
	if (!rom) {
		SDL_Log("failed to open file: %s", name);
		return false;
	}
	
	// get file size
	fseek(rom, 0, SEEK_END);
	size = ftell(rom);
	const long size_max = sizeof(mem) - pc; // get the maximum possible file size for this emulator
	rewind(rom);
	
	// make sure file is of correct size
	if (size <= 0) {
		SDL_Log("file too small/invalid");
		return false;
	}
	if (size > size_max) {
		SDL_Log("file too big");
		return false;
	}
	
	// read file to memory
	if (fread(&mem[pc], 1, size, rom) != (size_t)size) {
		SDL_Log("failed to read file to emulator memory");
		fclose(rom);
		return false;
	}
	
	fclose(rom);
	
	return true;
}

// clean up all of the sdl tools and arrays that have been made
void end(SDL_Window* window, SDL_Renderer* renderer) {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	
	window = NULL;
	renderer = NULL;
	
	SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	
	SDL_Quit();
}

// DECODE STAGE
// turns the binary code to human-readable assembly
void print_instruction() {	
	switch (first){
		case 0x0:
			switch (last_2) {
				case 0xE0:
					SDL_Log("CLS");
					break;
				case 0xEE:
					SDL_Log("RET");
					break;
				default:
					SDL_Log("undefined - 0");
			}
			break;
		case 0x1:
			SDL_Log("JP %x", last_3);
			break;
		case 0x2:
			SDL_Log("CALL %x", last_3);
			break;
		case 0x3:
			SDL_Log("SE v%x, 0x%x", second, last_2);
			break;
		case 0x4:
			SDL_Log("SNE v%x, 0x%x", second, last_2);
			break;
		case 0x5:
			switch (fourth) {
				case 0x0:
					SDL_Log("SE v%x, v%x", second, third);
					break;
				default:
					SDL_Log("undefined - 5");
			}
			break;
		case 0x6:
			SDL_Log("LD v%x, 0x%x", second, last_2);
			break;
		case 0x7:
			SDL_Log("ADD v%x, 0x%x", second, last_2);
			break;
		case 0x8:
			switch (fourth) {
				case 0x0:
					SDL_Log("LD v%x, v%x", second, third);
					break;
				case 0x1:
					SDL_Log("OR v%x, v%x", second, third);
					break;
				case 0x2:
					SDL_Log("AND v%x, v%x", second, third);
					break;
				case 0x3:
					SDL_Log("XOR v%x, v%x", second, third);
					break;
				case 0x4:
					SDL_Log("ADD v%x, v%x", second, third);
					break;
				case 0x5:
					SDL_Log("SUB v%x, v%x", second, third);
					break;				
				case 0x6:
					SDL_Log("SHR v%x, v%x", second, third);
					break;
				case 0x7:
					SDL_Log("SUBN v%x, v%x", second, third);
					break;
				case 0xE:
					SDL_Log("SHL v%x, v%x", second, third);
					break;
				default:
					SDL_Log("undefined - 8");
			}
			break;
		case 0x9:
			SDL_Log("SNE v%x, v%x", second, third);
			break;
		case 0xA:
			SDL_Log("LD instr, 0x%x", last_3);
			break;
		case 0xB:
			SDL_Log("JP v0, 0x%x", last_3);
			break;
		case 0xC:
			SDL_Log("RND v%x, 0x%x", second, last_2);
			break;
		case 0xD:
			SDL_Log("DRW v%x, v%x, 0x%x", second, third, fourth);
			break;
		case 0xE:
			switch (last_2) {
				case 0x9E:
					SDL_Log("SKP v%x", second);
					break;
				case 0xA1:
					SDL_Log("SKNP v%x", second);
					break;
				default:
					SDL_Log("undefined - e");
			}
			break;
		case 0xF:
			switch (last_2) {
				case 0x07:
					SDL_Log("LD v%x, DT", second);
					break;
				case 0x0A:
					SDL_Log("LD v%x, K", second);
					break;
				case 0x15:
					SDL_Log("LD DT, v%x", second);
					break;					
				case 0x18:
					SDL_Log("LD ST, v%x", second);
					break;
				case 0x1E:
					SDL_Log("ADD instr, v%x", second);
					break;
				case 0x29:
					SDL_Log("LD F, v%x", second);
					break;
				case 0x33:
					SDL_Log("LD B, v%x", second);
					break;
				case 0x55:
					SDL_Log("LD [instr], v%x", second);
					break;
				case 0x65:
					SDL_Log("LD v%x, [instr]", second);
					break;
				default:
					SDL_Log("undefined - f");
			}
			break;
		default:
			SDL_Log("undefined");
	}
}


// HELPER FUNCTIONS FOR EXECUTION
// slay all.
void clear_screen(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, bg_color.red, bg_color.green, bg_color.blue, bg_color.alpha);
	SDL_RenderClear(renderer);
}

// turns the bool array in screen[][] to rectangles to be rendered in the window
void update_draw_buffer() {
	// make a bunch of rectangles
	for (int r = 0; r < 32; r++) {
		for (int c = 0; c < 64; c++) {
			if (screen[r][c]) {
				SDL_FRect tester = {.x = c * SCALE, .y = r * SCALE, .w = SCALE, .h = SCALE};
				SDL_RenderFillRect(renderer, &tester);
			}
		}
	}
}

// reads num_rows bytes from memory, starting at address i
// display these rows XOR'd with what's on screen now starting at (start_x, start_y)
// set v[0xF] to 1 if this erases any pixels on screen, else 0
// TODO: rewrite only the area affected by the sprite
void draw_instr(uint8_t x_coord, uint8_t y_coord, uint8_t num_rows) {
	v[0xF] = 0;
	
	// printf("printing %d rows starting at (%d, %d): ", num_rows, x_coord, y_coord);
	
	for (int r = 0; r < num_rows; r++) {
		uint8_t curr_row = reverse_table[mem[i + r]];
		
		// printf("%x, ", curr_row);
		
		for (int c = 0; c < 8; c++) {
			bool screen_pixel = screen[y_coord + r] [x_coord + c];
			bool row_pixel	  = (curr_row & (1 << c)) > 0 ? true : false;
			
			// logic to check if pixel is erased and whether to update v[0xF]
			if (v[0xF] == 0 && screen_pixel == true && row_pixel == true) {
				v[0xF] = 1;
			}
			
			screen[y_coord + r] [x_coord + c] = screen_pixel ^ row_pixel;
		}
	}
	
	// printf("\n");
	
	// for (int r = 0; r < 32; r++) {
		// for (int c = 0; c < 64; c++) {
			// printf("%c", screen[r][c] ? '*' : ' ');
		// }
		// printf("\n");
	// }
	
	// printf("\n");
	
	update_draw_buffer();
}

// EXECUTE STAGE
// executes an instruction
// TODO: reduce overlapping code with print_instruction
void execute_instruction() {
	switch (first){
		case 0x0:
			switch (last_2) {
				case 0xE0:	// clear screen
					clear_screen(renderer);
					SDL_RenderPresent(renderer);
					break;
				case 0xEE:	// return
					if (stack_addr > -1) {
						pc = stack[stack_addr];
						stack[stack_addr] = 0;
						stack_addr--;
					} else {
						SDL_Log("stack underflow error");
					}
					break;
				default:
					SDL_Log("undefined - 0");
			}
			break;
		case 0x1:	// jump
			pc = last_3;
			break;
		case 0x2:	// call
			if (stack_addr < 11) {
				stack_addr++;
				stack[stack_addr] = pc;
				pc = last_3;
			} else {
				SDL_Log("stack overflow error");
			}
			break;
		case 0x3:	// skip-equals immediate
			if (v[second] == last_2) {
				pc += 2;
			}
			break;
		case 0x4:	// skip-not-equals immediate
			if (v[second] != last_2) {
				pc += 2;
			}
			break;
		case 0x5:	// skip-equals register
			switch (fourth) {
				case 0x0:
					if (v[second] == v[third]) {
						pc += 2;
					}
					break;
				default:
					SDL_Log("undefined - 5");
			}
			break;
		case 0x6:	// load immediate
			v[second] = last_2;
			break;
		case 0x7:	// add
			v[second] = v[second] + last_2;
			break;
		case 0x8:	// register-register ops
			switch (fourth) {
				case 0x0:	// load register
					v[second] = v[third];
					break;
				case 0x1:	// or
					v[second] = v[second] | v[third];
					break;
				case 0x2:	// and
					v[second] = v[second] & v[third];
					break;
				case 0x3:	// xor
					v[second] = v[second] ^ v[third];
					break;
				case 0x4:	// add with carry flag in vF
					uint16_t sum = v[second] + v[third];
					v[0xF] = (sum > 0xFF) ? 1 : 0;
					v[second] = (uint8_t) sum;
					break;
				case 0x5:	// subtract with borrow flag in vF
					v[0xF] = (v[second] > v[third]) ? 1 : 0;
					v[second] = v[second] - v[third];
					break;				
				case 0x6:	// shift right with half flag in vF
					v[0xF] = ((v[second] & 0x01) == 1) ? 1 : 0;
					v[second] = v[second] >> 1;
					break;
				case 0x7:	// negated subtraction with borrow flag in vF
					v[0xF] = (v[third] > v[second]) ? 1 : 0;
					v[second] = v[third] - v[second];
					break;
				case 0xE:	// shift left with overflow flag in vF
					v[0xF] = ((v[second] & 0x80) == 0x80) ? 1 : 0;
					v[second] = v[second] << 1;					
					break;
				default:
					SDL_Log("undefined - 8");
			}
			break;
		case 0x9:	// skip-not-equals register
			if (v[second] != v[third]) {
				pc += 2;
			}
			break;
		case 0xA:	// load to index
			i = last_3;
			break;
		case 0xB:	// jump to v0 + last 3 digits of instruction
			pc = v[0] + last_3;
			break;
		case 0xC:	// vx and random byte
			SDL_Log("RND v%x, 0x%x", second, last_2);
			v[second] = (uint8_t) (rand() % 256) & last_2;
			break;
		case 0xD:
			// TODO: use mod here to ensure that sprite is drawn on screen
			draw_instr(v[second] % 64, v[third] % 32, fourth);
			
			// actually draw - do this here to eliminate flickering	
			SDL_SetRenderDrawColor(renderer, fg_color.red, fg_color.green, fg_color.blue, fg_color.alpha);
			SDL_RenderPresent(renderer);
			break;
		case 0xE:
			switch (last_2) {
				case 0x9E:
					SDL_Log("SKP v%x", second);
					break;
				case 0xA1:
					SDL_Log("SKNP v%x", second);
					break;
				default:
					SDL_Log("undefined - e");
			}
			break;
		case 0xF:
			switch (last_2) {
				case 0x07:
					SDL_Log("LD v%x, DT", second);
					break;
				case 0x0A:
					SDL_Log("LD v%x, K", second);
					break;
				case 0x15:
					SDL_Log("LD DT, v%x", second);
					break;					
				case 0x18:
					SDL_Log("LD ST, v%x", second);
					break;
				case 0x1E:
					SDL_Log("ADD i, v%x", second);
					break;
				case 0x29:
					SDL_Log("LD F, v%x", second);
					break;
				case 0x33:
					SDL_Log("LD B, v%x", second);
					break;
				case 0x55:
					SDL_Log("LD [i], v%x", second);
					break;
				case 0x65:
					SDL_Log("LD v%x, [i]", second);
					break;
				default:
					SDL_Log("undefined - f");
			}
			break;
		default:
			SDL_Log("undefined");
	}
}

// INPUT HANDLING
// handle all input to the emulator
bool handle_input() {
	SDL_Event event;
	SDL_zero(event);
	
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
				return false;
			default:
				return true;
		}			
	}
	
	return true;
}

// emulation goes HERE!
int main(int argc, char** argv) {
	// argv -> ['chip-8.exe', rom name, debug, scale, foreground color, background color]
	if (argc > 2 && argc != 6) {
		SDL_Log("usage:   ./chip-8.exe  [rom location]    [debug] [scale factor] [foreground color] [background color]");
		SDL_Log("default: ./chip-8.exe roms/ibm_logo.ch8   false        10            FFFFFFFF           000000FF");
		SDL_Log("takes:   ./chip-8.exe     string          bool      integer       32-bit integer     32-bit integer");
		SDL_Log("color format: 0x[red][green][blue][alpha]\n	> each value is 1 byte\n	> as alpha increases, so does the opacity");
		
		return -1;
	} else if (argc == 6){
		SCALE	 = (uint8_t) strtol(argv[3], NULL, 10);
		debug	 = strcmp("true", argv[2]) == 0 ? true : false;
		
		uint32_t pre_fg = strtoll(argv[4], NULL, 16);
		uint32_t pre_bg = strtoll(argv[5], NULL, 16);
		
		printf("fg: %s, bg: %s\npre_fg: %x, pre_bg: %x\n\n", argv[4], argv[5], pre_fg, pre_bg);
		
		fg_color = (struct color) {(pre_fg & 0xFF000000) >> 24, (pre_fg & 0x00FF0000) >> 16, (pre_fg & 0x0000FF00) >> 8, pre_fg & 0x000000FF};
		bg_color = (struct color) {(pre_bg & 0xFF000000) >> 24, (pre_bg & 0x00FF0000) >> 16, (pre_bg & 0x0000FF00) >> 8, pre_bg & 0x000000FF};
	} else {
		SCALE = 10;
		debug = false;
		
		fg_color = (struct color) {0xFF, 0xFF, 0xFF, 0xFF};
		bg_color = (struct color) {0x00, 0x00, 0x00, 0xFF};
	}
	
	// set seed for random number gen
	srand(time(NULL));
		
	// put fontset into memory
	copy_fonts();
	
	// copy the rom file to memory
	if (!open_file(argv[1])) {
		return -1;
	}
	
	// initialize necessary subsystems, create the window and renderer
	if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) || !SDL_CreateWindowAndRenderer("chip-8 emulator :D", 64 * SCALE, 32 * SCALE, 0, &window, &renderer)) {
		SDL_Log("failed to initialize: %s\n", SDL_GetError());
		return -1;
	}
	
	// tells us where the window/renderer have been initialized in memory
	SDL_Log("window location:   %p\nrenderer location: %p\n\n", window, renderer);
	
	// if window and renderer and texture are created and file is valid, then the emulator can run
	running = true;
	
	// clear screen before beginning
	clear_screen(renderer);
	SDL_RenderPresent(renderer);
	
	// main emulation loop
	while (running) {		
		//fetch
		instr = mem[pc] << 8 | mem[pc + 1];
		pc += 2;
		
		// mask the digits we need in the opcode
		first  = (instr & 0xF000) >> 12;
		second = (instr & 0x0F00) >> 8;
		third  = (instr & 0X00F0) >> 4;
		fourth = (instr & 0X000F);
		last_2 = (instr & 0x00FF);
		last_3 = (instr & 0x0FFF);
		
		// decode instruction
		if (debug) {
			printf("0x%x: %4x | ", pc, instr);
			print_instruction();	
		}
		
		// execute
		execute_instruction();
		
		// handle the input and update running accordingly
		running = handle_input();
	}
	
	// clean up
	end(window, renderer);
	
	// end
	return 0;
}