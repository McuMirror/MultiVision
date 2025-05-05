#include "glwidget.h"

#define A_VER 3
#define T_VER 4
GLWidget::GLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , QOpenGLFunctions()
{
}

void GLWidget::initializeGL()
{
    qDebug() << "GLWidget::initializeGL.....";
    std::unique_lock<std::mutex> guard(m_mutex);
    // 初始化opengl （QOpenGLFunctions继承）函数
    initializeOpenGLFunctions();

    // program加载shader（顶点和片元）脚本
    // 顶点shader

    // 片元（像素）
    qDebug() << "QOpenGLShader::Vertex:   " << program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shaders/shader.vert");
    qDebug() << "QOpenGLShader::Fragment: " << program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shaders/shader.yuvfrag");

    // 设置顶点坐标的变量
    program.bindAttributeLocation("vertexIn", A_VER); // A_VER = 3 //vertexIn是顶点shader的输入:v4 vertexIn

    // 设置材质坐标
    program.bindAttributeLocation("textureIn", T_VER); // T_VER = 4 //textureIn是片元shader的输入: v2 textureIn

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

    // 增加&初始化投影矩阵
    QMatrix4x4 projection;
    projection.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    resizeGL(this->width, this->height); // 手动触发视口设置
    update(); // 强制立即重绘 ✅ 关键步骤
    program.setUniformValue("projection", projection);

    GLisInit = true;

    qDebug() << "initializeGL finished";
}

/*
 * opengl窗口初始化和texture初始化分开
 * 窗口初始化在第一次运行时调用一次
 * texture初始化在图像尺寸改变时调用
 */
int GLWidget::init_texture(int width, int height)
{
    qDebug() << "GLWidget::init_texture....";
    std::unique_lock<std::mutex> guard(m_mutex);
    this->width = width;
    this->height = height;
    delete[] datas[0];
    delete[] datas[1];
    delete[] datas[2];
    datas[0] = nullptr; // 重置指针避免野指针
    datas[1] = nullptr; // 重置指针避免野指针
    datas[2] = nullptr; // 重置指针避免野指针
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
    // qDebug() << "Maximum nr of vertex attributes supported: " << nrAttributes;
    const char* version = (const char*)glGetString(GL_VERSION);
    qDebug() << "OpenGL Version: " << version;

    qDebug() << "VideoOpenGLWidget::init_texture finished.";
    imageWidth = width;
    imageHeight = height;

    textIsInit = true;

    // setProjection();
    update(); // 强制立即重绘 ✅ 关键步骤

    return 0;
}
void GLWidget::paintGL()
{
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

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void GLWidget::resizeGL(int w, int h)
{
    qDebug() << "resizeGL";

    // 设置视口
    //  glViewport(0, 0, w, h);
    setProjection(w, h);
}
void GLWidget::setProjection(int w, int h)
{

    qDebug() << "setProjection" << imageWidth << "," << imageHeight << ". " << this->width << "," << this->height;
    if (imageWidth <= 0 || imageHeight <= 0)
        return;
    // 计算图像的宽高比
    float imageAspectRatio = static_cast<float>(imageWidth) / static_cast<float>(imageHeight);
    float windowAspectRatio = static_cast<float>(w) / static_cast<float>(h);

    // 计算投影矩阵
    QMatrix4x4 projection;
    if (windowAspectRatio > imageAspectRatio) {
        // 窗口比图像宽，上下留黑边
        float scale = imageAspectRatio / windowAspectRatio;
        projection.ortho(-1.0f, 1.0f, -scale, scale, -1.0f, 1.0f);
    } else {
        // 窗口比图像高，左右留黑边
        float scale = windowAspectRatio / imageAspectRatio;
        projection.ortho(-scale, scale, -1.0f, 1.0f, -1.0f, 1.0f);
    }

    // 设置投影矩阵
    program.setUniformValue("projection", projection);

    projectionIsInit = true;
}
void GLWidget::paintMat(cv::Mat* mat) // 原来是AVFrame* frame
{

    if (mat->type() == CV_8UC3) {
        // qDebug() << "mat->type() == CV_8UC3 ->>>" << mat->type();
        //  如果是 RGB 图像，先转换为 YUV 420

        cv::Mat clonedMat = mat->clone();

        cv::Mat yuvMat;

        cv::cvtColor(clonedMat, yuvMat, cv::COLOR_BGR2YUV_I420); // 转换为 YUV 420

        // 获取 Y、U、V 分量
        int ySize = width * height;
        int uvSize = ySize / 4;

        memcpy(datas[0], yuvMat.data, ySize);
        memcpy(datas[1], yuvMat.data + ySize, uvSize);
        memcpy(datas[2], yuvMat.data + ySize + uvSize, uvSize);

        imageWidth = mat->size().width;
        imageHeight = mat->size().height;
        update();
    } else {
        // qDebug() << "mat->type() != CV_8UC3 ->>>" << mat->type();
    }
}

void GLWidget::paintFrame(std::shared_ptr<AVFrame> frame)
{
    // m_render();
    //  qDebug() << "paintFrame ....";
    if (!frame) {
        qDebug() << "no frame";
        return;
    }
    qDebug() << "frame->format ->>>" << frame->format;
    std::unique_lock<std::mutex> guard(m_mutex);
    //  容错，保证尺寸正确
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
    // av_frame_free(&frame);
    update();
}
