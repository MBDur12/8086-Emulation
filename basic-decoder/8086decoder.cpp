#include <iostream>
#include <fstream>

using std::ifstream;

using u8 = u_int8_t;
using u32 = u_int32_t;
using s16 = int16_t;

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

struct instruction_operand
{
    operand_type Type;
    union
    {
        // ways to model effective addresses, registers, or immediates.
    };
};
// The decoder gets passed a list of these
struct instruction
{
    u32 Size;
    op_type Op;

    instruction_operand Operands[2];
};

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



    return 0;
}