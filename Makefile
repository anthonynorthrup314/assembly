## Basic C++ makefile

# Detect OS
# https://stackoverflow.com/a/14777895
ifeq ($(OS),Windows_NT)
	uname_S := Windows
else
	uname_S := $(shell uname -s)
endif

# Target extension
ifeq ($(uname_S),Windows)
	OUTFILE = main.exe
else
	OUTFILE = main.out
endif

# Macros
CC := gcc
CXX := gcc -lstdc++
REM := rm -f
REMRF := $(REM) -r
NULL := /dev/null
FILE := test.src

# Windows specific macros
ifeq ($(uname_S),Windows)
	REM := del /F /Q
	REMRF := $(REM) /S
	NULL: nul
endif

# Default target, build the executable
all: $(OUTFILE)

# Run the project
run: all
	./$(OUTFILE)

# Test a specific file
test: all
	./$(OUTFILE) $(FILE)

# Clean up
clean:
	$(REM) $(OUTFILE)
	$(REMRF) *.o

# The executable
$(OUTFILE): main.cpp.o
	$(CXX) $^ -o $@

# Object files from C++ source
%.cpp.o: %.cpp
	$(CC) -c $< -o $@

# Object files from C source
%.o: %.c
	$(CC) -c $< -o $@
