#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>

#include "decoder.hpp"

// Main decoding function
size_t decodeOP(const OPCODE op, const vector<u8> &buffer, size_t idx)
{
    opCodeDecoderFunction fnc = opCodeFuncLookup[op];
    size_t bytesProcessed = fnc(buffer, idx);
    return bytesProcessed;
}

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

std::string getASCopStr(u8 op)
{
    std::string opStr{};
    switch(op)
    {
        case 0b000:
            opStr = "add ";
            break;
        case 0b101:
            opStr = "sub ";
            break;
        case 0b111:
            opStr = "cmp ";
            break;
    }
    return opStr;
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
    
    s16 data = (buffer[idx+1]);
    bytesProcessed++;

    if (w == 1)
    {
        data = (buffer[idx+2] << 8) | data; // 2nd byte is most significant
        bytesProcessed++;
    }

    std::cout << "mov " << registerMap[reg][w] << ", " << data << '\n';
    return bytesProcessed;

}

s16 getDisplacement(u8 mod, const vector<u8> &buffer, bool isDirectlyAddressable, size_t &bytesProcessed, size_t dataIdx)
{
    s16 displacement{};
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
    s16 displacement = getDisplacement(mod, buffer, isDirectlyAddressable, bytesProcessed, idx+2);

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

size_t decodeASCmReg(const vector<u8> &buffer, size_t idx)
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

    // Determine operation
    u8 op = (firstByte & 0b00111000) >> 3;
    std::string opStr = getASCopStr(op);

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
    s16 displacement = getDisplacement(mod, buffer, isDirectlyAddressable, bytesProcessed, idx+2);

    if (d == 0)
    {
        if (isDirectlyAddressable)
        {
            std::cout << opStr << "[" << displacement << "], " << src << '\n';
        }
        else if (displacement > 0)
        {
            std::cout << opStr << "[" << dst << " + " << displacement << "], " << src << '\n';
        }
        else if (displacement < 0)
        {
            std::cout << opStr << "[" << dst << " - " << displacement << "], " << src << '\n';
        }
        else
        {
            std::cout << opStr << "[" << dst << "], " << src << '\n';
        }
    }
    else
    {
        if (isDirectlyAddressable)
        {
            std::cout << opStr << "[" << displacement << "], " << dst << '\n';
        }
        else if (displacement > 0)
        {
            std::cout << opStr << dst << ", " << "[" << src << " + " << displacement << "]" << '\n';
        }
        else if (displacement < 0)
        {
            std::cout << opStr << dst << ", " << "[" << src << " - " << displacement << "]" << '\n';
        }
        else
        {
            std::cout << opStr << dst << ", " << "[" << src << "]" << '\n';
        }
    }

    return bytesProcessed;
}

size_t decodeASCImmToRegMem(const vector<u8> &buffer, size_t idx)
{
    size_t bytesProcessed{0};
    u8 firstByte = buffer[idx];
    u8 secondByte = buffer[idx + 1];
    bytesProcessed += 2;

    u8 s = (firstByte & 0b00000010) >> 1;
    u8 w = firstByte & 0b00000001;
    
    u8 mod = (secondByte & 0b11000000) >> 6;
    u8 op = (secondByte & 0b00111000) >> 3;
    u8 rm = secondByte & 0b00000111;

    std::string opStr = getASCopStr(op);
    std::string dst = registerMap[rm][w];


    bool isDirectlyAddressable = mod == 0b00 && rm == 0b110;
    s16 displacement = getDisplacement(mod, buffer, isDirectlyAddressable, bytesProcessed, idx+2);

    s16 data = (buffer[idx+bytesProcessed]);
    bytesProcessed++;

    if (s == 0 && w == 1)
    {
        data = (buffer[idx+bytesProcessed+1] << 8) | data; // 2nd byte is most significant
        bytesProcessed++;
    }

    if (isDirectlyAddressable)
    {
        std::cout << opStr << "[" << displacement << "], " << dst << '\n';
    }
    else if (displacement > 0)
    {
        std::cout << opStr << dst << ", " << data << " + " << displacement << '\n';
    }
    else if (displacement < 0)
    {
        std::cout << opStr << dst << ", " << data << " - " << displacement << '\n';
    }
    else
    {
        std::cout << opStr << dst << ", " << data << '\n';
    }
    
    return bytesProcessed;
}

