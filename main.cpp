#include <iostream>
#include <iomanip>

#define MEMORY_SIZE 1024
#define REGISTER_COUNT 8
#define NO_REGISTER 0xF

const char* REGISTER_NAMES[REGISTER_COUNT] = { "eax", "ecx", "edx", "ebx",
                                               "esi", "edi", "esp", "ebp" };
const char* STATUS_NAMES[5] = { "???", "AOK", "HLT", "ADR", "INS" };

class to_hex {
  public:
    to_hex(int _value, int _count = 4, bool _prefix = true) :
      value(_value), count(_count), prefix(_prefix) {}
    friend std::ostream& operator<<(std::ostream& os, const to_hex o);
  private:
    int value;  
    int count;
    bool prefix;
};

class State {
  public:
    State();
    void print();
  protected:
    union REGISTERS {
      struct NAMES
      {
        int eax;
        int ecx;
        int edx;
        int ebx;
        int esi;
        int edi;
        int esp;
        int ebp;
      };
      int IDS[8];
    } registers;
    struct CONDITION_CODES {
      bool ZF; // Zero flag
      bool SF; // Sign flag (is negative)
      bool OF; // Overflow flag
    } condition_codes;
    int program_counter;
    enum PROGRAM_STATUS {
      AOK = 1, // Normal operation
      HLT, // halt instruction encountered
      ADR, // Invalid address encountered
      INS // Invalid instruction encountered
    } program_status;
    unsigned char memory[MEMORY_SIZE];
};

inline const char* bool_str(bool v) {
  return v ? "true" : "false";
}

int main(int argc, char* argv[])
{
  std::cout << "Args:" << std::endl;
  for (int i = 0; i < argc; i++)
    std::cout << std::setw(5) << i << ": " << argv[i] << std::endl;
  std::cout << std::endl;

  State state;
  state.print();

  return 0;
}

std::ostream& operator<<(std::ostream& os, const to_hex o) {
  if (o.prefix)
    os << "0x";
  os << std::setfill('0') << std::hex;
  for (int i = 0; i < o.count; i++) {
    int b = (o.value >> (i << 3)) & 0xFF;
    os << std::setw(2) << b;
  }
  os << std::setfill(' ') << std::dec;
  return os;
}

State::State() {
  memset(&this->registers, 0, sizeof(this->registers));
  memset(&this->condition_codes, 0, sizeof(this->condition_codes));
  memset(&this->program_counter, 0, sizeof(this->program_counter));
  this->program_status = AOK;
  memset(this->memory, 0, sizeof(this->memory));
}

void State::print() {
  std::cout << "Registers:" << std::endl;
  for (int i = 0; i < REGISTER_COUNT / 4; i++) {
    for (int j = i * 4; j < (i + 1) * 4 && j < REGISTER_COUNT; j++)
      std::cout << "  " << REGISTER_NAMES[j] << ": "
                << to_hex(this->registers.IDS[j]);
    std::cout << std::endl;
  }
  
  std::cout << "Condition Codes:" << std::endl
            << "  ZF:  " << std::setw(10) << bool_str(this->condition_codes.ZF)
            << "  SF:  " << std::setw(10) << bool_str(this->condition_codes.SF)
            << "  OF:  " << std::setw(10) << bool_str(this->condition_codes.OF)
            << std::endl;

  std::cout << "Program Counter:" << std::endl
            << "  PC:  " << to_hex(this->program_counter)
            << "  Mem:";
  for (int i = 0; i < 6; i++) {
    int index = this->program_counter + i;
    bool valid = index >= 0 && index < MEMORY_SIZE;
    std::cout << " " << to_hex(valid ? this->memory[index] : 0, 1, false);
  }
  std::cout << std::endl;

  std::cout << "Program Status:" << std::endl;
  {
    bool invalid = this->program_status < AOK || this->program_status > INS;
    int index = invalid ? 0 : this->program_status;
    std::cout << "  STR: " << std::setw(10) << STATUS_NAMES[index]
              << "  VAL: " << std::setw(10) << this->program_status
              << std::endl;
  }
}
