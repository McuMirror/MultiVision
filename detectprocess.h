#ifndef DETECTPROCESS_H
#define DETECTPROCESS_H

#include <QImage>
#include <opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect/face.hpp>
class DetectProcess {
public:
    DetectProcess();
    ~DetectProcess();
    int initialization(QString onnxPath, const QSize input_size, float score_threshold = 0.75f);

    int faceDetect(cv::Mat& input, cv::Mat& faces);
    int visualize(cv::Mat& input, cv::Mat& faces, int thickness = 1);
    bool enable = false;
    QSize getInputSize();

private:
    cv::Ptr<cv::FaceDetectorYN> faceDetector;
};

#endif // DETECTPROCESS_H
