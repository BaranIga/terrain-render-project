CXX      := g++
CC       := gcc
TARGET   := terrain.exe

SRC_CPP  := main.cpp hgt.cpp 
SRC_C    := extern/glad/src/glad.c

INC      := -Iextern/glad/include
CXXFLAGS := -std=c++17 -O2 -Wall
LDFLAGS  := -lglfw3 -lopengl32 -lgdi32

all: $(TARGET)

$(TARGET): $(SRC_CPP) $(SRC_C)
	$(CXX) $(CXXFLAGS) $(INC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
