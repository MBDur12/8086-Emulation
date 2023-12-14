#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <assert.h>
#include <bitset>

using u8 = u_int8_t;

void printBinary(std::string str, u8 val)
{
    std::cout << str <<  " 0b" << std::bitset<8>(val) << std::endl;
}

std::unordered_map<u8, std::vector<std::string>> registerMap = {
        {0b000, {"al", "ax"}},
        {0b001, {"cl", "cx"}},
        {0b010, {"dl", "dx"}},
        {0b011, {"bl", "bx"}},
        {0b100, {"ah", "sp"}},
        {0b101, {"ch", "bp"}},
        {0b110, {"dh", "si"}},
        {0b111, {"bh", "di"}}
};

void decodeInstructions(u8 byte1, u8 byte2)
{
    std::string opStr{};
    u8 opCode = byte1 >> 2;

    switch(opCode)
    {
        case 0b100010: // mov
            opStr = "mov ";
            break;
        default:
            std::cout << "Couldn't interpret instruction" << std::endl;
            break;
    }

    u8 w = byte1 & 0b00000001;
    u8 d = byte1 & 0b00000010;

    u8 mod = byte2 & 0b11000000;
    u8 reg = (byte2 & 0b00111000) >> 3;
    u8 rm = byte2 & 0b00000111;

    std::string src{};
    std::string dst{};

    assert(registerMap.find(reg) != registerMap.end());
    assert(registerMap.find(rm) != registerMap.end());

    if (d == 0) // REG is not destination
    {   
        src = registerMap[reg][w];
        dst = registerMap[rm][w];
    }
    else
    {
        src = registerMap[rm][w];
        dst = registerMap[reg][w];
    }

    std::cout << opStr << dst << ", " << src << std::endl;
}
int main(int argc, char**argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage <executable> <binary file>" << '\n';
        return 1;
    }

    std::ifstream inFile(argv[1], std::ios::binary);
    if (!inFile)
    {
        std::cout << "Could not open file: " << argv[1] << std::endl;
        return 1;
    }

    inFile.seekg(0, std::ios::end);
    size_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    std::vector<u8> buffer(fileSize);

    if (!inFile.read(reinterpret_cast<char *>(buffer.data()), fileSize)) {
        std::cerr << "Error reading file!" << std::endl;
        inFile.close();
        return 1;
    }
    inFile.close();

    std::cout << "bits 16\n" << std::endl;
    
    // for now assumes each instruction is 2 bytes
    for (size_t i = 0; i < fileSize; i+=2)
    {
        decodeInstructions(buffer[i], buffer[i+1]);
    }

    return 0;
}