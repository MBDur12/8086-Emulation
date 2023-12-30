#include <iostream>
#include <cstdio>
#include <fstream>

using std::ifstream; 

#include "8086instructions.hpp"
#include "8086instructions.cpp"

int main(int argc, char**argv)
{
    if (argc != 2)
    {
        std::cout << "Usage <exe> <binary file>" << '\n';
        return 1;
    }

    ifstream File(argv[1], std::ios::binary);
    if (!File)
    {
        std::cout << "Could not open file: " << argv[1] << '\n';
        return 1;
    }

    File.seekg(0, std::ios::end);
    size_t FileSize = File.tellg();
    File.seekg(0, std::ios::beg);

    u8 Source[FileSize];

    if (!File.read(reinterpret_cast<char *>(Source), FileSize)) {
        std::cerr << "Error reading file!" << '\n';
        File.close();
        return 1;
    }
    File.close();

    fprintf(stdout, "bits 16 \n\n");

    vector<instruction> Instructions = BuildInstructions(Source, FileSize);
    for (auto Inst : Instructions)
    {
        PrintInstruction(Inst);
    }

    return 0;
}