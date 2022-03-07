#include <iostream>
#include <cstdint>
#include <fstream>

// g++ -Wall -Wextra -O0 -o mymachine mymachine.cpp

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

    // 2^18 = 262144
    const int MEM_SIZE = 1 << 18;
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

    // cleanup
    delete[] memory;
    fin.close();
}