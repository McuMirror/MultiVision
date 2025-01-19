# OPENCV常用函数



## core模块

    
### cv2.imread

该函数 `imread` 从指定文件加载图像并返回。如果无法读取图像（因为文件丢失、权限问题、不支持的格式或无效格式），该函数将返回一个空矩阵（ `Mat::data==NULL` ）。

目前支持以下文件格式：

-   Windows 位图 - `*.bmp`, `*.dib`（始终支持）
-   JPEG 文件 - `*.jpeg`, `*.jpg`, `*.jpe`（见 *注意* 部分）
-   JPEG 2000 文件 - `*.jp2`（见 *注意* 部分）
-   可移植网络图形 - `*.png`（见 *注意* 部分）
-   WebP - `*.webp`（见 *注意* 部分）
-   AVIF - `*.avif`（见 *注意* 部分）
-   可移植图像格式 - `*.pbm`, `*.pgm`, `*.ppm`, `*.pxm`, `*.pnm`（始终支持）
-   PFM 文件 - `*.pfm`（见 *注意* 部分）
-   Sun 栅格图像 - `*.sr`, `*.ras`（始终支持）
-   TIFF 文件 - `*.tiff`, `*.tif`（见 *注意* 部分）
-   OpenEXR 图像文件 - `*.exr`（见 *注意* 部分）
-   Radiance HDR - `*.hdr`, `*.pic`（始终支持）
-   GDAL 支持的栅格和矢量地理空间数据（见 *注意* 部分）

@note
-   该函数通过图像内容来判断图像类型，而不是通过文件扩展名。
-   对于彩色图像，解码后的图像将按 **B G R** 顺序存储通道。
-   使用 `IMREAD_GRAYSCALE` 时，如果可用，将使用编解码器的内部灰度转换。结果可能与 `cvtColor()` 的输出不同。
-   在 Microsoft Windows\* 操作系统和 MacOSX\* 上，默认使用 OpenCV 图像（libjpeg、libpng、libtiff 和 libjasper）自带的编解码器。因此，OpenCV 总是能读取 JPEG、PNG 和 TIFF 格式。在 MacOSX 上，还可以选择使用原生 MacOSX 图像阅读器。但需要注意，目前这些原生图像加载器由于 MacOSX 内嵌的颜色管理，会导致图像的像素值有所不同。
-   在 Linux\*、BSD 系统和其他类 Unix 开源操作系统上，OpenCV 会查找操作系统图像中提供的编解码器。安装相关软件包（不要忘记开发文件，例如在 Debian\* 和 Ubuntu\* 上的 "libjpeg-dev"）以获得编解码器支持，或者在 CMake 中开启 `OPENCV_BUILD_3RDPARTY_LIBS` 标志。
-   如果在 CMake 中将 `WITH_GDAL` 标志设置为 true，并通过 `IMREAD_LOAD_GDAL` 来加载图像，那么将使用 [GDAL](http://www.gdal.org) 驱动程序解码图像，支持以下格式：[栅格](http://www.gdal.org/formats_list.html)、[矢量](http://www.gdal.org/ogr_formats.html)。
-   如果图像文件中嵌入了 EXIF 信息，则会考虑 EXIF 中的方向，并相应地旋转图像，除非传递了 `IMREAD_IGNORE_ORIENTATION` 或 `IMREAD_UNCHANGED` 标志。
-   使用 `IMREAD_UNCHANGED` 标志以保留 PFM 图像中的浮动点值。
-   默认情况下，图像的像素数不得超过 2^30。可以使用系统变量 `OPENCV_IO_MAX_IMAGE_PIXELS` 来设置此限制。

@param
-   @param filename 要加载的文件名。
-   @param flags 可以接受 `cv::ImreadModes` 的标志值。
 
CV_EXPORTS_W Mat imread( const String& filename, int flags = IMREAD_COLOR );
 
### cv2.imwrite
### cv2.imshow
### cv2.cvtColor
 


## imgproc模块
### cv2.GaussianBlur：高斯滤波
### cv2.medianBlur：中值滤波
### cv2.bilateralFilter：双边滤波
### cv2.Canny：边缘检测
### cv2.threshold：阈值处理
### cv2.adaptiveThreshold：自适应阈值处理

## highgui模块
Paragraph.

- bullet
+ other bullet
* another bullet
    * child bullet

1. ordered
2. next ordered
