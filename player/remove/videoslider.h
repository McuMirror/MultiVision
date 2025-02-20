#ifndef VIDEOSLIDER_H
#define VIDEOSLIDER_H

#include <QObject>
#include <QSlider>
class VideoSlider : public QSlider {
    Q_OBJECT
public:
    explicit VideoSlider(QWidget* parent = nullptr)
        : QSlider(parent)
    {
        // 可以在这里进行初始化设置
        qDebug() << "VideoSlider Constructor";
    }
};

#endif // VIDEOSLIDER_H
