EXECUTABLE = main.exe

SOURCES = board.cpp evaluation.cpp main.cpp moveGeneration.cpp moves.cpp moveSorter.cpp \
		  randomNumber.cpp search.cpp transpositionTable.cpp uci.cpp zobrist.cpp

OBJECTS = $(SOURCES:.cpp=.o)

CXXFLAGS = -pedantic -Wall -Wextra -Wshadow -Wcast-qual -std=c++17 -O3 -m64 -msse -msse3 -msse4 -mbmi2 -mpopcnt -flto

LDFLAGS = $(CXXFLAGS)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) 