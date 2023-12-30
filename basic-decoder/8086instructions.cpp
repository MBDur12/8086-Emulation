#include "8086instructions.hpp"

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
            else
            {
                
            }

        }

        Instructions.push_back(NewInstruction);
    }

    return Instructions;
}