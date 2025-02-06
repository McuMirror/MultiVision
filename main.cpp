#include "usb_receiver.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    USB_Receiver w;
    w.show();
    return a.exec();
}
