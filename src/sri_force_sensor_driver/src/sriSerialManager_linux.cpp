//This is a linux system application program, if it is applied in Window, this program file is not needed
//这是linux系统应用的程序，如果在Window中应用，不需这个程序文件
#include "sri_force_sensor_driver/sriSerialManager_linux.h"
#ifdef  IS_WINDOWS_OS
#else
#include <pthread.h>

SerialManeger_linux::SerialManeger_linux()
{
    m_serial = new serial_linux();
}

SerialManeger_linux::~SerialManeger_linux()
{

}
//Receive serial data
//接收串口数据
void* ReadSerialDate(void* para)
{
    SerialManeger_linux* p_serial_test = (SerialManeger_linux*)para;
    printf("ReadSerialDate Thread Start\n");
    while (1)
    {
        p_serial_test->Read();
        usleep(1000);
    }
    printf("Receive thread break!\n");
    pthread_detach(pthread_self());
    pthread_exit(0);
    return 0;
}

//Read serial port data
//读取串口数据
int SerialManeger_linux::Read()
{
    int dataBufferLen = REC_DATA_LEN;
    int recvDataLen = 0;
    unsigned char* dataBuffer = new unsigned char[REC_DATA_LEN];

    recvDataLen = m_serial->ReadData(m_serial_fd, dataBuffer, REC_DATA_LEN);
    if ((recvDataLen > 0) && (recvDataLen <= dataBufferLen))
    {
        OnReceivedData(dataBuffer, recvDataLen);
        //printf("REC Data %d\n", recvDataLen);
        memset(dataBuffer, 0, recvDataLen);
        return 0;
    }
    return -1;
}

//Process the received data
//处理接收到的数据
bool SerialManeger_linux::OnReceivedData(BYTE* data, int dataLen)
{
    for (size_t i = 0; i < mParserList.size(); ++i)
    {
        CSRICommParser* parser = mParserList[i];
        if (parser != NULL)
        {
            parser->OnReceivedData(data, dataLen);
        }
    }
    return true;
}

//Add parser
//添加解析器
bool SerialManeger_linux::AddCommParser(CSRICommParser* parser)
{
    mParserList.push_back(parser);
    return true;
}
bool SerialManeger_linux::SetNetworkFailureCallbackFunction(SRICommNetworkFailureCallbackFunction networkFailureCallback)
{
    mNetworkFailureCallback = networkFailureCallback;
    return true;
}

//Open Port
//打开串口
int SerialManeger_linux::OpenSerialPort(int index)
{
    m_serial_fd = m_serial->OpenPort(index);
    if (m_serial_fd < 0)
    {
        printf("Open serial port %d failed!\n", index);
        return -1;
    }
    else
    {
        printf("Open serial port %d Success\n", m_serial_fd);
    }
}

//running the serial port receiving thread
//启动串口接收线程
int SerialManeger_linux::SerialRun(int BaudRate)
{
    m_serial->SetPara(m_serial_fd, BaudRate);

    if (pthread_create(&m_rec_threadid, NULL, ReadSerialDate, this) != 0)
    {
        printf("creat Receive_Thread failed \n");
        m_serial->ClosePort(m_serial_fd);
        m_serial_fd = -1;
        return -1;
    }
    return 0;
}

//Close Port
//关闭串口
int SerialManeger_linux::ClosePort()
{
    m_serial->ClosePort(m_serial_fd);
}

//send data
//发送数据
int SerialManeger_linux::Write(char* data, int datalength)
{
    if (m_serial->WriteData(m_serial_fd, data, datalength) < 0)
    {
        printf("Write Data Fail!\n");
        return -1;
    }
    return 0;
}

#endif



