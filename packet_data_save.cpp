#include "packet_data_save.h"
#include "mainwindow.h"

#include <QVector>
#include <QMap>

PacketDataSaveThread::PacketDataSaveThread(QObject *pObj)
{
	m_pWindow = (MainWindow*)pObj;

	connect(this,SIGNAL(update_progressbar(int)), m_pWindow, SLOT(slot_update_progressbar(int)));
	connect(this,SIGNAL(update_status(UnpackStatus)), m_pWindow, SLOT(slot_update_status(UnpackStatus)));
}

PacketDataSaveThread::~PacketDataSaveThread()
{

}

int64_t PacketDataSaveThread::getFileSize(QString filename)
{
	int64_t filesize = 0;
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) return 0;
	filesize = file.size();
	file.close();

	return filesize;
}

int PacketDataSaveThread::savePacketData(int id, AVPacket *pkt)
{
	int ret = 0;
	QDir dir;
	QString filename = "./pkt/";

	if(!dir.exists("./pkt/"))
		dir.mkdir("./pkt/");
	filename.append(QString::number(pkt->stream_index)).append("_").append(QString::number(id)).append(".pkt");

    QFile file(filename);
	file.open(QIODevice::WriteOnly);
	file.write(QByteArray((const char *)pkt->data, pkt->size));
	file.close();

	return ret;
}

int PacketDataSaveThread::savePacketDataEntire(int stream_index, DataBuffer *packet)
{
    int ret = 0;
    QDir dir;
    QString filename = "./pkt/";

    if(!dir.exists("./pkt/"))
        dir.mkdir("./pkt/");
    if(!packet)
        return -1;
    filename.append(QString::number(stream_index)).append(".es");

    QFile file(filename);
    file.open(QIODevice::WriteOnly|QIODevice::Append);
    file.write(QByteArray((const char *)packet->data, packet->write_size));
    file.close();

    return ret;
}


void PacketDataSaveThread::run()
{
	AVFormatContext *ic = NULL;
	AVPacket pkt;
    QMap<int, DataBuffer> mDatas;
	long long value_last = 0;
	int wanted_stream[AVMEDIA_TYPE_NB];
	int err, ret = 0;
	int64_t filesize = 0, pos_max = 0;
	int video_index;
	int id_count = 1;

	av_init_packet(&pkt);
	memset(wanted_stream, -1, sizeof(wanted_stream));
	filesize = getFileSize(m_pWindow->m_fileName);

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

	video_index = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, wanted_stream[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
	av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, wanted_stream[AVMEDIA_TYPE_AUDIO], video_index, NULL, 0);

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
		if(ret < 0 || pkt.size <= 0 || pkt.data == NULL){
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
				pkt.pts = -1;
			int64_t pts_tmp = pkt.pts == -1 ? pkt.dts : pkt.pts;

			AVRational time_base = ic->streams[pkt.stream_index]->time_base;
			double pts_sec;
			pts_sec = pts_tmp != -1 ? ((double)pts_tmp * 1.0 * time_base.num / time_base.den) : -1;

			if(0 == strcmp(ic->iformat->name, "mov,mp4,m4a,3gp,3g2,mj2")){
				if(pts_sec > 0 && ic->duration > 0){
					long long value = PROGRESS_RANGE * pts_sec * 1000 * 1000 / ic->duration;
					if(value - value_last >= 10){
						value_last = value;
						emit update_progressbar((int)value);
					}
				}
			}else{
				if(filesize > 0 && pkt.pos > 0){
					pos_max = qMax(pkt.pos, pos_max);
					if(pos_max < filesize){
						long long value = (PROGRESS_RANGE - 1) * pos_max / filesize;
						if(value - value_last >= 10){
							value_last = value;
							emit update_progressbar((int)value);
						}
					}else{
						qDebug() << "pos_max = " << pos_max << ",filesize = " << filesize <<endl;
					}
				}
			}

			int need_save = 0;
			switch(m_pWindow->m_saveType){
				case SAVE_ROW_SEL:
					{
						if(!m_pWindow->m_saveSelect.count()){
							m_pWindow->setUnpackStatusWithLock(UNPACK_FINISH);
							break;
						}
						QList<int>::Iterator it = m_pWindow->m_saveSelect.begin();
						QList<int>::Iterator end = m_pWindow->m_saveSelect.end();
						while ( it != end ) {
							if(*it == id_count ) {
								need_save = 1;
								m_pWindow->m_saveSelect.erase(it);
								break;
							}else {
								++it;
							}
						}
						break;
					}
				case SAVE_ROW_INDEX:
					{
						QList<int>::Iterator it = m_pWindow->m_saveSelect.begin();
						QList<int>::Iterator end = m_pWindow->m_saveSelect.end();
						while ( it != end ) {
                            if(*it == pkt.stream_index ) {
								need_save = 1;
								break;
							}else {
								++it;
							}
						}
						break;
					}
				case SAVE_ALL_INDEX:
					need_save = 1;
					break;
                case SAVE_ROW_INDEX_ENTIRE:
                    {
                        int entire_save = 0;
                        QList<int>::Iterator it = m_pWindow->m_saveSelect.begin();
                        QList<int>::Iterator end = m_pWindow->m_saveSelect.end();
                        while ( it != end ) {
                            if(*it == pkt.stream_index ) {
                                entire_save = 1;
                                break;
                            }else {
                                ++it;
                            }
                        }
                        if(!entire_save)
                            break;
                    }
                case SAVE_ALL_INDEX_ENTIRE:
                {
                    if(pkt.size >= DATA_BUF_SIZE / 2){
                        qDebug() << "pkt.size if too large" <<endl;
                        m_pWindow->setUnpackStatusWithLock(UNPACK_ERROR);
                        emit update_status(UNPACK_ERROR);
                        break;
                    }
                    QMap<int, DataBuffer>::iterator iter = mDatas.find(pkt.stream_index);
                    if(iter == mDatas.end()){
                        DataBuffer df;
                        memset(&df, 0, sizeof(DataBuffer));
                        df.data = (unsigned char *)malloc(DATA_BUF_SIZE);
                        df.len = DATA_BUF_SIZE;
                        if(!df.data){
                            qDebug() << "out of mem df" <<endl;
                            m_pWindow->setUnpackStatusWithLock(UNPACK_ERROR);
                            emit update_status(UNPACK_ERROR);
                            break;
                        }
                        mDatas[pkt.stream_index] = df;
                    }
                    DataBuffer &df = mDatas[pkt.stream_index];
                    if(pkt.size + df.write_size > df.len){
                        //need save to file
                        if(savePacketDataEntire(pkt.stream_index, &df)){
                            //save error
                            m_pWindow->setUnpackStatusWithLock(UNPACK_ERROR);
                            emit update_status(UNPACK_ERROR);
                            break;
                        }
                        df.write_size = 0;
                    }

                    memcpy(df.data + df.write_size, pkt.data, pkt.size);
                    df.write_size += pkt.size;

                    break;
                }

				default:
					break;
			}
			if(need_save){
				if(savePacketData(id_count, &pkt)){
					//save error
					m_pWindow->setUnpackStatusWithLock(UNPACK_ERROR);
					emit update_status(UNPACK_ERROR);
					break;
				}
			}
			id_count++;

			av_free_packet(&pkt);
		}
	}

	if(m_pWindow->m_unpackStatus == UNPACK_FINISH){
		emit update_progressbar(PROGRESS_RANGE-1);
		emit update_status(UNPACK_FINISH);
	}

ERR:
    QMap<int,DataBuffer>::iterator iter;
    for(iter = mDatas.begin(); iter != mDatas.end(); ++iter)
    {
        DataBuffer &df = iter.value();
        if(df.write_size > 0){
            if(savePacketDataEntire(iter.key(), &df)){
                //save error
                m_pWindow->setUnpackStatusWithLock(UNPACK_ERROR);
                emit update_status(UNPACK_ERROR);
            }
        }
        free(df.data);
    }
	av_free_packet(&pkt);
	if(ic)
		avformat_close_input(&ic);
}

