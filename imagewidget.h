#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H
#include <QBuffer>
#include <QByteArray>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPainter>
#include <QPixmap>
#include <QWidget>

namespace Ui {
class ImageWidget;
}

class ImageWidget : public QWidget {
    Q_OBJECT

public:
    ImageWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        QWidget::setAcceptDrops(true);
    }

    void updatePix(QImage dispImage)
    {
        // m_pixmap = QPixmap::fromImage(dispImage).scaled(size().width(), size().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_pixmap = QPixmap::fromImage(dispImage); // 转换为 QPixmap
        update(); // 触发控件重绘
    }
    QPixmap getPixmap()
    {
        return m_pixmap;
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        if (!m_pixmap.isNull()) {
            painter.drawPixmap(0, 0, m_pixmap.scaled(size().width(), size().height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

private:
    QPixmap m_pixmap; // 存储显示的图像
};

#endif // IMAGEWIDGET_H
