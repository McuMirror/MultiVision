#ifndef XE_QTCVUTILS_H
#define XE_QTCVUTILS_H

#include <QByteArray>
#include <QImage>
#include <opencv.hpp>

class Xe_QtCVUtils {
public:
    Xe_QtCVUtils();

    enum UtilsStatus {
        Success = 0, // 成功
        Failure = -1, // 失败
        InvalidInput = -2, // 输入无效
        UnsupportedFormat = -3 // 不支持的格式
    };
    static UtilsStatus QImgToMat(QByteArray& input, cv::Mat& output, int flags);
    static UtilsStatus QImgToMat(QImage& input, cv::Mat& output);

    static UtilsStatus MatToQImg(cv::Mat& input, QByteArray& output, QImage::Format = QImage::Format_RGB888);
    static UtilsStatus MatToQImg(cv::Mat& input, QImage& output, QImage::Format = QImage::Format_RGB888);

    // filter
    static UtilsStatus FilterAveraging(cv::Mat& src, cv::Mat& dst, unsigned int kernel = 5);
    static UtilsStatus FilterGaussian(cv::Mat& src, cv::Mat& dst, unsigned int kernel = 5, float sigma = 1.5);
    static UtilsStatus FilterBilateral(cv::Mat& src, cv::Mat& dst, int d = 8, double sigmaColor = 75.0, double sigmaSpace = 75.0);

    // edge detect
    static UtilsStatus EdgeSobel(cv::Mat& src, cv::Mat& dst, unsigned int kernel = 5);
    static UtilsStatus EdgeCanny(cv::Mat& src, cv::Mat& dst, double threshold1, double threshold2, int apertureSize = 3);

    // mat转字符画
    static UtilsStatus CharacterDraw(cv::Mat& src, char* dst);
};

#endif // XE_QTCVUTILS_H
