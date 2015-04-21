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

	memset(&info, 0, sizeof(UnPackAudioInfo));
	if(vPacketInfo.size() == 0)return info;
	info.nFrameCount = vPacketInfo.size();
	unsigned long long nTotalSize = 0;

	info.ptsContinue.nFlag = 1;
	info.nStreamIndex = stream_index;
	if(vPacketInfo.size()){
		info.nMaxPacketSize = vPacketInfo[0].size;
		info.nMinPacketSize = vPacketInfo[0].size;
	}
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

bool PtsSort(const int64_t first_pts, const int64_t second_pts)
{
	return (first_pts < second_pts);
}

UnPackVideoInfo UnpackThread::getUnpackVidioInfo(VPacketInfo &vPacketInfo, int stream_index, AVRational time_base)
{
	UnPackVideoInfo info;

	memset(&info, 0, sizeof(UnPackVideoInfo));
	if(vPacketInfo.size() == 0)return info;
	QVector<int64_t> vKeyFrame;
	unsigned long long nTotalSize = 0;

	info.nFrameCount = vPacketInfo.size();
	info.ptsContinue.nFlag = 1;
	info.nStreamIndex = stream_index;

	if(vPacketInfo.size()){
		info.nMaxPacketSize = vPacketInfo[0].size;
		info.nMinPacketSize = vPacketInfo[0].size;
	}
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

	QVector<int64_t> vInterval;
	if(vKeyFrame.size()){
		for(int i = 1; i < vKeyFrame.size(); i++){
			vInterval.append(vKeyFrame[i] - vKeyFrame[i-1]);
		}
	}

	unsigned long long nTotalInterval = 0;
	int64_t nMaxInterval = 0, nMinInterval = 0, nAveInterval = 0; 
	if(vInterval.size()){
		nMaxInterval = vInterval[0];
		nMinInterval = vInterval[0];
	}
	for(int i = 0; i < vInterval.size(); i++){
		nTotalInterval += vInterval[i];
		nMaxInterval = qMax(nMaxInterval, vInterval[i]);
		nMinInterval = qMin(nMinInterval, vInterval[i]);
	}

	if(vInterval.size() > 0)
		nAveInterval = nTotalInterval / vInterval.size();

	info.dMaxInterval = nMaxInterval * 1.0 * time_base.num / time_base.den;
	info.dMinInterval = nMinInterval * 1.0 * time_base.num / time_base.den;
	info.dAveInterval = nAveInterval * 1.0 * time_base.num / time_base.den;

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
			if(unpackinfo.nVideoCount < MAX_VIDEO_STREAM){
				unpackinfo.videoInfo[unpackinfo.nVideoCount] = getUnpackVidioInfo(vPacketInfo, iter.key(), ic->streams[iter.key()]->time_base);
				unpackinfo.nVideoCount++;
			}
			continue;
		}
		if(ic->streams[iter.key()]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			if(unpackinfo.nAudioCount < MAX_AUDIO_STREAM){
				unpackinfo.audioInfo[unpackinfo.nAudioCount] = getUnpackAudioInfo(vPacketInfo, iter.key());
				unpackinfo.nAudioCount++;
			}
			continue;
		}
		if(ic->streams[iter.key()]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE){
			if(unpackinfo.nSubCount < MAX_SUB_STREAM){
				unpackinfo.subInfo[unpackinfo.nSubCount] = getUnpackAudioInfo(vPacketInfo, iter.key());
				unpackinfo.nSubCount++;
			}
			continue;
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
	long long value_last = 0;
	int wanted_stream[AVMEDIA_TYPE_NB];
	QMap<int, VPacketInfo> mPackets;
	int err, ret = 0;
	int64_t filesize = 0, pos_max = 0;
	UnPackInfo info;

	QSqlDatabase db = m_pWindow->getDatabase();
	db.open();
	QSqlQuery query(db);

	av_init_packet(&pkt);
	memset(wanted_stream, -1, sizeof(wanted_stream));
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

	if(ic->nb_programs <= 1){
		for(unsigned int i = 0; i < ic->nb_streams; i++){
			mPackets[i] = QVector<PacketInfo>();
		}
	}else{
		int video_index = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, wanted_stream[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
		int audio_index = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, wanted_stream[AVMEDIA_TYPE_AUDIO], video_index, NULL, 0);

		if(video_index >= 0)
			mPackets[video_index] = QVector<PacketInfo>();
		if(audio_index >= 0)
			mPackets[audio_index] = QVector<PacketInfo>();
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
			QMap<int, VPacketInfo>::iterator iter = mPackets.find(pkt.stream_index);
			if(iter != mPackets.end()){
				PacketInfo info;
				info.flags = pkt.flags;
				info.pts = pts_tmp;
				info.size = pkt.size;
				info.stream = pkt.stream_index;
				mPackets[pkt.stream_index].append(info);
			}

			AVRational time_base = ic->streams[pkt.stream_index]->time_base;
			double pts_sec, dur_sec;
			pts_sec = pts_tmp != -1 ? ((double)pts_tmp * 1.0 * time_base.num / time_base.den) : -1;
			dur_sec = pkt.duration != -1 ? ((double)pkt.duration * 1.0 * time_base.num / time_base.den) : -1;

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
			int start_size = pkt.size > DATA_START_SIZE ? DATA_START_SIZE : pkt.size;
			int end_size = pkt.size > DATA_END_SIZE ? DATA_START_SIZE : pkt.size;
			QByteArray data_start = QByteArray((const char *)pkt.data, start_size);
			QByteArray data_end = QByteArray((const char *)(pkt.data + (pkt.size - end_size)), start_size);

			query.prepare("insert into avindex values (NULL, :stream_index, :flags, :pos, :size, :pts, :dts, :duration, :data_start, :data_end, :dur_sec, :pts_sec);");
			query.bindValue(":stream_index", pkt.stream_index);
			query.bindValue(":flags", pkt.flags);
			query.bindValue(":pos", QVariant(qlonglong(pkt.pos)));
			query.bindValue(":size", pkt.size);
			query.bindValue(":pts", QVariant(qlonglong(pkt.pts)));
			query.bindValue(":dts", QVariant(qlonglong(pkt.dts)));
			query.bindValue(":duration", pkt.duration);
			query.bindValue(":data_start", data_start);
			query.bindValue(":data_end", data_end);
			query.bindValue(":dur_sec", dur_sec);
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
	if(ic)
		avformat_close_input(&ic);
	return;
ERR:
	av_free_packet(&pkt);
	db.close();
	if(ic)
		avformat_close_input(&ic);
}
