#include <opencv2/opencv.hpp>
#include <stdio.h>

using namespace cv;

#define MAX_SLIDER 50
#define SRC "Source Window"
#define DST "Destiny Window"
#define ESC 27
#define G_LOW 103
#define G_HIGH 73
#define B_LOW 98
#define B_HIGH 66
#define C_LOW 99
#define C_HIGH 67

int slider_val = 0;

void onSliderChange(int value, void*) {
    slider_val = value;
}

// set option in case something is clicked
void setOption(int &option, int key) {
    std::cout << key << std::endl;

    switch (key) {
        case ESC:
            option = ESC;
            break;

        case G_LOW:
        case G_HIGH:
            option = G_HIGH;
            break;

        case B_LOW:
        case B_HIGH:
            option = B_HIGH;
            break;

        case C_LOW:
        case C_HIGH:
            option = C_HIGH;
            break;
    }
}

void runOption(int option, int slider, Mat &src, Mat &dst) {
    int ksize;
    Mat src_gray, detected_edges;
    int lowThreshold = 150;
    const int maxThreshold = 200;
    const int ratio = 3;

    switch (option) {
        // convert to grayscale
        case G_HIGH:
            cvtColor(src, dst, COLOR_BGR2GRAY);
            break;

        // apply Gaussian kernel of informed size
        case B_HIGH:
            ksize = (slider * 2) + 1; // guarantees odd values only
            GaussianBlur(src, dst, Size(ksize, ksize), 0);
            break;

        // Applies Canny
        // https://insightfultscript.com/collections/programming/cpp/opencv/canny-edge-detection/
        case C_HIGH:
            ksize = (slider * 2) + 1;
            cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);  // Convert to grayscale
            GaussianBlur(src_gray, detected_edges, Size(ksize, ksize), 0);
            Canny(detected_edges, detected_edges, lowThreshold, lowThreshold * ratio, 5);
            dst = cv::Scalar::all(0);
	        src_gray.copyTo(dst, detected_edges);
            break;
    }
} 

int main(int argc, char** argv) {
    int camera = 0;
    VideoCapture cap;
    int option = 0, key;
    if (!cap.open(camera)) return 0;

    while (option != ESC) {
        Mat frame, editing_frame;
        cap >> frame;
        cap >> editing_frame;

        namedWindow(SRC, WINDOW_GUI_NORMAL);
        namedWindow(DST, WINDOW_GUI_NORMAL);
        resizeWindow(DST, 600, 500);
        createTrackbar("Input", DST, nullptr, MAX_SLIDER, onSliderChange);
        if (frame.empty()) break; // end of video stream

        setOption(option, waitKey(1));
        runOption(option, slider_val, frame, editing_frame);

        imshow(SRC, frame);
        imshow(DST, editing_frame);
    }

    cap.release(); // release the VideoCapture object

    return 0;
}
