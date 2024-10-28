all: main

main: bin/core.o src/main.cpp 
	@echo "# Compilling main code..."
	g++ -o bin/main bin/core.o src/main.cpp `pkg-config --cflags --libs gtk+-3.0`

bin/core.o: src/core.h src/core.cpp
	@echo "# Compilling core functions..."
	g++ -o bin/core.o -c src/core.cpp `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm bin/*