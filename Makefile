## Basic C++ makefile

# Detect OS
# https://stackoverflow.com/a/14777895

ifeq ($(OS),Windows_NT)
	ifneq (%PATH%,$(shell echo %PATH%))
		uname_S := Windows
	else
	ifneq (,$(findstring /cygdrive/,$(PATH)))
		uname_S := Cygwin
	else
		uname_S := Bash
	endif
	endif
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
CXX := g++ -std=c++11
REM := $(RM) -f
REMRF := $(REM) -r
NULL := /dev/null
FILE := test.src

# Windows specific macros
ifeq ($(uname_S),Windows)
	REM := del /Q /F
	REMRF := $(REM) /S
	NULL := nul
endif

# Default target, build the executable
all: $(OUTFILE)

# Test a file
test: all
	./$(OUTFILE) $(FILE)

# Clean up
clean:
	$(REM) $(OUTFILE) 2> $(NULL)
	$(REMRF) *.o 2> $(NULL)

# The executable
$(OUTFILE): main.cpp.o
	$(CXX) $^ -o $@

# Object files from C++ source
%.cpp.o: %.cpp
	$(CXX) -c $< -o $@

# Object files from C source
%.o: %.c
	$(CC) -c $< -o $@
