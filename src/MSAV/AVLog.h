#ifndef MSAV_AVLOG_H
#define MSAV_AVLOG_H

#include "MSAVGlobal.h"
#include "DPTR.h"

namespace MSAV {

class AVLogPrivate;
class MS_AV_EXPORT AVLog
{
    DPTR_DECLARE_PRIVATE(AVLog);
public:
    AVLog *instance();

    void log(LogLevel level, const char *fmt, ...);

    void setLevel(LogLevel level);

    LogLevel level() const;

private:
    explicit AVLog();
    ~AVLog();

private:
    DPTR_DECLARE(AVLog)
};

}
#endif //MSAV_AVLOG_H
