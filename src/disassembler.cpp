#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>

#include "decoder.hpp"
#include "decoder.cpp"

int main(int argc, char**argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage <exe> <binary file>" << '\n';
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
    std::cout << "bits 16 \n\n";
    size_t idx = 0;
    while (idx < fileSize)
    {
        u8 firstByte = buffer[idx];
        u8 op4 = (firstByte & 0b11110000) >> 4;

        if (op4 == 0b1011) // imm to register
        {
            size_t bytesProcessed = decodeOP(OPCODE::MOV_IMM_REG, buffer, idx);
            idx += bytesProcessed;
            continue;
        }

        u8 op6 = (firstByte & 0b11111100) >> 2;
        if (op6 == 0b100010) // reg/mem to/from reg
        {
            u8 secondByte = buffer[idx + 1];
            u8 mod = (secondByte & 0b11000000) >> 6;
            size_t bytesProcessed{0};

            if (mod == 0b11) // reg-to-reg, no displacement
            {
                bytesProcessed = decodeOP(OPCODE::MOV_REG_REG, buffer, idx);
            }
            else
            {
                bytesProcessed = decodeOP(OPCODE::MOV_MEM_REG, buffer, idx);
            }

            idx += bytesProcessed;
        }
        // Add, Sub, Cmp reg-mem
        else if (op6 == 0b000000 || op6 == 0b001010 || op6 == 0b001110)
        {
            size_t bytesProcessed = decodeOP(OPCODE::MOV_ASC_MREG, buffer, idx);
            idx += bytesProcessed;
        }
        else if (op6 == 0b100000) // ASC, imm-to-reg/mem
        {
            size_t bytesProcessed = decodeOP(OPCODE::MOV_ASC_IMREG, buffer, idx);
            idx += bytesProcessed; 
        }

    }

    return 0;
}