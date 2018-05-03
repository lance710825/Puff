#ifndef PUFF_AVDEMUXER_H
#define PUFF_AVDEMUXER_H

#include "AVGlobal.h"
#include "CObject.h"
#include "Packet.h"

struct AVCodecContext;

namespace Puff {

class AVDemuxerPrivate;
class PU_AV_EXPORT AVDemuxer: public CObject
{
    DPTR_DECLARE_PRIVATE(AVDemuxer);

public:
    AVDemuxer();
    ~AVDemuxer();

    enum StreamType {
        Stream_Audio,
        Stream_Video,
        Stream_Subtitle
    };

    void setMedia(const String &fileName);
    bool load();
    void unload();

    bool isLoaded() const;

    int  stream();
    Packet packet() const;
    int  readFrame();
    int  videoStream();
    int  audioStream();
    int  subtitleStream();

    bool setStreamIndex(StreamType type, int index);

    AVCodecContext* audioCodecCtx(int stream = -1) const;
    AVCodecContext* videoCodecCtx(int stream = -1) const;
    AVCodecContext* subtitleCodecCtx(int stream = -1) const;

private:
    DPTR_DECLARE(AVDemuxer)
};

}
#endif //PUFF_AVDEMUXER_H