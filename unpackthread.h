#ifndef UNPACKTHREAD_H
#define UNPACKTHREAD_H

#include <QThread>
#include <QLabel>

#include "third_ffmpeg.h"
#include "analysistype.h"

class MainWindow;


class UnpackThread: public QThread
{
     Q_OBJECT
public:
    UnpackThread(QObject *pObj = 0);
    ~UnpackThread();

signals:
        void set_unpack_info(UnPackInfo info);
        void bind_combox(int nb_streams);
        void bind_tableview();
        void update_progressbar(int value);
        void update_status(UnpackStatus status);
protected:
    void run();

private:
    MainWindow *m_pWindow;
    UnPackInfo getUnpackInfo(QMap<int, VPacketInfo> &mPackets, const AVFormatContext *ic);
    UnPackAudioInfo getUnpackAudioInfo(VPacketInfo &vPacketInfo, int stream_index);
    UnPackVideoInfo getUnpackVidioInfo(VPacketInfo &vPacketInfo, int stream_index, AVRational time_base);
    int64_t getFileSize(QString filename);
};


#endif // UNPACKTHREAD_H
