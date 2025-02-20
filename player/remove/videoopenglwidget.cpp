#include "videoopenglwidget.h"
#include "gl/GL.h"
#include <QThread>
#include <thread>

#define GET_STR(x) #x
#define A_VER 3
#define T_VER 4
static void m_render()
{

    glBegin(GL_TRIANGLES);
    glColor3b(10, 50, 60);
    constexpr int n = 100;
    constexpr float pi = 3.14159265;
    float radius = 0.5f;
    for (int i = 0; i < n; i++) {
        float angle = i / (float)n * pi * 2;
        float angle_next = (i + 1) / (float)n * pi * 2;
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(radius * sinf(angle), radius * cosf(angle), 0.0f);
        glVertex3f(radius * sinf(angle_next), radius * cosf(angle_next), 0.0f);
    }
    glEnd();
    qDebug() << "m_render finished";
}

const char* vertexShaderSource = R"(
    #version 130
    in vec2 vertexIn;
    in vec2 textureIn;
    out vec2 texCoord;

    void main() {
        gl_Position = vec4(vertexIn, 0.0, 1.0);
        texCoord = textureIn;
    }
)";

const char* fragmentShaderSource = R"(
    #version 130
    in vec2 texCoord;
    out vec4 fragColor;
    uniform sampler2D tex_rgb;

    void main() {
        fragColor = texture(tex_rgb, texCoord);
    }
)";

// 顶点shader
const char* vString = GET_STR(

    in vec4 vertexIn;
    in vec2 textureIn;
    varying vec2 textureOut;
    void main(void) {
        gl_Position = vertexIn;
        textureOut = textureIn;
    });

// 片元shader
const char* tString = GET_STR(

    varying vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;
    out vec4 fragColor;
    void main(void) {
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(tex_y, textureOut).r;
        yuv.y = texture2D(tex_u, textureOut).r - 0.5;
        yuv.z = texture2D(tex_v, textureOut).r - 0.5;
        rgb = mat3(1.0, 1.0, 1.0, 0.0, -0.3455, 1.779, 1.4075, -0.7169, 0.0)
            * yuv;
        fragColor = vec4(rgb, 1.0);
    });

VideoOpenGLWidget::VideoOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , QOpenGLFunctions()
{
    // initializeOpenGLFunctions();
}

VideoOpenGLWidget::~VideoOpenGLWidget()
{
}

/*
 * opengl窗口初始化和texture初始化分开
 * 窗口初始化在第一次运行时调用一次
 * texture初始化在图像尺寸改变时调用
 */
// 需要获取图像的大小来确定纹理空间
int VideoOpenGLWidget::init_texture(int width, int height)
{
    qDebug() << "VideoOpenGLWidget::init_texture....";
    std::unique_lock<std::mutex> guard(m_mutex);
    this->width = width;
    this->height = height;
    delete datas[0];
    delete datas[1];
    delete datas[2];
    // 分配纹理内存空间
    datas[0] = new unsigned char[width * height]; // Y
    datas[1] = new unsigned char[width * height / 4]; // U
    datas[2] = new unsigned char[width * height / 4]; // V

    if (texs[0]) {
        glDeleteTextures(3, texs);
    }
    // 创建纹理
    glGenTextures(3, texs);

    // Y
    glBindTexture(GL_TEXTURE_2D, texs[0]);
    // 图象从纹理图象空间映射到帧缓冲图象空间(映射需要重新构造纹理图像,这样就会造成应用到多边形上的图像失真),
    // 这时就可用glTexParmeteri函数来确定如何把纹理象素映射成像素.
    //  放大过滤，线性插值   GL_NEAREST(效率高，但马赛克严重)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // 创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    // U
    glBindTexture(GL_TEXTURE_2D, texs[1]);
    // 放大过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // 创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    //
    glBindTexture(GL_TEXTURE_2D, texs[2]);
    // 放大过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // 创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    qDebug() << "Maximum nr of vertex attributes supported: " << nrAttributes;
    const char* version = (const char*)glGetString(GL_VERSION);
    qDebug() << "OpenGL Version: " << version;

    qDebug() << "VideoOpenGLWidget::init finished.";

    return 0;
}

void VideoOpenGLWidget::paintFrame(AVFrame* frame)
{
    // m_render();
    //  qDebug() << "paintFrame ....";
    if (!frame) {
        qDebug() << "no frame";
        return;
    }

    std::unique_lock<std::mutex> guard(m_mutex);
    // 容错，保证尺寸正确
    if (frame->format == AV_PIX_FMT_YUV420P) {
        // 是 YUV 420 格式
        if (width == frame->linesize[0]) // 无需对齐
        {
            memcpy(datas[0], frame->data[0], width * height);
            memcpy(datas[1], frame->data[1], width * height / 4);
            memcpy(datas[2], frame->data[2], width * height / 4);
        } else // 行对齐问题
        {
            for (int i = 0; i < height; i++) // Y
                memcpy(datas[0] + width * i, frame->data[0] + frame->linesize[0] * i, width);
            for (int i = 0; i < height / 2; i++) // U
                memcpy(datas[1] + width / 2 * i, frame->data[1] + frame->linesize[1] * i, width);
            for (int i = 0; i < height / 2; i++) // V
                memcpy(datas[2] + width / 2 * i, frame->data[2] + frame->linesize[2] * i, width);
        }

        // qDebug() << "paintFrame finished";
    }

    else if (frame->format == AV_PIX_FMT_YUV422P) {
        // 是 YUV 420 格式
        if (width == frame->linesize[0]) // 无需对齐
        {
            memcpy(datas[0], frame->data[0], width * height);
            memcpy(datas[1], frame->data[1], width * height / 2);
            memcpy(datas[2], frame->data[2], width * height / 2);
        } else // 行对齐问题
        {
            for (int i = 0; i < height; i++) // Y
                memcpy(datas[0] + width * i, frame->data[0] + frame->linesize[0] * i, width);
            for (int i = 0; i < height; i++) // U
                memcpy(datas[1] + (width / 2) * i, frame->data[1] + frame->linesize[1] * i, width / 2);
            for (int i = 0; i < height; i++) // V
                memcpy(datas[2] + (width / 2) * i, frame->data[2] + frame->linesize[2] * i, width / 2);
        }
    }
    av_frame_free(&frame);
    update();
}

void VideoOpenGLWidget::paintFrameWithThread(AVFrame* frame)
{
}

/*
 * 在第一次调用 paintGL() 或 resizeGL() 之前，会调用此虚拟函数一次。在子类中重新实现它。
 * 此函数应设置任何所需的 OpenGL 资源。
 * 但请注意，帧缓冲区在此阶段尚不可用，因此请避免从此处发出绘制调用。
 * 将此类调用推迟到 paintGL()。
 */
void VideoOpenGLWidget::initializeGL()
{

    qDebug() << "initializeGL.....";
    std::unique_lock<std::mutex> guard(m_mutex);
    // 初始化opengl （QOpenGLFunctions继承）函数
    initializeOpenGLFunctions();

    // program加载shader（顶点和片元）脚本
    // 片元（像素）
    qDebug() << "QOpenGLShader::Fragment: " << program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");
    // 顶点shader
    qDebug() << "QOpenGLShader::Vertex:   " << program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader.vert");

    // 设置顶点坐标的变量
    program.bindAttributeLocation("vertexIn", A_VER); // A_VER = 3

    // 设置材质坐标
    program.bindAttributeLocation("textureIn", T_VER); // T_VER = 4

    // 编译shader
    qDebug() << "program.link() = " << program.link(); // Links together the shaders

    qDebug() << "program.bind() = " << program.bind(); // Binds this shader program to the active QOpenGLContext and makes it the current shader program.

    // 传递顶点和材质坐标
    // 顶点
    static const GLfloat ver[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
    };

    // 材质
    static const GLfloat tex[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    // 顶点
    glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, ver);
    glEnableVertexAttribArray(A_VER);

    // 材质
    glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, tex);
    glEnableVertexAttribArray(T_VER);

    // 从shader获取材质
    unis[0] = program.uniformLocation("tex_y");
    unis[1] = program.uniformLocation("tex_u");
    unis[2] = program.uniformLocation("tex_v");
    qDebug() << "initializeGL finished";
#define TEST
#ifndef TEST
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texs[0]); // 0层绑定到Y材质
    glUniform1i(unis[0], 0); // 2

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texs[1]); // 1层绑定到U材质
    glUniform1i(unis[1], 1); // 0

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, texs[2]); // 2层绑定到V材质
    glUniform1i(unis[2], 2); // 1

#endif
}
/*
 * 每当需要绘制小部件时，都会调用此虚拟函数。在子类中重新实现它。
 * 在调用此函数之前，上下文和帧缓冲区已绑定，并且通过调用 glViewport() 设置视口。
 * 未设置任何其他状态，框架也不执行任何清除或绘制操作。
 */
void VideoOpenGLWidget::paintGL()
{ // return;
  // QThread* currentThread = QThread::currentThread();
  // qDebug() << "paintGL::thread_id()" << currentThread;
    std::unique_lock<std::mutex> guard(m_mutex);
#ifdef TEST

    glActiveTexture(GL_TEXTURE0); // ---------------┐
    glBindTexture(GL_TEXTURE_2D, texs[0]); // ----一起用--->---->---->-------------------------------->-┐
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, datas[0]); // 更新纹理（显卡内）中的数据，从图像中复制
    glUniform1i(unis[0], 0); // 2

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texs[1]); // 1层绑定到U材质
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[1]);
    glUniform1i(unis[1], 1); // 0

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, texs[2]); // 2层绑定到V材质
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[2]);
    glUniform1i(unis[2], 2); // 1
#else
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texs[0]); // 0层绑定到Y材质
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, datas[0]);
    glUniform1i(unis[0], 0); // 2

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texs[1]); // 0层绑定到Y材质
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[1]);
    glUniform1i(unis[1], 1); // 2

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, texs[2]); // 0层绑定到Y材质
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[2]);
    glUniform1i(unis[2], 2); // 2
#endif
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
/*
 * 每当调整小部件的大小时，都会调用此虚拟函数。
 * 在子类中重新实现它。新大小通过 w 和 h 传递。
 */
void VideoOpenGLWidget::resizeGL(int w, int h)
{

    // update();
    //  qDebug() << "resizeGL " << w << ":" << h;
    // std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // SwapBuffers(window);
}
