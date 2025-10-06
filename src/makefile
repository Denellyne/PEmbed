CXX = gcc
CFLAGS = -lpng 
INCLUDES := $(shell find . -name '*.c')
OBJ := $(patsubst %.c,%.o,$(INCLUDES))
TEST := *.md
DEBUG = -g -Wall -Wextra -pedantic
OUT = ../bin/main
CORES = ./cores/core.*

all: run
cr : clean run

build: $(OBJ)
	@echo "Linking..."
	$(CXX) $(CFLAGS) $(OBJ) -o $(OUT) 
	

%.o: %.c
	@echo "Compiling..."
	$(CXX) $(CFLAGS) -c $< -o $@  	
	@echo "Compiled:" $@

run: $(OBJ)
	@echo "Linking..."
	$(CXX) $(CFLAGS) $(OBJ) -o $(OUT) 
	@$(OUT)

debug: 
	@echo "Compiling..."
	$(CXX) $(DEBUG) $(CFLAGS) $(INCLUDES) -o $(OUT)

dr:
	@echo "Compiling..."
	$(CXX) $(DEBUG) $(CFLAGS) $(INCLUDES) -o $(OUT)
	@$(OUT)

clean:
	@echo "Deleting..."
	@rm -f $(OBJ) $(OUT) $(TEST) $(CORES)
