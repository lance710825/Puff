#ifndef MSAV_AVTHREAD_P_H
#define MSAV_AVTHREAD_P_H

#include "DPTR.h"
#include "PacketQueue.h"
#include "AVDecoder.h"

namespace MSAV {

class AVThreadPrivate : public DptrPrivate<AVThread> {
public:
    AVThreadPrivate() :
        decoder(NULL)
    {
        packets.clear();
    }

    virtual ~AVThreadPrivate() {}

    AVDecoder *decoder;
    PacketQueue packets;
};

}
#endif //MSAV_AVTHREAD_P_H
