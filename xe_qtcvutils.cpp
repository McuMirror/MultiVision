#include "xe_qtcvutils.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QString>
#include <freetype.hpp>
#include <immintrin.h>

using namespace cv;

#define FONT_WIDTH 10
#define FONT_HIGH 16

namespace Xe_QtCVUtils {
std::vector<char> ASCII_LUT;
QString asc_table_qstring = " .:-=+*#%@&MWXOB@";
cv::Ptr<cv::freetype::FreeType2> ft2;
std::string font_path = "D:/JetBrainsMono-Regular.ttf";
static std::vector<int> IMAGEASCII_LUT(255);

UtilsStatus qImgToMat(QByteArray& input, cv::Mat& output, int flags)
{
    std::vector<uchar> buffer(input.begin(), input.end());
    output = cv::imdecode(buffer, flags);
    if (output.empty())
        return Xe_QtCVUtils::InvalidInput;
    return Xe_QtCVUtils::Success;
}

UtilsStatus qImgToMat(QImage& input, cv::Mat& output)
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

UtilsStatus matToQImg(cv::Mat& input, QByteArray& output, QImage::Format)
{

    return Xe_QtCVUtils::Success;
}

UtilsStatus matToQImg(cv::Mat& input, QImage& output, QImage::Format format)
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

UtilsStatus matToGray(cv::Mat& input, QImage& output)
{
    return Success;
}

UtilsStatus filterAveraging(cv::Mat& src, cv::Mat& dst, unsigned int kernel)
{
    cv::Size kernelSize(kernel, kernel);
    cv::blur(src, dst, kernelSize);
    return Success;
}

UtilsStatus filterGaussian(cv::Mat& src, cv::Mat& dst, unsigned int kernel, float sigma)
{
    cv::Size kernelSize(kernel, kernel);
    cv::GaussianBlur(src, dst, kernelSize, sigma);
    return Success;
}
// 双边滤波
UtilsStatus filterBilateral(cv::Mat& src, cv::Mat& dst, int d, double sigmaColor, double sigmaSpace)
{
    cv::bilateralFilter(src, dst, d, sigmaColor, sigmaSpace);

    return Success;
}

UtilsStatus edgeSobel(cv::Mat& src, cv::Mat& dst, unsigned int kernel)
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

UtilsStatus edgeCanny(cv::Mat& src, cv::Mat& dst, double threshold1, double threshold2, int apertureSize)
{
    cv::Canny(src, dst, threshold1, threshold2, apertureSize);
    return Success;
}

UtilsStatus characterDraw(cv::Mat& src, char* dst)
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
    return Success;
}

UtilsStatus hist(cv::Mat& src, cv::Mat& dst)
{
    return hist(src);
}

UtilsStatus hist(cv::Mat& src, int Thresh1, int Thresh2)
{
    if (src.type() != CV_8UC1) {
        return Failure;
    }

    int histSize = 256; // 灰度级别数量（0-255）
    float range[] = { 0, 256 }; // 灰度范围
    const float* histRange = { range };

    cv::Mat hist;
    cv::calcHist(&src, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

    // 创建显示直方图的图像
    int histWidth = 512, histHeight = 400;
    int binWidth = cvRound((double)histWidth / histSize);
    cv::Mat histImage(histHeight, histWidth, CV_8UC1, cv::Scalar(255));

    // 归一化直方图
    cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX);

    cv::cvtColor(histImage, histImage, COLOR_GRAY2BGR);
    // 绘制直方图
    for (int i = 1; i < histSize; i++) {
        line(histImage, cv::Point(binWidth * (i - 1), histHeight - cvRound(hist.at<float>(i - 1))),
            cv::Point(binWidth * i, histHeight - cvRound(hist.at<float>(i))),
            cv::Scalar(0), 2, 8, 0);
    }

    // 阈值范围の线
    hist_Threshold(histImage, Thresh1, Thresh2);

    // 显示直方图
    cv::imshow("Grayscale Histogram", histImage);

    return Success;
}

UtilsStatus hist_Threshold(cv::Mat& Hist, int Thresh1, int Thresh2)
{
    int histHeight = Hist.size().height;
    int histWidth = Hist.size().width;

    if (Thresh1 >= 0 && Thresh1 < histHeight) {
        int x = Thresh1 * histWidth / 255;
        line(Hist, cv::Point(x, 0), cv::Point(x, histHeight), cv::Scalar(0, 0, 255));
        std::string text = cv::format("%d", Thresh1);
        cv::putText(Hist, text, cv::Point(x, 30), 1, 1, cv::Scalar(0, 100, 155));
    }

    if (Thresh2 >= 0 && Thresh2 < histHeight) {
        int x = Thresh2 * histWidth / 255;
        line(Hist, cv::Point(x, 0), cv::Point(x, histHeight), cv::Scalar(0, 0, 255));
        std::string text = cv::format("%d", Thresh2);
        cv::putText(Hist, text, cv::Point(x, 30), 1, 1, cv::Scalar(100, 100, 155));
    }
    return Success;
}
// 直方图拉伸
UtilsStatus histogramStretching(cv::Mat& src, cv::Mat& dst, int threshold1, int threshold2)
{

    if (src.type() != CV_8UC1)
        return InvalidInput;
    if (threshold1 > threshold2)
        return InvalidInput;
    cv::Mat&& _src = std::move(src);

    // 遍历每个像素进行拉伸
    for (int i = 0; i < _src.rows; ++i) {
        for (int j = 0; j < _src.cols; ++j) {
            uchar pixel = _src.at<uchar>(i, j);

            if (pixel >= threshold1 && pixel <= threshold2) {
                // 按公式拉伸到新范围
                dst.at<uchar>(i, j) = cv::saturate_cast<uchar>(
                    (pixel - threshold1) * (255 - 0) / (threshold2 - threshold1));
            } else {
                // 超出原范围的像素直接设置为 0 或其他值
                dst.at<uchar>(i, j) = 0;
            }
        }
    }
    return Success;
}
UtilsStatus initASCIITable()
{

    if (!ASCII_LUT.empty())
        return Success;
    for (auto c : asc_table_qstring.toStdString()) {
        ASCII_LUT.push_back(c);
    }
    int index = 0;
    for (int i = 0; i < 255; i++) {
        index = round(((255 - i) / 255.0) * (ASCII_LUT.size() - 1));
        IMAGEASCII_LUT[i] = ASCII_LUT[index];
    }

    if (ft2.empty()) {
        ft2 = cv::freetype::createFreeType2();
        ft2->loadFontData(font_path, 0);
    }
    return Success;
}
UtilsStatus asciiMat(cv::Mat& src, cv::Mat& dst, int fontWidth, int fontheigh, int thickness, cv::LineTypes)
{

    // 如果没有初始化，初始化look up映射表
    if (ASCII_LUT.size() == 0)
        initASCIITable();

    auto&& ret = generateAsciiCharTable(src, fontWidth, fontheigh);

    std::vector<std::vector<char>> AscTable = std::move(ret);

    drawTextWithAsciiTable(ft2, AscTable, dst, cv::Point(0, 0), fontWidth, fontheigh);

    return Success;
}

// 把c∈[0,255]映射到[0,ASCII_LUT.size()-1]
char asciiTable(int c)
{
    // 0~255 -> 17
    if (c > 255)
        c = 255;

    auto tmp = (int)((c / 255.0) * ASCII_LUT.size());
    tmp = ASCII_LUT.size() - tmp - 1;
    return ASCII_LUT[tmp];
}
std::vector<std::vector<char>> generateAsciiCharTable(const cv::Mat& src, int font_width, int font_height)
{

    // 1. 转为灰度图
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src;
    }

    // 2. 创建积分图

    cv::Mat integralImg;
    cv::integral(gray, integralImg, CV_32S);

    // 3. 定义输出ASCII二维数组
    int rows = gray.rows / font_height;
    int cols = gray.cols / font_width;

    std::vector<std::vector<char>> asciiArt(rows, std::vector<char>(cols));

    // 4. 遍历计算每个块的均值
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int x1 = j * font_width;
            int y1 = i * font_height;
            int x2 = x1 + font_width;
            int y2 = y1 + font_height;

            // 从积分图中获取区域和
            int sum = integralImg.at<int>(y2, x2)
                - integralImg.at<int>(y1, x2)
                - integralImg.at<int>(y2, x1)
                + integralImg.at<int>(y1, x1);

            // 计算均值
            int avg = sum / (font_width * font_height);

            // 映射到字符集

            asciiArt[i][j] = IMAGEASCII_LUT[avg];
        }
    }
    return asciiArt;

    //    for (auto line : asciiArt) {
    //        for (auto c : line)
    //            std::cout << c;
    //         std::cout << std::endl;
    //    }
}
QString generateAsciiQString(const cv::Mat& src, int font_width, int font_height)
{

    // 1. 转为灰度图
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src;
    }

    // 2. 创建积分图
    cv::Mat integralImg;
    QElapsedTimer intergralTime;
    intergralTime.start();
    cv::integral(gray, integralImg, CV_32S);
    // qDebug() << "积分图耗时：" << intergralTime.elapsed();
    // 3. 定义输出ASCII QString
    int rows = gray.rows / font_height;
    int cols = gray.cols / font_width;
    unsigned int _size = rows * cols + rows;
    QString AsciiQString;
    AsciiQString.resize(_size);

    // 4. 遍历计算每个块的均值
    int index = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int x1 = j * font_width;
            int y1 = i * font_height;
            int x2 = x1 + font_width;
            int y2 = y1 + font_height;

            // 从积分图中获取区域和
            int sum = integralImg.at<int>(y2, x2)
                - integralImg.at<int>(y1, x2)
                - integralImg.at<int>(y2, x1)
                + integralImg.at<int>(y1, x1);

            // 计算均值
            int avg = sum / (font_width * font_height);

            // 映射到字符集
            char tmp = IMAGEASCII_LUT[avg];
            AsciiQString[index++] = tmp;
        }
        AsciiQString[index++] = '\n'; // 换行
    }
    return AsciiQString;

    //    for (auto line : asciiArt) {
    //        for (auto c : line)
    //            std::cout << c;
    //         std::cout << std::endl;
    //    }
}
QString vectorToQString(const std::vector<std::vector<QChar>>& data)
{
    QString result;

    // 预分配内存以减少重新分配的次数
    size_t totalSize = 0;
    for (const auto& row : data) {
        totalSize += row.size() + 1; // +1 用于换行符
    }
    result.reserve(totalSize);

    // 遍历每一行并追加到 QString
    for (const auto& row : data) {

        result.append(row.data(), row.size());
        result.append('\n');
    }

    return result;
}
/*
 * ft2->putText 非常慢，当绘制2000*1000 with 16*10字体的时候，即200*60+个字符
 * 需要800毫秒
 * 建议主要作为文本输出而不是渲染到图像上
 */
void drawTextWithAsciiTable(cv::Ptr<cv::freetype::FreeType2> ft2, const std::vector<std::vector<char>>& Text, cv::Mat& dst, cv::Point position, int font_width, int font_high, const cv::Scalar& color, int thickness, int line_type)
{

    cv::Point pos = position;
    // dst = cv::Mat::zeros(dst.size().width, dst.size().height, CV_8UC3); //"D:\arial.ttf" JetBrainsMono-Regular

    QElapsedTimer timer;
    timer.start();

    for (const std::vector<char>& lineText : Text) {
        std::string lineString;
        pos.y += font_high;
        for (const char& ch : lineText) {
            lineString.push_back(ch);
        }
        ft2->putText(dst, lineString, pos, font_high, color, thickness, line_type, true);
    }
    qDebug() << "Elapsed time:" << timer.elapsed() << "ms";
}

void drawTextWithAsciiTable(cv::Ptr<cv::freetype::FreeType2> ft2, const std::vector<std::string>& Text, cv::Mat& dst, cv::Point position, int font_width, int font_high, const cv::Scalar& color, int thickness, int line_type)
{
    cv::Point pos = position;
    for (const std::string& lineText : Text) {
        for (const char& ch : lineText) {
            ft2->putText(dst, std::string(1, ch), pos, font_high, color, thickness, line_type, true);
            pos.x += font_width; // 更新 X 坐标，增加固定偏移
        }
        pos.x = position.x;
        pos.y += font_high;
    }
}
}
