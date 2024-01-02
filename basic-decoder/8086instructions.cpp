#include "8086instructions.hpp"

string FetchRegister(register_entry Reg)
{
    return RegisterMap[Reg.Index][Reg.Offset];
}

void PrintInstruction(instruction Inst)
{
    instruction_operand Op0 = Inst.Operands[0];
    instruction_operand Op1 = Inst.Operands[1];

    if (Inst.Op == op_type::Op_mov)
    {
        // Imm-to-reg
        if (Op0.Type == operand_type::Operand_Reg && Op1.Type == operand_type::Operand_Imm)
        {
            string dest = FetchRegister(Op0.Register);
            fprintf(stdout, "mov %s, %d\n", dest.c_str(), Op1.Immediate.Value);
        }
        // Reg-to-reg
        else if (Op0.Type == operand_type::Operand_Reg && Op1.Type == operand_type::Operand_Reg)
        {
            string src = FetchRegister(Op0.Register);
            string dest = FetchRegister(Op1.Register);
            fprintf(stdout, "mov %s, %s\n", dest.c_str(), src.c_str());
        }
        // Source address calculation
        else if (Op0.Type == operand_type::Operand_Reg && Op1.Type == operand_type::Operand_Mem)
        {
            string src = FetchRM(Op1.Address.Register.Index);
            string dest = FetchRegister(Op0.Register);
            s16 Displacement = Op1.Address.Displacement;
            if (Displacement > 0)
            {
                fprintf(stdout, "mov %s, [%s + %d]\n", dest.c_str(), src.c_str(), Displacement);
            }
            else if (Displacement < 0)
            {
                fprintf(stdout, "mov %s, [%s - %d]\n", dest.c_str(), src.c_str(), Displacement);
            }
            else
            {
                fprintf(stdout, "mov %s, [%s]\n", dest.c_str(), src.c_str());
            }
        }
        // Destination address calculation
        else if (Op0.Type == operand_type::Operand_Mem && Op1.Type == operand_type::Operand_Reg)
        {
            string src = FetchRegister(Op1.Register);
            string dest = FetchRM(Op0.Address.Register.Index);

            s16 Displacement = Op0.Address.Displacement;
            if (Displacement > 0)
            {
                fprintf(stdout, "mov [%s + %d], %s\n", dest.c_str(), Displacement, src.c_str());
            }
            else if (Displacement < 0)
            {
                fprintf(stdout, "mov [%s - %d], %s\n", dest.c_str(), Displacement, src.c_str());
            }
            else
            {
                fprintf(stdout, "mov [%s], %s\n", dest.c_str(), src.c_str());
            }
        }
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
        u8 Op4 = (Byte1 & 0b11110000) >> 4;
        u8 Op6 = (Byte1 & 0b11111100) >> 2;

        if (Op4 == 0b1011) // imm-to-reg
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
        else if (Op6 == 0b100010) // reg/mem to/from reg
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


                if (D == 0)
                {
                    NewInstruction.Operands[1].Type = operand_type::Operand_Reg;
                    NewInstruction.Operands[1].Register.Index = Reg;
                    NewInstruction.Operands[1].Register.Offset = W;

                    NewInstruction.Operands[0].Type = operand_type::Operand_Mem;
                    NewInstruction.Operands[0].Address.Displacement = Displacement;
                    NewInstruction.Operands[0].Address.Register = {Rm, W};
                }
                else
                {
                    NewInstruction.Operands[1].Type = operand_type::Operand_Mem;
                    NewInstruction.Operands[1].Address.Displacement = Displacement;
                    NewInstruction.Operands[1].Address.Register = {Rm, W};

                    NewInstruction.Operands[0].Type = operand_type::Operand_Reg;
                    NewInstruction.Operands[0].Register.Index = Reg;
                    NewInstruction.Operands[0].Register.Offset = W;
                }
            }
        }
        else
        {
            fprintf(stdout, "Failed to Find OpCode at index %lu\n", CurrentIdx);
            break;
        }

        Instructions.push_back(NewInstruction);
        ++CurrentIdx;

    }
    return Instructions;
}

string FetchRM(u8 Val)
{
    std::string result{};
    switch (Val)
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