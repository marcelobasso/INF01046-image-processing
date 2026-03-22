OPENCV_LIBS=-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_video

all: main

main: src/main.cpp
	g++ -O2 -o main src/main.cpp `pkg-config --cflags opencv4` $(OPENCV_LIBS)
