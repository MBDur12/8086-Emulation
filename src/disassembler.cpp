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

// For reg-mem
std::string getRM(u8 val)
{
    std::string result{};
    switch (val)
    {
        case 0b000:
            result = "bx + si";
            break;
        case 0b001:
            result = "bx + di";
            break;
        case 0b010:
            result = "bp + si";
            break;
        case 0b011:
            result = "bp + di";
            break;
        case 0b100:
            result = "si";
            break;
        case 0b101:
            result = "di";
            break;
        case 0b110:
            result = "bp"; // Gets replaced when MOD is 00
            break;
        case 0b111:
            result = "bx";
            break;
    }
    return result;
}

void printBinary(std::string str, u8 val)
{
    std::cout << str <<  " 0b" << std::bitset<8>(val) << '\n';
}

size_t decodeRegToReg(const vector<u8> &buffer, size_t idx)
{
    u8 firstByte = buffer[idx];
    u8 secondByte = buffer[idx+1];

    u8 w = firstByte & 0b00000001;
    u8 d = (firstByte & 0b00000010) >> 1;
    
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
    size_t bytesProcessed{1};
    u8 firstByte = buffer[idx];
    u8 w = (firstByte & 0b00001000) >> 3;
    u8 reg = firstByte & 0b00000111;
    
    int16_t data = (buffer[idx+1]);
    bytesProcessed++;

    //...
    if (w == 1)
    {
        bytesProcessed++;
        data = (buffer[idx+2] << 8) | data; // 2nd byte is most significant
    }

    assert(registerMap.find(reg) != registerMap.end());

    std::cout << "mov " << registerMap[reg][w] << ", " << data << '\n';
    return bytesProcessed;

}

// Probably don't want to be passing this buffer around - pointers?
int16_t getDisplacement(u8 mod, const vector<u8> &buffer, bool isDirectlyAddressable, size_t &bytesProcessed, size_t dataIdx)
{
    int16_t displacement{};
    if (mod == 0b01) // 8-bit
    {
        displacement = buffer[dataIdx];
        bytesProcessed++;
    }
    else if (mod == 0b10 || isDirectlyAddressable) // 16-bit
    {
        displacement = (buffer[dataIdx+1] << 8) | buffer[dataIdx];
        bytesProcessed += 2;
    }

    return displacement;
}

size_t decodeMemToReg(const vector<u8> &buffer, size_t idx)
{
    size_t bytesProcessed{0};
    u8 firstByte = buffer[idx];
    u8 secondByte = buffer[idx+1];
    bytesProcessed += 2;

    u8 w = firstByte & 0b00000001;
    u8 d = (firstByte & 0b00000010) >> 1;

    u8 mod = (secondByte & 0b11000000) >> 6;
    u8 reg = (secondByte & 0b00111000) >> 3;
    u8 rm = secondByte & 0b00000111;

    std::string src{};
    std::string dst{};

    assert(registerMap.find(reg) != registerMap.end());
    assert(registerMap.find(rm) != registerMap.end());

    if (d == 0) // REG is not destination
    {   
        src = registerMap[reg][w];
        dst = getRM(rm);
    }
    else
    {
        src = getRM(rm);
        dst = registerMap[reg][w];
    }

    bool isDirectlyAddressable = mod == 0b00 && rm == 0b110;
    int16_t displacement = getDisplacement(mod, buffer, isDirectlyAddressable, bytesProcessed, idx+2);

    if (d == 0)
    {
        if (isDirectlyAddressable)
        {
            std::cout << "mov " << "[" << displacement << "], " << src << '\n';
        }
        else if (displacement > 0)
        {
            std::cout << "mov " << "[" << dst << " + " << displacement << "], " << src << '\n';
        }
        else if (displacement < 0)
        {
            std::cout << "mov " << "[" << dst << " - " << displacement << "], " << src << '\n';
        }
        else
        {
            std::cout << "mov " << "[" << dst << "], " << src << '\n';
        }
    }
    else
    {
        if (isDirectlyAddressable)
        {
            std::cout << "mov " << "[" << displacement << "], " << dst << '\n';
        }
        else if (displacement > 0)
        {
            std::cout << "mov " << dst << ", " << "[" << src << " + " << displacement << "]" << '\n';
        }
        else if (displacement < 0)
        {
            std::cout << "mov " << dst << ", " << "[" << src << " - " << displacement << "]" << '\n';
        }
        else
        {
            std::cout << "mov " << dst << ", " << "[" << src << "]" << '\n';
        }
    }

    return bytesProcessed;
}

// constexpr as this should not alter
constexpr opCodeDecoderFunction opCodeFuncLookup[]
{
    &decodeRegToReg,
    &decodeImmToReg,
    &decodeMemToReg,
};

// Main decoding function
size_t decodeOP(const OPCODE op, const vector<u8> &buffer, size_t idx)
{
    opCodeDecoderFunction fnc = opCodeFuncLookup[op];
    size_t bytesProcessed = fnc(buffer, idx);
    return bytesProcessed;
}

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

    }

    return 0;
}