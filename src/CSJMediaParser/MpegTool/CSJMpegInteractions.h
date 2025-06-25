// Defining the interaction classses which are responsible for interacting 
// between UI and the underlying logic modules.

#ifndef __CSJMPEGINTERACTIONS_H__
#define __CSJMPEGINTERACTIONS_H__

#include <QObject>

typedef enum {
    CSJMpegUpdateDataType_Stream = 0,
    CSJMpegUpdateDataType_Packet,
    CSJMpegUpdateDataType_Frame,

} CSJMpegUpdateDataType;

class CSJMpegDataUpdateDelegate : public QObject {
    Q_OBJECT
public:
    CSJMpegDataUpdateDelegate() = default;
    ~CSJMpegDataUpdateDelegate() = default;

    //virtual void onNotifyUpdateMediaData(CSJMpegUpdateDataType dataType) = 0;
public Q_SLOTS:
    virtual void onUpdateStreams() = 0;
    virtual void onUpdatePackets() = 0;
    virtual void onUpdateFrames() = 0;
};

#endif // __CSJMPEGINTERACTIONS_H__
