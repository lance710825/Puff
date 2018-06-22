#include "AVDemuxThread.h"
#include "AVDemuxer.h"
#include "AudioThread.h"
#include "VideoThread.h"
#include "PacketQueue.h"
#include "AVLog.h"

namespace Puff {

class AVDemuxThreadPrivate
{
public:
    AVDemuxThreadPrivate():
        demuxer(NULL),
        audio_thread(NULL),
        video_thread(NULL),
        stopped(false)
    {

    }
    ~AVDemuxThreadPrivate()
    {

    }
    AVDemuxer *demuxer;
    AVThread *audio_thread, *video_thread;
    bool stopped;
};

AVDemuxThread::AVDemuxThread():
    d_ptr(new AVDemuxThreadPrivate)
{

}

AVDemuxThread::~AVDemuxThread() {

}

void AVDemuxThread::setDemuxer(AVDemuxer *demuxer)
{
    DPTR_D(AVDemuxThread);
    d->demuxer = demuxer;
}

void AVDemuxThread::setAudioThread(AVThread *thread)
{
    DPTR_D(AVDemuxThread);
    d->audio_thread = thread;
}

AVThread *AVDemuxThread::audioThread()
{
    DPTR_D(const AVDemuxThread);
    return d->audio_thread;
}

void AVDemuxThread::setVideoThread(AVThread *thread)
{
    DPTR_D(AVDemuxThread);
    d->video_thread = thread;
}

AVThread *AVDemuxThread::videoThread()
{
    DPTR_D(const AVDemuxThread);
    return d->video_thread;
}

void AVDemuxThread::run()
{
    DPTR_D(AVDemuxThread);
    int stream;
    Packet pkt;
    int ret = -1;

    avdebug("demux thread id:%d.\n", id());
    if (d->audio_thread && !d->audio_thread->isRunning()) {
        d->audio_thread->start();
    }
    if (d->video_thread && !d->video_thread->isRunning()) {
        d->video_thread->start();
    }
    PacketQueue *vbuffer = d->video_thread ? d->video_thread->packets() : NULL;
    PacketQueue *abuffer = d->audio_thread ? d->audio_thread->packets() : NULL;

    if (vbuffer) {
        vbuffer->clear();
        vbuffer->setBlock(true);
    }
    if (abuffer) {
        abuffer->clear();
        abuffer->setBlock(true);
    }
    bool audio_has_pic = false;
    d->stopped = false;
    bool enqueue_eof = false;

    while (!d->stopped) {

        if (d->demuxer->atEnd()) {
            // wait for a/v thread finished
            if (!enqueue_eof) {
                if (abuffer) {
                    abuffer->enqueue(Packet::createEOF());
                    abuffer->blockEmpty(false);
                }
                if (vbuffer) {
                    vbuffer->enqueue(Packet::createEOF());
                    vbuffer->blockEmpty(false);
                }
                enqueue_eof = true;
            }
            if (abuffer->isEmpty() && vbuffer->isEmpty()) {
                break;
            }
            msleep(100);
            continue;
        }
        audio_has_pic = d->demuxer->hasAttachedPic();

        ret = d->demuxer->readFrame();
        if (ret < 0)
            continue;
        stream = d->demuxer->stream();
        pkt = d->demuxer->packet();

        if (stream == d->demuxer->videoStream()) {
            if (vbuffer) {
                if (!d->video_thread || !d->video_thread->isRunning()) {
                    vbuffer->clear();
                    continue;
                }
                vbuffer->blockFull(!d->audio_thread || !d->audio_thread->isRunning() || !abuffer || abuffer->isEnough());
                vbuffer->enqueue(pkt);
            }
        }
        else if (stream == d->demuxer->audioStream()) {
            if (abuffer) {
                if (!d->audio_thread || !d->audio_thread->isRunning()) {
                    abuffer->clear();
                    continue;
                }
                abuffer->blockFull(!d->video_thread || !d->video_thread->isRunning() || !vbuffer || audio_has_pic);
                abuffer->enqueue(pkt);
            }
        }
        else if (stream == d->demuxer->subtitleStream()) {

        }
        msleep(1);
    }

    CThread::run();
}

}
