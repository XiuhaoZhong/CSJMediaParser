#include "CSJMpegTool.h"

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "libavutil/ffversion.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"

#ifdef __cplusplus
}
#endif

#include "Utils/CSJStringUtils.h"
#include "CSJMediaData.h"
#include "CSJMediaDataManager.h"
#include "CSJMpegToolWorker.h"


using namespace std;
using namespace csjutils;


CSJMpegTool *CSJMpegTool::getInstance() {
    static CSJMpegTool mpegTool;
    return &mpegTool;
}

void CSJMpegTool::getStreamDuration(AVStream *st, int &hours, int &mins, int &secs, int &us) {
    if (!st) {
        return ;
    }

    int64_t totalMiniSeconds = st->duration * av_q2d(st->time_base) * 1000;
    us = totalMiniSeconds % 1000;
    totalMiniSeconds /= 1000;
    secs = totalMiniSeconds % 60;
    totalMiniSeconds /=  60;
    mins = totalMiniSeconds % 60;
    hours = totalMiniSeconds / 60;
}

CSJMpegTool::CSJMpegTool() {

}

CSJMpegTool::~CSJMpegTool() {

}

bool CSJMpegTool::parseMediaContent(QString fileUrl) {
    if (fileUrl.size() == 0) {
        return false;
    }

    CSJMpegToolWorker::getInstance()->setMediaUrl(fileUrl);
    //CSJMpegToolWorker::getInstance()->start();

    return true;
}

void CSJMpegTool::setDataUpdateDelegate(CSJMpegDataUpdateDelegate *dataDelegate) {
    if (!dataDelegate) {
        return ;
    }

    m_pDelegate = dataDelegate;
    connect(this, SIGNAL(updateStreams()), m_pDelegate, SLOT(onUpdateStreams()));
    connect(this, SIGNAL(updatePackets()), m_pDelegate, SLOT(onUpdatePackets()));
    connect(this, SIGNAL(updateFrames()), m_pDelegate, SLOT(onUpdateFrames()));
}

void CSJMpegTool::updateMediaData(CSJMpegUpdateDataType dataType) {
    if (!m_pDelegate) {
        return ;
    }

    switch (dataType) {
    case CSJMpegUpdateDataType_Stream:
        emit updateStreams();
        break;
    case CSJMpegUpdateDataType_Packet:
        emit updatePackets();
        break;
    case CSJMpegUpdateDataType_Frame:
        emit updateFrames();
        break;
    default:
        break;
    }

}

QString CSJMpegTool::getFFMpegVersion() {

    std::string verString = Format("FFMpeg version:\n {0}", FFMPEG_VERSION);
    std::string confString = Format("FFMpeg config:\n {0}", FFMPEG_CONFIGURATION);

    QString resString = QString::fromStdString(verString);
    resString.append("\n\n");
    resString.append(QString::fromStdString(confString));

    return resString;
}

QString CSJMpegTool::getFFMpegBuildConf() {
    return QString("FFMpeg conf!");
}

