//This is a linux system application program, if it is applied in Window, this program file is not needed
//����linuxϵͳӦ�õĳ��������Window��Ӧ�ã�������������ļ�

#include "sri_force_sensor_driver/sriSerial_linux.h"
#ifdef  IS_WINDOWS_OS
#else
serial_linux::serial_linux()
{

}
serial_linux::~serial_linux()
{

}

//BaudRate
//������
int serial_linux::BaudRate(int baudrate)
{
    switch (baudrate)
    {
    case 2400:
        return (B2400);
    case 4800:
        return (B4800);
    case 9600:
        return (B9600);
    case 19200:
        return (B19200);
    case 38400:
        return (B38400);
    case 57600:
        return (B57600);
    case 115200:
        return (B115200);
    default:
        return (B9600);
    }
}

//Configuration parameter
//���ò���
int serial_linux::SetPara(int serialfd, int speed, int databits, int stopbits, int parity)
{
    struct termios termios_new;
    bzero(&termios_new, sizeof(termios_new));//�ȼ���memset(&termios_new,sizeof(termios_new));
    cfmakeraw(&termios_new);//���ǽ��ն�����Ϊԭʼģʽ
    termios_new.c_cflag = BaudRate(speed);
    termios_new.c_cflag |= CLOCAL | CREAD;
    //  termios_new.c_iflag = IGNPAR | IGNBRK;

    termios_new.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 0:
        termios_new.c_cflag |= CS5;
        break;
    case 1:
        termios_new.c_cflag |= CS6;
        break;
    case 2:
        termios_new.c_cflag |= CS7;
        break;
    case 3:
        termios_new.c_cflag |= CS8;
        break;
    default:
        termios_new.c_cflag |= CS8;
        break;
    }

    switch (parity)
    {
    case 0:  				//as no parity
        termios_new.c_cflag &= ~PARENB;    //Clear parity enable
      //  termios_new.c_iflag &= ~INPCK; /* Enable parity checking */  //add by fu
        break;
    case 1:
        termios_new.c_cflag |= PARENB;     // Enable parity
        termios_new.c_cflag &= ~PARODD;
        break;
    case 2:
        termios_new.c_cflag |= PARENB;
        termios_new.c_cflag |= ~PARODD;
        break;
    default:
        termios_new.c_cflag &= ~PARENB;   // Clear parity enable
        break;
    }
    switch (stopbits)// set Stop Bit
    {
    case 1:
        termios_new.c_cflag &= ~CSTOPB;
        break;
    case 2:
        termios_new.c_cflag |= CSTOPB;
        break;
    default:
        termios_new.c_cflag &= ~CSTOPB;
        break;
    }
    tcflush(serialfd, TCIFLUSH); // ������뻺��
    tcflush(serialfd, TCOFLUSH); // ����������
    termios_new.c_cc[VTIME] = 1;   // MIN�� TIME������������֣�1.MIN = 0 , TIME =0  ��READ�����ش� ���򴫻� 0 ,����ȡ�κ���Ԫ
    termios_new.c_cc[VMIN] = 1;  //    2�� MIN = 0 , TIME >0  READ ���ض�������Ԫ,����ʮ��֮һ��󴫻�TIME �������������κ���Ԫ,�򴫻�0
    tcflush(serialfd, TCIFLUSH);  //    3�� MIN > 0 , TIME =0  READ ��ȴ�,ֱ��MIN��Ԫ�ɶ�
    return tcsetattr(serialfd, TCSANOW, &termios_new);  //    4�� MIN > 0 , TIME > 0 ÿһ����Ԫ֮���ʱ�����ᱻ���� READ ���ڶ���MIN��Ԫ,����ֵ��
}
//send data
//��������
int serial_linux::WriteData(int fd, const char* data, int datalength)//index �������ں� 0 ����/dev/ttyAMA1 ......
{
    if (fd < 0) { return -1; }
    int len = 0, total_len = 0;//modify8.
    for (total_len = 0; total_len < datalength;)
    {
        len = 0;
        len = write(fd, &data[total_len], datalength - total_len);
        printf("WriteData fd = %d ,len =%d,data = %s\n", fd, len, data);
        if (len > 0)
        {
            total_len += len;
        }
        else if (len <= 0)
        {
            len = -1;
            break;
        }
    }
    return len;
}

//Receive serial data
//���մ�������
int serial_linux::ReadData(int fd, unsigned char* data, int datalength)
{
    if (fd < 0) { return -1; }
    int len = 0;
    memset(data, 0, datalength);

    int max_fd = 0;
    fd_set readset = { 0 };
    struct timeval tv = { 0 };

    FD_ZERO(&readset);
    FD_SET((unsigned int)fd, &readset);
    max_fd = fd + 1;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    if (select(max_fd, &readset, NULL, NULL, &tv) < 0)
    {
        printf("ReadData: select error\n");
    }
    int nRet = FD_ISSET(fd, &readset);
    if (nRet)
    {
        len = read(fd, data, datalength);
    }
    return len;
}
//Close port
//�رմ���
void serial_linux::ClosePort(int fd)
{
    struct termios termios_old;
    if (fd > 0)
    {
        tcsetattr(fd, TCSADRAIN, &termios_old);
        ::close(fd);
    }
}


//Open Port 
//�򿪴���
int  serial_linux::OpenPort(int index)
{
    char* device;
    struct termios termios_old;
    int fd;
    switch (index)
    {
    case COM0:  device = "/dev/ttyAMA0";  break;
    case COM1:  device = "/dev/ttyAMA1";  break;
    case COM2:  device = "/dev/ttyAMA2";  break;
    case COM3:  device = "/dev/ttyAMA3";  break;
    case ttyUSB0:  device = "/dev/ttyUSB0";  break;
    case ttyUSB1:  device = "/dev/ttyUSB1";  break;
    case ttyUSB2:  device = "/dev/ttyUSB2";  break;
    default: device = "/dev/ttyUSB0"; break;
    }

    fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);//O_RDWR | O_NOCTTY | O_NDELAY   //O_NONBLOCK
    if (fd < 0)
    {
        return -1;
    }
    tcgetattr(fd, &termios_old);
    return fd;
}
#endif


