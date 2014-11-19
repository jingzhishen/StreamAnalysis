#include "unpackthread.h"
#include "mainwindow.h"

#include <QVector>
#include <QMap>

UnpackThread::UnpackThread(QObject *pObj)
{
	m_pWindow = (MainWindow*)pObj;

    connect(this,SIGNAL(set_unpack_info(UnPackInfo)), m_pWindow, SLOT(slot_set_unpack_info(UnPackInfo)));
    connect(this,SIGNAL(bind_tableview()), m_pWindow, SLOT(slot_bind_tableview()));
    connect(this,SIGNAL(bind_combox(int)), m_pWindow, SLOT(slot_bind_combox(int)));
    connect(this,SIGNAL(update_progressbar(int)), m_pWindow, SLOT(slot_update_progressbar(int)));
    connect(this,SIGNAL(update_status(UnpackStatus)), m_pWindow, SLOT(slot_update_status(UnpackStatus)));
}

UnpackThread::~UnpackThread()
{

}
UnPackAudioInfo UnpackThread::getUnpackAudioInfo(VPacketInfo &vPacketInfo, int stream_index)
{
	UnPackAudioInfo info;

	if(vPacketInfo.size() == 0)return info;
	memset(&info, 0, sizeof(UnPackAudioInfo));
	info.nFrameCount = vPacketInfo.size();
	unsigned long long nTotalSize = 0;

    info.ptsContinue.nFlag = 1;
    info.nStreamIndex = stream_index;
	for(int i = 0; i < vPacketInfo.size(); i++){
		nTotalSize += vPacketInfo[i].size;
		info.nMaxPacketSize = qMax(info.nMaxPacketSize, vPacketInfo[i].size);
		info.nMinPacketSize = qMin(info.nMinPacketSize, vPacketInfo[i].size);

        if(info.ptsContinue.nFlag && i > 0 && vPacketInfo[i].pts < vPacketInfo[i-1].pts){
            info.ptsContinue.nFlag = 0;
            info.ptsContinue.pts = vPacketInfo[i].pts;
        }
	}
    info.nAvePacketSize = nTotalSize / vPacketInfo.size();

	return info;
}

bool PtsSort(const int first_pts, const int second_pts)
{
    return (first_pts < second_pts);
}

UnPackVideoInfo UnpackThread::getUnpackVidioInfo(VPacketInfo &vPacketInfo, int stream_index, AVRational time_base)
{
	UnPackVideoInfo info;

	if(vPacketInfo.size() == 0)return info;
    QVector<int> vKeyFrame;
    unsigned long long nTotalSize = 0;

    memset(&info, 0, sizeof(UnPackVideoInfo));
	info.nFrameCount = vPacketInfo.size();
    info.ptsContinue.nFlag = 1;
    info.nStreamIndex = stream_index;

    for(int i = 0; i < vPacketInfo.size(); i++){
		nTotalSize += vPacketInfo[i].size;
		info.nMaxPacketSize = qMax(info.nMaxPacketSize, vPacketInfo[i].size);
		info.nMinPacketSize = qMin(info.nMinPacketSize, vPacketInfo[i].size);

        if(info.ptsContinue.nFlag && i > 0 && vPacketInfo[i].pts < vPacketInfo[i-1].pts){
            info.ptsContinue.nFlag = 0;
            info.ptsContinue.pts = vPacketInfo[i].pts;
        }

        if(vPacketInfo[i].flags == 1){
			info.nKeyFrameCount++;
            if(vPacketInfo[i].pts < 0)continue;
            vKeyFrame.append(vPacketInfo[i].pts);
        }
	}
    info.nAvePacketSize = nTotalSize / vPacketInfo.size();

    qSort(vKeyFrame.begin(),vKeyFrame.end(),PtsSort);

    QVector<int> vInterval;
    for(int i = 0; i < vKeyFrame.size(); i++){
        if(i != 0){
            vInterval.append(vKeyFrame[i] - vKeyFrame[i-1]);
        }
    }

	unsigned long long nTotalInterval = 0;
	for(int i = 0; i < vInterval.size(); i++){
		nTotalInterval += vInterval[i];
        info.dMaxInterval = qMax((int)info.dMaxInterval, vInterval[i]);
        info.dMinInterval = qMin((int)info.dMinInterval, vInterval[i]);
	}

    if(vInterval.size() > 0)
        info.dAveInterval = nTotalInterval / vInterval.size();

    info.dMaxInterval = (double)info.dMaxInterval * 1.0 * time_base.num / time_base.den;
    info.dMinInterval = (double)info.dMinInterval * 1.0 * time_base.num / time_base.den;
    info.dAveInterval = (double)info.dAveInterval * 1.0 * time_base.num / time_base.den;

	return info;
}


UnPackInfo UnpackThread::getUnpackInfo(QMap<int, VPacketInfo> &mPackets, const AVFormatContext *ic)
{
	UnPackInfo unpackinfo;

	memset(&unpackinfo, 0, sizeof(UnPackInfo));
	QMap<int,VPacketInfo>::iterator iter;
	for(iter = mPackets.begin(); iter != mPackets.end(); ++iter)
	{
		VPacketInfo &vPacketInfo = iter.value();
		if(ic->streams[iter.key()]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            unpackinfo.videoInfo = getUnpackVidioInfo(vPacketInfo, iter.key(), ic->streams[iter.key()]->time_base);
		}
		if(ic->streams[iter.key()]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            unpackinfo.audioInfo[unpackinfo.nAudioCount] = getUnpackAudioInfo(vPacketInfo, iter.key());
            unpackinfo.nAudioCount++;
		}
	}

	return unpackinfo;
}

int64_t UnpackThread::getFileSize(QString filename)
{
    int64_t filesize = 0;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return 0;
    filesize = file.size();
    file.close();

    return filesize;
}
void UnpackThread::run()
{
	AVFormatContext *ic = NULL;
	AVPacket pkt;
    QMap<int, VPacketInfo> mPackets;
    int err, ret = 0;
    int64_t filesize = 0, pos_max = 0;
    UnPackInfo info;

    QSqlDatabase db = m_pWindow->getDatabase();
    db.open();
    QSqlQuery query(db);

    av_init_packet(&pkt);
    filesize = getFileSize(m_pWindow->m_fileName);
    m_pWindow->setUnpackStatusWithLock(UNPACK_RUN);
    emit update_status(UNPACK_RUN);

    ic = avformat_alloc_context();
    err = avformat_open_input(&ic, m_pWindow->m_fileName.toStdString().data(), NULL, NULL);
    if(err < 0){
        m_pWindow->setUnpackStatusWithLock(UNPACK_ERROR);
        emit update_status(UNPACK_ERROR);
        goto ERR;
    }
    err = avformat_find_stream_info(ic, NULL);
    if(err < 0){
        m_pWindow->setUnpackStatusWithLock(UNPACK_ERROR);
        emit update_status(UNPACK_ERROR);
        goto ERR;
    }
    av_dump_format(ic, 0, m_pWindow->m_fileName.toStdString().data(), 0);

    for(unsigned int i = 0; i < ic->nb_streams; i++){
        if(ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO \
                || ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            mPackets[i] = QVector<PacketInfo>();
        }
    }

    db.transaction();
    while(1){
        m_pWindow->lock();
        if(m_pWindow->getUnpackStatus() == UNPACK_PAUSE){
            emit update_status(m_pWindow->getUnpackStatus());
            m_pWindow->m_waitCond.wait(&m_pWindow->m_mutex);
            emit update_status(m_pWindow->getUnpackStatus());
        }
        if(m_pWindow->getUnpackStatus() &  (UNPACK_FINISH | UNPACK_STOP | UNPACK_ERROR)){
            emit update_status(m_pWindow->getUnpackStatus());
            m_pWindow->unlock();
            break;
        }
        m_pWindow->unlock();

        ret = av_read_frame(ic, &pkt);
        if(ret < 0){
            if (ret == AVERROR_EOF || url_feof(ic->pb)){
                m_pWindow->setUnpackStatusWithLock(UNPACK_FINISH);
            }else{
                m_pWindow->setUnpackStatusWithLock(UNPACK_ERROR);
                emit update_status(UNPACK_ERROR);
            }
            break;
        }else{
            if(pkt.dts == AV_NOPTS_VALUE)
                pkt.dts = -1;
            if(pkt.pts == AV_NOPTS_VALUE)
                pkt.pts = pkt.dts;
            QMap<int, VPacketInfo>::iterator iter = mPackets.find(pkt.stream_index);
            if(iter != mPackets.end()){
                PacketInfo info;
                info.flags = pkt.flags;
                info.pts = pkt.pts;
                info.size = pkt.size;
                info.stream = pkt.stream_index;
                mPackets[pkt.stream_index].append(info);
            }

            AVRational time_base = ic->streams[pkt.stream_index]->time_base;
            double pts_sec;
            pts_sec = pkt.pts != -1 ? ((double)pkt.pts * 1.0 * time_base.num / time_base.den) : -1;

//            if(pts_sec > 0 && ic->duration > 0){
//                long long value = PROGRESS_RANGE * pts_sec * 1000 * 1000 / ic->duration;
//                emit update_progressbar((int)value);
//            }
            if(filesize > 0 && pkt.pos > 0){
                pos_max = qMax(pkt.pos, pos_max);
                if(pos_max < filesize){
                    long long value = (PROGRESS_RANGE - 1) * pos_max / filesize;
                    emit update_progressbar((int)value);
                }else{
                    qDebug() << "pos_max = " << pos_max << ",filesize = " << filesize <<endl;
                }
            }
            int start_size = pkt.size > DATA_START_SIZE ? DATA_START_SIZE : pkt.size;
            int end_size = pkt.size > DATA_END_SIZE ? DATA_START_SIZE : pkt.size;
            QByteArray data_start = QByteArray((const char *)pkt.data,start_size);
            QByteArray data_end = QByteArray((const char *)(pkt.data + (pkt.size - end_size)),start_size);

            query.prepare("insert into avindex values (NULL, :stream_index, :flags, :pos, :size, :pts, :dts, :data_start, :data_end, :pts_sec);");
            query.bindValue(":stream_index", pkt.stream_index);
            query.bindValue(":flags", pkt.flags);
            query.bindValue(":pos", pkt.pos);
            query.bindValue(":size", pkt.size);
            query.bindValue(":pts", pkt.pts);
            query.bindValue(":dts", pkt.dts);
            query.bindValue(":data_start", data_start);
            query.bindValue(":data_end", data_end);
            query.bindValue(":pts_sec", pts_sec);
            query.exec();

            av_free_packet(&pkt);
        }
    }
    db.commit();
    db.close();
    emit bind_tableview();
    emit bind_combox(ic->nb_streams);

    info = getUnpackInfo(mPackets,ic);
    emit set_unpack_info(info);

    if(m_pWindow->m_unpackStatus == UNPACK_FINISH){
        emit update_progressbar(PROGRESS_RANGE-1);
        emit update_status(UNPACK_FINISH);
    }

    av_free_packet(&pkt);
    avformat_close_input(&ic);
    return;
ERR:
    av_free_packet(&pkt);
    db.close();
	avformat_close_input(&ic);
//    exec();
}
