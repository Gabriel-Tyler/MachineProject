// Gabriel Tyler
// 04/03/22
// Read from a binary file and store intructions in memory allocated on the heap
// The Machine class manages and fetches the instructions from memory

#include <iostream> // ...
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iomanip>

using u8  = std::uint_fast8_t;
using i8  = std::int_fast8_t;
using u32 = std::uint_fast32_t;
using i32 = std::int_fast32_t;
using u64 = std::uint_fast64_t;
using i64 = std::int_fast64_t;

// have alu be a separate class?

namespace Opcodes
{
    enum Categories 
    {
        LOAD, STORE, BRANCH, JALR,
        JAL, OP_IMM, OP, AUIPC, LUI,
        OP_IMM_32, OP_32, SYSTEM,
        UNIMPL
    };
    constexpr Categories OC_MAP[4][8] = {
        // First row (inst[6:5] = 0b00)
        { LOAD, UNIMPL, UNIMPL, UNIMPL, OP_IMM, AUIPC, OP_IMM_32, UNIMPL }, 
        // Second row (inst[6:5] = 0b01)
        { STORE, UNIMPL, UNIMPL, UNIMPL, OP, LUI, OP_32, UNIMPL },
        // Third row (inst[6:5] = 0b10)
        { UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL },
        // Fourth row (inst[6:5] = 0b11)
        { BRANCH, JALR, UNIMPL, JAL, SYSTEM, UNIMPL, UNIMPL, UNIMPL }
    };
}

namespace Alu
{
    enum Commands 
    {
        ALU_ADD, ALU_SUB, ALU_MUL, ALU_DIV,
        ALU_REM, ALU_SLL, ALU_SRL, ALU_SRA,
        ALU_AND, ALU_OR,  ALU_XOR, ALU_NOT
    };
}

class Machine
{
public:
    struct FetchOut 
    {
        u32 instruction;

        // The code below allows us to cout a FetchOut structure.
        // We can use this to debug our code.
        // FetchOut fo = { 0xdeadbeef };
        // cout << fo << '\n';
        friend std::ostream& operator<<(std::ostream& out, const FetchOut& fo);
    };
    struct DecodeOut 
    {

        Opcodes::Categories op;
        u8  rd;
        u8  funct3;
        u8  funct7;
        i64 offset;    // Offsets for BRANCH and STORE
        i64 left_val;  // typically the value of rs1
        i64 right_val; // typically the value of rs2 or immediate

        // same as in FetchOut: cout << do << '\n';
        friend std::ostream& operator<<(std::ostream& out, const Machine::DecodeOut& dec);
    };
    struct ExecuteOut 
    {
        i64 result;
        u8  n, z, c, v;

        friend std::ostream& operator<<(std::ostream& out, const ExecuteOut& eo) 
        {
            std::ostringstream sout;
            sout << "Result: " << eo.result << " [NZCV]: " 
                << (u32)eo.n 
                << (u32)eo.z 
                << (u32)eo.c
                << (u32)eo.v;
            return out << sout.str();
        }
    };

public:
    static constexpr i32 NUM_REGS = 32;
    static constexpr i32 MEM_SIZE = 1 << 18;

private:
    char* _memory;       // The memory.
    i32 _memorySize;     // The size of the memory (should be MEM_SIZE)
    i64 _pc;             // The program counter
    i64 _regs[NUM_REGS]; // The register file

    FetchOut _FO; // Result of the fetch() method
    DecodeOut _DO; // Result of the decode() method
    ExecuteOut _EO; // Result of the execute() method

public:
    Machine(char* mem, i32 size);

    i64 GetPC() const;
    void SetPC(i64 to);

    i64 GetXReg(i32 which) const;
    void SetXReg(i32 which, i64 value);

    void Fetch();
    void Decode();
    void Execute();

    FetchOut& DebugFetchOut();
    DecodeOut& DebugDecodeOut();
    ExecuteOut& DebugExecuteOut();

private:
    // Read from the internal memory
    // Usage:
    // int  myintval  = memory_read<int>(0);  // Read the first 4 bytes
    // char mycharval = memory_read<char>(8); // Read byte index 8
    template <typename T>
    T MemoryRead(i64 address) const;

    // Write to the internal memory
    // Usage:
    // memory_write<int>(0, 0xdeadbeef); // Set bytes 0, 1, 2, 3 to 0xdeadbeef
    // memory_write<char>(8, 0xff);      // Set byte index 8 to 0xff
    template <typename T>
    void MemoryWrite(i64 address, T value);

    i64 SignExtend(u64 value, u32 index);

    void DecodeR();
    void DecodeI();
    void DecodeS();
    void DecodeB();
    void DecodeU();
    void DecodeJ();

    ExecuteOut ALU(Alu::Commands cmd, i64 left, i64 right);
};

std::ostream& operator<<(std::ostream& out, const Machine::FetchOut& fo) 
{
    std::ostringstream sout;
    sout << "0x" << std::hex << std::setfill('0') << std::right << std::setw(8) << fo.instruction;
    return out << sout.str();
}
std::ostream& operator<<(std::ostream& out, const Machine::DecodeOut& dec) 
{
    using namespace Opcodes;

    // We will write this under Testing
    std::ostringstream sout;
    sout << "Operation: ";
    switch (dec.op) 
    {
    case LUI:
        sout << "LUI"; break;
    case AUIPC:
        sout << "AUIPC"; break;
    case LOAD:
        sout << "LOAD"; break;
    case STORE:
        sout << "STORE"; break;
    case OP_IMM:
        sout << "OPIMM"; break;
    case OP_IMM_32:
        sout << "OPIMM32"; break;
    case OP:
        sout << "OP"; break;
    case OP_32:
        sout << "OP32"; break;
    case BRANCH:
        sout << "BRANCH"; break;
    case JALR:
        sout << "JALR"; break;
    case JAL:
        sout << "JAL"; break;
    case SYSTEM:
        sout << "SYSTEM"; break;
    case UNIMPL:
        sout << "NOT-IMPLEMENTED"; break;
    default: 
        sout << "???";
    }
    sout << '\n';
    sout << "RD       : " << (u32)dec.rd << '\n';
    sout << "funct3   : " << (u32)dec.funct3 << '\n';
    sout << "funct7   : " << (u32)dec.funct7 << '\n';
    sout << "offset   : " << dec.offset << '\n';
    sout << "left     : " << dec.left_val << '\n';
    sout << "right    : " << dec.right_val;
    return out << sout.str();
}
std::ostream& operator<<(std::ostream& out, const Machine::ExecuteOut& eo) 
{
    std::ostringstream sout;
    sout << "Result: " << eo.result << " [NZCV]: " 
        << (u32)eo.n 
        << (u32)eo.z 
        << (u32)eo.c
        << (u32)eo.v;
    return out << sout.str();
}

Machine::Machine(char* mem, i32 size)
    : _memory(mem), _memorySize(size), _pc(0ll)
{
    for (i32 i = 0; i < NUM_REGS; ++i)
        _regs[i] = 0ll;
    // set the stack pointer to be at the end of memory
    SetXReg(2, _memorySize);
}

i64 Machine::GetPC() const
{
    return _pc;
}
void Machine::SetPC(i64 to)
{
    _pc = to;
}

i64  Machine::GetXReg(i32 which) const
{
    which &= 0x1f; // Make sure the register number is 0 - 31
    return _regs[which];
}
void Machine::SetXReg(i32 which, i64 value)
{
    which &= 0x1f; // Make sure the register number is 0 - 31
    _regs[which] = value;
}

void Machine::Fetch()
{
    // read the instruction at the program counter memory address
    _FO.instruction = MemoryRead<u32>(_pc);
}
void Machine::Decode() 
{
    using namespace Opcodes;

    u8 OpcodeMapRow = (_FO.instruction >> 5) & 0b11;
    u8 OpcodeMapCol = (_FO.instruction >> 2) & 0b111;
    u8 InstSize     =  _FO.instruction & 0b11;
    if (InstSize != 3) 
    {
        std::cerr << "[DECODE] Invalid instruction (not a 32-bit instruction).\n";
        return;
    }

    _DO.op = OC_MAP[OpcodeMapRow][OpcodeMapCol];
    // Decode the rest of _DO based on the instruction type
    switch (_DO.op)
    {
    case LOAD:
    case JALR:
    case OP_IMM:
    case OP_IMM_32:
    case SYSTEM:
        DecodeI();
        break;
    case STORE:
        DecodeS();
        break;
    case BRANCH:
        DecodeB();
        break;
    case JAL:
        DecodeJ();
        break;
    case AUIPC:
    case LUI:
        DecodeU();
        break;
    case OP:
    case OP_32:
        DecodeR();
        break;
    default:
        std::cerr << "Invalid op type: " << _DO.op << '\n';
        break;
    }
}
void Machine::Execute() 
{
    using namespace Opcodes;
    using namespace Alu;

    Commands cmd;

    // Most instructions will follow left/right
    // but some won't, so we need these:
    i64 op_left  = _DO.left_val;
    i64 op_right = _DO.right_val; 
    
    if (_DO.op == BRANCH) 
    {
        // A branch needs to subtract the operands
        cmd = ALU_SUB;
    }
    else if (_DO.op == LOAD || _DO.op == STORE) 
    {
        // For loads and stores, we need to add the
        // offset with the base register.
        cmd = ALU_ADD;
    }
    else if (_DO.op == OP) 
    {
        // We can't tell which ALU command to use until
        // we read the funct3 and funct7
        switch (_DO.funct3) 
        {
        case 0b000: // ADD or SUB
            if (_DO.funct7 == 0) 
            {
                cmd = ALU_ADD;
            }
            else if (_DO.funct7 == 32) 
            {
                cmd = ALU_SUB;
            }
            break;
        // Finish the rest of the OP functions here.
        }
    }
    else if (_DO.op == OP_32) 
    {
        op_left  = SignExtend(op_left,  31);
        op_right = SignExtend(op_right, 31);
        // You still need to determine the ALU command
    }
    else if (_DO.op == OP_IMM) 
    {
        // Look and see which ALU op needs to be executed.
    }
    else if (_DO.op == OP_IMM_32) 
    { 
        op_left  = SignExtend(op_left,  31);
        op_right = SignExtend(op_right, 31);
        // This is just like OP_IMM except we truncated
        // the left and right ops.
    }
    else if (_DO.op == JALR) 
    {
        // JALR has an offset and a register value that
        // need to be added together.
        cmd = ALU_ADD;
    }
    _EO = ALU(cmd, op_left, op_right);
}

Machine::FetchOut& Machine::DebugFetchOut()
{
    return _FO;
}
Machine::DecodeOut& Machine::DebugDecodeOut()
{
    return _DO;
}
Machine::ExecuteOut& Machine::DebugExecuteOut()
{
    return _EO;
}

template <typename T>
T Machine::MemoryRead(i64 address) const
{
    return *reinterpret_cast<T*>(_memory + address);
}

template <typename T>
void Machine::MemoryWrite(i64 address, T value)
{
    *reinterpret_cast<T*>(_memory + address) = value;
}

i64 Machine::SignExtend(u64 value, u32 index) 
{
    if ((value >> index) & 1) 
    {
        // Sign bit is 1
        return value | (-1ull << index);
    }
    else 
    {
        // Sign bit is 0
        return value & ~(-1ull << index);
    }  
}

void Machine::DecodeR()
{
    _DO.rd     = (_FO.instruction >> 7)  & 0x1f;
    _DO.funct3 = (_FO.instruction >> 12) & 0b111;
    _DO.funct7 = (_FO.instruction >> 25) & 0x3f;
    _DO.offset = 0ll;
    _DO.left_val  = GetXReg(_FO.instruction >> 15);
    _DO.right_val = GetXReg(_FO.instruction >> 20);
}
void Machine::DecodeI()
{
    _DO.rd     = (_FO.instruction >> 7)  & 0x1f;
    _DO.funct3 = (_FO.instruction >> 12) & 0b111;
    _DO.funct7 = 0;
    _DO.offset = 0ll;
    _DO.left_val  = GetXReg(_FO.instruction >> 15);
    _DO.right_val = SignExtend((_FO.instruction >> 20), 11u);
}
void Machine::DecodeS()
{
    _DO.rd     = 0;
    _DO.funct3 = (_FO.instruction >> 12) & 0b111;
    _DO.funct7 = 0;
    _DO.offset = SignExtend(((_FO.instruction >> 7)  & 0x1f) |
                           (((_FO.instruction >> 25) & 0x7f) << 5), 11u);
    _DO.left_val  = GetXReg(_FO.instruction >> 15);
    _DO.right_val = GetXReg(_FO.instruction >> 20);
}
void Machine::DecodeB()
{
    _DO.rd     = 0;
    _DO.funct3 = (_FO.instruction >> 12) & 0b111;
    _DO.funct7 = 0;
    _DO.offset = SignExtend((((_FO.instruction >> 31) & 1) << 12)   |
                            (((_FO.instruction >> 25) & 0x3f) << 5) |
                            (((_FO.instruction >> 8)  & 0xf) << 1)  | 
                            (((_FO.instruction >> 7)  & 1) << 11), 11u);
    _DO.left_val  = GetXReg(_FO.instruction >> 15); // get_xreg truncates for us
    _DO.right_val = GetXReg(_FO.instruction >> 20);
}
void Machine::DecodeU()
{
    _DO.rd     = (_FO.instruction >> 7) & 0x1f;
    _DO.funct3 = 0;
    _DO.funct7 = 0;
    _DO.offset = SignExtend((((_FO.instruction >> 12) & 0xf'ffff) << 12), 31u);
    _DO.left_val  = 0ll;
    _DO.right_val = 0ll;
}
void Machine::DecodeJ()
{
    _DO.rd     = (_FO.instruction >> 7) & 0x1f;
    _DO.funct3 = 0;
    _DO.funct7 = 0;
    _DO.offset = SignExtend((((_FO.instruction >> 31) & 1) << 19) | 
                             ((_FO.instruction >> 21) & 0x3ff)    |
                            (((_FO.instruction >> 20) & 1) << 10) |
                            (((_FO.instruction >> 12) & 0xff) << 11), 19u);
    _DO.left_val  = 0ll;
    _DO.right_val = 0ll;
}

Machine::ExecuteOut Machine::ALU(Alu::Commands cmd, i64 left, i64 right)
{
    using namespace Alu;

    ExecuteOut ret;

    switch (cmd) 
    {
    case ALU_ADD:
        ret.result = left + right;
        break;
    case ALU_SUB:
        ret.result = left - right;
        break;
    case ALU_MUL:
        ret.result = left * right;
        break;
    case ALU_DIV:
        ret.result = left / right;
        break;
    case ALU_REM:
        ret.result = left % right;
        break;
    case ALU_SRL:
        ret.result = static_cast<u64>(left) >> right;
        break;
        // Finish the commands here.
    }

    // Now that we have the result, determine the flags.
    u8 sign_left   = (left  >> 63) & 1;
    u8 sign_right  = (right >> 63) & 1;
    u8 sign_result = (ret.result >> 63) & 1;

    ret.z = !ret.result;
    ret.n = sign_result;
    ret.v = ~sign_left & ~sign_right & sign_result |
             sign_left & sign_right & ~sign_result;
    ret.c = (ret.result > left) || (ret.result > right);

    return ret;
}

int main(int argc, char* argv[])
{
    // check if a file name is provided
    if (argc != 2) 
    {
        std::cerr << "Provide a file name\n";
        return 1;
    }

    // open binary file and check if opened 
    std::ifstream fin(argv[1], std::ios::binary);
    if (!fin.is_open())
    {
        std::cerr << "Could not open " << argv[1] << '\n';
        return 1;
    }
    
    // get size of file by pointing to end and getting position
    fin.seekg(0, fin.end);
    int fileSize = fin.tellg();
    std::cout << "fileSize = " << fileSize << '\n';

    // make sure the file size doesn't exceed the memory size
    if (fileSize > Machine::MEM_SIZE)
    {
        std::cerr << "File is too large\n";
        return 1;
    }

    // each instruction has to be four bytes 
    if (fileSize % 4 != 0)
    {
        std::cerr << argv[1] << " needs a multiple of four bytes\n";
        return 1;
    }

    // go to beginning of file
    fin.seekg(0, fin.beg);

    // read bytes from file into memory
    char* memory = new char[Machine::MEM_SIZE]; 
    fin.read(memory, fileSize);
    fin.close();

    // create the Machine using the allocated memory and debug
    Machine coolMachine(memory, Machine::MEM_SIZE);
    while (coolMachine.GetPC() < fileSize)
    {
        coolMachine.Fetch();
        std::cout << coolMachine.DebugFetchOut() << '\n';
        coolMachine.Decode();
        std::cout << coolMachine.DebugDecodeOut() << '\n';
        coolMachine.Execute();
        std::cout << coolMachine.DebugExecuteOut() << '\n';
        coolMachine.SetPC(coolMachine.GetPC() + 4);
    }

    // cleanup
    delete[] memory;

    return 0;
}