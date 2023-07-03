#include "chip8.h"
#include <fstream>
#include <iostream>

const int font_start = 0x50;
const int mem_start = 0x200;
const int font_size = 80;
const uint8_t font[font_size] = {
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

Chip8::Chip8() : delay_timer(0), sound_timer(0) {
    PC = mem_start;
    for (auto i = 0; i < font_size; i++) {
        memory[i + font_start] = font[i];
    }
    clear_display();
    clear_keys();
}

bool Chip8::load(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;
    file.seekg(0, std::ios::end);
    int size = file.tellg();
    file.seekg(0, std::ios::beg);
    char* buffer = new char[size];
    file.read(buffer, size);
    for (auto i = 0; i < size; i++) {
        memory[mem_start + i] = buffer[i];
    }
    delete[] buffer;
    return true;
}

void Chip8::emulate_cycle() {
    opcode = memory[PC] << 8 | memory[PC + 1];
    PC += 2;
    execute_op();
    if (delay_timer) delay_timer--;
    if (sound_timer) sound_timer--;
    if (sound_timer) std::cout << "beep!" << std::endl;
}

void Chip8::execute_op() {
    uint8_t nibbles[4];
    nibbles[0] = (opcode & 0xF000) >> 12;
    nibbles[1] = (opcode & 0x0F00) >> 8;
    nibbles[2] = (opcode & 0x00F0) >> 4;
    nibbles[3] = (opcode & 0x000F);
    uint16_t NN = (nibbles[2] << 4) | (nibbles[3]);
    uint16_t NNN = nibbles[1] << 8 | NN;
    switch(nibbles[0]) {
        case 0x0: {
            switch(NNN) {
                case 0x0E0: {
                    for (int y = 0; y < 32; y++) {
                        for (int x = 0; x < 64; x++) {
                            display[y][x] = false;
                        }
                    }
                    break;
                }
                case 0x0EE: {
                    PC = stk.top();
                    stk.pop();
                    break;
                }
            }
            break;
        }
        case 0x1: {
            PC = NNN;
            break;
        }
        case 0x2: {
            stk.push(PC);
            PC = NNN;
            break;
        }
        case 0x3: {
            if (registers[nibbles[1]] == NN) PC += 2;
            break;
        }
        case 0x4: {
            if (registers[nibbles[1]] != NN) PC += 2;
            break;
        }
        case 0x5: {
            if (registers[nibbles[1]] == registers[nibbles[2]]) PC += 2;
            break;
        }
        case 0x6: {
            registers[nibbles[1]] = NN;
            break;
        }
        case 0x7: {
            registers[nibbles[1]] += NN;
            break;
        }
        case 0x8: {
            switch (nibbles[3]) {
                case 0x0: {
                    registers[nibbles[1]] = registers[nibbles[2]];
                    break;
                }
                case 0x1: {
                    registers[nibbles[1]] |= registers[nibbles[2]];
                    break;
                }
                case 0x2: {
                    registers[nibbles[1]] &= registers[nibbles[2]];
                    break;
                }
                case 0x3: {
                    registers[nibbles[1]] ^= registers[nibbles[2]];
                    break;
                }
                case 0x4: {
                    int sum = registers[nibbles[1]] + registers[nibbles[2]];
                    registers[nibbles[1]] = sum%256;
                    if (sum > 255) registers[0xF] = 1;
                    else registers[0xF] = 0;
                    break;
                }
                case 0x5: {
                    int VF = 0;
                    if (registers[nibbles[1]] > registers[nibbles[2]]) VF = 1;
                    registers[nibbles[1]] -= registers[nibbles[2]];
                    registers[0xF] = VF;
                    break;
                }
                case 0x6: {
                    int VF = registers[nibbles[1]] & 1;
                    registers[nibbles[1]] >>= 1;
                    registers[0xF] = VF;
                    break;
                }
                case 0x7: {
                    int VF = 0;
                    if (registers[nibbles[2]] > registers[nibbles[1]]) VF = 1;
                    registers[nibbles[1]] = registers[nibbles[2]] - registers[nibbles[1]];
                    registers[0xF] = VF;
                    break;
                }
                case 0xE: {
                    int VF = ((registers[nibbles[1]] & 0x80) != 0);
                    registers[nibbles[1]] <<= 1;
                    registers[0xF] = VF;
                    break;
                }
            }
            break;
        }
        case 0x9: {
            if (registers[nibbles[1]] != registers[nibbles[2]]) PC += 2;
            break;
        }
        case 0xA: {
            I = NNN;
            break;
        }
        case 0xB: {
            PC = NNN + registers[nibbles[1]];
            break;
        }
        case 0xC: {
            registers[nibbles[1]] = (rand()%256) & NN;
            break;
        }
        case 0xD: {
            uint8_t x = registers[nibbles[1]]%64;
            uint8_t y = registers[nibbles[2]]%32;
            registers[0xF] = 0;
            for (int dy = 0; dy < nibbles[3]; dy++) {
                if (y + dy >= 32) break;
                uint8_t cur_byte = memory[I + dy];
                for (int dx = 0; dx < 8; dx++) {
                    if (x + dx >= 64) break;
                    if (cur_byte & (1 << (7 - dx))) {
                        if (display[y + dy][x + dx]) {
                            registers[0xF] = 1;
                        }
                        display[y + dy][x + dx] ^= true;
                    }
                }
            }
            break;
        }
        case 0xE: {
            switch(NN) {
                case 0x9E: {
                    if (keys[registers[nibbles[1]]]) PC += 2;
                    break;
                }
                case 0xA1: {
                    if (!keys[registers[nibbles[1]]]) PC += 2;
                    break;
                }
            }
            break;
        }
        case 0xF: {
            switch(NN) {
                case 0x07: {
                    registers[nibbles[1]] = delay_timer;
                    break;
                }
                case 0x15: {
                    delay_timer = registers[nibbles[1]];
                    break;
                }
                case 0x18: {
                    sound_timer = registers[nibbles[1]];
                    break;
                }
                case 0x1E: {
                    I += registers[nibbles[1]];
                    break;
                }
                case 0x0A: {
                    PC -= 2;
                    for (auto i = 0; i < 16; i++) {
                        if (keys[i]) {
                            registers[nibbles[1]] = i;
                            PC += 2;
                            break;
                        }
                    }
                    break;
                }
                case 0x29: {
                    I = (font_start + (registers[nibbles[1]] & 0x0F)*5);
                    break;
                }
                case 0x33: {
                    int num = registers[nibbles[1]];
                    for (int i = 0; i < 3; i++) {
                        memory[I + (2 - i)] = num%10;
                        num /= 10;
                    }
                    break;
                }
                case 0x55: {
                    for (int i = 0; i <= nibbles[1]; i++) {
                        memory[I + i] = registers[i];
                    }
                    break;
                }
                case 0x65: {
                    for (int i = 0; i <= nibbles[1]; i++) {
                        registers[i] = memory[I + i];
                    }
                    break;
                }
            }
            break;
        }
    }
}

void Chip8::print() {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (display[y][x]) std::cout << '*';
            else std::cout << '.';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Chip8::print_keys() {
    for (int i = 0;i < 16; i++) std::cout << (int)keys[i] << " ";
    std::cout << std::endl;
}

void Chip8::clear_keys() {
    for (int i = 0; i < 16; i++) keys[i] = false;
}

void Chip8::clear_display() {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            display[y][x] = 0;
        }
    }
}