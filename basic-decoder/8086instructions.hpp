#ifndef __INSTRUCTIONS_HPP__
#define __INSTRUCTIONS_HPP__

#include <iostream>
#include <unordered_map>
#include <vector>

using std::string;
using std::size_t;
using std::unordered_map;
using std::vector;

using u8 = u_int8_t;
using u32 = u_int32_t;
using s16 = int16_t;
using s32 = int;

enum register_index
{
    Reg_000,
    Reg_001,
    Reg_010,
    Reg_011,
    Reg_100,
    Reg_101,
    Reg_110,
    Reg_111,
};

unordered_map<u8, vector<string>> RegisterMap = {
        {0b000, {"al", "ax"}},
        {0b001, {"cl", "cx"}},
        {0b010, {"dl", "dx"}},
        {0b011, {"bl", "bx"}},
        {0b100, {"ah", "sp"}},
        {0b101, {"ch", "bp"}},
        {0b110, {"dh", "si"}},
        {0b111, {"bh", "di"}}
};

// Just implements a few basic Opcodes
enum op_type
{
    Op_mov,
    Op_add,
    Op_sub,
    Op_cmp,
    Op_jmp,
};

enum operand_type
{
    Operand_None,
    Operand_Reg,
    Operand_Mem,
    Operand_Imm,
};

// Operand is either immediate, register, or effective address
struct immediate
{
    s16 Value;
};

struct register_entry
{
    u8 Index;
    u8 Offset;
};

struct effective_address
{
    s16 Displacement;
    register_entry Register;
};

struct instruction_operand
{
    operand_type Type;
    union
    {
        effective_address Address;
        register_entry Register;
        immediate Immediate;
    };
};
// The decoder gets passed a list of these
struct instruction
{
    op_type Op;
    instruction_operand Operands[2];
};

string FetchRegister(register_entry Reg);
void PrintInstruction(instruction Inst);
vector<instruction> BuildInstructions(u8 *Source, size_t FileSize);
string FetchRM(u8 Val);

#endif