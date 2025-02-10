# CSJMediaParser

## 项目简介
    Windows/macOS多媒体内容分析工具以及播放器。
    UI框架基于QT，使用native的渲染API，Windows下的DX11，macOS下的Metal，多媒体相关解析处理基于FFMpeg。

## 功能特性
1. 支持音视频文件的播放，MP4/MOV/AAC/MP3等等
2. 支持MP4文件内容解析，音视频轨道、音视频具体参数，视频帧参数分析，图像分析
3. 支持RTMP码流分析
4. 支持MP4文件的音视频数据抽取

## 项目结构
```
CSJVisualScene/
├── src/                # 源代码目录
├── thirdParts/         # 依赖第三方库目录
├── documents/          # 相关记录文件
├── README.md           # 项目说明文件
├── CMakeLists.txt      # 项目配置文件
└── LICENSE             # 许可证文件
```