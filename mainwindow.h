#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef QT_GUI_LIB
#define WITH_GUI
#endif

#include <QCoreApplication>
#include <QTimer>
#include <QThread>
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QLinkedList>
#include <iostream>
#include <QEvent>

class CDebugThread : public QThread
{
    Q_OBJECT
public:
    CDebugThread(){_isStopped=false;}
    bool _isStopped;
    void run();
};


#ifdef WITH_GUI
#include <QMainWindow>
#include <QApplication>
#include <QtGui>
#include <QDialog>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qwidget.h>

class myDialog : public QDialog
{
    Q_OBJECT
public:
    myDialog(QWidget *parent=NULL);
protected:
    void closeEvent(QCloseEvent *event);
public slots:
    void run();/// This is the slot that gets called from main to start everything but, everthing is set up in the Constructor
    void aboutToQuitApp();/// slot that get signal when that application is about to quit
public:
    int confirmBox(const char*fmt, ...);
    void msgBox(const char*fmt, ...);
    QListWidget *bigList;
private:
    QLineEdit *_phoneExtensionLineEdit;
    QString _phoneExtension;
    void createMenu();
    void createHorizontalGroupBox();
    void createGridGroupBox();
    enum { NumGridRows = 3, NumButtons = 4 };
    QMenuBar *menuBar;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QTextEdit *smallEditor;
    QTextEdit *bigEditor;
    QLabel *labels[NumGridRows];
    QLineEdit *lineEdits[NumGridRows];
    QPushButton *buttons[NumButtons];
    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *buttonA;
    QPushButton *buttonB;
    QPushButton *buttonC;
    QPushButton *buttonD;
    QPushButton *buttonE;
    QPushButton *buttonF;
    QPushButton *buttonG;
    QPushButton *buttonH;
    QPushButton *buttonI;
    QPushButton *buttonJ;
    QMenu *fileMenu;
    QAction *exitAction;
    QTableWidget* tableLiveEvents;

public slots:
    void onButtonA();
    void onButtonB();
    void onButtonC();
    void onButtonD();
    void onButtonE();
    void onButtonF();
    void onButtonG();
    void onButtonH();
    void onButtonI();
    void onButtonJ();
    void onTimer1();
    void onTimer2();
    void slot_timerJournaldeBord();
    void onDisplay(QString str);
};

class LiveEvents : public QThread
{
    Q_OBJECT

public:
    int m_lastEventTs ; //=time(NULL);
    int m_lastAlarmTs ;//= time(NULL);
    QLinkedList<QStringList *>	qlist_journalDeBord;
    QMutex						qlist_journalDeBord_mutex;
    void run();
    /*signals:
    void signal_linkControlAlphacomUp();
    void signal_linkControlAlphacomDown();
    void signal_AlimentationUp();
    void signal_AlimentationDown();*/
};


#else


class myCoreApp : public QCoreApplication
{
    Q_OBJECT
public:
    myCoreApp(int & argc, char ** argv):QCoreApplication(argc,argv){}
    //explicit myCoreApp(int & argc, char ** argv);
    bool event(QEvent *event);
    void beforeExiting();
public slots:
};

class myMainClass : public QObject
{
    Q_OBJECT
private:
    QCoreApplication *app;
    bool _stopRunning;
public:
    explicit myMainClass(QObject *parent = 0);
    void quit();/// Call this to quit application
signals:
    void finished();/// Signal to finish, this is connected to Application Quit
public slots:
    void run();/// This is the slot that gets called from main to start everything but, everthing is set up in the Constructor
    void aboutToQuitApp();/// slot that get signal when that application is about to quit
};


#endif


#endif // MAINWINDOW_H
