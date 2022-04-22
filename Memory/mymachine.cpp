// Gabriel Tyler
// 04/20/22
// Machine Project: Memory
// Read from a binary file and store intructions in memory allocated on the heap
// The Machine class goes through the instruction pipeline based on the stored instructions

#include <cstdint> // [u]int_leastN_t
#include <fstream> // ifstream
#include <iomanip> 
#include <iostream> 
#include <sstream> // ostringstream

using u8  = std::uint_least8_t;
using i8  = std:: int_least8_t;
using u16 = std::uint_least16_t;
using i16 = std:: int_least16_t;
using u32 = std::uint_least32_t;
using i32 = std:: int_least32_t;
using u64 = std::uint_least64_t;
using i64 = std:: int_least64_t;

class Machine
{
public:
    enum Opcodes 
    {
        LOAD, STORE, BRANCH, JALR,
        JAL, OP_IMM, OP, AUIPC, LUI,
        OP_IMM_32, OP_32, SYSTEM,
        UNIMPL
    };
    enum Alu 
    {
        ADD, SUB, MUL, DIV,
        REM, SLL, SRL, SRA,
        AND, OR,  XOR, NOT,
        NO_OP
    };

    struct FetchOut 
    {
        u32 instruction;

        // usage: std::cout << DebugFetchOut();
        friend std::ostream& operator<<(std::ostream& out, const FetchOut& fo);
    };
    struct DecodeOut 
    {
        Opcodes op;
        u8  rd;
        u8  funct3;
        u8  funct7;
        i64 offset;   // Offsets for BRANCH and STORE
        i64 leftVal;  // typically the value of rs1
        i64 rightVal; // typically the value of rs2 or immediate

        // usage: std::cout << DebugDecodeOut();
        friend std::ostream& operator<<(std::ostream& out, const DecodeOut& dec);
    };
    struct ExecuteOut 
    {
        i64 result;
        u8  n, z, c, v;

        // usage: std::cout << DebugExecuteOut();
        friend std::ostream& operator<<(std::ostream& out, const ExecuteOut& eo);
    };
    struct MemoryOut 
    {
        i64 value;

        friend std::ostream& operator<<(std::ostream& out, const MemoryOut& mo); 
    };

    Machine(char* mem, i64 size);

    // get and set the program counter
    i64 GetPC() const;
    void SetPC(i64 to);

    // get and set an integer register
    i64 GetXReg(i32 which) const;
    void SetXReg(i32 which, i64 value);

    // public pipeline functions
    void Fetch();
    void Decode();
    void Execute();
    void Memory();

    // return different stages of the pipeline for debugging
    FetchOut& DebugFetchOut();
    DecodeOut& DebugDecodeOut();
    ExecuteOut& DebugExecuteOut();
    MemoryOut& DebugMemoryOut();

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

    // sign extend a value with sign bit at index
    i64 SignExtend(u64 value, u32 index) const;

    // decode different instruction types
    void DecodeR();
    void DecodeI();
    void DecodeS();
    void DecodeB();
    void DecodeU();
    void DecodeJ();

    // perform an operation in the alu
    ExecuteOut ALU(Alu cmd, i64 left, i64 right) const;

    static const Opcodes OC_MAP[4][8]; // defined outside of class
    static const i32 NUM_REGS = 32; // 32 registers

    char* _memory;       // The memory
    i64 _memorySize;     // The size of the memory (should be MEM_SIZE)
    i64 _pc;             // The program counter
    i64 _regs[NUM_REGS]; // The register file

    FetchOut _FO; // Result of the fetch() method
    DecodeOut _DO; // Result of the decode() method
    ExecuteOut _EO; // Result of the execute() method
    MemoryOut _MO;
};

// Because OpcodeMap is a static MD array, it has to be defined outside of the class for some reason
const Machine::Opcodes Machine::OC_MAP[4][8] = {
    // First row (inst[6:5] = 0b00)
    { LOAD, UNIMPL, UNIMPL, UNIMPL, OP_IMM, AUIPC, OP_IMM_32, UNIMPL }, 
    // Second row (inst[6:5] = 0b01)
    { STORE, UNIMPL, UNIMPL, UNIMPL, OP, LUI, OP_32, UNIMPL },
    // Third row (inst[6:5] = 0b10)
    { UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL },
    // Fourth row (inst[6:5] = 0b11)
    { BRANCH, JALR, UNIMPL, JAL, SYSTEM, UNIMPL, UNIMPL, UNIMPL }
};

std::ostream& operator<<(std::ostream& out, const Machine::FetchOut& fo) 
{
    std::ostringstream sout;
    sout << "0x" << std::hex << std::setfill('0') << std::right << std::setw(8) << fo.instruction;
    return out << sout.str();
}
std::ostream& operator<<(std::ostream& out, const Machine::DecodeOut& dec) 
{
    // We will write this under Testing
    std::ostringstream sout;
    sout << "Operation: ";
    switch (dec.op) 
    {
    case Machine::LUI:
        sout << "LUI"; break;
    case Machine::AUIPC:
        sout << "AUIPC"; break;
    case Machine::LOAD:
        sout << "LOAD"; break;
    case Machine::STORE:
        sout << "STORE"; break;
    case Machine::OP_IMM:
        sout << "OPIMM"; break;
    case Machine::OP_IMM_32:
        sout << "OPIMM32"; break;
    case Machine::OP:
        sout << "OP"; break;
    case Machine::OP_32:
        sout << "OP32"; break;
    case Machine::BRANCH:
        sout << "BRANCH"; break;
    case Machine::JALR:
        sout << "JALR"; break;
    case Machine::JAL:
        sout << "JAL"; break;
    case Machine::SYSTEM:
        sout << "SYSTEM"; break;
    case Machine::UNIMPL:
        sout << "NOT-IMPLEMENTED"; break;
    default: 
        sout << "???";
    }
    sout << '\n';
    sout << "RD       : " << (u32)dec.rd << '\n';
    sout << "funct3   : " << (u32)dec.funct3 << '\n';
    sout << "funct7   : " << (u32)dec.funct7 << '\n';
    sout << "offset   : " << dec.offset << '\n';
    sout << "left     : " << dec.leftVal << '\n';
    sout << "right    : " << dec.rightVal;
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
std::ostream& operator<<(std::ostream& out, const Machine::MemoryOut& mo) 
{
    std::ostringstream sout;
    sout << "0x" << std::hex << std::right << std::setfill('0') << std::setw(16) << mo.value;
    return out << sout.str();
}

Machine::Machine(char* mem, i64 size)
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
    u8 OpcodeMapRow = (_FO.instruction >> 5) & 0b11;
    u8 OpcodeMapCol = (_FO.instruction >> 2) & 0b111;
    u8 InstSize     =  _FO.instruction & 0b11;
    if (InstSize != 3) 
    {
        std::cerr << "[DECODE] Invalid instruction (not a 32-bit instruction).\n";
        return;
    }

    _DO.op = Machine::OC_MAP[OpcodeMapRow][OpcodeMapCol];
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
    // to grab Commands and Opcodes enums
    Alu cmd = NO_OP;

    // Most instructions will follow left/right
    // but some won't, so we need these:
    i64 opLeft  = _DO.leftVal;
    i64 opRight = _DO.rightVal; 

    switch (_DO.op)
    {
    case BRANCH: // BEQ, BNE, BLT, BGE
        cmd = SUB; 
        break;

    case AUIPC: // AUIPC
        opLeft = _pc;
        cmd = ADD; 
        break;
    
    case JALR: // JALR
        // offset and a register value need to be added together
        cmd = ADD;
        break; 
    
    case JAL:
        opLeft = _pc;
        cmd = ADD;
        break;

    case LUI:
        cmd = ADD;
        break;

    case LOAD: // LB, LH, LW, LD, LBU, LHU, LWU
        cmd = ADD;
        break;

    case STORE: // SB, SH, SW, SD
        opRight = _DO.offset;
        cmd = ADD;
        break;

    case OP:
        switch (_DO.funct3) 
        {
        case 0b000: // ADD, MUL, SUB 
            switch (_DO.funct7)
            {
            case 0: // ADD
                cmd = ADD;
                break;
            case 1: // MUL
                cmd = MUL;
                break;
            case 32: // SUB
                cmd = SUB;
                break;
            }
            break;
        case 0b001: // SLL
            cmd = SLL; 
            break;
        case 0b100: // XOR, DIV
            switch (_DO.funct7)
            {
            case 0: // XOR
                cmd = XOR;
                break;
            case 1: // DIV
                cmd = DIV;
                break;
            }
            break;
        case 0b101: // SRL, SRA
            switch (_DO.funct7)
            {
            case 0: // SRL
                cmd = SRL;
                break;
            case 32: // SRA
                cmd = SRA;
                break;
            }
            break;
        case 0b110: // OR, REM
            switch (_DO.funct7)
            {
            case 0: // OR
                cmd = OR;
                break;
            case 1: // REM
                cmd = REM;
                break;
            }
            break;
        case 0b111: // AND
            cmd = AND;
            break;
        }
        break;

    case OP_32:
        opLeft  = SignExtend(opLeft,  31u);
        opRight = SignExtend(opRight, 31u);
        // You still need to determine the ALU command
        switch (_DO.funct3)
        {
        case 0b000:
            switch (_DO.funct7)
            {
            case 0:
                cmd = ADD; // ADDW
                break;
            case 1:
                cmd = MUL; // MULW
                break;
            case 32:
                cmd = SUB; // SUBW
                break;
            }
            break;
        case 0b001:
            cmd = SLL; // SLLW
            break;
        case 0b100:
            cmd = DIV; // DIVW
            break;
        case 0b101:
            switch (_DO.funct7)
            {
            case 0:
                cmd = SRL; // SRLW
                break;
            case 1:
                cmd = DIV; // DIVUW
                break;
            case 32:
                cmd = SRA; // SRAW
                break;
            }
            break;
        case 0b110:
            cmd = REM; // REMW
            break;
        case 0b111:
            cmd = REM; // REMUW
            break;
        }
        break;

    case OP_IMM:
        // Look and see which ALU op needs to be executed.
        switch (_DO.funct3)
        {
        case 0b000: // ADDI
            cmd = ADD;
            break;
        case 0b001: // SLLI (64)
            cmd = SLL;
            break;
        case 0b100: // XORI
            cmd = XOR;
            break;
        case 0b101: // SRLI, SRAI (64)
            if (opRight >> 10)
                cmd = SRA; 
            else
                cmd = SRL;
            break;
        case 0b110: // ORI
            cmd = OR;
            break;
        case 0b111: // ANDI
            cmd = AND;
            break;
        }
        break;

    case OP_IMM_32:
        // This is just like OP_IMM except we truncated
        // the left and right ops.
        opLeft  = SignExtend(opLeft,  31u);
        opRight = SignExtend(opRight, 31u);

        // Look and see which ALU op needs to be executed.
        switch (_DO.funct3)
        {
        case 0b000:
            cmd = ADD; // ADDIW
            break;
        case 0b001:
            cmd = SLL; // SLLIW
            break;
        case 0b101:
            if (opRight >> 10)
                cmd = SRA; // SRAIW
            else
                cmd = SRL; // SRLIW
            break;
        }
        break;

    default:
        opLeft  = 0ll;
        opRight = 0ll;
        cmd = NO_OP;
    }
    
    _EO = ALU(cmd, opLeft, opRight);
}
void Machine::Memory() 
{
    // _MO.value = 0ll; // maybe reset the value in case there is no load

    // out of bounds checks are done inside of MemoryWrite and MemoryRead

    switch (_DO.op)
    {
    case STORE:
        switch (_DO.funct3) 
        {
        case 0b000: // SB
            MemoryWrite<u8>(_EO.result, _DO.rightVal);
            break;
        case 0b001: // SH
            MemoryWrite<u16>(_EO.result, _DO.rightVal);
            break;
        case 0b010: // SW
            MemoryWrite<u32>(_EO.result, _DO.rightVal);
            break;
        case 0b011: // SD
            MemoryWrite<u64>(_EO.result, _DO.rightVal);
            break;
        default:
            std::cerr << "[MEMORY: STORE]: Invalid funct3: " << _DO.funct3 << '\n';
            break;
        }
        break;

    case LOAD:
        switch (_DO.funct3) 
        {
        case 0b000: // LB
            _MO.value = MemoryRead<i8>(_EO.result);
            break;
        case 0b001: // LH
            _MO.value = MemoryRead<i16>(_EO.result);
            break;
        case 0b010: // LW 
            _MO.value = MemoryRead<i32>(_EO.result);
            break;
        case 0b011: // LD 
            _MO.value = MemoryRead<i64>(_EO.result);
            break;
        case 0b100: // LBU
            _MO.value = MemoryRead<u8>(_EO.result);
            break;
        case 0b101: // LHU
            _MO.value = MemoryRead<u16>(_EO.result);
            break;
        case 0b110: // LWU
            _MO.value = MemoryRead<u32>(_EO.result);
            break;
        default:
            std::cerr << "[MEMORY: LOAD]: Invalid funct3: " << _DO.funct3 << '\n';
            break;
        }
        break;

    default:
        // If this is not a LOAD or STORE, then this stage just copies
        // the ALU result.
        _MO.value = _EO.result;
    }
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
Machine::MemoryOut& Machine::DebugMemoryOut()
{
    return _MO;
}

template <typename T>
T Machine::MemoryRead(i64 address) const
{
    // address cannot be negative
    // _memory has addresses [0, _memorySize-1] (inclusive)
    // the max value of address is _memorySize - sizeof(T) (number of bytes in T)

    // example: T = char: max address = MEM_SIZE-1  
    //          T =  i64: max address = MEM_SIZE-8  

    i64 numBytes = static_cast<i64>(sizeof(T));
    if (address < 0 || address > _memorySize-numBytes)
    {
        std::cerr << "[MemoryRead]: address " << address << " would access undefined memory\n";
        return T(); // 0
    }
    return *reinterpret_cast<T*>(_memory + address);
}

template <typename T>
void Machine::MemoryWrite(i64 address, T value)
{
    // address cannot be negative
    // _memory has addresses [0, _memorySize-1] (inclusive)
    // the max value of address is _memorySize - sizeof(T) (number of bytes in T)

    i64 numBytes = static_cast<i64>(sizeof(T));
    if (address < 0 || address > _memorySize-numBytes)
    {
        std::cerr << "[MemoryWrite]: address " << address << " would access undefined memory\n";
        return;
    }
    *reinterpret_cast<T*>(_memory + address) = value;
}

i64 Machine::SignExtend(u64 value, u32 index) const
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
    _DO.leftVal  = GetXReg(_FO.instruction >> 15);
    _DO.rightVal = GetXReg(_FO.instruction >> 20);
}
void Machine::DecodeI()
{
    _DO.rd     = (_FO.instruction >> 7)  & 0x1f;
    _DO.funct3 = (_FO.instruction >> 12) & 0b111;
    _DO.funct7 = 0;
    _DO.offset = 0ll;
    _DO.leftVal  = GetXReg(_FO.instruction >> 15);
    _DO.rightVal = SignExtend((_FO.instruction >> 20) & 0xfff, 11u);
}
void Machine::DecodeS()
{
    _DO.rd     = 0;
    _DO.funct3 = (_FO.instruction >> 12) & 0b111;
    _DO.funct7 = 0;
    _DO.offset = SignExtend(((_FO.instruction >> 7)  & 0x1f) |
                           (((_FO.instruction >> 25) & 0x7f) << 5), 11u);
    _DO.leftVal  = GetXReg(_FO.instruction >> 15);
    _DO.rightVal = GetXReg(_FO.instruction >> 20);
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
    _DO.leftVal  = GetXReg(_FO.instruction >> 15); // get_xreg truncates for us
    _DO.rightVal = GetXReg(_FO.instruction >> 20);
}
void Machine::DecodeU()
{
    _DO.rd     = (_FO.instruction >> 7) & 0x1f;
    _DO.funct3 = 0;
    _DO.funct7 = 0;
    _DO.offset = 0ll;
    _DO.leftVal  = 0ll;
    _DO.rightVal = SignExtend((((_FO.instruction >> 12) & 0xf'ffff) << 12), 31u);
}
void Machine::DecodeJ()
{
    _DO.rd     = (_FO.instruction >> 7) & 0x1f;
    _DO.funct3 = 0;
    _DO.funct7 = 0;
    _DO.offset = 0ll;
    _DO.leftVal  = 0ll;
    _DO.rightVal = SignExtend((((_FO.instruction >> 31) & 1) << 19) | 
                                ((_FO.instruction >> 21) & 0x3ff)    |
                               (((_FO.instruction >> 20) & 1) << 10) |
                               (((_FO.instruction >> 12) & 0xff) << 11), 19u);
}

Machine::ExecuteOut Machine::ALU(Machine::Alu cmd, i64 left, i64 right) const
{
    ExecuteOut ret;

    // perform the appropriate operation 
    switch (cmd) 
    {
    case NO_OP:
        break;
    case ADD:
        ret.result = left + right;
        break;
    case SUB:
        ret.result = left - right;
        break;
    case MUL:
        ret.result = left * right;
        break;
    case DIV:
        ret.result = left / right;
        break;
    case REM:
        ret.result = left % right;
        break;
    case AND:
        ret.result = left & right;
        break;
    case OR:
        ret.result = left | right;
        break;
    case XOR:
        ret.result = left ^ right;
        break;
    case NOT:
        ret.result = ~right;
        break;
    case SRL:
        ret.result = static_cast<u64>(left) >> right;
        break;
    case SLL:
        ret.result = left << right;
        break;
    case SRA:
        ret.result = left >> right;
        break;
    }

    // Now that we have the result, determine the flags.
    u8 sign_left   = (left  >> 63) & 1;
    u8 sign_right  = (right >> 63) & 1;
    u8 sign_result = (ret.result >> 63) & 1;

    ret.z = !ret.result;
    ret.n = sign_result;
    ret.v = (~sign_left & ~sign_right &  sign_result) |
            ( sign_left &  sign_right & ~sign_result);
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

    constexpr i64 MEM_SIZE = 1 << 18; // 2^18
    
    // get size of file by pointing to end and getting position
    fin.seekg(0, fin.end);
    int fileSize = fin.tellg();
    std::cout << "fileSize = " << fileSize << '\n';

    // make sure the file size doesn't exceed the memory size
    if (fileSize > MEM_SIZE)
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
    char* memory = new char[MEM_SIZE]; 
    fin.read(memory, fileSize);
    fin.close();

    // create the Machine using the allocated memory and debug
    Machine coolMachine(memory, MEM_SIZE);
    while (coolMachine.GetPC() < fileSize)
    {
        coolMachine.Fetch();
        std::cout << coolMachine.DebugFetchOut() << '\n';
        coolMachine.Decode();
        std::cout << coolMachine.DebugDecodeOut() << '\n';
        coolMachine.Execute();
        std::cout << coolMachine.DebugExecuteOut() << '\n';
        coolMachine.Memory();
        std::cout << coolMachine.DebugMemoryOut() << '\n';
        coolMachine.SetPC(coolMachine.GetPC() + 4);
        // std::cout << '\n';
    }

    // cleanup
    delete[] memory;

    return 0;
}