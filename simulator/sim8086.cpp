#include <iostream>
#include <fstream>
#include "sim86_shared.h"

using std::ifstream;

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

    // Read in contents + arrays in cpp?
    

    return 0;
}