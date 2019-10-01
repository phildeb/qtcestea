#include "mainwindow.h"

#include <csta_gui.h>
#include <utils.h>
#include <sip.h>
#include <rtp.h>
#include <csta.h>
#include <termios.h>

QAsyncQueue<CVoicePacket*> g_sound_msg_queue_in(100);
QAsyncQueue<CVoicePacket*> g_sound_msg_queue_out(100);

CCSTALink theCSTAlink;
CUserAgentGroup theGroupOfAgent;


#ifdef WITH_GUI

static myDialog* dialog=NULL;
static LiveEvents m_liveEvents;

void myDialog::closeEvent(QCloseEvent *event){
    //event->accept();event->ignore();
    theGroupOfAgent.writeSettings();
    theGroupOfAgent.stopRegistration();
    theGroupOfAgent.stopNetwork();
    theCSTAlink.stopNetwork();
    QDialog::closeEvent(event);
}

void myDialog::aboutToQuitApp()
{
    qDebug() << "aboutToQuitApp... stop threads ... sleep .... delete obj";
}

void myDialog::run()
{    // Add your main code here
    qDebug() << "myDialog.Run";
    // you must call quit when complete or the program will stay in the messaging loop
    theCSTAlink.init();
    theCSTAlink.startNetwork();usleep(ONE_SEC_USLEEP/2);

    theGroupOfAgent.readSettings();
    theGroupOfAgent.startNetwork();
    theGroupOfAgent.startRegistration();
    theGroupOfAgent.setCstaLink(&theCSTAlink);
}


void myDialog::onButtonA()
{
    if (confirmBox("_myCSTAlink.tryConnect ?")==QMessageBox::Cancel)        return;
    theCSTAlink.init();
    theCSTAlink.startNetwork();
    //if (confirmBox("_myCSTAlink.startAuth ?")==QMessageBox::Cancel)        return;
    //theCSTAlink.startAuth();
}

void myDialog::onButtonB()
{
    QString msg = QString("CSTA: Start monitor extension %1 ?").arg(_phoneExtensionLineEdit->text());
    if (confirmBox(qPrintable(msg))==QMessageBox::Cancel) return;
    _phoneExtension = _phoneExtensionLineEdit->text();
    //theCSTAlink.startMonitor(_phoneExtensionLineEdit->text(), "", 0 , 0);
    theCSTAlink.startMonitor("19303818", "", 0 , 0);
    theCSTAlink.startMonitor("3777", "", 0 , 0);
}

void myDialog::onButtonJ()
{
    QString msg = QString("CSTA: Stop monitor extension id %1 ?").arg(_phoneExtensionLineEdit->text());
    if (confirmBox(qPrintable(msg))==QMessageBox::Cancel) return;
    /*theCSTAlink.stopMonitor(theCSTAlink._monitorCrossRefId);*/
    theCSTAlink.stopMonitor(_phoneExtensionLineEdit->text(),"",0,0);
}

void myDialog::onButtonC()
{

    //_myCSTAlink.startRecord("3001",5003,5004);
    QString msg = QString("CSTA: Start recording extension %1 ?").arg(_phoneExtensionLineEdit->text());
    if (confirmBox(qPrintable(msg))==QMessageBox::Cancel) return;
    _phoneExtension = _phoneExtensionLineEdit->text();
    //_myCSTAlink.startRecord(qPrintable(_phoneExtensionLineEdit->text()),55555,55556,0,0,"29.11.0.222");
}

void myDialog::onButtonI()
{
    //lineEditPhoneExtension
    QString msg = QString("CSTA: Stop recording extension %1 ?").arg(_phoneExtensionLineEdit->text());
    if (confirmBox(qPrintable(msg))==QMessageBox::Cancel) return;
    _phoneExtension = _phoneExtensionLineEdit->text();
    //theCSTAlink.stopRecord(_phoneExtension.toLatin1().constData());
}

void myDialog::onButtonD()
{
    theCSTAlink.stopNetwork();
    buttonD->setText("disconnecting...");
    //display("onButtonD");
    //if (confirmBox("sip_ANSWER ?")==QMessageBox::Cancel) return;
    //sip_ANSWER(&my_call);
    //_myCSTAlink.tryConnect();
}

void myDialog::onButtonE()
{    /*    QString m_sSettingsFile = QApplication::applicationDirPath() + "/csta_recorder.ini";
    //QSettings Settings("csta_recorder.ini", QSettings::IniFormat);
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    settings.beginGroup("sip");
    settings.setValue("extension", UA_NUMBER_LOCAL);//    settings.setValue("sip/extension",UA_NUMBER_LOCAL);
    settings.setValue("password", UA_PASSWD);
    settings.setValue("port", SIP_PORT_DEFAULT);
    settings.setValue("rtp_start", SIP_PORT_DEFAULT);
    settings.setValue("rtp_begin", SIP_MIN_RTP_PORT);
    settings.setValue("rtp_end", SIP_MAX_RTP_PORT);
    settings.endGroup();
    settings.beginGroup("network");
    settings.setValue("ip_caller", IP_CALLER);
    settings.setValue("ip_registrar", SIP_PROXY_ADDR);
    settings.endGroup();
    QString sText = settings.value("sip/extension", "").toString();
    qDebug() << settings.fileName();
    settings.sync();    */
    //bouton.setText(settings.value("Identite/nom").toString());
    //if (confirmBox("_myUserAgent.initNetwork ?")==QMessageBox::Cancel) return;
    //_myUserAgent.initNetwork();
}

void myDialog::onButtonF()
{
    if (confirmBox("_myUserAgent.sipRegister ?")==QMessageBox::Cancel) return;
    theGroupOfAgent.stopRegistration();
}
void myDialog::onButtonG()
{
    if (confirmBox("_myUserAgent.sipInvite ?")==QMessageBox::Cancel) return;
    //_myUserAgent.sipInvite();
    if (theGroupOfAgent._uaList.size()>0 ){
        theGroupOfAgent._uaList[0]->changeState(USER_AGENT_INITIATE_OUTGOING );
        theGroupOfAgent._uaList[0]->sipInvite();
    }
}

void myDialog::onButtonH()
{
    if (confirmBox("sip_bye ?")==QMessageBox::Cancel) return;
    //_myUserAgent.sipByeIncomingCall();
    //sip_bye(&my_call);
    if (theGroupOfAgent._uaList.size()>0 ){
        theGroupOfAgent._uaList[0]->sipByeOutgoingCall();
    }
}


void display(QString s) // todo emit with qstring
{
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    QString text = date.toString("dd/MM/yy") + " - " +  time.toString("hh:mm");
    if ( dialog!=NULL ) {
        dialog->bigList->addItem(text+s);
        dialog->bigList->scrollToBottom();
    }
}

void myDialog::onDisplay(QString )
{
    static QTime rateTimer;
    rateTimer.start();

    for( int iLine = 0; iLine < 5; iLine++ )
    {
        bigList->addItem(
              QString( "%1: This is a dummy text" )
              .arg( QString::number( iLine ).rightJustified( 5, '0' ) )
              );

        // Limit at 60 updates/s
        if(rateTimer.elapsed() > 1000/60) {
            bigList->scrollToBottom();
            QApplication::processEvents();
            rateTimer.restart();
        }
    }
    // For the items added after the last processEvents()
    bigList->scrollToBottom();
}

myDialog::myDialog(QWidget *parent)
{    // default settings
    int nWidth = 300;
    int nHeight = 400;
    if (parent != NULL)
        setGeometry(parent->x() + parent->width()/2 - nWidth/2,parent->y() + parent->height()/2 - nHeight/2,nWidth, nHeight);
    else
        resize(nWidth, nHeight);
    _phoneExtension = "19303814"; // todo: qsettings usage
    createMenu();
    createHorizontalGroupBox();
    createGridGroupBox();

    bigEditor = new QTextEdit;
    bigList = new QListWidget;
    bigEditor->setPlainText(tr("This widget takes up all the remaining space in the top-level layout."));

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    okButton->setDefault(true);

    buttonA->setText("CSTA INIT");
    buttonB->setText("CSTA MONITOR");
    buttonC->setText("CSTA RECORD");
    buttonD->setText("CSTA DISC");
    buttonE->setText("INIT SIP");
    buttonF->setText("REGISTER");
    buttonG->setText("INVITE");
    buttonH->setText("BYE");
    buttonI->setText("STOP IP RECORDING");
    buttonJ->setText("STOP MONITOR");
    _phoneExtensionLineEdit->setText(_phoneExtension);

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(buttonA, SIGNAL(clicked()), this, SLOT(onButtonA()) );
    connect(buttonB, SIGNAL(clicked()), this, SLOT(onButtonB()) );
    connect(buttonC, SIGNAL(clicked()), this, SLOT(onButtonC()) );
    connect(buttonD, SIGNAL(clicked()), this, SLOT(onButtonD()) );
    connect(buttonE, SIGNAL(clicked()), this, SLOT(onButtonE()) );
    connect(buttonF, SIGNAL(clicked()), this, SLOT(onButtonF()) );
    connect(buttonG, SIGNAL(clicked()), this, SLOT(onButtonG()) );
    connect(buttonH, SIGNAL(clicked()), this, SLOT(onButtonH()) );
    connect(buttonI, SIGNAL(clicked()), this, SLOT(onButtonI()) );
    connect(buttonJ, SIGNAL(clicked()), this, SLOT(onButtonJ()) );

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(horizontalGroupBox);
    mainLayout->addWidget(gridGroupBox);
    //mainLayout->addWidget(bigEditor);
    //mainLayout->addWidget(bigList);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Basic Layouts"));

    bigList->addItem("azerty");
    bigList->setAutoScroll(true);

    tableLiveEvents = new QTableWidget();//View();
    //tableLiveEvents->setObjectName(QString::fromUtf8("tableLiveEvents"));
    //tableLiveEvents->setGeometry(QRect(17, 670, 1057, 185));
    //maingrid->addWidget(tableLiveEvents,10,0,2,5);
    mainLayout->addWidget(tableLiveEvents);
    tableLiveEvents->setFont( QFont("Times", 8 , QFont::Bold ) ) ;
    //tableLiveEvents->setRowHeight(i, 14);

    tableLiveEvents->setColumnCount(5);
    tableLiveEvents->setAlternatingRowColors(true);
    tableLiveEvents->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    tableLiveEvents->setSelectionMode(QAbstractItemView::NoSelection);
    tableLiveEvents->setEditTriggers( QAbstractItemView::NoEditTriggers );
      QStringList list; list << "Date" << "Heure" << "Type" << "Message" << "Code" ;
    tableLiveEvents->setHorizontalHeaderLabels(list);
    tableLiveEvents->setColumnWidth(3, 510);

    /*int rowcount=0;    {
          rowcount = tableLiveEvents->rowCount();
            tableLiveEvents->insertRow(rowcount );
            QTableWidgetItem* item0=new QTableWidgetItem ("123");	item0->setBackgroundColor( Qt::yellow );
            QTableWidgetItem* item1=new QTableWidgetItem ("description");
            QTableWidgetItem* item2=new QTableWidgetItem ("345");
            tableLiveEvents->setItem(rowcount,0,item0);
            tableLiveEvents->setItem(rowcount,1,item1);
            tableLiveEvents->setItem(rowcount,2,item2);

            tableLiveEvents->item(rowcount,1)->setText("jklhkljlkjl");
            tableLiveEvents->item(rowcount,2)->setText("opghkpghkopg");
            tableLiveEvents->resizeColumnsToContents();    }*/
    QTimer *timer2 = new QTimer(this);
    connect(timer2, SIGNAL(timeout()), this, SLOT(onTimer2()));
    //timer2->setSingleShot(true);
    timer2->start(1000);
    QTimer *timer3 = new QTimer(this);
    /*connect(&m_liveEvents, SIGNAL(signal_linkControlAlphacomUp()), this, SLOT(slot_linkControlAlphacomUp()));
    connect(&m_liveEvents, SIGNAL(signal_linkControlAlphacomDown()), this, SLOT(slot_linkControlAlphacomDown()));
    connect(&m_liveEvents, SIGNAL(signal_AlimentationUp()), this, SLOT(slot_AlimentationUp()));
    connect(&m_liveEvents, SIGNAL(signal_AlimentationDown()), this, SLOT(slot_AlimentationDown()));*/
    m_liveEvents.start();
    //pthread_t t_id;pthread_create (&t_id, NULL, connexion_mysql_thread, NULL);
    connect(timer3, SIGNAL(timeout()), this, SLOT(slot_timerJournaldeBord()));
    timer3->start(3000);
    emit onDisplay("jhwxghjsdgfsdjgfffffffffd");
}

void LiveEvents::run()
{
    fprintf(stderr,"LiveEvents::run %s %d\n",__FILE__,__LINE__);
    while(1)
    {
        sleep(1);
        QStringList* li = new QStringList;
          //time_t t = m_lastEventTs = sqlQuery.value(0).toInt();//time(NULL);
        time_t t = time(NULL);
          char horodate[128];
          strftime(horodate, sizeof(horodate), "%d / %m /%y", localtime(&t));
          *li<< horodate;
          strftime(horodate, sizeof(horodate), "%H:%M:%S", localtime(&t));
          *li<< horodate;
          *li << "EVENEMENT";
          *li << "EVENEMENT";
          *li << "EVENEMENT";
          *li << "EVENEMENT";
          *li << "EVENEMENT";
          *li << "EVENEMENT";
          //*li << sqlQuery.value(1).toString().latin1();
          //*li << sqlQuery.value(2).toString().latin1();
        if ( m_liveEvents.qlist_journalDeBord_mutex.tryLock() )
        {
            m_liveEvents.qlist_journalDeBord.append(li);
            m_liveEvents.qlist_journalDeBord_mutex.unlock();
        }
    }
}

void  myDialog::slot_timerJournaldeBord()
{
    //fprintf(stderr,"slot_timerJournaldeBord\n");
    m_liveEvents.qlist_journalDeBord_mutex.lock();
    {
        while (m_liveEvents.qlist_journalDeBord.size()>0)
        {
            QStringList* li = m_liveEvents.qlist_journalDeBord.takeFirst();
            if ( li ) {
                if ( li->size() >= 5 ){
                    int rowcount = tableLiveEvents->rowCount();
                    tableLiveEvents->insertRow(rowcount );
                    tableLiveEvents->setRowHeight(rowcount, 15);
                        QTableWidgetItem* item0=new QTableWidgetItem (li->at(0));
                        QTableWidgetItem* item1=new QTableWidgetItem (li->at(1));
                        QTableWidgetItem* item2=new QTableWidgetItem (li->at(2));
                        QTableWidgetItem* item3=new QTableWidgetItem (li->at(3));
                        QTableWidgetItem* item4=new QTableWidgetItem (li->at(4));
                        if ( li->at(2)=="ALARME") item0->setBackgroundColor( Qt::red );
                        else item0->setBackgroundColor( Qt::yellow );
                        tableLiveEvents->setItem(rowcount,0,item0);
                        tableLiveEvents->setItem(rowcount,1,item1);
                        tableLiveEvents->setItem(rowcount,2,item2);
                        tableLiveEvents->setItem(rowcount,3,item3);
                        tableLiveEvents->setItem(rowcount,4,item4);
                        if ( rowcount > 200 ) {
                            tableLiveEvents->removeRow(0);
                        }
                        tableLiveEvents->scrollToItem(item0);
                    }
                    delete li;
            }
        }
    }
    m_liveEvents.qlist_journalDeBord_mutex.unlock();
}

void myDialog::onTimer1()
{
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    QString text = date.toString("dd/MM/yy") + " - " +  time.toString("hh:mm");
}
void myDialog::onTimer2()
{
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    QString text = date.toString("dd/MM/yy") + " - " +  time.toString("hh:mm");
}

int myDialog::confirmBox(const char*fmt, ...)
{
    va_list ap;
    char str[1024];
    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    va_end(ap);
    QMessageBox msgBox(this);
    msgBox.setText(QString::fromUtf8(str));
    msgBox.setStandardButtons(QMessageBox::Apply|QMessageBox::Cancel);
    return msgBox.exec();
}

void myDialog::msgBox(const char*fmt, ...)
{
    va_list ap;
    char str[1024];
    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    va_end(ap);
    QMessageBox msgBox(this);
    msgBox.setText(QString::fromUtf8(str));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}
void myDialog::createMenu()
{
    menuBar = new QMenuBar;

    fileMenu = new QMenu(tr("&File"), this);
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);

    connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}

void myDialog::createHorizontalGroupBox()
{
    horizontalGroupBox = new QGroupBox(tr("Horizontal layout"));
    QHBoxLayout *layout = new QHBoxLayout;
    for (int i = 0; i < NumButtons; ++i) {
        buttons[i] = new QPushButton(tr("Button %1").arg(i + 1));
        //layout->addWidget(buttons[i]);
    }
    buttonA = new QPushButton(tr("A"));
    layout->addWidget(buttonA);
    buttonB = new QPushButton(tr("B"));
    layout->addWidget(buttonB);
    buttonC = new QPushButton(tr("C"));
    layout->addWidget(buttonC);
    buttonD = new QPushButton(tr("D"));
    layout->addWidget(buttonD);
    buttonE = new QPushButton(tr("E"));
    layout->addWidget(buttonE);
    buttonF = new QPushButton(tr("F"));
    layout->addWidget(buttonF);
    buttonG = new QPushButton(tr("G"));
    layout->addWidget(buttonG);
    buttonH = new QPushButton(tr("H"));
    layout->addWidget(buttonH);
    buttonI = new QPushButton(tr("I"));
    layout->addWidget(buttonI);
    buttonJ = new QPushButton(tr("J"));
    layout->addWidget(buttonJ);
    horizontalGroupBox->setLayout(layout);
}

void myDialog::createGridGroupBox()
{
    gridGroupBox = new QGroupBox(tr("Grid layout"));
    QGridLayout *layout = new QGridLayout;

    for (int i = 0; i < NumGridRows; ++i) {
        labels[i] = new QLabel(tr("Line %1:").arg(i + 1));
        lineEdits[i] = new QLineEdit;
        layout->addWidget(labels[i], i + 1, 0);
        layout->addWidget(lineEdits[i], i + 1, 1);
    }
    _phoneExtensionLineEdit = new QLineEdit;
    layout->addWidget(_phoneExtensionLineEdit);

    smallEditor = new QTextEdit;
    smallEditor->setPlainText(tr("This widget takes up about two thirds of the grid layout."));

    layout->addWidget(_phoneExtensionLineEdit, 4, 0, 2, 1);
    layout->addWidget(smallEditor, 4, 2, 2, 1);

    layout->setColumnStretch(1, 10);
    layout->setColumnStretch(2, 20);
    gridGroupBox->setLayout(layout);
}

#else


/*  The constructor gets a instance of the QT application and sets it to “app”
    The “run” slot is where your code will actually start execution.
    When you are through running your code you must call “quit” to stop the application.  This will tell the QT application to terminate.
    While the QT application is in the process of terminating it will execute the slot aboutToQuitApp().  This is a good place to do any clean-up work.*/
myMainClass::myMainClass(QObject *parent) :    QObject(parent)
{    // get the instance of the main application
    app = QCoreApplication::instance();
    // setup everything here
    // create any global objects
    // setup debug and warning mode
    _stopRunning = false;
    //QCoreApplication::instance()->installEventFilter(&_eventFilter);
    QObject::connect(this,SIGNAL(finished()), app, SLOT(quit()) ,  Qt::QueuedConnection );
    //QObject::connect(this,SIGNAL(signal_finished(int)),this->parent(),SLOT(slot_beforeExiting()) ,  Qt::QueuedConnection );
}

// x sec after the application starts this method will run
// all QT messaging is running at this point so threads, signals and slots
// will all work as expected.
void myMainClass::run()
{    // Add your main code here
    qDebug() << "MainClass.Run";
    // you must call quit when complete or the program will stay in the messaging loop
    theCSTAlink.init();
    theCSTAlink.startNetwork();usleep(ONE_SEC_USLEEP/2);

    theGroupOfAgent.readSettings();
    theGroupOfAgent.startNetwork();
    theGroupOfAgent.startRegistration();
    theGroupOfAgent.setCstaLink(&theCSTAlink);

    forever    {
        char key = getch();
        qDebug()<< "key pressed:" << key;

        if ( key=='m'){
            //_myCSTAlink.startMonitor(IP_TOUCH_PHONE_NUMBER);
        }
        if ( key=='R'){
            //_myCSTAlink.startRecord(IP_TOUCH_PHONE_NUMBER,55555,55556);
        }

        if ( key=='r'){
           printf("_myUserAgent.sipRegister...");
            //_myUserAgent.sipRegister();
        }
        if ( key=='d'){
            //_myUserAgent.sipInvite();
        }
        if ( key=='h'){
            //_myUserAgent.sipBye();
        }
        if ( key=='q' || key=='Q'){
            quit();
            return;
        }
        //emit KeyPressed(key);
    }
    while(!_stopRunning){
        qDebug() << "MainClass running";
        sleep(1);
    }
    quit();
}

// call this routine to quit the application
void myMainClass::quit()
{
    qDebug() << "MainClass::quit";
    // you can do some cleanup here then do emit finished to signal CoreApplication to quit
    theGroupOfAgent.writeSettings();
    theGroupOfAgent.stopRegistration();
    theGroupOfAgent.stopNetwork();
    theCSTAlink.stopNetwork();
    emit finished();
}

// shortly after quit is called the CoreApplication will signal this routine
//this is a good place to delete any objects that were created in the constructor and/or to stop any threads
void myMainClass::aboutToQuitApp()
{
    qDebug() << "aboutToQuitApp... stop threads ... sleep .... delete obj";
}

bool myCoreApp::event(QEvent *event)
{
    qDebug() << "CoreApp::event..";
    /*if (event->type() == QEvent::KeyPress) {
        qDebug() << "QEvent::KeyPress..";
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Q)
        {
            qDebug() << "QEvent::KeyPress:"<<ke->key();
            qDebug("Quit?");
            //qApp->quit();
            return true;
        }
    }*/
    return QCoreApplication::event(event);
}

void myCoreApp::beforeExiting()
{
    qDebug() << "CoreApp::beforeExiting..";
    quit();
}


#endif
