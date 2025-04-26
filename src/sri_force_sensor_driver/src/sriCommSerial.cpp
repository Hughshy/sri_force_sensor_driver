//This is a window system application program, if it is applied in linux, this program file is not needed
#include "sri_force_sensor_driver/sriCommSerial.h"

CSRICommSerial::CSRICommSerial()
{



}


CSRICommSerial::~CSRICommSerial()
{
	mParserList.clear();	
}

//Open Serial communication
bool CSRICommSerial::OpenPort(std::string   cPort)
{	
	LPCWSTR  Port;
	Port = stringToLPCWSTR(cPort);
	try
	{
		hCom = CreateFile(
			Port,						  //�˿ںţ���Ҫ�򿪵Ĵ����߼���
			GENERIC_READ | GENERIC_WRITE, //����ģʽ����������д
			0,							  //����ģʽ��ָ���������ԣ����ڴ��ڲ��ܹ������ò���������Ϊ0,��ռ��ʽ
			NULL,						  //��ȫ���ã����ð�ȫ�����Խṹ��ȱʡֵΪNULL
			OPEN_EXISTING,				  //�򿪶����Ǵ������ò�����ʾ�豸�������,���򴴽�ʧ��
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, //�ص���ʽ
			NULL //�Դ��ڶ��Ըò���������ΪNULL
		);
		if (hCom == INVALID_HANDLE_VALUE)
		{
			printf("�򿪴���ʧ��!\n");
			return FALSE;
		}
		else
		{
			printf("�򿪴��ڳɹ���\n");
		}
		return TRUE;
	}
	catch(std::exception ex)
	{
		mLastError = ex.what();
		return false;
	}
	return true;
}

//runing system
bool CSRICommSerial::run(int BaudRate)
{
	if (Config_Com(BaudRate) == false)
	{
		return false;
	}

	return true;
}


//Configure serial port parameters and start serial port communication
bool CSRICommSerial::Config_Com(int BaudRate)
{
	SetupComm(hCom, 8192, 8192); //���뻺����������������Ĵ�С����1024

	COMMTIMEOUTS TimeOuts; 
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 0; //�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(hCom, &TimeOuts); //���ó�ʱ

	DCB dcb;
	GetCommState(hCom, &dcb);
	dcb.BaudRate = BaudRate; //������Ϊ9600
	dcb.ByteSize = 8; //ÿ���ֽ���8λ
	dcb.Parity = NOPARITY; //����żУ��λ
	dcb.StopBits = TWOSTOPBITS; //����ֹͣλ
	SetCommState(hCom, &dcb);

	//��ջ���
	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);

	m_ovRead.hEvent = CreateEvent(NULL, false, false, NULL);
	m_ovWrite.hEvent = CreateEvent(NULL, false, false, NULL);
	m_ovWait.hEvent = CreateEvent(NULL, false, false, NULL);

	//SetCommMask����Ҫ��ص��¼� 
	//EV_RXCHAR�����뻺���������յ����ݣ������յ�һ���ֽڲ��������뻺������
	//EV_ERR����·״̬���󣬰�����CE_FRAME / CE_OVERRUN / CE_RXPARITY 3�ִ���
	SetCommMask(hCom, EV_ERR | EV_RXCHAR);

	//_beginThreadex������ȡ�߳�  
	m_Thread = (HANDLE)_beginthreadex(NULL, 0, &CSRICommSerial::ComRecv, this, 0, NULL);
	m_IsOpen = true;

	return TRUE;
}
//Serial port sending data
//���ڷ�������
bool CSRICommSerial::ComWrite(LPBYTE buf, int& len)
{
	BOOL rtn = FALSE;
	DWORD WriteSize = 0;   //DWORD ���� unsigned long
	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_TXABORT);
	m_ovWait.Offset = 0;
	rtn = WriteFile(hCom, buf, len, &WriteSize, &m_ovWrite);
	if (FALSE == rtn && WSAGetLastError() == ERROR_IO_PENDING)//��̨��ȡ
	{
		//�ȴ�����д�����
		printf("�ѷ��� ��");
		for (int i = 0; i < len; i++)
			printf("%c ", buf[i]);
		printf("\n");
	}
	return rtn != FALSE;
}

//Serial port to receive data
//���ڽ�������
unsigned int __stdcall CSRICommSerial::ComRecv(void* LPParam)//unsigned int __stdcall CSRICommSerial::ComRecv(void* LPParam)
{
	CSRICommSerial* obj = static_cast<CSRICommSerial*>(LPParam);
	DWORD WaitEvent = 0, Bytes = 0;
	BOOL Status = FALSE;
	BYTE ReadBuf[4096];
	int dataBufferLen = 4096;
	int recvDataLen = 0;


	DWORD Error;
	COMSTAT cs = { 0 };
	//int i;

	while (obj->m_IsOpen)
	{
		WaitEvent = 0;
		obj->m_ovWait.Offset = 0;
		Status = WaitCommEvent(obj->hCom, &WaitEvent, &obj->m_ovWait);
		/*
		WaitCommEvent�ȴ�����ͨ���¼��ķ���
		��;�������ж���SetCommMask()�������õĴ���ͨ���¼��Ƿ��ѷ�����
		ԭ�ͣ�BOOL WaitCommEvent(HANDLE hFile,LPDWORD lpEvtMask,LPOVERLAPPED lpOverlapped);
		����˵����
		-hFile�����ھ��
		-lpEvtMask:����ִ����������⵽����ͨ���¼��Ļ��ͽ���д��ò����С�
		-lpOverlapped���첽�ṹ�����������첽���������
		*/

		//GetLastError()��������ERROR_IO_PENDING,�����������ڽ��ж�����

		if (FALSE == Status && WSAGetLastError() == ERROR_IO_PENDING)
		{
			// GetOverlappedResult���������һ��������ΪTRUE��������һֱ�ȴ���ֱ����������ɻ����ڴ�������ء�
			Status = GetOverlappedResult(obj->hCom, &obj->m_ovWait, &Bytes, TRUE);
		}
		//��ʹ��ReadFile �������ж�����ǰ��Ӧ��ʹ��ClearCommError�����������
		ClearCommError(obj->hCom, &Error, &cs);
		if (TRUE == Status //�ȴ��¼��ɹ�
			&& WaitEvent & EV_RXCHAR//�����������ݵ���
			&& cs.cbInQue > 0)//������		
		{
			Bytes = 0;
			obj->m_ovRead.Offset = 0;
			/*
			BOOL ReadFile(
			HANDLE hFile, //���ڵľ��
			LPVOID lpBuffer,// ��������ݴ洢�ĵ�ַ,����������ݽ��洢���Ը�ָ���ֵΪ�׵�ַ��һƬ�ڴ���
			DWORD nNumberOfBytesToRead,// Ҫ��������ݵ��ֽ���
			LPDWORD lpNumberOfBytesRead,// ָ��һ��DWORD��ֵ������ֵ���ض�����ʵ�ʶ�����ֽ���
			LPOVERLAPPED lpOverlapped // �ص�����ʱ���ò���ָ��һ��OVERLAPPED�ṹ��ͬ������ʱ���ò���ΪNULL
			);
			*/
			memset(ReadBuf, 0, sizeof(ReadBuf));
			Status = ReadFile(obj->hCom, &ReadBuf, dataBufferLen, &Bytes, &obj->m_ovRead);

			//if (Status != FALSE)
			//{
			//	printf("�յ� ��");
			//	for (recvDataLen = 0; recvDataLen < Bytes; recvDataLen++)
			//	{
			//		printf("%d ", ReadBuf[recvDataLen]);
			//	}
			//	printf("\n");
			//}
			//PurgeComm������մ��ڵ��������������
			PurgeComm(obj->hCom, PURGE_RXCLEAR | PURGE_RXABORT);

			if (Status != FALSE)
			{			
				if ((Bytes > 0) && (Bytes < dataBufferLen))//
				{
					obj->OnReceivedData(ReadBuf, Bytes);
					memset(ReadBuf, 0, Bytes);
				}
				else if (Bytes == 0)
				{
					continue;
				}
			}

		}
	}
	return 0;
}
//On Received Data
//��������
bool CSRICommSerial::OnReceivedData(BYTE* data, int dataLen)
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

//Close Port
//�رմ���
void CSRICommSerial::Close_Com()
{
	m_IsOpen = false;
	if (INVALID_HANDLE_VALUE != hCom)
	{
		CloseHandle(hCom);
		hCom = INVALID_HANDLE_VALUE;
	}
	if (NULL != m_ovRead.hEvent)
	{
		CloseHandle(m_ovRead.hEvent);
		m_ovRead.hEvent = NULL;
	}
	if (NULL != m_ovWrite.hEvent)
	{
		CloseHandle(m_ovWrite.hEvent);
		m_ovWrite.hEvent = NULL;
	}
	if (NULL != m_ovWait.hEvent)
	{
		CloseHandle(m_ovWait.hEvent);
		m_ovWait.hEvent = NULL;
	}
	if (NULL != m_Thread)
	{
		WaitForSingleObject(m_Thread, 5000);//�ȴ��߳̽���  
		CloseHandle(m_Thread);
		m_Thread = NULL;
	}
}






//Check if the connection timed out
//������ӳ�ʱ
bool CSRICommSerial::CheckTimeoutError()
{
#ifdef  IS_WINDOWS_OS
	int errorCode = WSAGetLastError();
	if (WSAETIMEDOUT == errorCode)//
	{
		return true;
	}
#else
	//#define ETIMEDOUT       110     /* Connection timed out */  
	if (ETIMEDOUT == errno)
	{
		return true;
	}
#endif
	return false;
}

//Get error information
//��ȡ������Ϣ
void CSRICommSerial::GetLastSerialError(std::string functionName ="")
{
	mLastError = "";
#ifdef  IS_WINDOWS_OS
	char buffer[2048];
	memset(buffer, 0, 2048);
	sprintf(buffer, "Serial %s error: %d\n", functionName.data(), WSAGetLastError());
	mLastError = buffer;
#else
	char buffer[2048];
	memset(buffer, 0, 2048);
	sprintf(buffer, "Serial %s error: %s(errno: %d)\n", functionName.data(), strerror(errno), errno);
	mLastError = buffer;
#endif

	//OnNetworkFailure();
}


//Add parser
//���ӽ�����
bool CSRICommSerial::AddCommParser(CSRICommParser* parser)
{
	mParserList.push_back(parser);
	return true;
}

bool CSRICommSerial::SetNetworkFailureCallbackFunction(SRICommNetworkFailureCallbackFunction networkFailureCallback)
{
	mNetworkFailureCallback = networkFailureCallback;
	return true;
}

std::string CSRICommSerial::GetLastError()
{
	return mLastError;
}

LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
	return wcstring;
}