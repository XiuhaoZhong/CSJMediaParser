# CSJMediaParser

## 项目简介
    Windows/macOS多媒体内容分析工具以及播放器。

## 功能特性
1. 支持音视频文件的播放，MP4/MOV/AAC/MP3等等
2. 支持MP4文件内容解析，音视频轨道、音视频具体参数，视频帧参数分析，图像分析
3. 支持RTMP码流分析
4. 支持MP4文件的音视频数据抽取

## 整体架构
    整体为三大模块，UI、多媒体数据分析以及播放器。
    UI基于QT Gui。
    多媒体数据分析部分基于FFMpeg，采用MVC结构，将多媒体数据展示和数据的读取分析分离，通过QT事件机制进行整体控制。
    播放器基于ffplay，使用C++11线程模型，视频渲染和音频播放均采用native API。对于Windows，使用DirecX11和CoreAudio，对于macOS，使用Metal和AudioUnit。

## 项目结构
```
CSJMediaParser/
├── src/                # 源代码目录
├── thirdParts/         # 依赖第三方库目录
├── documents/          # 相关记录文件
├── README.md           # 项目说明文件
├── CMakeLists.txt      # 项目配置文件
└── LICENSE             # 许可证文件
```