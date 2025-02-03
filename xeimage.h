#ifndef XEIMAGE_H
#define XEIMAGE_H

#include <QImage>
#include <QObject>

class XeImage : public QObject, public QImage {
    Q_OBJECT
public:
    explicit XeImage(QObject* parent = nullptr);
    void update(const QImage& newImage);

signals:
    void imageUpdated();
};

#endif // XEIMAGE_H
