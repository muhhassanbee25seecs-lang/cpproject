#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <iostream>

using namespace cv;
using namespace cv::face;
using namespace std;

int main() {
    // 1. Load the model
    Ptr<LBPHFaceRecognizer> model = LBPHFaceRecognizer::create();
    try {
        model->read("../model/face_model.xml");
    } catch (...) {
        cout << "Recognized Student ID: Unknown | Confidence: 0.0" << endl;
        return -1;
    }

    // 2. Load the test image (captured by Dialog1)
    Mat img = imread("../test.jpg", IMREAD_GRAYSCALE);
    if (img.empty()) {
        cout << "Recognized Student ID: Unknown" << endl;
        return -1;
    }

    // 3. CRITICAL: Normalize lighting to match the Training data
    equalizeHist(img, img);

    // 4. Detect Face
    CascadeClassifier faceCascade;
    faceCascade.load("../haarcascade_frontalface_default.xml");
    vector<Rect> faces;
    faceCascade.detectMultiScale(img, faces, 1.1, 5);

    if (faces.empty()) {
        cout << "Recognized Student ID: Unknown" << endl;
        return 0;
    }

    // 5. Predict
    Mat face = img(faces[0]);
    resize(face, face, Size(200, 200));

    int label = -1;
    double confidence = 0.0;
    model->predict(face, label, confidence);

    // 6. Output for Dialog1 to read
    if (label != -1) {
        cout << "Recognized Student ID: " << label << endl;
    } else {
        cout << "Recognized Student ID: Unknown" << endl;
    }

    return 0;
}