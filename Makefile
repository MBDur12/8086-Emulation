CXX = g++

CXXFLAGS = -Wall -c -std=c++11 
EXE = sm8086

all: $(EXE)

$(EXE): disassembler.o decoder.o
	$(CXX) $^ -o $@

disassembler.o: src/disassembler.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -g

clean:
	rm -f *.o && rm -f $(EXE)