#ifndef ANALYSISTYPE_H
#define ANALYSISTYPE_H

#include <QMetaType>
const int MAX_AUDIO_STREAM = 50;
const int MAX_VIDEO_STREAM = 50;
const int MAX_SUB_STREAM = 50;

typedef struct{
    int nFlag;//pts是否连续,1:连续(默认)，0：不连续
    long long pts;
}PtsContinue;

typedef struct{
    int nStreamIndex;

    int nMaxPacketSize;
    int nMinPacketSize;
    int nAvePacketSize;//平均视频包大小

    double dMaxInterval;//最大视频关键帧时间间隔(s)
    double dMinInterval;
    double dAveInterval;//平均关键帧间隔

    int nFrameCount;//packet 个数总大小
    int nKeyFrameCount;

    PtsContinue ptsContinue;
}UnPackVideoInfo;

typedef struct{
    int nStreamIndex;

    int nMaxPacketSize;
    int nMinPacketSize;
    int nAvePacketSize;//平均大小

    int nFrameCount;//packet 个数总大小

    PtsContinue ptsContinue;
}UnPackAudioInfo;

typedef UnPackAudioInfo UnPackSubInfo;

typedef struct{
    int nVideoCount;
    int nAudioCount;
    int nSubCount;
    UnPackVideoInfo videoInfo[MAX_VIDEO_STREAM];
    UnPackAudioInfo audioInfo[MAX_AUDIO_STREAM];
    UnPackSubInfo subInfo[MAX_SUB_STREAM];
}UnPackInfo;

typedef struct{
    int stream;
    int flags;
    int size;
    long long pts;
}PacketInfo;
typedef  QVector<PacketInfo> VPacketInfo;


typedef enum{
    UNPACK_START = 0x1,
    UNPACK_PAUSE = 0x2,
    UNPACK_RUN   = 0x4,
    UNPACK_FINISH= 0x8,
    UNPACK_ERROR = 0x10,
    UNPACK_STOP  = 0x20,
    UNPACK_DATA_SAVING  = 0x40
}UnpackStatus;

typedef enum{
    SAVE_NONE,
    SAVE_ROW_SEL,
    SAVE_ROW_INDEX,
    SAVE_ROW_INDEX_ENTIRE,
    SAVE_ALL_INDEX,
    SAVE_ALL_INDEX_ENTIRE
}DataSaveType;

typedef struct{
    unsigned char *data;
    int len;
    int write_size;
}DataBuffer;


Q_DECLARE_METATYPE(UnpackStatus)
Q_DECLARE_METATYPE(UnPackInfo)

#endif // ANALYSISTYPE_H
