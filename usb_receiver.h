#ifndef USB_RECEIVER_H
#define USB_RECEIVER_H
#include "detectprocess.h"
#include "xe_qtcvutils.h"
#include "xeimage.h"
#include <QDateTime>
#include <QElapsedTimer>
#include <QList>
#include <QMainWindow>
#include <QScrollBar>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <list>
QT_BEGIN_NAMESPACE
namespace Ui {
class USB_Receiver;
}
QT_END_NAMESPACE

class USB_Receiver : public QMainWindow {
    Q_OBJECT

public:
    USB_Receiver(QWidget* parent = nullptr);
    ~USB_Receiver();
    void processes(cv::Mat& mat);
public slots:
    void processes(); // with m_image

private slots:
    void on_refreshPortsButton_clicked();

    void on_connectButton_clicked();
    void onPortReadyRead();

    void onError(QSerialPort::SerialPortError error);

    void on_pushClear_clicked();
    void on_tickTimeout();
    void on_uvcTimeout();

    void on_detectButton_clicked();

    void on_blurButton_clicked();
    void on_blurKernelBox_valueChanged(int arg1);

    void on_gaussianButton_clicked();
    void on_gaussianKernelSpinBox_valueChanged(int arg1);
    void on_gaussianScrollBar_valueChanged(int value);

    void on_switchInputButton_clicked();

    void on_uvcButton_clicked();

    void on_bilateralDScrollBar_valueChanged(int value);

    void on_bilateralSCScrollBar_valueChanged(int value);

    void on_bilateralSSScrollBar_valueChanged(int value);

    void on_bilateralButton_clicked();

    void on_sobelButton_clicked();

    void on_cannyButton_clicked();

    void on_cannyThreshold1ScrollBar_valueChanged(int value);

    void on_cannyThreshold2ScrollBar_valueChanged(int value);

    void on_actionsave_triggered();

    void on_bilateralSSScrollBar_sliderReleased();

    void on_bilateralSCScrollBar_sliderReleased();

    void on_bilateralDScrollBar_sliderReleased();

    void on_grayButton_clicked();

    void on_histButton_clicked();

    void on_thresholdScrollBar_sliderReleased();

    void on_thresholdScrollBar_2_sliderReleased();

    void on_thresholdButton_clicked();

    void on_asciiArtButton_clicked();

    void on_fontComboBox_currentFontChanged(const QFont& f);

    void on_fontSizeSpinBox_valueChanged(int arg1);

    void on_font_w_valueChanged(int arg1);

    void on_font_h_valueChanged(int arg1);

    void on_pushButton_clicked();

private:
    Ui::USB_Receiver* ui;
    void updatePortList(); // 更新com口列表
    void updateCurrentPortInfo(QString description, QString manufacturer);
    void initFontsList();
    QSerialPort* serialPort;
    QByteArray jpegDataBuffer; //  JPEG

    void updateImage(QImage& img);
    QDateTime currentDateTime = QDateTime::currentDateTime();

    QTimer* tickTimer;
    QTimer* uvcTimer;
    QElapsedTimer jpegFrameTime;
    std::list<uint32_t> bytesList;
    unsigned long bytesSum;
    unsigned int tickTimeout;
    unsigned int uvcTimeout;

    // Xe_QtCVUtils QtCVUtils;
    // QImage m_image;
    XeImage* xm_image;
    QString onnx = "D:/openCV/onnx_model/face_detection_yunet_2023mar.onnx";
    DetectProcess m_detect;

    cv::VideoCapture uvcCap; // uvc class

protected:
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
};

#endif // USB_RECEIVER_H
