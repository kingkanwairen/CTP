#ifndef USTP_CTP_LOADER_H
#define USTP_CTP_LOADER_H

#include <QtCore/QObject>
#include "USTPMdSpi.h"
#include "USTPTradeSpi.h"
#include "USTPBase64.h"

class USTPCtpLoader : public QObject
{
public:

	static bool initialize();

	static bool finalize();

	static bool start();

	static bool stop();

	static QString getBrokerId();

	static QString getDateTime();

	static QString getAuthUser();

	static QString getMdPath();

	static QString getTradePath();

	static CThostFtdcMdApi* getMdApi();

	static CThostFtdcTraderApi* getTradeApi();

	static USTPMdSpi* getMdSpi();

	static USTPTradeSpi* getTradeSpi();

protected:

	USTPCtpLoader();

	virtual~ USTPCtpLoader();

private:

	static USTPCtpLoader *mThis;

private:

	bool loadXMLFile(const QString& path);

	bool openTradeLog();

	bool closeTradeLog();

	bool openProfileFile();

	bool closeProfileFile();

	bool startCtpMdThread();

	bool startCtpTradeThread();
	
	QString mBrokerId;
	QString mTradePath;
	QString mMdPath;
	QString mAuthUser;
	QString mDateTime;
	CThostFtdcMdApi* m_pMdApi;
	CThostFtdcTraderApi* m_pTradeApi;
	USTPMdSpi* m_pMdSpi;
	USTPTradeSpi* m_pTradeSpi;
	USTPBase64* m_pBase64;
};

#endif
