// Gabriel Tyler
// 03/07/22
// Read from a binary file and store intructions in memory allocated on the heap
// The Machine class manages and fetches the instructions from memory

#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iomanip>

struct FetchOut 
{
    uint32_t instruction;

    // The code below allows us to cout a FetchOut structure.
    // We can use this to debug our code.
    // FetchOut fo = { 0xdeadbeef };
    // cout << fo << '\n';
    friend std::ostream& operator<<(std::ostream& out, const FetchOut& fo) 
    {
        std::ostringstream sout;
        sout << "0x" << std::hex << std::setfill('0') << std::right << std::setw(8) << fo.instruction;
        return out << sout.str();
    }
};

class Machine
{
public:
    // constants
    static const int NUM_REGS = 32;

private:
    char* _memory;           // The memory.
    int _memorySize;         // The size of the memory (should be MEM_SIZE)
    int64_t _pc;             // The program counter
    int64_t _regs[NUM_REGS]; // The register file

    FetchOut _fo; // Result of the fetch() method.

private:
    // Read from the internal memory
    // Usage:
    // int  myintval  = memory_read<int>(0);  // Read the first 4 bytes
    // char mycharval = memory_read<char>(8); // Read byte index 8
    template <typename T>
    T memory_read(int64_t address) const
    {
        return *reinterpret_cast<T*>(_memory + address);
    }

    // Write to the internal memory
    // Usage:
    // memory_write<int>(0, 0xdeadbeef); // Set bytes 0, 1, 2, 3 to 0xdeadbeef
    // memory_write<char>(8, 0xff);      // Set byte index 8 to 0xff
    template <typename T>
    void memory_write(int64_t address, T value)
    {
        *reinterpret_cast<T*>(_memory + address) = value;
    }

public:
    Machine(char* mem, int size)
        : _memory(mem), _memorySize(size), _pc(0)
    {
        // set the stack pointer to be at the end of memory
        set_xreg(2, _memorySize);
    }

    int64_t get_pc() const
    {
        return _pc;
    }
    void set_pc(int64_t to)
    {
        _pc = to;
    }

    int64_t get_xreg(int which) const
    {
        which &= 0x1f; // Make sure the register number is 0 - 31
        return _regs[which];
    }
    void set_xreg(int which, int64_t value)
    {
        which &= 0x1f; // Make sure the register number is 0 - 31
        _regs[which] = value;
    }

    void fetch()
    {
        // read the instruction at the program counter memory address
        _fo.instruction = memory_read<uint32_t>(_pc);
    }
    FetchOut& debug_fetch_out()
    {
        return _fo;
    }
};

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

    // the size of the memory buffer, 2^18
    constexpr int MEM_SIZE = 1 << 18;

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

    // create the Machine using the allocated memory and debug
    Machine coolMachine(memory, MEM_SIZE);
    while (coolMachine.get_pc() < fileSize)
    {
        coolMachine.fetch();
        std::cout << coolMachine.debug_fetch_out() << '\n';
        coolMachine.set_pc(coolMachine.get_pc() + 4);
    }

    // cleanup
    delete[] memory;
    fin.close();

    return 0;
}