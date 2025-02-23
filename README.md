# MultiVision
QT+OPENCV+FFMPEG+OPENGL 处理UVC/串口/本地图像，执行滤波、人脸识别等
支持多源输入与实时视觉处理以及多源输出
依赖opencv-4.10.0, ffmpeg-n6.1, Qt6 OpenGL Widgets module
## 核心功能

### 多源输入支持
- 📷 摄像头实时采集（UVC协议）
- 🗂 本地图像/视频加载（JPEG/PNG/MP4/AVI）
- 📡 串口图像流解析（自定义协议）
- 🎥 FFmpeg视频解码(包含一个ffmpeg播放器模块）

### 图像处理管线
#### 基础处理
- 直方图分割
- 空间滤波（均值/高斯/双边）
- 边缘检测（Canny/Sobel）

#### 高级功能
- 实时人脸检测（Face Detect）
- 艺术效果生成（ASCII字符画转换）

### 多模态输出
- 🖥️ Qt Widgets （QPainter）
- 🎮 OpenGL 渲染
- 📝 终端字符界面输出

## 技术亮点
- 播放器多线程处理(视频解包解码线程分离)
- OPENCV图像处理，算法可串联
- OPENGL硬件渲染
- 文本(字符画输出)  
![image](https://github.com/minasanohayo/MultiVision/blob/main/BAD_APPLE_2025.gif "preview")
![image](https://github.com/minasanohayo/MultiVision/blob/main/QQ20250221-223305.png "preview")
