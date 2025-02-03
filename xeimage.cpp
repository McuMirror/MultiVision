#include "xeimage.h"
XeImage::XeImage(QObject* parent)
    : QObject(parent)
    , QImage()
{
}
void XeImage::update(const QImage& newImage)
{

    // 更新图像
    QImage::operator=(newImage);
    emit imageUpdated();
}
