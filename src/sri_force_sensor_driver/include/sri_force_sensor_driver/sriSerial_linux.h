#ifndef SERIALLINUX_H
#define SERIALLINUX_H
#include "sriCommDefine.h"
#ifdef  IS_WINDOWS_OS


#else
#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <sys/types.h>
#include  <sys/signal.h>
#include  <sys/stat.h>
#include  <fcntl.h>
#include  <termios.h>
#include  <errno.h>
#include  <limits.h>
#include  <string.h>
    enum
    {
        COM0 = 0,
        COM1,
        COM2,
        COM3,
        ttyUSB0,
        ttyUSB1,
        ttyUSB2
    };
    class serial_linux
    {
    public:
        serial_linux();
        ~serial_linux();

        int OpenPort(int index);
        int SetPara(int serialfd, int speed = 2, int databits = 8, int stopbits = 1, int parity = 0);
        int WriteData(int fd, const char* data, int datalength);
        int ReadData(int fd, unsigned char* data, int datalength);
        void ClosePort(int fd);
        int BaudRate(int baudrate);
    };
#endif


#endif
