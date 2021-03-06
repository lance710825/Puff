#include "VideoThread.h"
#include "AVThread_p.h"
#include "VideoDecoder.h"
#include "AVLog.h"
#include "OutputSet.h"
extern "C" {
#include "libavutil/time.h"
}

namespace Puff {

class VideoThreadPrivate: public AVThreadPrivate
{
public:
    VideoThreadPrivate() {}
    ~VideoThreadPrivate() {}

    VideoFrame current_frame;
};

VideoThread::VideoThread():
    AVThread(new VideoThreadPrivate)
{

}

VideoThread::~VideoThread()
{

}

void VideoThread::run()
{
    DPTR_D(VideoThread);

    avdebug("video thread id:%d.\n", id());
    auto *dec = dynamic_cast<VideoDecoder *>(d->decoder);
    AVClock *clock = d->clock;
    d->stopped = false;

    Packet pkt;
    bool pkt_empty = false;
    bool wait_key_frame = false;
    d->seeking = false;

    while (true) {
        executeNextTask();

        if (d->seek_requested) {
            d->seek_requested = false;
            dec->flush();
            pkt = Packet();
            d->seeking = true;
            wait_key_frame = true;
            avdebug("request seek video thread\n");
        }

        if (d->stopped)
            break;
        if (d->paused && !d->seeking) {
            continue;
        }

        pkt = d->packets.dequeue(&pkt_empty);

        if (pkt.isEOF()) {
            sendVideoFrame(VideoFrame());
            break;
        } else {
            if (!pkt.isValid()) {
                dec->flush();
                wait_key_frame = true;
                avdebug("video pkt is not valid, pts is %.3f\n", pkt.pts);
                continue;
            }
        }

        if (!dec->decode(pkt)) {
            pkt = Packet();
            continue;
        }
        VideoFrame frame = dec->frame();
        if (!frame.isValid()) {
            continue;
        }
        if (!sendVideoFrame(frame))
            continue;
        if (d->seeking) {
            d->seeking = false;
            continue;
        }
        d->current_frame = frame;
        double delay = puAbs(pkt.pts - frame.timestamp());
//        avdebug("video frame, pts: %.3f timestamp: %.3f, delay: %.3f, clock: %.3f\n",
//                pkt.pts, frame.timestamp(), frame.timestamp() - clock->value(), clock->value());
//        avdebug("av_gettime: %lld, %lld\n", av_gettime(), av_gettime_relative());
        if (delay > 0.5) {
            avdebug("continue\n");
            continue;
        }
        if (frame.timestamp() > 0 && clock->value() > 0) {
            double delay = puMin(0.1, frame.timestamp() - clock->value());
            if (delay > 0) {
                msleep((unsigned int)(delay * 1000));
            }
        } else {
            msleep(34);
        }
    }
    d->packets.clear();
    AVThread::run();
}

bool VideoThread::sendVideoFrame(VideoFrame &frame)
{
    DPTR_D(VideoThread);
    d->output->lock();
    d->output->sendVideoFrame(frame);
    d->output->unlock();
    return true;
}

}
