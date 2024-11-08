SRC_PATH = src
BIN_PATH = bin
GTK_PKG = `pkg-config --cflags --libs gtk+-3.0`
FLAGS = -O2

all: main

main: $(BIN_PATH)/core.o $(SRC_PATH)/main.cpp 
	@echo "# Compilling main code..."
	g++ -o photoshop $(FLAGS) $(BIN_PATH)/utils.o $(BIN_PATH)/core.o $(SRC_PATH)/main.cpp $(GTK_PKG)

$(BIN_PATH)/core.o: $(BIN_PATH)/utils.o $(SRC_PATH)/core.h $(SRC_PATH)/core.cpp
	@echo "# Compilling core functions..."
	g++ -o $(BIN_PATH)/core.o -c $(FLAGS) $(SRC_PATH)/core.cpp $(GTK_PKG)

$(BIN_PATH)/utils.o: $(SRC_PATH)/utils.h $(SRC_PATH)/utils.cpp
	@echo "# Compilling utils..."
	g++ -o $(BIN_PATH)/utils.o -c $(FLAGS) $(SRC_PATH)/utils.cpp $(GTK_PKG)

clean:
	rm $(BIN_PATH)/*
	rm photoshop
