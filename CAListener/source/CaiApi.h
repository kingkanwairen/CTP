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
	
	//消息响应
	virtual void OnRecvMsg(char* pText) = 0;

	//连接断开
	virtual void OnCorrelatorDisconnect(char* pReason) = 0;

};



class CAIAPI CaiApi
{ 

public:
	
	//创建指向
	static CaiApi* CreateCaiApi(char* hostname = "localhost", int port = 15903, int nType = 0);
	
	//释放连接
	virtual bool Release() = 0;

	//初始化
	virtual bool Init(char** pChannels) = 0;
	
	//注册回调
	virtual void RegisterSpi(CaiSpi *pSpi) = 0;

	//注入MonitorScript
	virtual bool CreateMonitorScript(char* pScript) = 0;

	//发送Event
	virtual bool SendEvent(char* pEvent) = 0;	

	//获取EngineStatus
	virtual void GetEngineStatus(char* pBuffer, int size) = 0;


	//删除MonitorScript
	virtual bool DeleteMonitorScript(char* pName) = 0;

	//获取异常信息
	virtual void GetException(char* pInfo, int size) = 0;
};

#endif
