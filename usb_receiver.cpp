#include "usb_receiver.h"
// #include "ft2build.h"
#include "ui_usb_receiver.h"
#include "xe_qtcvutils.h"

#include <QMessageBox>
#include <QRandomGenerator>
#include <freetype.hpp>
// #include FT_FREETYPE_H
#include <QElapsedTimer>
#include <opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv_modules.hpp>

using namespace cv;

USB_Receiver::USB_Receiver(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::USB_Receiver)
{
    ui->setupUi(this);

    ui->groupBox_1->layout()->setContentsMargins(0, 0, 0, 0); // 去掉边框
    ui->groupBox_2->layout()->setContentsMargins(0, 0, 0, 0); // 去掉边框
    serialPort = new QSerialPort(this);
    tickTimer = new QTimer(this); // stm32-usb虚拟串口接收速率计算
    uvcTimer = new QTimer(this); // UVC 刷新帧

    xm_image = new XeImage();
    connect(tickTimer, &QTimer::timeout, this, &USB_Receiver::on_tickTimeout);

    connect(serialPort, &QSerialPort::readyRead, this, &USB_Receiver::onPortReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred, this, &USB_Receiver::onError);

    connect(xm_image, &XeImage::imageUpdated, this, qOverload<>(&USB_Receiver::processes));

    tickTimeout = 500;
    tickTimer->start(tickTimeout);
    uvcTimeout = 30;
    uvcTimer->start(uvcTimeout);
    updatePortList();
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    initFontsList();

    qDebug() << ui->asciiTextBrowser->font().pointSize();

    cv::setUseOptimized(false); // OPENCV禁用优化,否则 BGR->YUV转不了，不知道为什么

#define TEST_BLOCK 0
#if TEST_BLOCK

    QWidget::setAcceptDrops(true);

    for (int i = 20; i < 127; i += 10) {
        char character = static_cast<char>(i);
        std::string text(1, character); // 转换为字符串
        cv::Size textSize = cv::getTextSize(text, 2, 1, 1, nullptr);
        double width = textSize.width;
        double height = textSize.height;
        double aspectRatio = width / height;

        // std::cout << "Width: " << width << ", Height: " << height << ", Aspect Ratio: " << aspectRatio << " :" << text << std::endl;
    }

    cv::Mat img = cv::Mat::zeros(600, 600, CV_8UC3); //"D:\arial.ttf" JetBrainsMono-Regular
    img.setTo(cv::Scalar(100, 255, 100)); //

    std::string font_path = "D:/JetBrainsMono-Regular.ttf";
    cv::Ptr<cv::freetype::FreeType2> ft2 = cv::freetype::createFreeType2();
    ft2->loadFontData(font_path, 0);

    // 设置文本位置和颜色
    cv::Point position(0, 50);
    cv::Scalar color(120, 120, 255); // 白色

    // 使用指定字体绘制文本
    int thickness = -1;
    int font_size = 16; // 设置字体大小
    int font_width = 10; // 设置字体大小
    for (int l = 0; l < 20; l++) {
        QString lineText;
        int randomNumber = QRandomGenerator::global()->bounded('A', 'z');
        for (int i = 0; i < 20; i++) {

            lineText.append(QChar(randomNumber));
        }

        //        auto tsize = ft2->getTextSize(lineText.toStdString(), font_size, thickness, 0);
        //        qDebug() << l << "(" << tsize.height << "," << tsize.width << ")";

        position.y += font_size;
        vector<string> stringText = { lineText.toStdString() };
        Xe_QtCVUtils::drawTextWithAsciiTable(ft2, stringText, img, position, font_width, font_size, color, thickness, LINE_8);
    }

    cv::imshow("Text with TTF Font", img);

#endif
}
USB_Receiver::~USB_Receiver()
{
    delete ui;
}

void USB_Receiver::on_refreshPortsButton_clicked()
{
    updatePortList();
}

/*
 * 更新可选端口列表
 * 如果当前的端口在更新后依然存在，则继续选中
 */
void USB_Receiver::updatePortList()
{
    /*
     *  portInfo.portName()
     *  portInfo.systemLocation()
     *  portInfo.description()
     *  portInfo.manufacturer()  // Manufacturer
     *  portInfo.serialNumber()  // Serial number
     */
    auto currentPort = ui->portComboBox->currentText();
    bool flag = false;
    ui->portComboBox->clear();
    foreach (const QSerialPortInfo& portInfo, QSerialPortInfo::availablePorts()) {
        ui->portComboBox->addItem(portInfo.portName());
        if (currentPort == portInfo.portName())
            flag = true;
        qDebug() << "Port:" << portInfo.portName() << "\n"
                 << "Description:" << portInfo.description() << "\n"
                 << "Manufacturer:" << portInfo.manufacturer() << "\n";
    }
    if (flag)
        ui->portComboBox->setCurrentText(currentPort);
}

void USB_Receiver::on_connectButton_clicked()
{
    if (ui->portComboBox->isEnabled()) {
        // 尝试连接
        QString portName = ui->portComboBox->currentText();
        serialPort->setPortName(portName);

        if (serialPort->open(QIODevice::ReadOnly)) {
            statusBar()->showMessage("已连接到串口 " + portName);
            ui->connectButton->setText(tr("disconnect")); // 禁用连接按钮
            ui->portComboBox->setEnabled(false);
            QSerialPortInfo info(portName); // 创建指定串口的 QSerialPortInfo 对象
            updateCurrentPortInfo(info.description(), info.manufacturer());
            onPortReadyRead();
        } else {
            statusBar()->showMessage("连接失败");
        }
    } else {
        serialPort->close();
        statusBar()->showMessage("断开连接");

        ui->connectButton->setText(tr("connect"));
        ui->portComboBox->setEnabled(true);
    }
}

void USB_Receiver::updateCurrentPortInfo(QString description, QString manufacturer)
{
    ui->label_Description->setText(description);
    ui->label_Manufacturer->setText(manufacturer);
}

void USB_Receiver::initFontsList()
{

    QString exeDir = QCoreApplication::applicationDirPath();
    QDir::setCurrent(exeDir);
    qDebug() << QDir::currentPath();
    statusBar()->showMessage(QDir::currentPath());
    QDir dir("fonts");
    if (!dir.exists()) {
        qWarning() << "Folder does not exist:";
        statusBar()->showMessage("Fonts folder does not exist");
        return;
    } else {
        statusBar()->showMessage("Fonts folder load success");
        // 遍历文件夹中的所有文件
        QStringList fontFiles = dir.entryList(QStringList() << "*.ttf"
                                                            << "*.otf",
            QDir::Files);
        for (const QString& fontFile : fontFiles) {
            QString fontPath = dir.filePath(fontFile);
            int fontId = QFontDatabase::addApplicationFont(fontPath);
            if (fontId == -1) {
                qWarning() << "Failed to load font:" << fontPath;
            } else {
                qDebug() << "Loaded font:" << fontPath;
            }
        }
    }
}

void USB_Receiver::processes()
{
    if (xm_image->isNull())
        return;
    // QImage ..->..Mat
    cv::Mat mat;

    if (Xe_QtCVUtils::qImgToMat(*xm_image, mat) != 0)
        return;
    processes(mat);
}

void USB_Receiver::processes(cv::Mat& mat)
{
    static int width;
    static int height;

    int _W = mat.size().width; // 768
    int _H = mat.size().height; // 576
    int _DIMS = mat.dims; // 2
    auto _TYPE = mat.type(); // 16

    cv::Mat yuvMat;
    cv::cvtColor(mat, yuvMat, cv::COLOR_BGR2YUV_I420); // 不关闭优化的话会出错

    if (mat.size().width != width || height != mat.size().height) {
        ui->openGLWidget->init_texture(mat.size().width, mat.size().height);
        qDebug() << "init_texture finished ~";
    }
    width = mat.size().width;
    height = mat.size().height;
    // ----begin with mat----

    if (ui->blurButton->isChecked()) {

        Xe_QtCVUtils::filterAveraging(mat, mat, ui->blurKernelBox->value());
        // qDebug() << "averagingFilter success!";
    }

    if (ui->gaussianButton->isChecked()) {

        Xe_QtCVUtils::filterGaussian(mat, mat, ui->gaussianKernelSpinBox->value(), ui->gaussianScrollBar->value() / 10);

        // qDebug() << "gaussianFilter success!";
    }

    if (ui->bilateralButton->isChecked()) {

        // 必须为 CV_8UC1 或 CV_8UC3
        // qDebug() << mat.type() == cv::Formatted;
        // 双边滤波器不允许原地修改
        cv::Mat tmp = std::move(mat);

        Xe_QtCVUtils::filterBilateral(tmp, mat,
            ui->bilateralDScrollBar->value(),
            ui->bilateralSCScrollBar->value(),
            ui->bilateralSSScrollBar->value());
        qDebug() << "bilateralFilter success!";
    }
    if (ui->sobelButton->isChecked()) {
        cv::Mat tmp = std::move(mat);

        Xe_QtCVUtils::edgeSobel(tmp, mat, 5);

        //        qDebug() << "sobel sucess : " << mat.size().height
        //                 << "*" << mat.size().width
        //                 << "*" << mat.channels();

        // cv::imshow("sobel", mat);
    }

    // 灰度图/阈值分割
    if (ui->grayButton->isChecked()) {
        cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
        if (ui->histButton->isChecked())
            Xe_QtCVUtils::hist(mat,
                ui->thresholdScrollBar->value(),
                ui->thresholdScrollBar_2->value());
    }

    if (ui->thresholdButton->isChecked()) {
        Xe_QtCVUtils::histogramStretching(mat, mat, ui->thresholdScrollBar->value(), ui->thresholdScrollBar_2->value());
    }
    // 分割
    if (ui->cannyButton->isChecked()) {
        cv::Mat tmp = std::move(mat);
        Xe_QtCVUtils::edgeCanny(tmp, mat,
            ui->cannyThreshold1ScrollBar->value(),
            ui->cannyThreshold2ScrollBar->value());
        if (mat.empty())
            std::cerr << "canny fail";
        else {
            //            qDebug() << "canny sucess : " << mat.size().height
            //                     << "*" << mat.size().width
            //                     << "*" << mat.channels();

            // cv::imshow("canny", mat);
        }
    }
    if (ui->radioButton->isChecked())
        cv::bitwise_not(mat, mat);

    //  字符画
    QElapsedTimer _ascii_start;

    if (ui->asciiArtButton->isChecked()) {
        static QString asciiQString;
        asciiQString.clear();
        asciiQString = Xe_QtCVUtils::generateAsciiQString(mat, ui->font_w->value(), ui->font_h->value());
        _ascii_start.start();

        // 如果用QTextBrowser，需要消耗几十毫秒；用QPlainTextEdit更快，<1毫秒，因为不需要HTML富文本
        ui->asciiTextBrowser->setPlainText(asciiQString);
        //  Xe_QtCVUtils::asciiMat(mat, mat);
        //  qDebug() << "asciiMat done";
        return;
    }
    // qDebug() << "asciiArt 花费时间" << _ascii_start.elapsed();

    if (m_detect.enable) {
        // 判断模型的尺寸与输入图像的图像是否一致
        if (m_detect.getInputSize().width() != mat.size().width || m_detect.getInputSize().height() != mat.size().height) {
            QSize detectSize(mat.size().width, mat.size().height);
            m_detect.initialization(onnx, detectSize, 0.8);
        }

        cv::Mat face;
        m_detect.faceDetect(mat, face);
        m_detect.visualize(mat, face, 2);
    }

    ui->openGLWidget->paintMat(&mat);
    //  ----end with mat----

    QImage disp;
    int ret = Xe_QtCVUtils::matToQImg(mat, disp); // Mat ..->..QImage

    ui->imageWidget->updatePix(disp);
}

void USB_Receiver::updateImage(QImage& img)
{
    if (!img.isNull()) {
        xm_image->update(img);
    }
}

void USB_Receiver::dropEvent(QDropEvent* event)
{
    event->acceptProposedAction();

    if (event->mimeData()->hasUrls()) // 如果拖放的是url地址
    {
        //        QElapsedTimer timer;

        //        int msc;
        QString imgPath = event->mimeData()->urls()[0].toLocalFile(); // 接收第一个url地址，并转换为LocalFile格式
        QFileInfo imgPathInfo;
        QMimeDatabase db;

        QMimeType mime = db.mimeTypeForFile(imgPath, QMimeDatabase::MatchContent);
        QByteArray format = mime.preferredSuffix().toLatin1();

        if (QImage(imgPath, format).isNull())
            return;
        QImage&& _img = QImage(imgPath);
        updateImage(_img);

        qDebug() << "Drop Image Suscess";

        if (m_detect.enable == true)
            m_detect.initialization(onnx, xm_image->size(), 0.8);
        processes();

    } else
        event->ignore();
    event->accept();
}

void USB_Receiver::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
}
// 拖动并离开窗口
void USB_Receiver::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}
/*
 * 读取串口传来的信息并解码为jpeg图像
 * 开头为FFD8FF
 * 结尾为FFD9
 */
void USB_Receiver::onPortReadyRead()
{

    QByteArray data = serialPort->readAll();

    // ui->plainTextEdit->appendPlainText(QString::number(data.size()));

    if (data.contains(QByteArray::fromHex("FFD8FF"))) {
        // 如果发现开始标志，清空缓存并加入新数据
        jpegFrameTime.start();
        jpegDataBuffer.clear();
        jpegDataBuffer.append(data);
        statusBar()->showMessage("find \" FFD8FF \"");

    } else if (!jpegDataBuffer.isEmpty()) {
        jpegDataBuffer.append(data); // 如果缓存不为空，但没有发现开始标志，继续追加数据
    }

    int endPos = data.indexOf(QByteArray::fromHex("FFD9"));
    if (endPos != -1) {
        // 只保留结束标志之前的数据
        QByteArray completeData = jpegDataBuffer + data.left(endPos + 2);
        statusBar()->showMessage("find \" FFD9 \"");

        // 调用渲染函数显示完整 JPEG 数据
        if (completeData.size() > 1000) {
            // qDebug() << "time:" << jpegFrameTime.elapsed();
            // qDebug() << "size:" << completeData.size();
            // qDebug() << "speed:" << 1000 * (completeData.size() / 1024) / (jpegFrameTime.elapsed());
            bytesSum += completeData.size();
#ifdef WRITE_IMG

            // 创建 QFile 对象
            QFile file("JPEG_DATA_" + currentDateTime.toString("MM_dd_hh_mm_ss") + ".jpg");

            // 打开文件，准备写入
            if (file.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
                // 写入 QByteArray 数据
                QDateTime currentDateTime = QDateTime::currentDateTime();
                qDebug() << "Current Date and Time: " << currentDateTime.toString();
                file.write(data);
                file.close();
                qDebug() << "Data written to file successfully.";
            }

#endif
            xm_image->update(QImage::fromData(completeData));
            // m_image.mirror(true, true);

            ui->lcdBytes->display(QString::number(completeData.size()));
        } else {

            statusBar()->showMessage("completeData.size() < 500");
        }

        // 清理缓存，丢弃已经处理过的数据
        jpegDataBuffer.clear();
        // 丢弃结束标志后的数据
        data.remove(0, endPos + 2);
    } else if (jpegDataBuffer.size() > 1024 * 1024)
        jpegDataBuffer.clear();
}

void USB_Receiver::onError(QSerialPort::SerialPortError error)
{
    qDebug() << error;
    if (error == (QSerialPort::DeviceNotFoundError | QSerialPort::ResourceError | QSerialPort::TimeoutError)) {

        serialPort->close();
        statusBar()->showMessage("断开连接");

        ui->connectButton->setText(tr("connect"));
        ui->portComboBox->setEnabled(true);
    }
}

void USB_Receiver::on_pushClear_clicked()
{
}
// 每tickTimeout毫秒统计一次
// 在这段时间接收了bytesSum字节
// 取最大15次内的平均
void USB_Receiver::on_tickTimeout()
{

    bytesList.push_back(bytesSum);
    if (bytesList.size() > 10)
        bytesList.pop_front();
    double bytesListSum = 0.0;
    for (auto b : bytesList) {
        bytesListSum += b;
    }
    double bytesListMean = bytesListSum / bytesList.size();
    double byteSpeed = bytesListMean / (tickTimeout / 1000.0); //  单位时间内的比特数/单位时间(S)

    if (byteSpeed < 1)
        byteSpeed = 0;
    ui->lcdBytesSpeed->display(QString::number(byteSpeed / 1024.0, 'f', 1));
    bytesSum = 0;
    //
}

void USB_Receiver::on_uvcTimeout()
{

    cv::Mat frame;
    uvcCap >> frame; // 从 uvcCap 中读取一帧并存储到 frame 中

    if (frame.empty()) {
        return;
    }

    processes(frame);
}
void USB_Receiver::on_switchInputButton_clicked()
{
    int widgetNumb = ui->stackedWidget->count();
    int currentNumb = ui->stackedWidget->currentIndex();
    currentNumb++;
    if (currentNumb >= widgetNumb)
        currentNumb = 0;
    ui->stackedWidget->setCurrentIndex(currentNumb);
}

void USB_Receiver::on_uvcButton_clicked()
{
    if (ui->uvcButton->isChecked()) {
        // 关闭usbd的连接
        if (serialPort->isOpen()) {
            serialPort->close();
            statusBar()->showMessage("断开连接");

            ui->connectButton->setText(tr("connect"));
            ui->portComboBox->setEnabled(true);
        }
        // 打开UVC
        if (!uvcCap.open(0)) { // 0 表示设备 0
            std::cerr << "Error: Unable to open camera." << std::endl;
            return;
        }
        statusBar()->showMessage("已连接到uvc");

        connect(uvcTimer, &QTimer::timeout, this, &USB_Receiver::on_uvcTimeout);
    } else {
        uvcCap.release();
        statusBar()->showMessage("断开连接");
        disconnect(uvcTimer, &QTimer::timeout, this, &USB_Receiver::on_uvcTimeout);
    }
}
void USB_Receiver::on_detectButton_clicked()
{

    if (ui->detectButton->isChecked()) {
        m_detect.enable = true;
        if (!xm_image->isNull()) {
            m_detect.initialization(onnx, xm_image->size(), 0.8);
            processes();
        }

    } else {

        m_detect.enable = false;
    }

    //
}

void USB_Receiver::on_blurButton_clicked()
{
    processes();
}
void USB_Receiver::on_blurKernelBox_valueChanged(int arg1)
{
    processes();
}
void USB_Receiver::on_gaussianButton_clicked()
{
    processes();
}

void USB_Receiver::on_gaussianKernelSpinBox_valueChanged(int arg1)
{
    processes();
}
void USB_Receiver::on_gaussianScrollBar_valueChanged(int value)
{
    ui->gaussianSigmaLabel->setText(QString("sg:") + QString::number(value / 10.0, 'f', 1));
    processes();
}

void USB_Receiver::on_bilateralButton_clicked()
{
    processes();
}
void USB_Receiver::on_bilateralDScrollBar_valueChanged(int value)
{

    ui->bilateralDLabel->setText(QString("d:") + QString::number(value));
}
void USB_Receiver::on_bilateralDScrollBar_sliderReleased()
{
    processes();
}

void USB_Receiver::on_bilateralSCScrollBar_valueChanged(int value)
{
    ui->bilateralSCLabel->setText(QString("SC:") + QString::number(value));
}
void USB_Receiver::on_bilateralSCScrollBar_sliderReleased()
{
    processes();
}

void USB_Receiver::on_bilateralSSScrollBar_valueChanged(int value)
{
    ui->bilateralSSLabel->setText(QString("SS:") + QString::number(value));
}

void USB_Receiver::on_bilateralSSScrollBar_sliderReleased()
{
    processes();
}

void USB_Receiver::on_sobelButton_clicked()
{
    processes();
}

void USB_Receiver::on_cannyButton_clicked()
{
    processes();
}

void USB_Receiver::on_cannyThreshold1ScrollBar_valueChanged(int value)
{
    ui->cannyThreshold1Label->setText(QString("th1:") + QString::number(value));
    if (ui->cannyButton->isChecked())
        processes();
}

void USB_Receiver::on_cannyThreshold2ScrollBar_valueChanged(int value)
{
    ui->cannyThreshold2Label->setText(QString("th2:") + QString::number(value));
    if (ui->cannyButton->isChecked())
        processes();
}

void USB_Receiver::on_actionsave_triggered()
{
    currentDateTime = QDateTime::currentDateTime();

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Image"), currentDateTime.toString("MM_dd_hh_mm_ss"), tr("Images (*.png *.jpg *.bmp)"));

    QPixmap&& tmp = ui->imageWidget->getPixmap();

    if (!filePath.isEmpty()) {
        // 保存文件
        if (tmp.save(filePath)) {
            QMessageBox::information(this, tr("Success"), tr("Image saved successfully!"));
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to save the image."));
        }
    }
}

void USB_Receiver::on_grayButton_clicked()
{
    processes();
}

void USB_Receiver::on_histButton_clicked()
{
}

void USB_Receiver::on_thresholdScrollBar_sliderReleased()
{
    processes();
}

void USB_Receiver::on_thresholdScrollBar_2_sliderReleased()
{
    processes();
}

void USB_Receiver::on_thresholdButton_clicked()
{
    processes();
}

void USB_Receiver::on_asciiArtButton_clicked()
{

    if (ui->asciiArtButton->isChecked()) {

        Xe_QtCVUtils::initASCIITable();
        ui->displayStacked->setCurrentIndex(1);
    } else {
        ui->displayStacked->setCurrentIndex(0);
    }

    processes();
}

void USB_Receiver::on_fontComboBox_currentFontChanged(const QFont& f)
{

    // ui->asciiTextBrowser->setPlainText(ui->asciiTextBrowser->toPlainText()); // 清除 HTML 格式
    ui->asciiTextBrowser->setFont(f);
}

void USB_Receiver::on_fontSizeSpinBox_valueChanged(int arg1)
{
    auto currentFont = ui->asciiTextBrowser->font();
    currentFont.setPixelSize(arg1);

    ui->asciiTextBrowser->setFont(currentFont);
}

void USB_Receiver::on_font_w_valueChanged(int arg1)
{
    processes();
}

void USB_Receiver::on_font_h_valueChanged(int arg1)
{
    processes();
}

void USB_Receiver::on_pushButton_clicked()
{
    static int displayStackedMaxIndex = ui->displayStacked->count();

    int i = ui->displayStacked->currentIndex();
    qDebug() << "max index is :" << displayStackedMaxIndex << " , now index is : " << i;
    ui->displayStacked->setCurrentIndex((++i) % displayStackedMaxIndex);
}
