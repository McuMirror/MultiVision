#include "usb_receiver.h"
#include "ui_usb_receiver.h"
#include "xe_qtcvutils.h"
#include <QMessageBox>
#include <opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect/face.hpp>
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

    connect(tickTimer, &QTimer::timeout, this, &USB_Receiver::on_tickTimeout);

    connect(serialPort, &QSerialPort::readyRead, this, &USB_Receiver::onPortReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred, this, &USB_Receiver::onError);

    tickTimeout = 500;
    tickTimer->start(tickTimeout);
    uvcTimeout = 30;
    uvcTimer->start(uvcTimeout);
    updatePortList();

    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
    QWidget::setAcceptDrops(true);
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

void USB_Receiver::proceses()
{
    if (m_image.isNull())
        return;
    // QImage ..->..Mat
    cv::Mat mat;

    if (QtCVUtils.QImgToMat(m_image, mat) != 0)
        return;
    proceses(mat);
}

void USB_Receiver::proceses(cv::Mat& mat)
{
    // ----begin with mat----

    if (ui->blurButton->isChecked()) {

        Xe_QtCVUtils::FilterAveraging(mat, mat, ui->blurKernelBox->value());
        // qDebug() << "averagingFilter success!";
    }

    if (ui->gaussianButton->isChecked()) {

        Xe_QtCVUtils::FilterGaussian(mat, mat, ui->gaussianKernelSpinBox->value(), ui->gaussianScrollBar->value() / 10);

        // qDebug() << "gaussianFilter success!";
    }

    if (ui->bilateralButton->isChecked()) {

        // 必须为 CV_8UC1 或 CV_8UC3
        // qDebug() << mat.type() == cv::Formatted;
        // 双边滤波器不允许原地修改
        cv::Mat tmp = std::move(mat);

        Xe_QtCVUtils::FilterBilateral(tmp, mat,
            ui->bilateralDScrollBar->value(),
            ui->bilateralSCScrollBar->value(),
            ui->bilateralSSScrollBar->value());
        // qDebug() << "bilateralFilter success!";
    }
    if (ui->sobelButton->isChecked()) {
        cv::Mat tmp = std::move(mat);

        Xe_QtCVUtils::EdgeSobel(tmp, mat, 5);

        //        qDebug() << "sobel sucess : " << mat.size().height
        //                 << "*" << mat.size().width
        //                 << "*" << mat.channels();

        // cv::imshow("sobel", mat);
    }

    if (ui->cannyButton->isChecked()) {
        cv::Mat tmp = std::move(mat);
        Xe_QtCVUtils::EdgeCanny(tmp, mat,
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

    // ----end with mat----

    QImage disp;
    int ret = QtCVUtils.MatToQImg(mat, disp); // Mat ..->..QImage

    ui->imageWidget->updatePix(disp);
}

void USB_Receiver::updateImage(QImage& img)
{
    if (!img.isNull()) {
        m_image = img;
        proceses();
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
            m_detect.initialization(onnx, m_image.size(), 0.8);
        proceses();

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
    int byte_len = data.size();
    // ui->plainTextEdit->appendPlainText(QString::number(data.size()));

    if (data.contains(QByteArray::fromHex("FFD8FF"))) {
        // 如果发现开始标志，清空缓存并加入新数据
        jpegDataBuffer.clear();
        jpegDataBuffer.append(data);
    } else if (!jpegDataBuffer.isEmpty()) {
        jpegDataBuffer.append(data); // 如果缓存不为空，但没有发现开始标志，继续追加数据
    }

    int endPos = data.indexOf(QByteArray::fromHex("FFD9"));
    if (endPos != -1) {
        // 只保留结束标志之前的数据
        QByteArray completeData = jpegDataBuffer + data.left(endPos + 2);

        // 调用渲染函数显示完整 JPEG 数据
        if (completeData.size() > 1000) {
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
            m_image = QImage::fromData(completeData);
            m_image.mirror(true, true);
            proceses();
            ui->lcdBytes->display(QString::number(completeData.size()));
        }

        // 清理缓存，丢弃已经处理过的数据
        jpegDataBuffer.clear();
        // 丢弃结束标志后的数据
        data.remove(0, endPos + 2);
    }
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

    proceses(frame);
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
        if (!m_image.isNull()) {
            m_detect.initialization(onnx, m_image.size(), 0.8);
            proceses();
        }

    } else {

        m_detect.enable = false;
    }

    //
}

void USB_Receiver::on_blurButton_clicked()
{
    proceses();
}
void USB_Receiver::on_blurKernelBox_valueChanged(int arg1)
{
    proceses();
}
void USB_Receiver::on_gaussianButton_clicked()
{
    proceses();
}

void USB_Receiver::on_gaussianKernelSpinBox_valueChanged(int arg1)
{
    proceses();
}
void USB_Receiver::on_gaussianScrollBar_valueChanged(int value)
{
    ui->gaussianSigmaLabel->setText(QString("sg:") + QString::number(value / 10.0, 'f', 1));
    proceses();
}

void USB_Receiver::on_bilateralButton_clicked()
{
    proceses();
}
void USB_Receiver::on_bilateralDScrollBar_valueChanged(int value)
{

    ui->bilateralDLabel->setText(QString("d:") + QString::number(value));
}
void USB_Receiver::on_bilateralDScrollBar_sliderReleased()
{
    proceses();
}

void USB_Receiver::on_bilateralSCScrollBar_valueChanged(int value)
{
    ui->bilateralSCLabel->setText(QString("SC:") + QString::number(value));
}
void USB_Receiver::on_bilateralSCScrollBar_sliderReleased()
{
    proceses();
}

void USB_Receiver::on_bilateralSSScrollBar_valueChanged(int value)
{
    ui->bilateralSSLabel->setText(QString("SS:") + QString::number(value));
}

void USB_Receiver::on_bilateralSSScrollBar_sliderReleased()
{
    proceses();
}

void USB_Receiver::on_sobelButton_clicked()
{
    proceses();
}

void USB_Receiver::on_cannyButton_clicked()
{
    proceses();
}

void USB_Receiver::on_cannyThreshold1ScrollBar_valueChanged(int value)
{
    ui->cannyThreshold1Label->setText(QString("th1:") + QString::number(value));
    if (ui->cannyButton->isChecked())
        proceses();
}

void USB_Receiver::on_cannyThreshold2ScrollBar_valueChanged(int value)
{
    ui->cannyThreshold2Label->setText(QString("th2:") + QString::number(value));
    if (ui->cannyButton->isChecked())
        proceses();
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
