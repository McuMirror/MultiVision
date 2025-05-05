#ifndef XE_QTCVUTILS_H
#define XE_QTCVUTILS_H

#include "freetype.hpp"
#include <QByteArray>
#include <QImage>
#include <opencv.hpp>
#include <opencv_modules.hpp>
#include <vector>

namespace Xe_QtCVUtils {

enum UtilsStatus {
    Success = 0, // 成功
    Failure = -1, // 失败
    InvalidInput = -2, // 输入无效
    UnsupportedFormat = -3 // 不支持的格式
};
UtilsStatus qImgToMat(QByteArray& input, cv::Mat& output, int flags);
UtilsStatus qImgToMat(QImage& input, cv::Mat& output);

UtilsStatus matToQImg(cv::Mat& input, QByteArray& output, QImage::Format = QImage::Format_RGB888);
UtilsStatus matToQImg(cv::Mat& input, QImage& output, QImage::Format = QImage::Format_RGB888);
UtilsStatus matToGray(cv::Mat& input, QImage& output);
// filter
UtilsStatus filterAveraging(cv::Mat& src, cv::Mat& dst, unsigned int kernel = 5);
UtilsStatus filterGaussian(cv::Mat& src, cv::Mat& dst, unsigned int kernel = 5, float sigma = 1.5);
UtilsStatus filterBilateral(cv::Mat& src, cv::Mat& dst, int d = 8, double sigmaColor = 75.0, double sigmaSpace = 75.0);

// edge detect
UtilsStatus edgeSobel(cv::Mat& src, cv::Mat& dst, unsigned int kernel = 5);
UtilsStatus edgeCanny(cv::Mat& src, cv::Mat& dst, double threshold1, double threshold2, int apertureSize = 3);

// mat转字符画
UtilsStatus characterDraw(cv::Mat& src, char* dst);

//
UtilsStatus hist(cv::Mat& src, cv::Mat& dst);

UtilsStatus hist(cv::Mat& src, int Thresh1 = -1, int Thresh2 = -1);
UtilsStatus hist_Threshold(cv::Mat& Hist, int Thresh1, int Thresh2);

// 初始化字符映射
extern cv::Ptr<cv::freetype::FreeType2> ft2;
extern std::string font_path;
extern std::vector<char> ASCII_LUT;
UtilsStatus initASCIITable();
UtilsStatus initASCIITable(QString s);
char asciiTable(int c);
UtilsStatus asciiMat(cv::Mat& src, cv::Mat& dst, int fontWidth, int fontheigh, int thickness = -1, cv::LineTypes = cv::LINE_4);
std::vector<std::vector<char>> generateAsciiCharTable(const cv::Mat& input, int font_width, int font_height);
QString generateAsciiQString(const cv::Mat& src, int font_width, int font_height);
void drawTextWithAsciiTable(cv::Ptr<cv::freetype::FreeType2> ft2,
    const std::vector<std::vector<char>>& Text,
    cv::Mat& img,
    cv::Point position,
    int font_width,
    int font_high,
    const cv::Scalar& color = cv::Scalar(255),
    int thickness = 1, int line_type = cv::LINE_4);

void drawTextWithAsciiTable(cv::Ptr<cv::freetype::FreeType2> ft2,
    const std::vector<std::string>& Text,
    cv::Mat& img,
    cv::Point position,
    int font_width,
    int font_high,
    const cv::Scalar& color = cv::Scalar(255),
    int thickness = 1, int line_type = cv::LINE_4);
QString vectorToQString(const std::vector<std::vector<QChar>>& data);
UtilsStatus histogramStretching(cv::Mat& src, cv::Mat& dst, int threshold1, int threshold2);

int __classification(const cv::Mat& canny_edges);
};

#endif // XE_QTCVUTILS_H
