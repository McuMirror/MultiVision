#ifndef MCV_H
#define MCV_H

#include <QObject>

#include <opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv_modules.hpp>
class mCv : public QObject {
    Q_OBJECT
public:
    mCv();
    void packageDetect(const cv::Mat& src, cv::Mat& dst, double threshold1, double threshold2, int apertureSize);
    cv::Mat getWarp(const cv::Mat& img, std::vector<cv::Point> points, float w, float h);

    void f_sift(cv::Mat src);
    void template_sift_init(cv::Mat template_img);
    void sift_detect(cv::Mat img, cv::Mat dst);

private:
    cv::Ptr<cv::SIFT> template_sift;
    std::vector<cv::KeyPoint> template_keypoints;
    cv::Mat template_descriptors;
};

#endif // MCV_H
