#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "third_ffmpeg.h"
#include "unpackthread.h"
#include "packet_data_save.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->txt_pkt_data->setFont(QFont("Monospace"));
	ui->txt_unpackinfo->setFont(QFont("Monospace"));

	m_unpackStatus = UNPACK_START;
	m_pUnpackThread = NULL;

	initStatusBar();
	createDB();
	avInit();

	m_timer = new QTimer(this);
	//connect(m_timer,SIGNAL(timeout()),this,SLOT(slot_bind_tableview()));
	ui->tvw_unpack->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tvw_unpack, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slot_show_unpack_menu()));//this是datatable所在窗口
}

MainWindow::~MainWindow()
{
	m_unpackStatus = UNPACK_STOP;
	delete ui;
	delete m_lblStatus;
	delete m_lblAuthor;
	delete m_proBar;
	delete m_timer;
	QFile::remove(DB_FILENAME);
}

void MainWindow::slot_show_unpack_menu()
{
	m_menu = new QMenu(ui->tvw_unpack);

	QAction *action1 = m_menu->addAction("保存选中行包数据");
	QAction *action2 = m_menu->addAction("保存选中行对应索引包数据");
	QAction *action3 = m_menu->addAction("保存全部索引包数据");

    connect(action1, SIGNAL(triggered(bool)), this, SLOT(slot_save_row_cur()));
    connect(action2, SIGNAL(triggered(bool)), this, SLOT(slot_save_row_index()));
    connect(action3, SIGNAL(triggered(bool)), this, SLOT(slot_save_all_index()));

	m_menu->exec(QCursor::pos());//在当前鼠标位置显示
}

void MainWindow::slot_save_row_cur()
{
    QModelIndexList indexes = ui->tvw_unpack->selectionModel()->selectedIndexes();
	QModelIndex index;

    if(indexes.count() == 0)
    {
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("select nothing!!	"));
        return;
    }

    lock();
    if(!(getUnpackStatus() & (UNPACK_FINISH | UNPACK_STOP))){
        unlock();
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
        return;
    }
    unlock();

    m_saveSelect.clear();
	foreach(index, indexes) {
        int id = ui->tvw_unpack->model()->index(index.row(),0).data().toString().toInt();
        m_saveSelect.append(id);
	}
	m_saveType = SAVE_ROW_SEL;
    m_pSaveThread = new PacketDataSaveThread(this);
	setUnpackStatus(UNPACK_DATA_SAVING);
	slot_update_status(UNPACK_DATA_SAVING);
	m_pSaveThread->start();
}

void MainWindow::slot_save_row_index()
{
    QModelIndexList indexes = ui->tvw_unpack->selectionModel()->selectedIndexes();
    QModelIndex index;

    if(indexes.count() == 0)
    {
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("select nothing!!	"));
        return;
    }

    lock();
    if(!(getUnpackStatus() & (UNPACK_FINISH | UNPACK_STOP))){
        unlock();
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
        return;
    }
    unlock();

    m_saveSelect.clear();
    foreach(index, indexes) {
        int id = ui->tvw_unpack->model()->index(index.row(),1).data().toString().toInt();
        m_saveSelect.append(id);
    }
    m_saveType = SAVE_ROW_INDEX;
    m_pSaveThread = new PacketDataSaveThread(this);
    setUnpackStatus(UNPACK_DATA_SAVING);
    slot_update_status(UNPACK_DATA_SAVING);
    m_pSaveThread->start();
}

void MainWindow::slot_save_all_index()
{
    QModelIndexList indexes = ui->tvw_unpack->selectionModel()->selectedIndexes();

    if(indexes.count() == 0)
    {
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("select nothing!!	"));
        return;
    }

    lock();
    if(!(getUnpackStatus() & (UNPACK_FINISH | UNPACK_STOP))){
        unlock();
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
        return;
    }
    unlock();

    m_saveSelect.clear();
    m_saveType = SAVE_ALL_INDEX;
    m_pSaveThread = new PacketDataSaveThread(this);
    setUnpackStatus(UNPACK_DATA_SAVING);
    slot_update_status(UNPACK_DATA_SAVING);
    m_pSaveThread->start();
}

void MainWindow::initStatusBar()
{
	QStatusBar* bar = ui->statusBar;
	m_lblStatus = new QLabel;
	m_lblAuthor = new QLabel;
	m_proBar = new QProgressBar();

	m_lblStatus->setMinimumSize(150, 20);
	m_lblStatus->setFrameShadow(QFrame::Sunken);
	m_lblStatus->setText(QString::fromUtf8("欢迎使用码流分析器"));

	m_lblAuthor->setText(tr("author: yingc   ,mail: jingzhishen@126.com"));
	m_lblAuthor->setFrameStyle(QFrame::Box | QFrame::Sunken);
	m_lblAuthor->setTextFormat(Qt::RichText);
	m_lblAuthor->setOpenExternalLinks(true);

	m_proBar->setTextVisible(true);
	m_proBar->setRange(0,PROGRESS_RANGE-1);
	m_proBar->setValue(0);

	bar->addWidget(m_lblStatus);
	bar->addWidget(m_proBar);
	bar->addPermanentWidget(m_lblAuthor);
}

void MainWindow::createDB()
{
	bool b_success = false;
	QFile::remove(DB_FILENAME);

	if(QSqlDatabase::contains(DB_FILENAME))
		db = QSqlDatabase::database(DB_FILENAME);
	else
		db = QSqlDatabase::addDatabase("QSQLITE", DB_FILENAME);
	db.setDatabaseName(DB_FILENAME);
	db.open();

	QSqlQuery query(db);
	b_success = query.exec("create table avindex (id integer primary key autoincrement, stream_index int, flags int, pos int, size int, pts int, dts int, duration  int, data_start blob, data_end blob, dur_sec double, pts_sec double )");	//创建一个表
	if(!b_success)
		goto error;

	db.close();
	return;

error:
	QMessageBox::warning(0, QObject::tr("Database Error!!  "),db.lastError().text());
	db.close();
	exit(1);
}

bool MainWindow::avInit()
{
	av_register_all();

	return true;
}

void MainWindow::avClose()
{

}

void MainWindow::resetControl()
{
	delete ui->tvw_unpack->model();

	m_proBar->setValue(0);
	ui->txt_pkt_data->setText("");
	ui->txt_unpackinfo->setText("");

	ui->cmb_index->clear();
}

void MainWindow::setUnpackStatusWithLock(UnpackStatus status)
{
	lock();
	setUnpackStatus(status);
	unlock();
}

void MainWindow::initDatabase(QSqlDatabase &db)
{
	if(QSqlDatabase::contains(DB_FILENAME))
		db = QSqlDatabase::database(DB_FILENAME);
	else
		db = QSqlDatabase::addDatabase("QSQLITE", DB_FILENAME);
	db.setDatabaseName(DB_FILENAME);
}

QSqlDatabase& MainWindow::getDatabase()
{
	return db;
}

void MainWindow::bindTableView(const QString &sql)
{
	delete ui->tvw_unpack->model();

	db.open();
	QSqlQueryModel *model = new QSqlQueryModel(ui->tvw_unpack);

	model->setQuery(sql, db);
	//model->removeColumn(0); // don't show the ID
	//"select id,stream_index,flags,pos,size,pts,dts,duration,dur_sec,pts_sec from avindex ";
	model->setHeaderData(0,Qt::Horizontal,QObject::tr("id"));
	model->setHeaderData(1,Qt::Horizontal,QObject::tr("index"));
	model->setHeaderData(2,Qt::Horizontal,QObject::tr("flags"));
	model->setHeaderData(3,Qt::Horizontal,QObject::tr("pos"));
	model->setHeaderData(4,Qt::Horizontal,QObject::tr("size"));
	model->setHeaderData(5,Qt::Horizontal,QObject::tr("pts"));
	model->setHeaderData(6,Qt::Horizontal,QObject::tr("dts"));
	model->setHeaderData(7,Qt::Horizontal,QObject::tr("duration"));
	model->setHeaderData(8,Qt::Horizontal,QObject::tr("dur_sec"));
	model->setHeaderData(9,Qt::Horizontal,QObject::tr("pts_sec"));

	ui->tvw_unpack->setModel(model);
	ui->tvw_unpack->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	ui->tvw_unpack->setColumnWidth(1,40);
	ui->tvw_unpack->setColumnWidth(2,40);
	ui->tvw_unpack->setColumnWidth(3,60);
	ui->tvw_unpack->setColumnWidth(4,70);
	ui->tvw_unpack->setColumnWidth(6,80);
	ui->tvw_unpack->setColumnWidth(7,65);
	ui->tvw_unpack->setColumnWidth(8,65);
	ui->tvw_unpack->setColumnWidth(9,80);
	ui->tvw_unpack->setColumnHidden(0,true);
	//ui->tvw_unpack->resizeColumnsToContents();

	db.close();
}

void MainWindow::slot_bind_tableview()
{
	bindTableView();
}
void MainWindow::slot_update_progressbar(int value)
{
	if(value < PROGRESS_RANGE && value > 0)
		m_proBar->setValue(value);
}

void MainWindow::slot_update_status(UnpackStatus status)
{
	lock();
	switch(status){
		case UNPACK_START:
			m_lblStatus->setText("status:	start...");
			break;
		case UNPACK_PAUSE:
			m_lblStatus->setText("status:	pause...");
			break;
		case UNPACK_RUN:
			m_lblStatus->setText("status:	run...");
			break;
		case UNPACK_FINISH:
			m_lblStatus->setText("status:	finish...");
			break;
		case UNPACK_ERROR:
			m_lblStatus->setText("status:	error...");
			break;
		case UNPACK_STOP:
			m_lblStatus->setText("status:	stop...");
			break;
		case UNPACK_DATA_SAVING:
			m_lblStatus->setText("status:	packet data saving...");
			break;
		default:break;
	}
	unlock();
}

void MainWindow::slot_bind_combox(int nb_streams)
{
	ui->cmb_index->clear();
	ui->cmb_index->addItem("All");
	for(int i = 0; i < nb_streams; i++){
		ui->cmb_index->addItem(QString::number(i));
	}
}

void MainWindow::slot_set_unpack_info(UnPackInfo info)
{
	QString result;
	int nVideoCount = 0, nAudioCount = 0, nSubCount = 0;

	while(nVideoCount < info.nVideoCount){
		if(info.videoInfo[nVideoCount].nFrameCount > 0){
			result.append("video ").append(QString::number(info.videoInfo[nVideoCount].nStreamIndex)).append(":\n");
			result.append("packet info: \n");
			result.append("max=").append(QString::number(info.videoInfo[nVideoCount].nMaxPacketSize)).append(",");;
			result.append("min=").append(QString::number(info.videoInfo[nVideoCount].nMinPacketSize)).append(",");
			result.append("ave=").append(QString::number(info.videoInfo[nVideoCount].nAvePacketSize)).append("");
			result.append("\ninterval info: \n");
			result.append("max=").append(QString::number(info.videoInfo[nVideoCount].dMaxInterval)).append(",");
			result.append("min=").append(QString::number(info.videoInfo[nVideoCount].dMinInterval)).append(",");
			result.append("ave=").append(QString::number(info.videoInfo[nVideoCount].dAveInterval)).append("");

			result.append("\ncontinue = ").append(QString::number(info.videoInfo[nVideoCount].ptsContinue.nFlag)).append(" ");
			if(!info.videoInfo[nVideoCount].ptsContinue.nFlag)
				result.append("pts=").append(QString::number(info.videoInfo[nVideoCount].ptsContinue.pts)).append(" ");

			result.append("\nnframe=").append(QString::number(info.videoInfo[nVideoCount].nFrameCount)).append(" ");
			result.append("nkeyframe=").append(QString::number(info.videoInfo[nVideoCount].nKeyFrameCount)).append(" ");
			result.append("\n");
		}
		nVideoCount++;
	}
	while(nAudioCount < info.nAudioCount){
		if(info.audioInfo[nAudioCount].nFrameCount > 0){
			result.append("\n");
			result.append("audio ").append(QString::number(info.audioInfo[nAudioCount].nStreamIndex)).append(":\n");
			result.append("packet info: \n");
			result.append("max=").append(QString::number(info.audioInfo[nAudioCount].nMaxPacketSize)).append(",");
			result.append("min=").append(QString::number(info.audioInfo[nAudioCount].nMinPacketSize)).append(",");
			result.append("ave=").append(QString::number(info.audioInfo[nAudioCount].nAvePacketSize)).append("");

			result.append("\ncontinue = ").append(QString::number(info.audioInfo[nAudioCount].ptsContinue.nFlag)).append(" ");
			if(!info.audioInfo[nAudioCount].ptsContinue.nFlag)
				result.append("pts=").append(QString::number(info.audioInfo[nAudioCount].ptsContinue.pts)).append(" ");

			result.append("\nnframe=").append(QString::number(info.audioInfo[nAudioCount].nFrameCount));
			result.append("\n");
		}
		nAudioCount++;
	}
	while(nSubCount < info.nSubCount){
		if(info.subInfo[nSubCount].nFrameCount > 0){
			result.append("\n");
			result.append("sub ").append(QString::number(info.subInfo[nSubCount].nStreamIndex)).append(":\n");
			result.append("packet info: \n");
			result.append("max=").append(QString::number(info.subInfo[nSubCount].nMaxPacketSize)).append(",");
			result.append("min=").append(QString::number(info.subInfo[nSubCount].nMinPacketSize)).append(",");
			result.append("ave=").append(QString::number(info.subInfo[nSubCount].nAvePacketSize)).append("");

			result.append("\ncontinue = ").append(QString::number(info.subInfo[nSubCount].ptsContinue.nFlag)).append(" ");
			if(!info.subInfo[nSubCount].ptsContinue.nFlag)
				result.append("pts=").append(QString::number(info.subInfo[nSubCount].ptsContinue.pts)).append(" ");

			result.append("\nnframe=").append(QString::number(info.subInfo[nSubCount].nFrameCount));
			result.append("\n");
		}
		nSubCount++;
	}

	ui->txt_unpackinfo->setText(result);
}

void MainWindow::on_btn_openfile_clicked()
{
	lock();
	if(getUnpackStatus() &	(UNPACK_RUN)){
		unlock();
		QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
		return;
	}
	unlock();

	static QString dir = ".";
	QString s_filefilter = tr("Media Files(*.flv *.rm *.rmvb *.real *.mp4 *.mov *.avi *.mpg *.mpeg  *.mkv *.ts *.m2ts *.3gp *.vob *.dat *.asf  *.mp3 *.aac *.m4a *.ac3 *.wav *.mjpeg)");
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Media"), dir, s_filefilter);
	if(!fileName.isEmpty()){
		if(fileName.count('/') > 0)
			dir = fileName.mid(0, fileName.lastIndexOf("/")+1);
		if(m_pUnpackThread){
			if(!(m_pUnpackThread->isFinished())){
				m_pUnpackThread->wait();
			}

			createDB();
			resetControl();
		}else{
			m_pUnpackThread = new UnpackThread(this);
		}
		m_fileName = fileName;
		ui->lineEdit->setText(fileName);

		setUnpackStatusWithLock(UNPACK_START);
		slot_update_status(UNPACK_START);
		m_lblStatus->setText("status:	start...");
		m_pUnpackThread->start();
	}
}

void MainWindow::on_cmb_index_currentIndexChanged(int index)
{
	lock();
	if(getUnpackStatus() &	(UNPACK_RUN)){
		unlock();
		QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
		return;
	}
	unlock();

	QString sql = SQL_SELECT;
	QString where;
	if(index != 0){
		int cmb_keyframe = ui->cmb_keyframe->currentIndex();
		if(0 != cmb_keyframe){
			where = QString(" where stream_index = %1 and flags = %2;").arg(index - 1).arg(cmb_keyframe-1);
		}else{
			where = QString(" where stream_index = %1;").arg(index - 1);
		}
	}else{
		ui->cmb_keyframe->setCurrentIndex(0);
	}
	sql.append(where);
	bindTableView(sql);
}


void MainWindow::on_cmb_keyframe_currentIndexChanged(int index)
{
	lock();
	if(getUnpackStatus() &	(UNPACK_RUN)){
		unlock();
		QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
		return;
	}
	unlock();

	QString sql = SQL_SELECT;
	QString where;
	int stream_index = ui->cmb_index->currentIndex();
	if(index == 0){
		if(0 != stream_index){
			where = QString(" where stream_index = %1;").arg(stream_index - 1);
		}
	}else{
		if(0 != stream_index){
			where = QString(" where stream_index = %1 and flags = %2;").arg(stream_index - 1).arg(index-1);
		}
	}
	sql.append(where);
	bindTableView(sql);
}


void MainWindow::on_tvw_unpack_clicked(const QModelIndex &index)
{
	lock();
	if(getUnpackStatus() &	(UNPACK_RUN)){
		unlock();
		QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
		return;
	}
	unlock();

	QString sql;
	QString result;
	QString result_start="32FB:";
	QString result_end	="32LB:";

	db.open();
	int id = ui->tvw_unpack->model()->index(index.row(),0).data().toString().toInt();

	sql = QString("select data_start,data_end from avindex where id = %1;").arg(id);
	QSqlQuery query(db);
	query.exec(sql);
	if(true == query.first()){
		QByteArray data_start = query.value(0).toByteArray();
		QByteArray data_end = query.value(1).toByteArray();

		result.append(result_start);
		for(int i = 0; i < data_start.length(); i++){
			char tmp[3];
			snprintf(tmp,sizeof(tmp),"%02x",(unsigned char)((data_start.data())[i]));
			if((i+1) % 8 == 0)
				result.append(tmp).append("   ");
			else
				result.append(tmp).append(" ");
		}
		result.append("\n");

		result.append(result_end);
		for(int i = 0; i < data_end.length(); i++){
			char tmp[3];
			snprintf(tmp,sizeof(tmp),"%02x",(unsigned char)((data_end.data())[i]));
			if((i+1) % 8 == 0)
				result.append(tmp).append("   ");
			else
				result.append(tmp).append(" ");
		}

		ui->txt_pkt_data->setText(result);
	}

	db.close();
}

void MainWindow::on_btn_pause_clicked()
{
	lock();
	UnpackStatus status = getUnpackStatus();
	switch(status){
		case UNPACK_RUN:
			setUnpackStatus(UNPACK_PAUSE);
			break;
		case UNPACK_PAUSE:
			setUnpackStatus(UNPACK_RUN);
			m_waitCond.wakeAll();
			break;
		default:break;
	}
	unlock();
}

void MainWindow::on_btn_stop_clicked()
{
	lock();
	if(getUnpackStatus() &	(UNPACK_RUN | UNPACK_PAUSE)){
		setUnpackStatus(UNPACK_STOP);
		m_waitCond.wakeAll();
	}
	unlock();
}

void MainWindow::on_btn_execsql_clicked()
{
	QString where = ui->txt_where->text().trimmed();
	if(where.length() < 5)
		return;
	lock();
	if(getUnpackStatus() &	(UNPACK_FINISH | UNPACK_STOP)){
		QString sql = SQL_SELECT + where;
		bindTableView(sql);
	}else{
		unlock();
		QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
		return;
	}
	unlock();
}

void MainWindow::on_btn_execsql_2_clicked()
{
	QString sql = ui->txt_select->text().trimmed();
	if(sql.length() < 5)
		return;
	lock();
	if(getUnpackStatus() &	(UNPACK_FINISH | UNPACK_STOP)){
		QString result;
		db.open();
		QSqlQuery query(db);
		query.exec(sql);
		if(true == query.first()){
			int col_num = query.record().count();
			for(int col = 0; col < col_num; col++){
				result.append(query.value(col).toByteArray().data());
				if(col + 1 != col_num)
					result.append(" , ");
			}

			ui->txt_selectout->setText(result);
		}else{
			QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("sql error!!	"));
		}

		db.close();
	}else{
		unlock();
		QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
		return;
	}
	unlock();
}
