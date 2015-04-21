#ifndef PACKET_DATA_SAVE_H
#define PACKET_DATA_SAVE_H

#include <QThread>
#include <QLabel>

#include "third_ffmpeg.h"
#include "analysistype.h"

class MainWindow;

const int DATA_BUF_SIZE = 0x400000;

class PacketDataSaveThread: public QThread
{
     Q_OBJECT
public:
    PacketDataSaveThread(QObject *pObj = 0);
    ~PacketDataSaveThread();

signals:
        void update_progressbar(int value);
        void update_status(UnpackStatus status);
protected:
    void run();

private:
    MainWindow *m_pWindow;

    int64_t getFileSize(QString filename);
    int savePacketData(int id, AVPacket *pkt);
    int savePacketDataEntire(int stream_index, DataBuffer *packet);
};

#endif // PACKET_DATA_SAVE_H
