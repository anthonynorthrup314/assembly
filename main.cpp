#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <regex>
#include <algorithm>
#include <stdexcept>

#define MEMORY_SIZE 1024
#define REGISTER_COUNT 8
#define NO_REGISTER 0xF

#define INDEX(v) if((v) >= MEMORY_SIZE) throw ""

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

class Result {
  public:
    Result() : was_error(false), line(0), problem(""), value("") {}
    void set(std::string _problem, std::string _value);
    template <typename T> void set(std::string _problem, T _value);
    void warn();
    void error();
  protected:
    void info();

  public:
    bool was_error;
    int line;
    std::string problem;
    std::string value;
};

class State {
  public:
    State();
    void print();
    void print_memory(int lines);
    Result compile(std::vector<std::string> source, bool as_errors = false);
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

bool valid_label(std::string label);
std::string to_lower(std::string s);
bool valid_command(std::string command);

int main(int argc, char* argv[])
{
  // Convert arguments to vector of strings
  std::vector<std::string> args(argv, argv + argc);

  // No source file specified?
  if (args.size() <= 1) {
    std::cout << "Usage: main <source-file>" << std::endl;
    return 0;
  }

  // Get source filename
  std::string source_filename = args[1];

  // Read source
  std::ifstream source_file(source_filename);
  
  // Invalid file?
  if (!source_file) {
    std::cout << "Unable to read: " << source_filename << std::endl;
    return 0;
  }

  // Convert to vector
  std::vector<std::string> source;
  std::string line;
  while (std::getline(source_file, line))
    source.push_back(line);
  
  // Compile code
  State state;
  Result res = state.compile(source);

  // Error compiling?
  if (res.was_error) {
    std::cout << "Error Compiling:" << std::endl;
    res.error();
    return 0;
  }

  // Debug print some memory
  state.print_memory(4);

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

void Result::set(std::string _problem, std::string _value) {
  this->problem.assign(_problem);
  this->value.assign(_value);
}

template <typename T> void Result::set(std::string _problem, T _value) {
  this->set(_problem, std::to_string(_value));
}

void Result::warn() {
  std::cout << "Warning: ";
  this->info();
}

void Result::error() {
  std::cout << "Error: ";
  this->info();
}

void Result::info() {
  std::cout << "Line " << this->line << ": " << this->problem << " ("
            << this->value << ")" << std::endl;
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

void State::print_memory(int lines) {
  std::cout << "Memory" << std::endl;
  for (int i = 0; i < lines; i++) {
    for (int j = 0; j < 8; j++) {
      std::cout << "  ";
      for (int k = 0; k < 4; k++)
        std::cout << to_hex(this->memory[i * 32 + j * 4 + k], 1, false);
    }
    std::cout << std::endl;
  }
}

Result State::compile(std::vector<std::string> source, bool as_errors) {
  Result result;
  result.was_error = true;

  int pos = 0;
  std::map<std::string, int> labels;
  std::string last_label("");
  std::map<int, std::pair<int, std::string> > put_labels;

  for (int i = 0; i < source.size(); i++) {
    std::string line = source[i];
    size_t index = 0;
    result.line = i;

    // Trim left
    index = line.find_first_not_of(" \t\n\r");
    line.erase(0, index);

    // Trim right
    index = line.find_last_not_of(" \t\n\r");
    line.erase(index + 1);

    // Remove comments
    index = line.find(';');
    if (index != std::string::npos)
      line.erase(index);
    
    // Ignore blank lines
    if (line.size() == 0)
      continue;
    
    // Macro?
    if (line.at(0) == '.') {
      // Remove '.'
      line.erase(0, 1);

      // Split into command and arguments
      std::string args("");
      std::string arg_type("number");
      index = line.find(' ');
      if (index != std::string::npos) {
        args.assign(line.substr(index + 1));
        line.erase(index);
      }

      try {
        if (line == "pos") {
          pos = std::stoi(args, nullptr, 0);
        } else if (line == "align") {
          int alignment = std::stoi(args, nullptr, 0);

          // Valid alignment?
          if (alignment > 1) {
            // Round up to nearest block
            if (pos % alignment != 0)
              pos = pos - (pos % alignment) + alignment;
          } else {
            result.set("Invalid alignment", alignment);
            return result;
          }
        } else if (line == "long") {
          unsigned int value = std::stoi(args, nullptr, 0);

          // Overflow?
          if (pos + 4 >= MEMORY_SIZE) {
            result.set("Not enough memory for long value", pos);
            return result;
          }

          // Set in memory
          for (int i = 0; i < 4; i++) {
            this->memory[pos++] = value & 0xFF;
            value >>= 8;
          }
        } else {
          result.set("Unknown macro", line);
          if (as_errors)
            return result;
          result.warn();
        }
      } catch (const std::invalid_argument& e) {
        result.set("Macro '" + line + "' requires a " + arg_type, args);
        return result;
      }

      continue;
    }

    // Label?
    if (line.at(line.size() - 1) == ':') {
      // Remove colon
      line.erase(line.size() - 1);

      // Valid label
      bool valid = false;

      // Local label?
      std::string full_label = line;
      if (line.at(0) == '@') {
        full_label.assign(last_label + line);
        valid = valid_label(line.substr(1));
      } else {
        last_label = line;
        valid = valid_label(line);
      }

      // Invalid characters?
      if (!valid) {
        result.set("Label contains invalid characters", full_label);
        return result;
      }

      // Already declared?
      if (labels.find(full_label) != labels.end()) {
        result.set("Label already defined", full_label);
        return result;
      }

      // Store in map
      labels.insert(std::pair<std::string, int>(full_label, pos));

      continue;
    }

    // Split into command and optional args
    index = line.find_first_of(" \t");
    std::string command = to_lower(line.substr(0, index));
    if (index != std::string::npos)
      index = line.find_first_not_of(" \t", index + 1);
    bool had_args = index != std::string::npos;
    std::string args = had_args ? line.substr(index) : "";

    // Valid command?
    if (!valid_command(command)) {
      result.set("Invalid command name", command);
      return result;
    }

    // Compile command
    int command_size = 6;
    try {
      if (command == "halt") {
        command_size = 1; INDEX(pos + command_size);
        this->memory[pos++] = 0x00;
      } else if (command == "nop") {
        command_size = 1; INDEX(pos + command_size);
        this->memory[pos++] = 0x10;
      } else if (command == "rrmovl") {
        command_size = 2; INDEX(pos + command_size);
        this->memory[pos++] = 0x20;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos++;
      } else if (command == "irmovl") {
        command_size = 6; INDEX(pos + command_size);
        this->memory[pos++] = 0x30;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos += 5;
      } else if (command == "rmmovl") {
        command_size = 6; INDEX(pos + command_size);
        this->memory[pos++] = 0x40;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos += 5;
      } else if (command == "mrmovl") {
        command_size = 6; INDEX(pos + command_size);
        this->memory[pos++] = 0x50;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos += 5;
      } else if (command == "addl" || command == "subl"
                 || command == "andl" || command == "xorl") {
        command_size = 2; INDEX(pos + command_size);
        this->memory[pos++] = 0x60;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos++;
      } else if (command == "jmp" || command == "jle"
                 || command == "jl" || command == "je"
                 || command == "jne" || command == "jge"
                 || command == "jg") {
        command_size = 5; INDEX(pos + command_size);
        this->memory[pos++] = 0x70;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos += 4;
      } else if (command == "cmovle" || command == "cmovl"
                 || command == "cmove" || command == "cmovne"
                 || command == "cmovge" || command == "cmovg") {
        command_size = 2; INDEX(pos + command_size);
        this->memory[pos++] = 0x20;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos++;
      } else if (command == "call") {
        command_size = 2; INDEX(pos + command_size);
        this->memory[pos++] = 0x80;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos++;
      } else if (command == "ret") {
        command_size = 1; INDEX(pos + command_size);
        this->memory[pos++] = 0x90;
      } else if (command == "pushl") {
        command_size = 2; INDEX(pos + command_size);
        this->memory[pos++] = 0xA0;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos++;
      } else if (command == "popl") {
        command_size = 2; INDEX(pos + command_size);
        this->memory[pos++] = 0xB0;
        // TODO verify arguments and handle them
        std::cout << command;
        if (had_args) std::cout << " (" << args << ")";
        std::cout << std::endl;
        pos++;
      } else {
        result.set("Invalid or unimplemented command", command);
        return result;
      }
    } catch(const char* e) {
      result.set("Not enough memory for " + command + ", space left: "
                 + std::to_string(MEMORY_SIZE - pos), command_size);
      return result;
    }
  }

  result.was_error = false;
  return result;
}

bool valid_label(std::string label) {
  std::regex pattern("^[a-zA-Z][a-zA-Z0-9_]*$");
  return std::regex_match(label, pattern);
}

std::string to_lower(std::string s) {
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

bool valid_command(std::string command) {
  std::regex pattern("^[a-zA-Z]+$");
  return std::regex_match(command, pattern);
}
