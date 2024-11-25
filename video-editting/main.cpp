#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace std;
using namespace cv;
#define MAX_SLIDER 100
#define SRC "Source Window"
#define DST "Destiny Window"

typedef map<char, pair<bool, float>> OperationMap;

// Globals
int slider_val = 0;
int last_pressed = -1;

// Key-Value Mapping
const map<int, char> key_values = {
    {27, 'E'}, {71, 'G'}, {103, 'G'}, {66, 'B'}, {98, 'B'}, {67, 'C'}, {99, 'C'}, 
    {83, 'S'}, {115, 'S'}, {90, 'Z'}, {122, 'Z'}, {82, 'R'}, {114, 'R'}, {72, 'H'}, 
    {104, 'H'}, {86, 'V'}, {118, 'V'}, {73, 'I'}, {105, 'I'}, {79, 'O'}, {111, 'O'}, 
    {78, 'N'}, {110, 'N'}, {32, ' '}, {84, 'T'}, {116, 'T'} // restart
};

// Slider Callback
void onSliderChange(int value, void*) {
    slider_val = value;
}

// Reset all operation flags and values
void resetOperations(OperationMap &operations) {
    bool recording = operations['T'].first;
    for (auto &op : operations) {
        op.second = make_pair(false, 0);
    }
    operations['R'].second = -1; // Correction factor
    operations['T'].first = recording;
}

// Resizes the output window for zoom operations
void applyZoom(OperationMap &operations) {
    float scale = pow(2, operations['Z'].second);
    resizeWindow(DST, 600 / scale, 500 / scale);
}

void printOptions() {
    cout << "Options:" << endl;
    cout << "  G: Convert to Grayscale" << endl;
    cout << "  B: Apply Gaussian Blur" << endl;
    cout << "  C: Apply Canny Edge Detection" << endl;
    cout << "  S: Apply Sobel Operator" << endl;
    cout << "  Z: Zoom Out by 2x" << endl;
    cout << "  R: Rotate Image" << endl;
    cout << "  H: Flip Horizontally" << endl;
    cout << "  V: Flip Vertically" << endl;
    cout << "  I: Increase Brightness" << endl;
    cout << "  O: Change Contrast" << endl;
    cout << "  N: Negative Image" << endl;
    cout << "  T: Record Video" << endl;
    cout << "  Space: Reset" << endl;
}

// Set operations based on user input
void handleKeyInput(OperationMap &operations, int key) {
    if (key_values.find(key) != key_values.end()) {
        char optionKey = key_values.at(key);

        switch (optionKey) {
            case ' ':
                resetOperations(operations);
                resizeWindow(DST, 600, 500); // Reset window size
                break;
            case 'Z':
                operations['Z'].first = true;
                operations['Z'].second += 1;
                applyZoom(operations);
                break;
            case 'R':
                operations['R'].first = true;
                operations['R'].second += 1;
                break;
            case 'T':
                operations[optionKey].first = !operations[optionKey].first;
                operations['T'].second += 1;
                if (operations[optionKey].first)
                    cout << "Recording video..." << endl;
                else
                    cout << "Video saved!" << endl;
                break;
            default:
                operations[optionKey].first = !operations[optionKey].first;
                operations[optionKey].second = slider_val;
        }
    }
}

// Runs the enabled operations
void processImage(OperationMap &operations, int slider, Mat &src, Mat &dst, VideoWriter &video) {
    float aux;
    int ksize;
    Mat src_gray, detected_edges, grad_x, grad_y, abs_grad_x, abs_grad_y;

    if (operations['G'].first || operations['S'].first) {
        cvtColor(dst, dst, COLOR_BGR2GRAY);
    }

    if (operations['B'].first) {
        ksize = (operations['B'].second * 2) + 1; // guarantees odd values only
        if (key_values.at(last_pressed) == 'B') {
            ksize = (slider * 2) + 1; // guarantees odd values only
            operations['B'].second = slider;
        }

        GaussianBlur(dst, dst, Size(ksize, ksize), 0);
    }

    if (operations['C'].first) {
        ksize = (operations['C'].second * 2) + 1;
        if (key_values.at(last_pressed) == 'C') {
            ksize = (slider * 2) + 1;
            operations['C'].second = slider;
        }

        cvtColor(dst, src_gray, COLOR_BGR2GRAY);
        GaussianBlur(src_gray, detected_edges, Size(ksize, ksize), 0);
        Canny(detected_edges, detected_edges, 150, 450, 5);
        dst = cv::Scalar::all(0);
        src_gray.copyTo(dst, detected_edges);
    }

    if (operations['S'].first) {
        ksize = operations['S'].second;
        if (key_values.at(last_pressed) == 'S') {
            ksize = (slider * 2) + 1;
            ksize = ksize > 30 ? 31 : ksize;
            operations['S'].second = ksize;
        }

        GaussianBlur(dst, src_gray, Size(ksize, ksize), 0);
        Sobel(src_gray, grad_x, CV_64F, 1, 0, ksize, 1, 0, BORDER_DEFAULT);
        Sobel(src_gray, grad_y, CV_64F, 0, 1, ksize, 1, 0, BORDER_DEFAULT);
        convertScaleAbs(grad_x, abs_grad_x);
        convertScaleAbs(grad_y, abs_grad_y);
        addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);
    }

    if (operations['I'].first) {
        aux = operations['I'].second;
        if (key_values.at(last_pressed) == 'I')
            aux = operations['I'].second = slider;
        dst.convertTo(dst, -1, 1, slider);
    }

    if (operations['O'].first) {
        aux = operations['O'].second;
        if (key_values.at(last_pressed) == 'O')
            aux = operations['O'].second = 1 + (slider / 50.0f);
        dst.convertTo(dst, -1, aux, 0);
    }

    if (operations['N'].first) {
        dst = Scalar::all(255) - dst;
    }

    if (operations['H'].first) {
        flip(dst, dst, 1); // Horizontal flip
    }

    if (operations['V'].first) {
        flip(dst, dst, 0); // Vertical flip
    }

    if (operations['R'].first) {
        if ((int) operations['R'].second % 4 != 3) {
            rotate(dst, dst, (int) operations['R'].second % 4);
        }
    }

    if (operations['T'].first) {
        if (dst.channels() == 1) {
            cvtColor(dst, dst, COLOR_GRAY2BGR);
        }
        video.write(dst);
    } else if (operations['T'].second > 0) {
        video.release();
    }
}

// Main Function
int main(int argc, char** argv) {
    int camera = 0;
    
    // Define the codec and create VideoWriter object.The output is stored in 'outcpp.avi' file.
    VideoCapture cap(camera);
    if (!cap.isOpened()) {
        cerr << "Error: Unable to open the camera.\n";
        return -1;
    }
    // Default resolutions of the frame are obtained.The default resolutions are system dependent.
    int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(CAP_PROP_FPS);
    VideoWriter video("output_video.avi", VideoWriter::fourcc('M','J','P','G'), fps, Size(frame_width,frame_height));

    if (!video.isOpened()) {
        cerr << "Error: VideoWriter failed to initialize." << endl;
        return -1;
    }

    // Operation Map Initialization
    OperationMap operations = {
        {'G', {false, 0}}, {'B', {false, 0}}, {'C', {false, 0}}, {'S', {false, 0}}, 
        {'Z', {false, 0}}, {'R', {false, -1}}, {'H', {false, 0}}, {'V', {false, 0}}, 
        {'I', {false, 0}}, {'O', {false, 0}}, {'N', {false, 0}}, {'T', {false, 0}}
    };

    // UI Initialization
    namedWindow(SRC, WINDOW_GUI_NORMAL);
    namedWindow(DST, WINDOW_GUI_NORMAL);
    createTrackbar("Input", DST, nullptr, MAX_SLIDER, onSliderChange);
    printOptions(); 
    resizeWindow(DST, 600, 500);

    while (last_pressed != 27) { // Escape key to exit
        Mat frame, editing_frame;
        cap >> frame;
        if (frame.empty()) break; // End of video stream
        frame.copyTo(editing_frame);

        int key = waitKey(1);
        if (key > 0 && key_values.find(key) != key_values.end()) {
            last_pressed = key;
        }

        handleKeyInput(operations, key);
        processImage(operations, slider_val, frame, editing_frame, video);

        imshow(SRC, frame);
        imshow(DST, editing_frame);
    }

    cap.release();
    return 0;
}
