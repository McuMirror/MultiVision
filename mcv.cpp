#include "mcv.h"
#include "qdebug.h"
#include <iostream>
mCv::mCv()
{
    std::cout << "mcv_ok" << std::endl;
}

// void f(const cv::Mat& src, cv::Mat& dst, double threshold1, double threshold2, int apertureSize)
//{
//     //转为灰度图
//     cv::Mat gray;
//     cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

//    //边缘检测
//    cv::Mat canny;
//    cv::Canny(src, canny, threshold1, threshold2, apertureSize);

//    //提取轮廓
//    std::vector<std::vector<cv::Point>> contours;
//    std::vector<cv::Vec4i> hierarchy; // 添加层级参数
//    cv::findContours(canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
//}

void mCv::packageDetect(const cv::Mat& src, cv::Mat& dst, double threshold1, double threshold2, int apertureSize)
{
    /*
     * 1、图像预处理：
     * 转换为灰度图，进行高斯模糊降噪。
     * 使用颜色阈值或HSV空间分割（例如提取包裹的特定颜色范围）。
     * 2、边缘检测与轮廓提取：
     * 通过Canny边缘检测或自适应阈值二值化提取物体边缘。
     * 使用findContours查找所有轮廓。
     * 3、轮廓筛选：
     * 根据面积、长宽比、轮廓近似多边形（approxPolyDP）筛选出符合包裹形状的轮廓。
     * 排除过小或不符合几何特征的干扰区域。
     * 4、结果输出：
     * 在原图上绘制包围矩形，标注数量。
     */
    cv::Mat gray;
    if (src.type() == CV_8UC1)
        gray = src.clone();
    else if (src.type() == CV_8UC3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else if (src.type() == CV_8UC4)
        cv::cvtColor(src, gray, cv::COLOR_BGRA2GRAY);
    else
        return;

    cv::Mat canny;
    cv::Canny(gray, canny, threshold1, threshold2, apertureSize);

    // 膨胀后
    cv::dilate(canny, canny,
        getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)),
        cv::Point(-1, -1), 4);

    // 再腐蚀
    cv::erode(canny, canny,
        getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)),
        cv::Point(-1, -1), 4);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy; // 添加层级参数

    cv::findContours(canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // cv::drawContours(src, contours, -1, cv::Scalar(255, 0, 255), 2);

    if (contours.size() > 0) {
        std::vector<std::vector<cv::Point>> conPoly(contours.size()); // 这干嘛的
        std::vector<cv::Rect> boundRect(contours.size());
        int i = -1;
        for (auto contour : contours) {
            i++;
            auto area = cv::contourArea(contour);
            if (area < -10000 && area < 1005000) {
                cv::RotatedRect minRect = cv::minAreaRect(contour);
                // 获取矩形的四个顶点
                cv::Point2f vertices[4];
                minRect.points(vertices);
                // 绘制最小外接矩形
                for (int j = 0; j < 4; j++) {
                    cv::line(src, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 255, 0), 2);
                }
            } else if (area > 100000) {
                float peri = arcLength(contour, true);
                cv::approxPolyDP(contour, conPoly[i], 0.02 * peri, true); // contour ---》conPoly[i]
                boundRect[i] = cv::boundingRect(conPoly[i]);
                cv::drawContours(src, conPoly, i, cv::Scalar(255, 0, 255), 2);

                cv::Mat warp = getWarp(src, conPoly[i], 500, 800);
                dst = std::move(warp);
                return;
            }
        }
    }
    dst = std::move(src);
}

cv::Mat mCv::getWarp(const cv::Mat& img, std::vector<cv::Point> points, float w, float h)
{
    w = (float)img.size().width;
    h = (float)img.size().height;
    cv::Mat imgWarp;

    // 对于凸四边形的顶点排序
    std::sort(points.begin(), points.end(), [](const auto& a, const auto& b) { return a.y < b.y; });
    if (points[0].x > points[1].x)
        swap(points[0], points[1]);
    if (points[2].x > points[3].x)
        swap(points[2], points[3]);

    cv::Point2f src[4] = { points[0], points[1], points[2], points[3] };

    cv::Point2f dst[4] = {
        { 0.0f, 0.0f },
        { w, 0.0f },
        { 0.0f, h },
        { w, h }
    };

    cv::Mat matrix = getPerspectiveTransform(src, dst);

    cv::warpPerspective(img, imgWarp, matrix, cv::Point(w, h));
    return imgWarp;
}

void mCv::f_sift(cv::Mat src)
{

    // cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
    //  检测关键点和描述符
    //     std::vector<cv::KeyPoint> keypoints;
    //     cv::Mat descriptors;
    //     sift->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);

    // 绘制关键点
    cv::Mat output;
    drawKeypoints(src, template_keypoints, output, cv::Scalar(0, 255, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    // 显示结果
    imshow("SIFT Features", output);
}

void mCv::template_sift_init(cv::Mat template_img)
{
    cv::Mat gray;
    if (template_img.type() == CV_8UC1)
        gray = template_img.clone();
    else if (template_img.type() == CV_8UC3)
        cv::cvtColor(template_img, gray, cv::COLOR_BGR2GRAY);
    else if (template_img.type() == CV_8UC4)
        cv::cvtColor(template_img, gray, cv::COLOR_BGRA2GRAY);
    else
        return;

    template_sift = cv::SIFT::create();

    template_sift->detectAndCompute(gray, cv::noArray(), template_keypoints, template_descriptors);
}

void mCv::sift_detect(cv::Mat img, cv::Mat dst)
{

    cv::Mat gray;
    if (img.type() == CV_8UC1)
        gray = img.clone();
    else if (img.type() == CV_8UC3)
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else if (img.type() == CV_8UC4)
        cv::cvtColor(img, gray, cv::COLOR_BGRA2GRAY);
    else
        return;

    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    sift->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);

    // """KnnMatt获得Top2"""特征匹配（使用FLANN匹配器）

    cv::FlannBasedMatcher matcher;
    std::vector<std::vector<cv::DMatch>> knnMatches;
    matcher.knnMatch(template_descriptors, descriptors, knnMatches, 2);

    // 筛选匹配点（Lowe's ratio test）
    std::vector<cv::DMatch> goodMatches;
    const float ratio_thresh = 0.55f;
    for (auto& match : knnMatches) {
        if (match[0].distance < ratio_thresh * match[1].distance) {
            goodMatches.push_back(match[0]);
        }
    }

    // 至少需要 个匹配点来计算单应性矩阵
    if (goodMatches.size() < 6) {
        // qDebug() << "Matches not enough";
        return;
    }

    // 提取匹配点坐标
    std::vector<cv::Point2f> srcPoints, dstPoints;
    for (auto& m : goodMatches) {
        srcPoints.push_back(template_keypoints[m.queryIdx].pt);
        dstPoints.push_back(keypoints[m.trainIdx].pt);
    }

    // 计算单应性矩阵（使用RANSAC去除异常值）
    qDebug() << "Matches points count: " << srcPoints.size();

    // 归一化坐标到[-1,1]范围
    //    std::vector<cv::Point2f> srcNorm, dstNorm;
    //    cv::Mat T1, T2;

    //    cv::normalize(srcPoints, srcNorm);
    //    cv::normalize(dstPoints, dstNorm);

    cv::Mat H = cv::findHomography(srcPoints, dstPoints, cv::RANSAC, 5.0); // RANSAC算法

    // H = cv::Mat::eye(3, 3, CV_64F);
    if (H.empty() || H.rows != 3 || H.cols != 3) {
        qDebug() << "Homography matrix is invalid!";
        return;
    }
    // 定义模板图的四个角点
    std::vector<cv::Point2f> objCorners(4);
    objCorners[0] = cv::Point2f(20, 20);
    objCorners[1] = cv::Point2f(384, 16);
    objCorners[2] = cv::Point2f(404, 598);
    objCorners[3] = cv::Point2f(33, 600);

    // 将角点映射到目标图像
    std::vector<cv::Point2f> sceneCorners;

    perspectiveTransform(objCorners, sceneCorners, H);

    for (int j = 0; j < H.rows; ++j) {
        qDebug() << H.at<float>(j, 0) << H.at<float>(j, 1) << H.at<float>(j, 2);
    }
    qDebug() << "";
    // 绘制映射后的边界框
    line(img, sceneCorners[0], sceneCorners[1], cv::Scalar(0, 255, 0), 2);
    line(img, sceneCorners[1], sceneCorners[2], cv::Scalar(0, 255, 0), 2);
    line(img, sceneCorners[2], sceneCorners[3], cv::Scalar(0, 255, 0), 2);
    line(img, sceneCorners[3], sceneCorners[0], cv::Scalar(0, 255, 0), 2);
}
