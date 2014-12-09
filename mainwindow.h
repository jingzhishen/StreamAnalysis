#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtSql>
#include <QtSql/qsql.h>
#include <QtSql/qsqlquerymodel.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QDebug>

#include <iostream>

#include "analysistype.h"
using namespace std;

namespace Ui {
	class MainWindow;
}

const int DATA_START_SIZE = 32;//存储在数据库中的部分数据，用于对比板子上获取的packet 数据是否跟工具解析出来的数据一致(byte)
const int DATA_END_SIZE = 16;//the last datasize
const int PROGRESS_RANGE = 1000;
//const QString DB_FILENAME = ":memory:";
const QString DB_FILENAME = "avindex.db";
const QString SQL_SELECT = "select id,stream_index,flags,pos,size,pts,dts,duration,dur_sec,pts_sec from avindex ";
class UnpackThread;


class MainWindow : public QMainWindow
{
	Q_OBJECT
		friend class UnpackThread;

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

    UnpackStatus getUnpackStatus(){return m_unpackStatus;}
    void setUnpackStatus(UnpackStatus status){m_unpackStatus = status;}
    void setUnpackStatusWithLock(UnpackStatus status);
    void lock(){m_mutex.lock();}
    void unlock(){m_mutex.unlock();}
    QSqlDatabase& getDatabase();

private slots:
    void on_btn_openfile_clicked();
	void on_cmb_index_currentIndexChanged(int index);
	void on_cmb_keyframe_currentIndexChanged(int index);
	void on_tvw_unpack_clicked(const QModelIndex &index);
    void slot_set_unpack_info(UnPackInfo info);
    void slot_bind_combox(int nb_streams);
    void slot_bind_tableview();
    void slot_update_progressbar(int value);
    void slot_update_status(UnpackStatus status);
    void on_btn_pause_clicked();

    void on_btn_stop_clicked();

    void on_btn_execsql_clicked();

private:
	Ui::MainWindow *ui;

	QLabel* m_lblStatus; //用于显示状态信息
	QLabel* m_lblAuthor;
    QProgressBar *m_proBar;

    UnpackThread *m_pUnpackThread;
    QTimer *m_timer;

	QString m_fileName;
	QWaitCondition m_waitCond;
	QMutex m_mutex;
    UnpackStatus m_unpackStatus;
    QSqlDatabase db;

	void initStatusBar(); //初始化状态栏
	void createDB();
	void avClose();
	bool avInit();


    void initDatabase(QSqlDatabase &db);
    void bindTableView(const QString &sql = SQL_SELECT);
    void resetControl();
};

#endif // MAINWINDOW_H
