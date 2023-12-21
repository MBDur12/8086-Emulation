#pragma once
#ifndef _DECODERH_
#define _DECODERH_

#include <iostream>
#include <vector>
#include <unordered_map>

using std::size_t;
using std::vector;

using u8 = u_int8_t;
using s16 = int16_t;
using opCodeDecoderFunction = size_t (*)(const vector<u8> &, size_t);

enum OPCODE
{
    MOV_REG_REG,
    MOV_IMM_REG,
    MOV_MEM_REG,
    MOV_ASC_MREG,
    MOV_ASC_IMREG
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

std::string getRM(u8 val);
std::string getASCOP(u8 op);

size_t decodeRegToReg(const vector<u8> &buffer, size_t idx);
size_t decodeImmToReg(const vector<u8> &buffer, size_t idx);
size_t decodeMemToReg(const vector<u8> &buffer, size_t idx);

size_t decodeASCmReg(const vector<u8> &buffer, size_t idx);
size_t decodeASCImmToRegMem(const vector<u8> &buffer, size_t idx);

s16 getDisplacement(u8 mod, const vector<u8> &buffer, bool isDirectlyAddressable, size_t &bytesProcessed, size_t dataIdx);

size_t decodeOP(const OPCODE op, const vector<u8> &buffer, size_t idx);

constexpr opCodeDecoderFunction opCodeFuncLookup[]
{
    &decodeRegToReg,
    &decodeImmToReg,
    &decodeMemToReg,
    &decodeASCmReg,
    &decodeASCImmToRegMem,
};
#endif



