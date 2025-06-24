## Developing Recordings(Watch this document before develop!)

### 2025-03-31
#### The key points
1. Based function checkAndBindResources, every time render the content, should check if the content that will be rendered changed. The changes could be several kinds, including only size changes, only pixel format changes, or both change.
2. During the rendering, if the changes above happen, should set the relative flags, in the code, m_bNeedUpdateSize/m_bNeedUpdateResources indicate the changes.
3. The render should hold a CSJVideoData object so that the renderer can keep a frame of video data, and bind the video data into texture when the renderer renders. So there should be a improve that keep a CSJVideoData object rather than updating the texture immediately.

#### Improve results
Please implement the key points above, and record the details, and if there are some other improvements during the implementing, please record them too.

### 2025-04-16
#### The key points
1. Changed the thoughts about update video content during the rendering, I add comments in the code, please implement!

### 2025-04-23
#### The key points
1. There is a problem, when I want to render a image, I use DDSTextureLoader to create a texture from a image file, this logic is conflicts with my previous logic that abstract all the picture data as a CSJVideoData object, so I'm going to change the way I use now,
2. I will add a image load library for loading pictures and I will create the texture by myself. The library should stbi or SOIL2

### 2025-04-24
#### The Key points
1. Suddenly, I think about another to solve the image rendering problem, the DDSTextureLoader can load texture from memory, so I don't need to use a image loading library, I can use DDSTextureLoader to load memory.
2. I have already code the main point of the resolution above, I will complete the rest of the code and test the render function.

### 2025-04-29
1. I've already sovled the logic that rendering a picture in new function above I mentioned.
2. I should think about this part of code again and improve it
3. The next step is that add the new pixel shader to render yuv video data

## Notes:
1. When I bind the index buffer, I used a wrong type DXGI_FORMAT_R8G8B8A8_UNORM, this type is used to describe pixel data not index data, the right type is DXGI_FORMAT_R32_UINT!!!

### 2025-05-01
1. I improved the logic of rendering content when the video data changed.

### 2025-05-02
1. Start to develop video playing window, arrage the player controll module, the mainly work is playing status controll buttons, start/pause/resume/stop/ff/fb buttons, and the playing status switching controlled by these buttons.
2. Construct the thread model of media player, the mainly work is arraging threads for detail works, such as reading media file, decoding media data(video, audio and maybe subtitles), switching thread status responding with player status.

### 2025-05-03
1. I've completed the playing control interfaces, and the control buttons functions are ok, playing status switching is OK, but the UI style is very simple, and need to improve the layout and button style.
2. The player threads need to be developed, and need add a interface to get the playing status in the player kernel class.

### 2025-05-04
1. I get an idea that load icons effeciently, putting all the icons into a big image file, then load the file from QPixmap, and when I need to load an icon, just indicate the location of the icon, at the same time, I can use the QPixMap::setDevicePixelRatio fucntion to garantee the icon will be rendered normal on High resolutions screens. So, I must write an icon manager to archive this idea. 

### 2025-05-18
1. I completed the threads model in the CSJFFPlayerKernel for the first version, then should use the real threads replacing the test threads, and debugging the decoder workflow and improving the thread model.

### 2025-06-02
1. Add the library spdlog as the logger module

### 2025-06-03
1. Walked the stream_open function, and add the relative logs.
2. The next step is to walk stream_component_open function and add relative logs.
3. Then start the play thread model and debug.

### 2025-06-23
1. I upgraded ffmpeg to latest version, and x264/x265/fdk-aac libraries into ffmpeg
2. After thinking about the current constructre, there should be some improvements.
    1. Separate the UI logic and media logic, as far, the CSJPlayerKernel is separated from UI
    2. Media data parsering and analysing should be improved, the operations should be executed during the playing process, so this is a big point.

### 2025-06-24
1. I start to recreate the constructure of current project, the main thoughts:
    1. Add a layer named CSJMediaEngine, and provides the interfaces that are used to deal multimedia, currently the main interfaces are CSJMediaPlayer and CSJMediaEngineInfo. 
    2. CSJMediaPlayer will take place previous player
    3. CSJMediaEngineInfo provides interfaces to check the supportings in the current media engine. These supportings include media file type, video/audio format, video/audio decoder and encoder, network protocols and so on.
2. I make the basis constructure of the CSJMediaEngine, and plan to archine it as an dynamic library, thus, it can be update convenient.
3. This is the major that current phase needed to be done.
4. You'd better make constructure images and documents to record the details.