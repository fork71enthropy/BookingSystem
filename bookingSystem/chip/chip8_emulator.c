#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE_FACTOR 10
#define MEMORY_SIZE 4096
#define PROGRAM_START 0x200
#define FONTSET_SIZE 80

// Police de caractères CHIP-8 (0-F)
uint8_t fontset[FONTSET_SIZE] = {
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

// Mapping des touches CHIP-8 vers SDL
SDL_Keycode keymap[16] = {
    SDLK_x,    // 0
    SDLK_1,    // 1
    SDLK_2,    // 2
    SDLK_3,    // 3
    SDLK_q,    // 4
    SDLK_w,    // 5
    SDLK_e,    // 6
    SDLK_a,    // 7
    SDLK_s,    // 8
    SDLK_d,    // 9
    SDLK_z,    // A
    SDLK_c,    // B
    SDLK_4,    // C
    SDLK_r,    // D
    SDLK_f,    // E
    SDLK_v     // F
};

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[16];              // Registres V0-VF
    uint16_t I;                 // Registre d'index
    uint16_t pc;                // Compteur de programme
    uint8_t sp;                 // Pointeur de pile
    uint16_t stack[16];         // Pile
    uint8_t delay_timer;        // Timer de délai
    uint8_t sound_timer;        // Timer de son
    uint8_t display[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint8_t keypad[16];         // État des touches
    int draw_flag;              // Flag pour redessiner l'écran
} Chip8;

void chip8_init(Chip8 *chip8) {
    chip8->pc = PROGRAM_START;
    chip8->I = 0;
    chip8->sp = 0;
    
    memset(chip8->memory, 0, MEMORY_SIZE);
    memset(chip8->V, 0, 16);
    memset(chip8->stack, 0, 32);
    memset(chip8->display, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    memset(chip8->keypad, 0, 16);
    
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->draw_flag = 0;
    
    // Charger le fontset en mémoire
    memcpy(chip8->memory, fontset, FONTSET_SIZE);
    
    srand(time(NULL));
}

int chip8_load_rom(Chip8 *chip8, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur: impossible d'ouvrir le fichier %s\n", filename);
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    
    if (size > (MEMORY_SIZE - PROGRAM_START)) {
        printf("Erreur: le ROM est trop grand\n");
        fclose(file);
        return 0;
    }
    
    size_t result = fread(chip8->memory + PROGRAM_START, 1, size, file);
    if (result != size) {
        printf("Erreur: lecture du fichier\n");
        fclose(file);
        return 0;
    }
    
    fclose(file);
    printf("ROM chargé: %ld octets\n", size);
    return 1;
}

void chip8_cycle(Chip8 *chip8) {
    // Récupérer l'opcode
    uint16_t opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];
    
    // Décoder et exécuter l'opcode
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n = opcode & 0x000F;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = opcode & 0x00FF;
    
    chip8->pc += 2;
    
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: // CLS - Clear display
                    memset(chip8->display, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
                    chip8->draw_flag = 1;
                    break;
                case 0x00EE: // RET - Return from subroutine
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    break;
            }
            break;
            
        case 0x1000: // JP addr - Jump to location nnn
            chip8->pc = nnn;
            break;
            
        case 0x2000: // CALL addr - Call subroutine at nnn
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = nnn;
            break;
            
        case 0x3000: // SE Vx, byte - Skip next instruction if Vx = kk
            if (chip8->V[x] == kk)
                chip8->pc += 2;
            break;
            
        case 0x4000: // SNE Vx, byte - Skip next instruction if Vx != kk
            if (chip8->V[x] != kk)
                chip8->pc += 2;
            break;
            
        case 0x5000: // SE Vx, Vy - Skip next instruction if Vx = Vy
            if (chip8->V[x] == chip8->V[y])
                chip8->pc += 2;
            break;
            
        case 0x6000: // LD Vx, byte - Set Vx = kk
            chip8->V[x] = kk;
            break;
            
        case 0x7000: // ADD Vx, byte - Set Vx = Vx + kk
            chip8->V[x] += kk;
            break;
            
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: // LD Vx, Vy
                    chip8->V[x] = chip8->V[y];
                    break;
                case 0x0001: // OR Vx, Vy
                    chip8->V[x] |= chip8->V[y];
                    break;
                case 0x0002: // AND Vx, Vy
                    chip8->V[x] &= chip8->V[y];
                    break;
                case 0x0003: // XOR Vx, Vy
                    chip8->V[x] ^= chip8->V[y];
                    break;
                case 0x0004: // ADD Vx, Vy
                    {
                        uint16_t sum = chip8->V[x] + chip8->V[y];
                        chip8->V[0xF] = (sum > 255) ? 1 : 0;
                        chip8->V[x] = sum & 0xFF;
                    }
                    break;
                case 0x0005: // SUB Vx, Vy
                    chip8->V[0xF] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;
                    chip8->V[x] -= chip8->V[y];
                    break;
                case 0x0006: // SHR Vx
                    chip8->V[0xF] = chip8->V[x] & 0x1;
                    chip8->V[x] >>= 1;
                    break;
                case 0x0007: // SUBN Vx, Vy
                    chip8->V[0xF] = (chip8->V[y] > chip8->V[x]) ? 1 : 0;
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    break;
                case 0x000E: // SHL Vx
                    chip8->V[0xF] = (chip8->V[x] & 0x80) >> 7;
                    chip8->V[x] <<= 1;
                    break;
            }
            break;
            
        case 0x9000: // SNE Vx, Vy - Skip next instruction if Vx != Vy
            if (chip8->V[x] != chip8->V[y])
                chip8->pc += 2;
            break;
            
        case 0xA000: // LD I, addr - Set I = nnn
            chip8->I = nnn;
            break;
            
        case 0xB000: // JP V0, addr - Jump to location nnn + V0
            chip8->pc = nnn + chip8->V[0];
            break;
            
        case 0xC000: // RND Vx, byte - Set Vx = random byte AND kk
            chip8->V[x] = (rand() % 256) & kk;
            break;
            
        case 0xD000: // DRW Vx, Vy, nibble - Display n-byte sprite
            {
                uint8_t xPos = chip8->V[x] % SCREEN_WIDTH;
                uint8_t yPos = chip8->V[y] % SCREEN_HEIGHT;
                chip8->V[0xF] = 0;
                
                for (int row = 0; row < n; row++) {
                    uint8_t sprite_byte = chip8->memory[chip8->I + row];
                    for (int col = 0; col < 8; col++) {
                        if ((sprite_byte & (0x80 >> col)) != 0) {
                            int x_coord = (xPos + col) % SCREEN_WIDTH;
                            int y_coord = (yPos + row) % SCREEN_HEIGHT;
                            int pixel_index = y_coord * SCREEN_WIDTH + x_coord;
                            
                            if (chip8->display[pixel_index] == 1)
                                chip8->V[0xF] = 1;
                            chip8->display[pixel_index] ^= 1;
                        }
                    }
                }
                chip8->draw_flag = 1;
            }
            break;
            
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E: // SKP Vx - Skip next instruction if key Vx is pressed
                    if (chip8->keypad[chip8->V[x]])
                        chip8->pc += 2;
                    break;
                case 0x00A1: // SKNP Vx - Skip next instruction if key Vx is not pressed
                    if (!chip8->keypad[chip8->V[x]])
                        chip8->pc += 2;
                    break;
            }
            break;
            
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: // LD Vx, DT - Set Vx = delay timer
                    chip8->V[x] = chip8->delay_timer;
                    break;
                case 0x000A: // LD Vx, K - Wait for key press
                    {
                        int key_pressed = 0;
                        for (int i = 0; i < 16; i++) {
                            if (chip8->keypad[i]) {
                                chip8->V[x] = i;
                                key_pressed = 1;
                                break;
                            }
                        }
                        if (!key_pressed)
                            chip8->pc -= 2;
                    }
                    break;
                case 0x0015: // LD DT, Vx - Set delay timer = Vx
                    chip8->delay_timer = chip8->V[x];
                    break;
                case 0x0018: // LD ST, Vx - Set sound timer = Vx
                    chip8->sound_timer = chip8->V[x];
                    break;
                case 0x001E: // ADD I, Vx - Set I = I + Vx
                    chip8->I += chip8->V[x];
                    break;
                case 0x0029: // LD F, Vx - Set I = location of sprite for digit Vx
                    chip8->I = chip8->V[x] * 5;
                    break;
                case 0x0033: // LD B, Vx - Store BCD representation of Vx
                    chip8->memory[chip8->I] = chip8->V[x] / 100;
                    chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
                    chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
                    break;
                case 0x0055: // LD [I], Vx - Store registers V0 through Vx in memory
                    for (int i = 0; i <= x; i++)
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    break;
                case 0x0065: // LD Vx, [I] - Read registers V0 through Vx from memory
                    for (int i = 0; i <= x; i++)
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    break;
            }
            break;
            
        default:
            printf("Opcode inconnu: 0x%04X\n", opcode);
            break;
    }
    
    // Mettre à jour les timers
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;
    
    if (chip8->sound_timer > 0) {
        if (chip8->sound_timer == 1)
            printf("\a"); // Beep
        chip8->sound_timer--;
    }
}

void draw_screen(SDL_Renderer *renderer, Chip8 *chip8) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (chip8->display[y * SCREEN_WIDTH + x]) {
                SDL_Rect rect = {
                    x * SCALE_FACTOR,
                    y * SCALE_FACTOR,
                    SCALE_FACTOR,
                    SCALE_FACTOR
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    
    SDL_RenderPresent(renderer);
    chip8->draw_flag = 0;
}

void handle_input(Chip8 *chip8, SDL_Event *event, int *running) {
    if (event->type == SDL_QUIT) {
        *running = 0;
    } else if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        int key_state = (event->type == SDL_KEYDOWN) ? 1 : 0;
        
        for (int i = 0; i < 16; i++) {
            if (event->key.keysym.sym == keymap[i]) {
                chip8->keypad[i] = key_state;
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <rom_file>\n", argv[0]);
        printf("\nClavier CHIP-8:\n");
        printf("1 2 3 C    →    1 2 3 4\n");
        printf("4 5 6 D    →    Q W E R\n");
        printf("7 8 9 E    →    A S D F\n");
        printf("A 0 B F    →    Z X C V\n");
        return 1;
    }
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCALE_FACTOR,
        SCREEN_HEIGHT * SCALE_FACTOR,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        printf("Erreur création fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur création renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    Chip8 chip8;
    chip8_init(&chip8);
    
    if (!chip8_load_rom(&chip8, argv[1])) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    int running = 1;
    SDL_Event event;
    
    printf("Émulateur CHIP-8 démarré!\n");
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            handle_input(&chip8, &event, &running);
        }
        
        // Exécuter plusieurs cycles par frame pour une meilleure vitesse
        for (int i = 0; i < 10; i++) {
            chip8_cycle(&chip8);
        }
        
        if (chip8.draw_flag) {
            draw_screen(renderer, &chip8);
        }
        
        SDL_Delay(16); // ~60 FPS
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
