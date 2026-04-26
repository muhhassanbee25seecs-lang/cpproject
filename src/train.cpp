#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <filesystem>
#include <iostream>

using namespace cv;
using namespace cv::face;
using namespace std;

// Helper to save and return augmented versions of a face
void augmentAndSave(const Mat& face, vector<Mat>& images, vector<int>& labels, int label, string folderPath) {
    // 1. Save and Add Standardized Original
    images.push_back(face);
    labels.push_back(label);
    imwrite(folderPath + "/image_norm.jpg", face);

    // Brightness offsets for variations
    struct Variation { int offset; string name; };
    vector<Variation> vars = {
        {-60, "_vdark"}, {-30, "_dark"}, {30, "_bright"}, {60, "_vbright"}
    };

    for (const auto& v : vars) {
        Mat modified;
        face.convertTo(modified, -1, 1.0, v.offset);
        
        images.push_back(modified);
        labels.push_back(label);

        // Physically save to dataset folder so you see them in VS Code
        imwrite(folderPath + "/image" + v.name + ".jpg", modified);
    }
}

void trainModel(string datasetPath, string modelPath) {
    vector<Mat> images;
    vector<int> labels;
    CascadeClassifier faceCascade;

    if (!faceCascade.load("../haarcascade_frontalface_default.xml")) {
        cout << "❌ Error: Missing Haar Cascade XML!" << endl;
        return;
    }

    for (const auto &dir : filesystem::directory_iterator(datasetPath)) {
        if (!dir.is_directory()) continue;

        int currentLabel = stoi(dir.path().filename().string());
        cout << "🔄 Training ID " << currentLabel << " with 5 lighting profiles..." << endl;

        for (const auto &file : filesystem::directory_iterator(dir.path())) {
            string filePath = file.path().string();
            // ONLY process the original 'image.jpg' to avoid re-processing augmented ones
            if (file.path().filename() != "image.jpg") continue;

            Mat img = imread(filePath, IMREAD_GRAYSCALE);
            if (img.empty()) continue;

            // Normalize lighting
            equalizeHist(img, img);

            vector<Rect> faces;
            faceCascade.detectMultiScale(img, faces, 1.1, 5, 0, Size(50, 50));

            if (!faces.empty()) {
                Mat face = img(faces[0]);
                resize(face, face, Size(200, 200));
                augmentAndSave(face, images, labels, currentLabel, dir.path().string());
            }
        }
    }

    if (images.empty()) return;

    // Use Radius 2 for better lighting tolerance
    Ptr<LBPHFaceRecognizer> model = LBPHFaceRecognizer::create(2, 8, 8, 8, 125.0);
    model->train(images, labels);
    model->save(modelPath);
    cout << "✅ Training Complete. Dataset expanded." << endl;
}

int main() {
    trainModel("../dataset", "../model/face_model.xml");
    return 0;
}