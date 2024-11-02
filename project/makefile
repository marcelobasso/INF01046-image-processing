all: main

main: bin/core.o src/main.cpp 
	@echo "# Compilling main code..."
	g++ -o photoshop bin/utils.o bin/core.o src/main.cpp `pkg-config --cflags --libs gtk+-3.0`

bin/core.o: bin/utils.o src/core.h src/core.cpp
	@echo "# Compilling core functions..."
	g++ -o bin/core.o -c src/core.cpp `pkg-config --cflags --libs gtk+-3.0`

bin/utils.o: src/utils.h src/utils.cpp
	@echo "# Compilling utils..."
	g++ -o bin/utils.o -c src/utils.cpp `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm bin/*
	rm photoshop