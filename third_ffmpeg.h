#ifndef THIRDFFMPEG_H
#define THIRDFFMPEG_H

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#endif

#ifndef UINT64_C
#define UINT64_C(c) (c ## ULL)
#endif

extern "C"
{
    #include "libavutil/avstring.h"
    //#include "libavutil/colorspace.h"
    #include "libavutil/mathematics.h"
    #include "libavutil/pixdesc.h"
    #include "libavutil/imgutils.h"
    #include "libavutil/dict.h"
    #include "libavutil/parseutils.h"
    #include "libavutil/samplefmt.h"
    #include "libavutil/avassert.h"
    #include "libavutil/time.h"
    #include "libavformat/avformat.h"
    #include "libavdevice/avdevice.h"
    #include "libswscale/swscale.h"
    #include "libavutil/opt.h"
    #include "libavcodec/avfft.h"
    //#include "libavutil/buffer_internal.h"
    #include "libswresample/swresample.h"
}

#endif // THIRDFFMPEG_H
