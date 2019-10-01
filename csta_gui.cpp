#if 0

#include <QtGui>
#include <csta_gui.h>
#include <sip.h>
#include <csta.h>


/*
//by Wim Peeters: a console application
int main2(int argc, char *argv[])
{
   QCoreApplication app(argc, argv); //renamed the a to app
   QTextStream qout(stdout); //I connect the stout to my qout textstream

   qout <<     "1. Starting the application\n";
   std::cout << "2. Some normal iostream output before using qDebug\n";
   qDebug() << "3. Some output with qDebug after the iostream output\n";

   //QTimer::singleShot(5000, &app, SLOT(quit())); //stop after 5 seconds
   //QTimer::singleShot(500000, &app, SLOT(quit())); //stop after 5 seconds
   //init_pcap(argc,argv);

   client();

   return app.exec(); //and we run the application
}
class myListWidget : public QListWidget
{
Q_OBJECT
public:
    myListWidget():QListWidget()
    {

      connect(this,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(itemClicked(QListWidgetItem*)));
    };

    ~myListWidget ()
    {};
private slots:

    void itemClicked(QListWidgetItem* item)
    {
      QMessageBox::information(this,"Hello!","You clicked \""+item->text()+"\"");
    };
};

int main6(int argc,char ** argv)
{
  QApplication app(argc,argv);

  QListWidget * list = new QListWidget();
  list->addItem("New Item 1");
  list->addItem("New Item 2");
  list->addItem("New Item 3");
  list->addItem("New Item 4");
  list->setWindowTitle("QListWidget Clicked Signal Example");

  list->show();
  return app.exec();

};

int main3( int argc, char **argv )
{
    QApplication a( argc, argv );

    QPushButton quit( "Quit", 0 );
    quit.resize( 75, 30 );
    quit.setFont( QFont( "Times", 18, QFont::Bold ) );

    QObject::connect( &quit, SIGNAL(clicked()), &a, SLOT(quit()) );

    //a.setMainWidget( &quit );
    quit.show();
    return a.exec();
}*/



#endif
