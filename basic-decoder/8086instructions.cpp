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
            u8 W = (Byte1 & 0b00001000) >> 3;
            s16 Data = Source[++CurrentIdx];
            
            
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
        }
        else if (Op == 0b100010) // reg/mem to/from reg
        {
            NewInstruction.Op = op_type::Op_mov;
            u8 Byte2 = Source[++CurrentIdx];
            u8 Mod = (Byte2 & 0b11000000) >> 6;
            u8 W = Byte1 & 0b00000001;
            u8 D = (Byte1 & 0b00000010) >> 1;
            u8 Reg = (Byte2 & 0b00111000) >> 3;
            u8 Rm = Byte2 & 0b00000111;
            
            if (Mod == 0b11) // reg-to-reg
            {
                NewInstruction.Operands[0].Type = operand_type::Operand_Reg;
                NewInstruction.Operands[1].Type = operand_type::Operand_Reg;
                NewInstruction.Operands[0].Register.Offset = W;
                NewInstruction.Operands[1].Register.Offset = W;    

                if (D == 0) // REG is src
                {
                    NewInstruction.Operands[0].Register.Index = Reg;
                    NewInstruction.Operands[1].Register.Index = Rm; 
                }
                else
                {
                    NewInstruction.Operands[0].Register.Index = Rm;
                    NewInstruction.Operands[1].Register.Index = Reg; 
                }
                
            }
            else // Memory involved
            {
                bool DirectlyAddressable = Mod == 0b00 && Rm == 0b110;
                
                // Compute displacement (if any)
                s16 Displacement{};

                if (Mod == 0b01) // 8-bit
                {
                    Displacement = Source[++CurrentIdx];
                }
                else if (Mod == 0b10 || DirectlyAddressable) // 16-bit
                {
                    Displacement = (Source[CurrentIdx + 2] << 8) | Source[CurrentIdx + 1];
                    CurrentIdx += 2;
                }   


                if (D == 0) // REG is src
                {
                    NewInstruction.Operands[0].Type = operand_type::Operand_Reg;
                    NewInstruction.Operands[0].Register.Index = Reg;
                    NewInstruction.Operands[0].Register.Offset = W;
                }
                else
                {
                    NewInstruction.Operands[1].Type = operand_type::Operand_Reg;
                    NewInstruction.Operands[1].Register.Index = Reg;
                    NewInstruction.Operands[1].Register.Offset = W;
                }
                
            }

        }

        Instructions.push_back(NewInstruction);
        ++CurrentIdx;

    }
    return Instructions;
}