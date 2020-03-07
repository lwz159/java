FFMpeg播放器解码基本流程：

`av_register_all()`->`avformat_open_input()`->`av_find_stream_info()`->`acvodec_find_decoder()`->`avcodec_open()`->`av_read_frame()`->循环获取`AVPacket`->`acvodec_decode_video2()`

`Avpacket`是解复用之后，解码前的数据，仍然是压缩的数据

`av_register_all()`：用来注册所有编译的`muxers`、`demuxers`以及`ptotocols`（protocols是什么？干什么用的？）（具体原理先不管，先记住必须首先调用`av_register_all`）

`avformat_open_input`：与此相关的一个数据结构是`AVFormatContext`（`AVFormatContext`的具体作用是什么，还不清楚，但是可以用来进行输入输出操作，这个只能用`avformat_alloc_context`来进行内存空间的分配，如何释放？），`avformat_open_input`是来将`AVFormatContext`和一个文件关联起来的（可能不止是硬盘上的文件）。

`av_find_stream_info()`：查找文件中流的信息，并填充道`AVFormatContext`中的`AVStream`中

`avcodec_find_decoder`：是用来查找解码器的，于此相关需要借助于`AVcodecContext`，即编解码器的上下文。`AVCodecContext`通过`avcodec_alloc_context3`来进行空间分配和初始化，并通过`avcodec_parameters_to_context`将流中的解码器相关信息填充道`AvcodecContext`中。

`avcodec_open`：打开视频解码器，具体作用暂时不清楚。



`Android`视频图像是如何绘制的还不是很清楚。

视频的宽高有什么意义？对于视频的显示有什么帮助？

视频播放的软解和硬解具体区别是怎么实现的？

PCM编码原始数据和`MP3`，`AAC`等编码格式的区别是什么？

图像分辨率值得是每英寸图像内有多少个像素，480 * 960值得就是每英寸水平像素是480，竖直方向是960.