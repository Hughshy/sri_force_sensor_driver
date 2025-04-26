#ifndef SERIALMANAGERLINUX_H
#define SERIALMANAGERLINUX_H
#include "sriSerial_linux.h"
#include "sriCommDefine.h"
#include "sriCommParser.h"
#ifdef  IS_WINDOWS_OS
#else
#define REC_DATA_LEN  8192//8k
class SerialManeger_linux
{
public:
    SerialManeger_linux();
    ~SerialManeger_linux();
    int SerialRun(int BaudRate);//SerialManeger_linux::
    int OpenSerialPort(int index);//SerialManeger_linux::

    int Write(char* data, int datalength);
    int Read();
    int ClosePort();

    bool AddCommParser(CSRICommParser* parser);
    bool SetNetworkFailureCallbackFunction(SRICommNetworkFailureCallbackFunction networkFailureCallback);
    bool OnReceivedData(BYTE* data, int dataLen);
private:

    std::vector<CSRICommParser*> mParserList;
    SRICommNetworkFailureCallbackFunction mNetworkFailureCallback;

   serial_linux *m_serial;
    int m_serial_fd;
    pthread_t m_rec_threadid;
    //unsigned char m_rec_data
    //unsigned char m_rec_data[REC_DATA_LEN];
};
#endif




#endif // SERIALTEST_H
