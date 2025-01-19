#include "xe_qtcvutils.h"
#include <QDebug>
using namespace std;
using namespace cv;

Xe_QtCVUtils::Xe_QtCVUtils()
{
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::QImgToMat(QByteArray& input, cv::Mat& output, int flags)
{
    std::vector<uchar> buffer(input.begin(), input.end());
    output = cv::imdecode(buffer, flags);
    if (output.empty())
        return Xe_QtCVUtils::InvalidInput;
    return Xe_QtCVUtils::Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::QImgToMat(QImage& input, cv::Mat& output)
{
    if (input.format() != QImage::Format_RGB888 && input.format() != QImage::Format_ARGB32 && input.format() != QImage::Format_RGB32) {
        qDebug() << input.format() << "..Xe_QtCVUtils: Unsupported QImage format!";
        return Xe_QtCVUtils::InvalidInput;
    }

    if (input.format() == QImage::Format_RGB888) {

        output = cv::Mat(input.height(), input.width(), CV_8UC3, const_cast<uchar*>(input.bits()), input.bytesPerLine());
        cv::cvtColor(output, output, cv::COLOR_RGB2BGR);

    } else if (input.format() == QImage::Format_ARGB32) {
        output = cv::Mat(input.height(), input.width(), CV_8UC4, const_cast<uchar*>(input.bits()), input.bytesPerLine());
        cv::cvtColor(output, output, cv::COLOR_BGRA2BGR);

    } else if (input.format() == QImage::Format_RGB32) {
        output = cv::Mat(input.height(), input.width(), CV_8UC4, const_cast<uchar*>(input.bits()), input.bytesPerLine());
        cv::cvtColor(output, output, cv::COLOR_BGRA2BGR);
    }

    if (output.empty())
        return Xe_QtCVUtils::InvalidInput;

    return Xe_QtCVUtils::Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::MatToQImg(cv::Mat& input, QByteArray& output, QImage::Format)
{
    return Xe_QtCVUtils::Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::MatToQImg(cv::Mat& input, QImage& output, QImage::Format format)
{
    if (input.type() == CV_8UC1)
        output = QImage(input.data, input.cols, input.rows, input.step, QImage::Format_Grayscale8);
    else if (input.type() == CV_8UC3)
        output = QImage(input.data, input.cols, input.rows, input.step, QImage::Format_RGB888);
    else if (input.type() == CV_8UC4)
        output = QImage(input.data, input.cols, input.rows, input.step, QImage::Format_RGBA8888);

    if (output.isNull())
        std::cerr << "MatToQImg fails" << std::endl;

    if (input.type() == CV_8UC3 || input.type() == CV_8UC4)
        output = output.rgbSwapped(); // 将 BGR 转为 RGB

    return Xe_QtCVUtils::Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::FilterAveraging(cv::Mat& src, cv::Mat& dst, unsigned int kernel)
{
    cv::Size kernelSize(kernel, kernel);
    cv::blur(src, dst, kernelSize);
    return Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::FilterGaussian(cv::Mat& src, cv::Mat& dst, unsigned int kernel, float sigma)
{
    cv::Size kernelSize(kernel, kernel);
    cv::GaussianBlur(src, dst, kernelSize, sigma);
    return Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::FilterBilateral(cv::Mat& src, cv::Mat& dst, int d, double sigmaColor, double sigmaSpace)
{
    cv::bilateralFilter(src, dst, d, sigmaColor, sigmaSpace);
    return Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::EdgeSobel(cv::Mat& src, cv::Mat& dst, unsigned int kernel)
{
    cv::Mat sobelX, sobelY;

    if (src.type() == CV_8UC3) {
        cv::Mat grayMat;
        cv::cvtColor(src, grayMat, cv::COLOR_BGR2GRAY);
        cv::Sobel(grayMat, sobelX, CV_8U, 1, 0); // 水平梯度
        cv::Sobel(grayMat, sobelY, CV_8U, 0, 1); // 垂直梯度
    } else {

        cv::Sobel(src, sobelX, CV_8U, 1, 0); // 水平梯度
        cv::Sobel(src, sobelY, CV_8U, 0, 1); // 垂直梯度
    }

    cv::addWeighted(sobelX, 0.5, sobelY, 0.5, 0, dst); // 合并水平和垂直梯度

    return Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::EdgeCanny(cv::Mat& src, cv::Mat& dst, double threshold1, double threshold2, int apertureSize)
{
    cv::Canny(src, dst, threshold1, threshold2, apertureSize);
    return Success;
}

Xe_QtCVUtils::UtilsStatus Xe_QtCVUtils::CharacterDraw(cv::Mat& src, char* dst)
{

    if (src.type() == CV_8UC3) {
        cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
    } else if (src.type() == CV_8UC4) {
        cv::cvtColor(src, src, cv::COLOR_BGRA2GRAY);
    } else if (src.type() == CV_8UC1)
        ;
    else {
        std::cerr << "CharacterDraw InvalidInput!";
        return InvalidInput;
    }
}
