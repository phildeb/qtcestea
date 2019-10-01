#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include "mainwindow.h"
#include <QDebug>
#include <QDir>
#include <sys/ioctl.h>
#include <net/if.h>

#ifdef WITH_GUI
#include <QApplication>
#endif

unsigned char adapter_address[6] =  //    Anti EXE Patch : bitwise not on stored MAC
{ ~0x00, ~0x14, ~0x22, ~0x7C, ~0x18, ~0x91 };


void sig_handler(int signo)
{
   syslog (LOG_INFO,"sig_handler %d",signo);
   if (signo == SIGINT){
      printf("received SIGINT\n");
      syslog (LOG_INFO,"SIGINT");
      exit(-1);
   }
   if (signo == SIGUSR1)
      printf("received SIGUSR1\n");
   if (signo == SIGKILL)
      printf("received SIGKILL\n");
   if (signo == SIGSTOP)
      printf("received SIGSTOP\n");
}

int getmac( const char* eth_name /*= "eth0"*/)
{
   int s;
   struct ifreq buffer;

   fprintf(stderr,"get MAC of %s", eth_name);

   s = socket(PF_INET, SOCK_DGRAM, 0);
   memset(&buffer, 0x00, sizeof(buffer));
   strcpy(buffer.ifr_name, eth_name);

   ioctl(s, SIOCGIFHWADDR, &buffer);
   close(s);

   unsigned int sum=0;
   for( s = 0; s < 6; s++ )
   {
      adapter_address[s] = (unsigned char)buffer.ifr_hwaddr.sa_data[s];
      fprintf(stderr,"%.2X ", (unsigned char)buffer.ifr_hwaddr.sa_data[s]);
      //syslog (LOG_INFO,"%.2X ", (unsigned char)buffer.ifr_hwaddr.sa_data[s]);
      sum += adapter_address[s];
   }
   //mylog(2,"sum %s = %d",eth_name,sum);
   return sum;
}


bool CheckMAC()
{
   bool bFound = false;

   if ( 0==getmac("eth0") )
      getmac("eth1");

   unsigned char MACAllowed[][6] = {    // Anti EXE Patch : bitwise not on stored MAC
      { ~0xd4, ~0xae, ~0x52, ~0xd5, ~0x08, ~0x46 },
      { ~0x00, ~0x25, ~0x64, ~0x3B, ~0xBF, ~0xCE },
      { ~0xD0, ~0x67, ~0xE5, ~0xEC, ~0x8C, ~0x55 },
      { ~0x00, ~0x15, ~0xC5, ~0xBB, ~0xE4, ~0x05 }
      //d4:ae:52:d5:08:46
   };

   unsigned iMAC = 0;
   for (iMAC = 0 ; iMAC < ( sizeof(MACAllowed) / sizeof(MACAllowed[0]) )  ; iMAC++)
   {
      unsigned iBYTE = 0;
      //Compare with current allowed MAC
      for (iBYTE = 0 ; (iBYTE < 6) ; iBYTE++)
      {
         if ((unsigned char) MACAllowed[iMAC][iBYTE] != (unsigned char) ~adapter_address[iBYTE])
         {
            break;
         }
      }
      if (iBYTE == 6) {
         bFound = true;
         break;
      }
   }
   if ( bFound == true ) {
      printf("MAC FOUND\n");
      fprintf(stderr,"Linux version supported\n");
   }else{
      printf("MAC NOT FOUND\n");
      fprintf(stderr,"Linux version not supported\n");
   }
   return bFound;
}


int main(int argc, char *argv[])
{
#ifdef WITH_GUI

   qDebug() << "main GUI starting...";
   QApplication app(argc, argv);
   myDialog myDlg;
   myDlg.show();
   QTimer::singleShot(1000, &myDlg, SLOT(run()));

#else

   qDebug() << "main CONSOLE starting...";
   /*A few things worth noting here.
     A instance of the MainClass class called myMain is created.
     A signal in that MainClass called "finished" actually quits the application.
     A signal from the app works with a slot in myMain called aboutToQuitApp
     A 10ms timer sends a signal to the slot run in the myMain class.  This bootstraps your code.
     The last line “return app.exec()” starts all of the QT messaging including the Slots and Signals system across various threads.
     By the time myMain gets the signal on the “run” Slot the QT application structure is up and running.*/
   //QCoreApplication app(argc, argv);
   myCoreApp app(argc, argv);
   myMainClass myMain;// create the main class

   // connect up the signals
   //QObject::connect(&myMain, SIGNAL(finished()),&app, SLOT(slot_beforeExiting()));
   QObject::connect(&app, SIGNAL(aboutToQuit()),&myMain, SLOT(aboutToQuitApp()));
   // This code will start the messaging engine in QT and in ... sec it will start the execution in the MainClass.run routine;
   qDebug() << "timer starting...";
   QTimer::singleShot(1000, &myMain, SLOT(run()));

#endif


   openlog("csta_rec", LOG_PID|LOG_CONS, LOG_USER);
   syslog (LOG_INFO, "argv[0]=[%s]",argv[0]);
   syslog (LOG_INFO, "starting xrec "__DATE__" "__TIME__);
   syslog (LOG_INFO, qPrintable(app.applicationName()) );
   syslog (LOG_INFO, qPrintable(app.applicationDirPath()));
   syslog (LOG_INFO, qPrintable(app.applicationFilePath().constData() ) );
   syslog (LOG_INFO, qPrintable(QString("PID:%1").arg(app.applicationPid()) ) );
   syslog (LOG_INFO, qPrintable(QDir::currentPath()));

   if (signal(SIGINT, sig_handler) == SIG_ERR)
      fprintf(stderr,"\ncan't catch SIGINT\n");
   if (signal(SIGUSR1, sig_handler) == SIG_ERR)
      printf("\ncan't catch SIGUSR1\n");
   if (signal(SIGKILL, sig_handler) == SIG_ERR)
      printf("\ncan't catch SIGKILL\n");
   if (signal(SIGSTOP, sig_handler) == SIG_ERR)
      printf("\ncan't catch SIGSTOP\n");

   CheckMAC();

   if ( argc >= 2 ) {
      syslog (LOG_INFO, "chdir [%s]",argv[1]);
      syslog (LOG_INFO, qPrintable(QDir::currentPath()));
      chdir(argv[1]);
   }

   CDebugThread th;
   th._isStopped = false;
   th.start();

   return app.exec();
}
