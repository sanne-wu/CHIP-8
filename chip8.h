#include <stdint.h>
#include <stack>
#include <string>
class Chip8 {
    uint16_t opcode;
    uint8_t memory[4096];
    uint8_t registers[16];
    std::stack<uint16_t> stk;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t I;
    uint16_t PC;
    public:
        bool display[32][64];
        bool keys[16];
        Chip8();
        bool load(const std::string& filename);
        void emulate_cycle();
        void execute_op();
        void print();
        void clear_display();
        void clear_keys();
        void print_keys();
};