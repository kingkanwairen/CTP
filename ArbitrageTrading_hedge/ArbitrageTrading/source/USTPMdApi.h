#ifndef USTP_MD_API_H
#define USTP_MD_API_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include "ThostFtdcMdApi.h"

class USTPMdApi : public QObject
{
	Q_OBJECT
public:

	USTPMdApi();

	virtual~ USTPMdApi();

	static bool initilize(CThostFtdcMdApi* pMdApi);

	static bool finalize();

	static int reqUserLogin(const QString& brokerId, const QString& userId, const QString& password);

	static int subMarketData(const QString& instrumentId);

private:
	static USTPMdApi* mThis;
	CThostFtdcMdApi* m_pMdApi;
	int nRequestId;
};

#endif