#ifndef CAIAPI_H
#define CAIAPI_H

#ifdef CAIAPI
#define CAIAPI _declspec(dllexport)
#else
#define CAIAPI _declspec(dllimport)
#endif


class CaiSpi
{
public:
	
	//��Ϣ��Ӧ
	virtual void OnRecvMsg(char* pText) = 0;

	//���ӶϿ�
	virtual void OnCorrelatorDisconnect(char* pReason) = 0;

};



class CAIAPI CaiApi
{ 

public:
	
	//����ָ��
	static CaiApi* CreateCaiApi(char* hostname = "localhost", int port = 15903, int nType = 0);
	
	//�ͷ�����
	virtual bool Release() = 0;

	//��ʼ��
	virtual bool Init(char** pChannels) = 0;
	
	//ע��ص�
	virtual void RegisterSpi(CaiSpi *pSpi) = 0;

	//ע��MonitorScript
	virtual bool CreateMonitorScript(char* pScript) = 0;

	//����Event
	virtual bool SendEvent(char* pEvent) = 0;	

	//��ȡEngineStatus
	virtual void GetEngineStatus(char* pBuffer, int size) = 0;


	//ɾ��MonitorScript
	virtual bool DeleteMonitorScript(char* pName) = 0;

	//��ȡ�쳣��Ϣ
	virtual void GetException(char* pInfo, int size) = 0;
};

#endif
