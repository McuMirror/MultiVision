#ifndef VIDEOOPENGLWIDGET_H
#define VIDEOOPENGLWIDGET_H

#include "QOpenGLFunctions"
#include <QObject>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QWidget>
#include <mutex>
extern "C" {
#include "libavcodec/avcodec.h"
}
class VideoOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    VideoOpenGLWidget(QWidget* parent = nullptr);
    ~VideoOpenGLWidget();
    int init_texture(int w, int h);

    // static void m_render();

protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;

private:
    std::mutex m_mutex;
    GLuint m_textureID; // OpenGL texture ID
    int m_frameWidth = 0; // Width of the frame
    int m_frameHeight = 0; // Height of the frame
    bool m_textureCreated = false; // Flag to check if texture has been created
    int m_width;
    int m_height;
    GLuint m_framebuffer;
    // GLuint m_shaderProgram;
    QOpenGLShaderProgram m_Program; // shader程序
private:
    QOpenGLShaderProgram program; // shader程序
    GLuint unis[3] = { 0 }; // shader中yuv变量地址
    GLuint texs[3] = { 0 }; // openg的 texture地址

    // 材质内存空间
    unsigned char* datas[3] = { 0 };
    int width = 240;
    int height = 128;

public slots:
    void paintFrameWithThread(AVFrame* frame);
    void paintFrame(AVFrame* frame);
    // void paintMat(cv::Mat* mat);
};

#endif // VIDEOOPENGLWIDGET_H
