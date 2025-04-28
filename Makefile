# Makefile for Virtual Memory Management Project
# Author: Yicheng Zhao, 1312068

EXE = translate
SRC = main.c

# Build target: generate executable translate
$(EXE): $(SRC)
	gcc -O3 -Wall -o $(EXE) $<

# Run task1
run_task1: $(EXE)
	./$(EXE) -f addresses.txt -t task1

# Run task2
run_task2: $(EXE)
	./$(EXE) -f addresses.txt -t task2

# Run task3
run_task3: $(EXE)
	./$(EXE) -f addresses.txt -t task3

# Run task4
run_task4: $(EXE)
	./$(EXE) -f addresses.txt -t task4

# Format source code using clang-format
format:
	clang-format -style=file -i *.c

# Clean build files
clean:
	rm -f $(EXE)