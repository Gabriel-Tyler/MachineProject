// Gabriel Tyler
// 04/03/22
// Read from a binary file and store intructions in memory allocated on the heap
// The Machine class manages and fetches the instructions from memory

#include <iostream> // ...
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iomanip>

// fix the decoding, offset is calculated wrong and values are in the wrong place

using u8  = std::uint_fast8_t;
using i8  = std::int_fast8_t;
using u32 = std::uint_fast32_t;
using i32 = std::int_fast32_t;
using i64 = std::int_fast64_t;

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

class Machine
{
public:
    struct FetchOut 
    {
        uint32_t instruction;

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

        friend std::ostream& operator<<(std::ostream& out, const Machine::DecodeOut& dec);
    };

public:
    static constexpr i32 NUM_REGS = 32;
    static constexpr i32 MEM_SIZE = 1 << 18;

private:
    char* _memory;           // The memory.
    int _memorySize;         // The size of the memory (should be MEM_SIZE)
    i64 _pc;             // The program counter
    i64 _regs[NUM_REGS]; // The register file

    FetchOut  _fo; // Result of the fetch() method.
    DecodeOut _do; // Result of the decode() method

public:
    Machine(char* mem, int size);

    i64 GetPC() const;
    void SetPC(i64 to);

    i64 GetXReg(i32 which) const;
    void SetXReg(i32 which, i64 value);

    void Fetch();
    void Decode();
    FetchOut& DebugFetchOut();
    DecodeOut& DebugDecodeOut();

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

    i64 SignExtend(u32 value, u32 index);

    void DecodeR();
    void DecodeI();
    void DecodeS();
    void DecodeB();
    void DecodeU();
    void DecodeJ();
};

std::ostream& operator<<(std::ostream& out, const Machine::FetchOut&  fo) 
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
        sout << "LUI";
        break;
    case AUIPC:
        sout << "AUIPC";
        break;
    case LOAD:
        sout << "LOAD";
        break;
    case STORE:
        sout << "STORE";
        break;
    case OP_IMM:
        sout << "OPIMM";
        break;
    case OP_IMM_32:
        sout << "OPIMM32";
        break;
    case OP:
        sout << "OP";
        break;
    case OP_32:
        sout << "OP32";
        break;
    case BRANCH:
        sout << "BRANCH";
        break;
    case JALR:
        sout << "JALR";
        break;
    case JAL:
        sout << "JAL";
        break;
    case SYSTEM:
        sout << "SYSTEM";
        break;
    case UNIMPL:
        sout << "NOT-IMPLEMENTED";
        break;
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

Machine::Machine(char* mem, int size)
    : _memory(mem), _memorySize(size), _pc(0)
{
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
    _fo.instruction = MemoryRead<uint32_t>(_pc);
}
void Machine::Decode() 
{
    using namespace Opcodes;

    u8 OpcodeMapRow = (_fo.instruction >> 5) & 0b11;
    u8 OpcodeMapCol = (_fo.instruction >> 2) & 0b111;
    u8 InstSize     =  _fo.instruction & 0b11;
    if (InstSize != 3) 
    {
        std::cerr << "[DECODE] Invalid instruction (not a 32-bit instruction).\n";
        return;
    }

    _do.op = OC_MAP[OpcodeMapRow][OpcodeMapCol];
    // Decode the rest of _do based on the instruction type
    switch (_do.op)
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
        std::cerr << "Invalid op type: " << _do.op << '\n';
        break;
    }
}
Machine::FetchOut&  Machine::DebugFetchOut()
{
    return _fo;
}
Machine::DecodeOut& Machine::DebugDecodeOut()
{
    return _do;
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

i64 Machine::SignExtend(u32 value, u32 index) 
{
    if ((value >> index) & 1) 
    {
        // Sign bit is 1
        return value | (-1u << index);
    }
    else 
    {
        // Sign bit is 0
        return value & ~(-1u << index);
    }  
}

void Machine::DecodeR()
{
    _do.rd     = (_fo.instruction >> 7)  & 0x1f;
    _do.funct3 = (_fo.instruction >> 12) & 7;
    _do.funct7 = (_fo.instruction >> 25) & 0x3f;
    _do.offset = 0ll;
    _do.left_val  = GetXReg(_fo.instruction >> 15);
    _do.right_val = GetXReg(_fo.instruction >> 20);
}
void Machine::DecodeI()
{
    _do.rd     = (_fo.instruction >> 7)  & 0x1f;
    _do.funct3 = (_fo.instruction >> 12) & 7;
    _do.funct7 = 0;
    _do.offset = SignExtend((_fo.instruction >> 20), 11u);
    _do.left_val  = GetXReg(_fo.instruction >> 15);
    _do.right_val = 0ll;
}
void Machine::DecodeS()
{
    _do.rd     = 0;
    _do.funct3 = (_fo.instruction >> 12) & 7;
    _do.funct7 = 0;
    _do.offset = SignExtend(((_fo.instruction >> 7)  & 0x1f) |
                           (((_fo.instruction >> 25) & 0x7f) << 5), 11u);
    _do.left_val  = GetXReg(_fo.instruction >> 15);
    _do.right_val = GetXReg(_fo.instruction >> 20);
}
void Machine::DecodeB()
{
    _do.rd     = 0;
    _do.funct3 = (_fo.instruction >> 12) & 0b111;
    _do.funct7 = 0;
    _do.offset = SignExtend((((_fo.instruction >> 31) & 1) << 12)   |
                            (((_fo.instruction >> 25) & 0x3f) << 5) |
                            (((_fo.instruction >> 8)  & 0xf) << 1)  | 
                            (((_fo.instruction >> 7)  & 1) << 11), 11u);
    _do.left_val  = GetXReg(_fo.instruction >> 15); // get_xreg truncates for us
    _do.right_val = GetXReg(_fo.instruction >> 20);
}
void Machine::DecodeU()
{
    _do.rd     = (_fo.instruction >> 7) & 0x1f;
    _do.funct3 = 0;
    _do.funct7 = 0;
    _do.offset = SignExtend((((_fo.instruction >> 12) & 0xf'ffff) << 12), 31u);
    _do.left_val  = 0ll;
    _do.right_val = 0ll;
}
void Machine::DecodeJ()
{
    _do.rd     = (_fo.instruction >> 7) & 0x1f;
    _do.funct3 = 0;
    _do.funct7 = 0;
    _do.offset = SignExtend((((_fo.instruction >> 31) & 1) << 19) | 
                             ((_fo.instruction >> 21) & 0x3ff)    |
                            (((_fo.instruction >> 20) & 1) << 10) |
                            (((_fo.instruction >> 12) & 0xff) << 11), 19u);
    _do.left_val  = 0ll;
    _do.right_val = 0ll;
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
        coolMachine.SetPC(coolMachine.GetPC() + 4);
    }

    // cleanup
    delete[] memory;

    return 0;
}