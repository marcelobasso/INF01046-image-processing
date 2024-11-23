#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace cv;

map<int, char> key_values = {
    {27, 'E'}, // escape
    {73, 'G'}, // grayscale
    {103, 'G'},
    {66, 'B'},  // blur
    {98, 'B'},     
    {67, 'C'}, // canny
    {99, 'C'}, 
    {83, 'S'}, // sobel  
    {115, 'S'}, 
    {90, 'Z'}, // zoom_out
    {122, 'Z'}, 
    {88, 'R'}, // rotation
    {120, 'R'}, 
    {72, 'H'}, // mirror_h
    {104, 'H'}, 
    {86, 'V'}, // mirror_v
    {118, 'V'}, 
    {65, 'B'}, // brightness
    {97, 'B'}, 
    {79, 'O'}, // contrast
    {111, 'O'}, 
    {78, 'N'}, // negative
    {110, 'N'},
    {32, ' '} // restart
};

#define MAX_SLIDER 50
#define SRC "Source Window"
#define DST "Destiny Window"

int slider_val = 0;
void onSliderChange(int value, void*) {
    slider_val = value;
}

// set option in case something is clicked
int setOption(map<char, pair<bool, float>> &options, int key) {
    // Check if the key exists in key_values
    if (key_values.find(key) != key_values.end()) {
        char optionKey = key_values.at(key); // Get corresponding option key ('G', 'B', etc.)
        
        // Update the options map
        if (key_values.at(key) == ' ') {
            for (map<char, pair<bool, float>>::iterator it = options.begin(); it != options.end(); ++it) {
                options[it->first] = make_pair(false, 0);
            }
        } else if (options.find(optionKey) != options.end()) {
            options[optionKey] = make_pair(true, slider_val);
        }
    }

    return key;
}

void runOptions(map<char, pair<bool, float>> &options, int slider, int last_pressed, Mat &src, Mat &dst) {
    int ksize;
    Mat src_gray, detected_edges;
    int lowThreshold = 150;
    const int maxThreshold = 200;
    const int ratio = 3;

    if (options['G'].first) {
        cvtColor(dst, dst, COLOR_BGR2GRAY);
    }

    if (options['B'].first) {
        if (key_values.at(last_pressed) == 'B') {
            ksize = (slider * 2) + 1; // guarantees odd values only
            options['B'].second = slider;
        } else {
            ksize = (options['B'].second * 2) + 1; // guarantees odd values only
        }

        GaussianBlur(dst, dst, Size(ksize, ksize), 0);
    }

    if (options['C'].first) {
        if (key_values.at(last_pressed) == 'C') {
            ksize = (slider * 2) + 1;
            options['C'].second = slider;
        } else {
            ksize = (options['C'].second * 2) + 1;
        }

        cvtColor(dst, src_gray, cv::COLOR_BGR2GRAY);  // Convert to grayscale
        GaussianBlur(src_gray, detected_edges, Size(ksize, ksize), 0);
        Canny(detected_edges, detected_edges, lowThreshold, lowThreshold * ratio, 5);
        dst = cv::Scalar::all(0);
        src_gray.copyTo(dst, detected_edges);
    }
} 

int main(int argc, char** argv) {
    int camera = 0;
    VideoCapture cap;

    map<char, pair<bool, float>> options = {
        {'G', make_pair(false, 0)}, // grayscale
        {'B', make_pair(false, 0)}, // blur     
        {'C', make_pair(false, 0)}, // canny
        {'S', make_pair(false, 0)}, // sobel  
        {'Z', make_pair(false, 0)}, // zoom_out
        {'R', make_pair(false, 0)}, // rotation
        {'H', make_pair(false, 0)}, // mirror_h
        {'V', make_pair(false, 0)}, // mirror_v
        {'B', make_pair(false, 0)}, // brightness
        {'O', make_pair(false, 0)}, // contrast
        {'N', make_pair(false, 0)}  // negative
    };
    std::vector<char> keys;
    int last_pressed, set_result;

    // creates array with all the options
    for (map<char, pair<bool, float>>::iterator it = options.begin(); it != options.end(); ++it) {
        keys.push_back(it->first);
    }

    if (!cap.open(camera)) return 0;
    while (last_pressed != 27) {
        Mat frame, editing_frame;
        cap >> frame;
        cap >> editing_frame;

        namedWindow(SRC, WINDOW_GUI_NORMAL);
        namedWindow(DST, WINDOW_GUI_NORMAL);
        resizeWindow(DST, 600, 500);
        createTrackbar("Input", DST, nullptr, MAX_SLIDER, onSliderChange);
        if (frame.empty()) break; // end of video stream

        set_result = setOption(options, waitKey(1));
        last_pressed = set_result > 0 ? set_result : last_pressed;
        runOptions(options, slider_val, last_pressed, frame, editing_frame);

        imshow(SRC, frame);
        imshow(DST, editing_frame);
    }

    cap.release(); // release the VideoCapture object

    return 0;
}
