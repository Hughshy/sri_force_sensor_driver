#ifndef SRI_COMM_SERIAL_H
#define SRI_COMM_SERIAL_H

#include "sriCommDefine.h"
#include "sriCommParser.h"



class CSRICommSerial
{
public:
	CSRICommSerial();
	~CSRICommSerial();
	bool OnReceivedData(BYTE* data, int dataLen);
	bool CSRICommSerial::OpenPort(std::string   Port);
	bool Config_Com(int BaudRate);//���ô���
	bool ComWrite(LPBYTE buf, int& len);
	unsigned static int __stdcall ComRecv(void*);//��ȡ����//
	void Close_Com();
	bool AddCommParser(CSRICommParser* parser);
	std::string GetLastError();

	bool CSRICommSerial::run(int BaudRate);

	bool SetNetworkFailureCallbackFunction(SRICommNetworkFailureCallbackFunction networkFailureCallback);

private:
	HANDLE hCom;
	OVERLAPPED m_ovWrite;//����д������
	OVERLAPPED m_ovRead;//���ڶ�ȡ����
	OVERLAPPED m_ovWait;//���ڵȴ�����
	volatile bool m_IsOpen;//�����Ƿ��
	HANDLE m_Thread;//��ȡ�߳̾��



	struct sockaddr_in mLocalAddr;
	struct sockaddr_in mRemoteAddr;
	bool CheckTimeoutError();
	std::thread mThread;
	bool mIsStopThread;
	bool mIsTreadStoped;

	void CSRICommSerial::GetLastSerialError(std::string functionName);

	std::vector<CSRICommParser*> mParserList;

	SRICommNetworkFailureCallbackFunction mNetworkFailureCallback;

	std::string mLastError;

};

#endif

LPCWSTR stringToLPCWSTR(std::string orig);