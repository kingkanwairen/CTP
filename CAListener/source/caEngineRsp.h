#ifndef caEngineRsp_h
#define caEngineRsp_h

#include "CaiApi.h"
#include<vector>
#include <QtCore/QObject>

using namespace std;

class caEngineRsp : public QObject, public CaiSpi
{
	Q_OBJECT

public:
	caEngineRsp(void);
	virtual ~caEngineRsp(void);
	virtual void OnRecvMsg(char* pText);
	virtual void OnCorrelatorDisconnect(char* pReason);

signals:
	void onUpdateOrder(const QStringList& orderList);
	void onUpdateTrade(const QStringList& tradeList);
	void onUpdatePosition(const QStringList& tradeList);
	void onUpdateLog(const QStringList& logList);
	void onDisconnetEngine(const QString& reson);
};

#endif