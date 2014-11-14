#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "third_ffmpeg.h"
#include "unpackthread.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
<<<<<<< HEAD
	ui->txt_pkt_data->setFont(QFont("Monospace"));
	ui->txt_unpackinfo->setFont(QFont("Monospace"));

	m_unpackStatus = UNPACK_START;
	m_pUnpackThread = NULL;
=======
    ui->txt_pkt_data->setFont(QFont("Monospace"));
    ui->txt_unpackinfo->setFont(QFont("Monospace"));

	m_unpackStatus = UNPACK_START;
    m_pUnpackThread = NULL;
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c

	initStatusBar();
	createDB();
	avInit();

<<<<<<< HEAD
	m_timer = new QTimer(this);
	//connect(m_timer,SIGNAL(timeout()),this,SLOT(slot_bind_tableview()));
=======
    m_timer = new QTimer(this);
    //connect(m_timer,SIGNAL(timeout()),this,SLOT(slot_bind_tableview()));
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

MainWindow::~MainWindow()
{
	m_unpackStatus = UNPACK_STOP;
	delete ui;
	delete m_lblStatus;
	delete m_lblAuthor;
<<<<<<< HEAD
	delete m_proBar;
	delete m_timer;
=======
    delete m_proBar;
    delete m_timer;
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
	QFile::remove(DB_FILENAME);
}

void MainWindow::initStatusBar()
{
	QStatusBar* bar = ui->statusBar;
	m_lblStatus = new QLabel;
	m_lblAuthor = new QLabel;
<<<<<<< HEAD
	m_proBar = new QProgressBar();

	m_lblStatus->setMinimumSize(150, 20);
	m_lblStatus->setFrameShadow(QFrame::Sunken);
	m_lblStatus->setText(QString::fromUtf8("欢迎使用码流分析器"));

	m_lblAuthor->setText(tr("author: yingc	,mail: jingzhishen@126.com"));
=======
    m_proBar = new QProgressBar();

    m_lblStatus->setMinimumSize(150, 20);
	m_lblStatus->setFrameShadow(QFrame::Sunken);
	m_lblStatus->setText(QString::fromUtf8("欢迎使用码流分析器"));

	m_lblAuthor->setText(tr("author: yingc  ,mail: jingzhishen@126.com"));
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
	m_lblAuthor->setFrameStyle(QFrame::Box | QFrame::Sunken);
	m_lblAuthor->setTextFormat(Qt::RichText);
	m_lblAuthor->setOpenExternalLinks(true);

<<<<<<< HEAD
	m_proBar->setTextVisible(true);
	m_proBar->setRange(0,PROGRESS_RANGE-1);
	m_proBar->setValue(0);

	bar->addWidget(m_lblStatus);
	bar->addWidget(m_proBar);
=======
    m_proBar->setTextVisible(true);
    m_proBar->setRange(0,PROGRESS_RANGE-1);
    m_proBar->setValue(0);

	bar->addWidget(m_lblStatus);
    bar->addWidget(m_proBar);
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
	bar->addPermanentWidget(m_lblAuthor);
}

void MainWindow::createDB()
{
	bool b_success = false;
<<<<<<< HEAD
	QFile::remove(DB_FILENAME);

	if(QSqlDatabase::contains(DB_FILENAME))
		db = QSqlDatabase::database(DB_FILENAME);
	else
		db = QSqlDatabase::addDatabase("QSQLITE", DB_FILENAME);
	db.setDatabaseName(DB_FILENAME);
	db.open();

	QSqlQuery query(db);
	b_success = query.exec("create table avindex (id integer primary key autoincrement, stream_index int, flags int, pos int, size int, pts int, dts int, data_start blob, data_end blob, pts_sec double )");	//创建一个表
=======
    QFile::remove(DB_FILENAME);

    if(QSqlDatabase::contains(DB_FILENAME))
        db = QSqlDatabase::database(DB_FILENAME);
    else
        db = QSqlDatabase::addDatabase("QSQLITE", DB_FILENAME);
    db.setDatabaseName(DB_FILENAME);
	db.open();

	QSqlQuery query(db);
    b_success = query.exec("create table avindex (id integer primary key autoincrement, stream_index int, flags int, pos int, size int, pts int, dts int, data_start blob, data_end blob, pts_sec double )");   //创建一个表
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
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
<<<<<<< HEAD
	delete ui->tvw_unpack->model();

	m_proBar->setValue(0);
	ui->txt_pkt_data->setText("");
	ui->txt_unpackinfo->setText("");

	ui->cmb_index->clear();
=======
    delete ui->tvw_unpack->model();

    m_proBar->setValue(0);
    ui->txt_pkt_data->setText("");
    ui->txt_unpackinfo->setText("");

    ui->cmb_index->clear();
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

void MainWindow::setUnpackStatusWithLock(UnpackStatus status)
{
<<<<<<< HEAD
	lock();
	setUnpackStatus(status);
	unlock();
=======
    lock();
    setUnpackStatus(status);
    unlock();
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

void MainWindow::initDatabase(QSqlDatabase &db)
{
<<<<<<< HEAD
	if(QSqlDatabase::contains(DB_FILENAME))
		db = QSqlDatabase::database(DB_FILENAME);
	else
		db = QSqlDatabase::addDatabase("QSQLITE", DB_FILENAME);
	db.setDatabaseName(DB_FILENAME);
=======
    if(QSqlDatabase::contains(DB_FILENAME))
        db = QSqlDatabase::database(DB_FILENAME);
	else
        db = QSqlDatabase::addDatabase("QSQLITE", DB_FILENAME);
    db.setDatabaseName(DB_FILENAME);
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

QSqlDatabase& MainWindow::getDatabase()
{
<<<<<<< HEAD
	return db;
=======
    return db;
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

void MainWindow::bindTableView(const QString &sql)
{
<<<<<<< HEAD
	delete ui->tvw_unpack->model();

	db.open();
	QSqlQueryModel *model = new QSqlQueryModel(ui->tvw_unpack);

	model->setQuery(sql, db);
	//model->removeColumn(0); // don't show the ID

	model->setHeaderData(0,Qt::Horizontal,QObject::tr("id"));
	model->setHeaderData(1,Qt::Horizontal,QObject::tr("index"));
	model->setHeaderData(2,Qt::Horizontal,QObject::tr("flags"));
	model->setHeaderData(3,Qt::Horizontal,QObject::tr("pos"));
	model->setHeaderData(4,Qt::Horizontal,QObject::tr("size"));
	model->setHeaderData(5,Qt::Horizontal,QObject::tr("pts"));
	model->setHeaderData(6,Qt::Horizontal,QObject::tr("dts"));
	model->setHeaderData(7,Qt::Horizontal,QObject::tr("pts_sec"));

	ui->tvw_unpack->setModel(model);
	ui->tvw_unpack->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	ui->tvw_unpack->setColumnWidth(1,50);
	ui->tvw_unpack->setColumnWidth(2,40);
	ui->tvw_unpack->setColumnWidth(4,70);
	ui->tvw_unpack->setColumnWidth(6,80);
	ui->tvw_unpack->setColumnWidth(7,80);
	ui->tvw_unpack->setColumnHidden(0,true);
	//ui->tvw_unpack->resizeColumnsToContents();

	db.close();
=======
    delete ui->tvw_unpack->model();

    db.open();
    QSqlQueryModel *model = new QSqlQueryModel(ui->tvw_unpack);

    model->setQuery(sql, db);
    //model->removeColumn(0); // don't show the ID

    model->setHeaderData(0,Qt::Horizontal,QObject::tr("id"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("index"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("flags"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("pos"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("size"));
    model->setHeaderData(5,Qt::Horizontal,QObject::tr("pts"));
    model->setHeaderData(6,Qt::Horizontal,QObject::tr("dts"));
    model->setHeaderData(7,Qt::Horizontal,QObject::tr("pts_sec"));

    ui->tvw_unpack->setModel(model);
    ui->tvw_unpack->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tvw_unpack->setColumnWidth(1,50);
    ui->tvw_unpack->setColumnWidth(2,40);
    ui->tvw_unpack->setColumnWidth(4,70);
    ui->tvw_unpack->setColumnWidth(6,80);
    ui->tvw_unpack->setColumnWidth(7,80);
    ui->tvw_unpack->setColumnHidden(0,true);
    //ui->tvw_unpack->resizeColumnsToContents();

    db.close();
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

void MainWindow::slot_bind_tableview()
{
<<<<<<< HEAD
	bindTableView();
}
void MainWindow::slot_update_progressbar(int value)
{
	if(value < PROGRESS_RANGE && value > 0)
		m_proBar->setValue(value);
=======
    bindTableView();
}
void MainWindow::slot_update_progressbar(int value)
{
    if(value < PROGRESS_RANGE && value > 0)
        m_proBar->setValue(value);
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

void MainWindow::slot_update_status(UnpackStatus status)
{
<<<<<<< HEAD
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
		default:break;
	}
	unlock();
=======
    lock();
    switch(status){
        case UNPACK_START:
            m_lblStatus->setText("status:   start...");
            break;
        case UNPACK_PAUSE:
            m_lblStatus->setText("status:   pause...");
            break;
        case UNPACK_RUN:
            m_lblStatus->setText("status:   run...");
            break;
        case UNPACK_FINISH:
            m_lblStatus->setText("status:   finish...");
            break;
        case UNPACK_ERROR:
            m_lblStatus->setText("status:   error...");
            break;
        case UNPACK_STOP:
            m_lblStatus->setText("status:   stop...");
            break;
        default:break;
    }
    unlock();
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

void MainWindow::slot_bind_combox(int nb_streams)
{
<<<<<<< HEAD
	ui->cmb_index->clear();
=======
    ui->cmb_index->clear();
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
	ui->cmb_index->addItem("All");
	for(int i = 0; i < nb_streams; i++){
		ui->cmb_index->addItem(QString::number(i));
	}
}

void MainWindow::slot_set_unpack_info(UnPackInfo info)
{
	QString result;
	int nAudioCount = 0;

	if(info.videoInfo.nFrameCount > 0){
<<<<<<< HEAD
		result.append("video => ").append("stream index->").append(QString::number(info.videoInfo.nStreamIndex)).append(":\n");
=======
        result.append("video => ").append("stream index->").append(QString::number(info.videoInfo.nStreamIndex)).append(":\n");
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
		result.append("packet=> ");
		result.append("max: ").append(QString::number(info.videoInfo.nMaxPacketSize)).append(" ");;
		result.append("min: ").append(QString::number(info.videoInfo.nMinPacketSize)).append(" ");
		result.append("ave: ").append(QString::number(info.videoInfo.nAvePacketSize)).append(" ");
		result.append("\ninterval=> ");
		result.append("max: ").append(QString::number(info.videoInfo.dMaxInterval)).append(" ");
		result.append("min: ").append(QString::number(info.videoInfo.dMinInterval)).append(" ");
		result.append("ave: ").append(QString::number(info.videoInfo.dAveInterval)).append(" ");

		result.append("\nframecount: ").append(QString::number(info.videoInfo.nFrameCount)).append(" ");
		result.append("keyframecount: ").append(QString::number(info.videoInfo.nKeyFrameCount)).append(" ");
		result.append("\n");
	}
	while(nAudioCount < info.nAudioCount){
		if(info.audioInfo[nAudioCount].nFrameCount > 0){
<<<<<<< HEAD
			result.append("audio => ").append("stream index->").append(QString::number(info.audioInfo[nAudioCount].nStreamIndex)).append(":\n");
=======
            result.append("audio => ").append("stream index->").append(QString::number(info.audioInfo[nAudioCount].nStreamIndex)).append(":\n");
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
			result.append("packet=> ");
			result.append("max: ").append(QString::number(info.audioInfo[nAudioCount].nMaxPacketSize)).append(" ");
			result.append("min: ").append(QString::number(info.audioInfo[nAudioCount].nMinPacketSize)).append(" ");
			result.append("ave: ").append(QString::number(info.audioInfo[nAudioCount].nAvePacketSize)).append(" ");
			result.append("\nframecount:").append(QString::number(info.audioInfo[nAudioCount].nFrameCount));
			result.append("\n");

			nAudioCount++;
		}
	}

	ui->txt_unpackinfo->setText(result);
}

void MainWindow::on_btn_openfile_clicked()
{
<<<<<<< HEAD
	lock();
	if(getUnpackStatus() &	(UNPACK_RUN)){
		unlock();
		QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
		return;
	}
	unlock();

	static QString dir = ".";
	QString s_filefilter = tr("Media Files(*.flv *.mp4 *.mov *.avi *.mpg *.mpeg  *.mkv *.ts *.m2ts *.3gp *.vob *.dat *.asf *.mpeg *.mp3 *.aac *.m4a *.ac3 )");
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
		m_lblStatus->setText("status:	start...");
		m_pUnpackThread->start();
=======
    lock();
    if(getUnpackStatus() &  (UNPACK_RUN)){
        unlock();
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!  "));
        return;
    }
    unlock();

    static QString dir = ".";
	QString s_filefilter = tr("Media Files(*.flv *.mp4 *.mov *.avi *.mpg *.mpeg  *.mkv *.ts *.m2ts *.3gp *.vob *.dat *.asf *.mpeg *.mp3 *.aac *.m4a *.ac3 )");
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
        m_lblStatus->setText("status:   start...");
        m_pUnpackThread->start();
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
	}
}

void MainWindow::on_cmb_index_currentIndexChanged(int index)
{
<<<<<<< HEAD
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
=======
    lock();
    if(getUnpackStatus() &  (UNPACK_RUN)){
        unlock();
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!  "));
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
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}


void MainWindow::on_cmb_keyframe_currentIndexChanged(int index)
{
<<<<<<< HEAD
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
=======
    lock();
    if(getUnpackStatus() &  (UNPACK_RUN)){
        unlock();
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!  "));
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
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}


void MainWindow::on_tvw_unpack_clicked(const QModelIndex &index)
{
<<<<<<< HEAD
	lock();
	if(getUnpackStatus() &	(UNPACK_RUN)){
		unlock();
		QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!	"));
		return;
	}
	unlock();

	QString sql;
	QString result;
	QString result_start="The first 32B:";
	QString result_end	="The last	32B:";
=======
    lock();
    if(getUnpackStatus() &  (UNPACK_RUN)){
        unlock();
        QMessageBox::warning(0, QObject::tr("warning  "),QObject::tr("not finish yet!!  "));
        return;
    }
    unlock();

	QString sql;
	QString result;
    QString result_start="The first 32B:";
    QString result_end  ="The last  32B:";
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c

	db.open();
	int id = ui->tvw_unpack->model()->index(index.row(),0).data().toString().toInt();

	sql = QString("select data_start,data_end from avindex where id = %1;").arg(id);
	QSqlQuery query(db);
	query.exec(sql);
	if(true == query.first()){
		QByteArray data_start = query.value(0).toByteArray();
		QByteArray data_end = query.value(1).toByteArray();

<<<<<<< HEAD
		result.append(result_start);
		for(int i = 0; i < data_start.length(); i++){
=======
        result.append(result_start);
        for(int i = 0; i < data_start.length(); i++){
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
			char tmp[3];
			snprintf(tmp,sizeof(tmp),"%02x",(data_start.data())[i]);
			result.append(tmp).append(" ");
		}
		result.append("\n");

		result.append(result_end);
		for(int i = 0; i < data_end.length(); i++){
			char tmp[3];
			snprintf(tmp,sizeof(tmp),"%02x",(data_end.data())[i]);
			result.append(tmp).append(" ");
		}

<<<<<<< HEAD
		ui->txt_pkt_data->setText(result);
	}

	db.close();
=======
        ui->txt_pkt_data->setText(result);
	}

    db.close();
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

void MainWindow::on_btn_pause_clicked()
{
<<<<<<< HEAD
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
=======
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
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}

void MainWindow::on_btn_stop_clicked()
{
<<<<<<< HEAD
	lock();
	if(getUnpackStatus() &	(UNPACK_RUN | UNPACK_PAUSE)){
		setUnpackStatus(UNPACK_STOP);
		m_waitCond.wakeAll();
	}
	unlock();
=======
    lock();
    if(getUnpackStatus() &  (UNPACK_RUN | UNPACK_PAUSE)){
        setUnpackStatus(UNPACK_STOP);
        m_waitCond.wakeAll();
    }
    unlock();
>>>>>>> 3b46b43bb5902163f3d66450aa7f9543e284a64c
}
