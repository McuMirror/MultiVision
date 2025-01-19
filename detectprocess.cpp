#include "detectprocess.h"
#include <QDebug>
#define MAX_LENGTH 6000
DetectProcess::DetectProcess()
{
}

DetectProcess::~DetectProcess()
{
    faceDetector.release();
}

int DetectProcess::initialization(QString onnxPath, const QSize input_size, float score_threshold)
{

    cv::Size initSize = cv::Size(input_size.width(), input_size.height());

    if (fmax(initSize.height, initSize.width) > MAX_LENGTH) {
        double factor = (double)MAX_LENGTH / (double)fmax(initSize.height, initSize.width);
        qDebug() << factor;
        if (initSize.height >= initSize.width) {

            initSize.width = initSize.width * factor;
            initSize.height = MAX_LENGTH;
        } else {
            initSize.height = initSize.height * factor;
            initSize.width = MAX_LENGTH;
        }
    }

    qDebug() << "model size :" << QSize(initSize.width, initSize.height);
    faceDetector = cv::FaceDetectorYN::create(onnxPath.toStdString(), "", initSize, score_threshold);
    if (faceDetector.empty()) {
        std::cerr << "Failed to load the face detection model!" << std::endl;
        return -1;
    }
    enable = true;
    qDebug() << "DetectProcess initia sucess ";
    return 0;
}

int DetectProcess::faceDetect(cv::Mat& input, cv::Mat& faces)
{
    if (input.type() == CV_8UC1)
        cv::cvtColor(input, input, cv::COLOR_GRAY2BGR); // 转换为三通道图像
    faceDetector->detect(input, faces);
    return 0;
}

int DetectProcess::visualize(cv::Mat& input, cv::Mat& faces, int thickness)
{
    if (input.empty())
        return -1;

    // faces是一个nx15的二维Mat，每一行分别是：
    // [x1, y1, w, h, x_re, y_re, x_le, y_le, x_nt, y_nt, x_rcm, y_rcm, x_lcm, y_lcm, score]
    // 其中，x1, y1是人脸框左上角坐标，w和h分别是人脸框的宽和高；
    //      {x, y}_{re, le, nt, rcm, lcm}分别是人脸右眼瞳孔、左眼瞳孔、鼻尖、右嘴角和左嘴角的坐标；
    //       score是该人脸的得分。
    for (int i = 0; i < faces.rows; i++) {
        // Print results
        qDebug() << "Face " << i
                 << ", top-left coordinates: (" << faces.at<float>(i, 0) << ", " << faces.at<float>(i, 1) << "), "
                 << "box width: " << faces.at<float>(i, 2) << ", box height: " << faces.at<float>(i, 3) << ", "
                 << "score: " << cv::format("%.2f", faces.at<float>(i, 14));

        // Draw bounding box
        rectangle(input, cv::Rect2i(int(faces.at<float>(i, 0)), int(faces.at<float>(i, 1)), int(faces.at<float>(i, 2)), int(faces.at<float>(i, 3))), cv::Scalar(0, 255, 0), thickness);

        // Draw landmarks
        circle(input, cv::Point2i(int(faces.at<float>(i, 4)), int(faces.at<float>(i, 5))), 2, cv::Scalar(255, 0, 0), thickness);
        circle(input, cv::Point2i(int(faces.at<float>(i, 6)), int(faces.at<float>(i, 7))), 2, cv::Scalar(0, 0, 255), thickness);
        circle(input, cv::Point2i(int(faces.at<float>(i, 8)), int(faces.at<float>(i, 9))), 2, cv::Scalar(0, 255, 0), thickness);
        circle(input, cv::Point2i(int(faces.at<float>(i, 10)), int(faces.at<float>(i, 11))), 2, cv::Scalar(255, 0, 255), thickness);
        circle(input, cv::Point2i(int(faces.at<float>(i, 12)), int(faces.at<float>(i, 13))), 2, cv::Scalar(0, 255, 255), thickness);
    }
    return 0;
}

QSize DetectProcess::getInputSize()
{
    if (faceDetector.empty()) {
        return QSize(-1, -1);
    }
    return QSize(faceDetector.get()->getInputSize().width, faceDetector.get()->getInputSize().height);
}
