#ifndef UTILS_H
#define UTILS_H

#include "common.h"

void initTermios(int echo);
void resetTermios(void);
char getch(void);

std::string RemoveChars(const std::string& source, const std::string& chars) ;
std::string ascii_only(const std::string& source);
std::string chiffre_only(const std::string& source);

extern unsigned char hex2bin(char c);
void hexdump(unsigned char* buf, int len, const char* prefix);
void mylog(unsigned char level,const char* fmt, ...);

int string_append_CRLF(const BYTE* stringtocopy, BYTE* data,int size);
char* strstr_ignoring_CRLF(char* stringtofind, char* data,int size);
char* strcpy_till_CRLF(char* dataout, char* datain);
int get_line_before_CRLF(const char* sip_data, int sip_data_len, char* dest, int dest_len);
int first_CRLFCRLF_len(char* data,int size);
int first_CRLF_len(char* data,int size);

QString ascii_only(QString& source);
QString filter_ascii_digits_only(QByteArray ba);

BYTE Linear16ToAlaw(short originalSample);
unsigned short AlawToLinear16(BYTE originalSample);

template<class T> class QAsyncQueue
{
public:
    QAsyncQueue(uint _max = -1) : _max(_max){}
    void setMax(int mm){
        _max = mm; // -1 : infinite
    }
    ~QAsyncQueue()    {
        clean();
    }
    uint count(){
        _mutex.lock();
        int count = _queue.count();
        _mutex.unlock();
        return count;
    }
    bool isFull(){
        if (-1 == _max)
            return false;

        _mutex.lock();
        int count = _queue.count();
        _mutex.unlock();
        return count >= _max;
    }
    bool isEmpty(){
        _mutex.lock();
        bool empty = _queue.isEmpty();
        _mutex.unlock();
        return empty;
    }
    void clean(){
        _mutex.lock();
        _queue.clear();
        _mutex.unlock();
    }
    void push(const T& t){
        _mutex.lock();
        _queue.enqueue(t);
        _mutex.unlock();
    }
    T pull(){
        _mutex.lock();
        T i = _queue.dequeue();
        _mutex.unlock();
        return i;
    }
private:
    QQueue<T> _queue;
    QMutex _mutex;
    int _max;
};
#endif // UTILS_H
