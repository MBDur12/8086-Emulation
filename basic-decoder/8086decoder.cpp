#include <iostream>
#include <cstdio>
#include <fstream>
#include <unordered_map>
#include <vector>

using std::string;
using std::ifstream;
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

string FetchRegister(register_entry Reg)
{
    return RegisterMap[Reg.Index][Reg.Offset];
}

void PrintInstruction(instruction Inst)
{
    switch (Inst.Op)
    {
        case op_type::Op_mov:
            // for reg-to-reg
            string src = FetchRegister(Inst.Operands[0].Register);
            string dest = FetchRegister(Inst.Operands[1].Register);
            fprintf(stdout, "mov %s, %s\n", dest.c_str(), src.c_str());
            break;
    }
}

vector<instruction> BuildInstructions(u8 *Source, size_t FileSize)
{
    vector<instruction>Instructions{};
    size_t CurrentIdx{0};

    while (CurrentIdx < FileSize)
    {
        instruction NewInstruction;
        u8 Byte1 = Source[CurrentIdx];
        u8 Op = (Byte1 & 0b11111100) >> 2;

        if (Op == 0b101100) // imm-to-reg
        {
            ++CurrentIdx;
            u8 W = (Byte1 & 0b00001000) >> 3;
            s16 Data = Source[CurrentIdx];

            if (W == 1)
            {
                Data = (Source[++CurrentIdx] << 8) | Data; // 2nd byte is most significant
            }
            NewInstruction.Op = op_type::Op_mov;

            NewInstruction.Operands[0].Type = operand_type::Operand_Reg;
            NewInstruction.Operands[0].Register.Index = Byte1 & 0b00000111;
            NewInstruction.Operands[0].Register.Offset = W;
            
            NewInstruction.Operands[1].Type = operand_type::Operand_Imm;
            NewInstruction.Operands[1].Immediate = {Data};
            ++CurrentIdx;
        }
        else if (Op == 0b100010) // reg/mem to/from reg
        {
            ++CurrentIdx;
            u8 Byte2 = Source[CurrentIdx];
            u8 Mod = (Byte2 & 0b11000000) >> 6;

            if (Mod == 0b11) // reg-to-reg
            {
                u8 W = Byte1 & 0b00000001;
                u8 D = (Byte1 & 0b00000010) >> 1;
                
                u8 Reg = (Byte2 & 0b00111000) >> 3;
                u8 Rm = Byte2 & 0b00000111;

                NewInstruction.Op = op_type::Op_mov;
                NewInstruction.Operands[0].Type = operand_type::Operand_Reg;
                NewInstruction.Operands[1].Type = operand_type::Operand_Reg;
                NewInstruction.Operands[0].Register.Offset = W;
                NewInstruction.Operands[1].Register.Offset = W;    

                if (D == 0) // Reg is src
                {
                    NewInstruction.Operands[0].Register.Index = Reg;
                    NewInstruction.Operands[1].Register.Index = Rm; 
                }
                else
                {
                    NewInstruction.Operands[0].Register.Index = Rm;
                    NewInstruction.Operands[1].Register.Index = Reg; 
                }
                ++CurrentIdx;
            }
        }

        Instructions.push_back(NewInstruction);
    }

    return Instructions;
}

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