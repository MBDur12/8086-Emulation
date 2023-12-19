#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <assert.h>
#include <bitset>

using std::size_t;
using std::vector;
// HELPERS //
using u8 = u_int8_t;
using opCodeDecoderFunction = size_t (*)(const vector<u8> &, size_t);

enum OPCODE
{
    MOV_REG_REG,
    MOV_IMM_REG,
    MOV_MEM_REG,
};

// constexpr as this should not alter
constexpr opCodeDecoderFunction opCodeFuncLookup[]
{
    &decodeRegToReg,
    &decodeImmToReg,
    &decodeMemToReg,
};

std::unordered_map<u8, vector<std::string>> registerMap = {
        {0b000, {"al", "ax"}},
        {0b001, {"cl", "cx"}},
        {0b010, {"dl", "dx"}},
        {0b011, {"bl", "bx"}},
        {0b100, {"ah", "sp"}},
        {0b101, {"ch", "bp"}},
        {0b110, {"dh", "si"}},
        {0b111, {"bh", "di"}}
};

void printBinary(std::string str, u8 val)
{
    std::cout << str <<  " 0b" << std::bitset<8>(val) << '\n';
}

size_t decodeRegToReg(const vector<u8> &buffer, size_t idx)
{
    u8 firstByte = buffer[idx];
    u8 secondByte = buffer[idx+1];

    u8 w = firstByte & 0b00000001;
    u8 d = firstByte & 0b00000010;
    
    u8 reg = (secondByte & 0b00111000) >> 3;
    u8 rm = secondByte & 0b00000111;

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

    std::cout << "mov " << dst << ", " << src << '\n';
    return 2;
}

size_t decodeImmToReg(const vector<u8> &buffer, size_t idx)
{
    return 0;
}

size_t decodeMemToReg(const vector<u8> &buffer, size_t idx)
{
    return 0;
}


// Main decoding function
size_t decodeOP(const OPCODE op, const vector<u8> &buffer, size_t idx)
{
    opCodeDecoderFunction fnc = opCodeFuncLookup[op];
    size_t bytesProcessed = fnc(buffer, idx);
    return bytesProcessed;
}
// void decodeRegToReg(u8 byte1, u8 byte2)
// {
//     u8 w = byte1 & 0b00000001;
//     u8 d = byte1 & 0b00000010;

//     u8 mod = byte2 & 0b11000000;
//     u8 reg = (byte2 & 0b00111000) >> 3;
//     u8 rm = byte2 & 0b00000111;

//     std::string src{};
//     std::string dst{};

//     assert(registerMap.find(reg) != registerMap.end());
//     assert(registerMap.find(rm) != registerMap.end());

//     if (d == 0) // REG is not destination
//     {   
//         src = registerMap[reg][w];
//         dst = registerMap[rm][w];
//     }
//     else
//     {
//         src = registerMap[rm][w];
//         dst = registerMap[reg][w];
//     }

//     std::cout << "mov " << dst << ", " << src << '\n';
// }



int main(int argc, char**argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage ./sm8086 <binary file>" << '\n';
        return 1;
    }

    std::ifstream inFile(argv[1], std::ios::binary);
    if (!inFile)
    {
        std::cout << "Could not open file: " << argv[1] << '\n';
        return 1;
    }

    inFile.seekg(0, std::ios::end);
    size_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    vector<u8> buffer(fileSize);

    if (!inFile.read(reinterpret_cast<char *>(buffer.data()), fileSize)) {
        std::cerr << "Error reading file!" << '\n';
        inFile.close();
        return 1;
    }
    inFile.close();
    
    // Instruction decoding
    size_t idx = 0;
    while (idx < fileSize)
    {
        u8 firstByte = buffer[idx];
        u8 op4 = firstByte & 0b11110000;

        if (op4 == 0b10110000) // imm to register
        {
            size_t bytesProcessed = decodeOP(OPCODE::MOV_IMM_REG, buffer, idx);
            idx += bytesProcessed;
        }

        u8 op6 = firstByte & 0b11111100;
        if (op6 == 0b10001000) // reg/mem to/from reg
        {
            u8 secondByte = buffer[idx + 1];
            u8 mod = secondByte & 0b11000000;
            size_t bytesProcessed{};

            if (mod == 0b11000000) // reg-to-reg, no displacement
            {
                bytesProcessed = decodeOP(OPCODE::MOV_REG_REG, buffer, idx);
            }
            else
            {
                bytesProcessed = decodeOP(OPCODE::MOV_MEM_REG, buffer, idx);
            }

            idx += bytesProcessed;
        }
    }

    return 0;
}