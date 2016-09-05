#ifndef USTP_COMPLEX_MD_H
#define USTP_COMPLEX_MD_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QThread>
class USTPMarketWidget;

class USTPComplexMd : public QObject
{
	Q_OBJECT
public:

	USTPComplexMd(const QString& selIns, USTPMarketWidget* pMarketWidget);

	virtual~USTPComplexMd();

public slots:
	void doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
		const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
		const double& lowestPrice, const int& volume, const double& preClosePrice, const double& holdPosition);
	void doGetSpeDepthMd(const QString& selIns, const int& direction);

signals:
	void onUSTPSpeComplexMd(const QString& selIns, const QString& bidPrice, const QString& bidVolume, const QString& askPrice, const QString& askVolume,
		const QString& tradeVolume, const QString& lastPrice, const QString& upDownPrice, const QString& preClosePrice, const QString& openPrice, const QString& holdQty);
		void onUpdateSpeDepthMd(const QString& selIns, const int& direction, const double& orderPrice);
private:
	void initConnect(USTPMarketWidget* pMarketWidget);

	int getInsPrcision(const double& value);

private:
	QString mInstrument;
	double mBidPrice;
	double mAskPrice;
	int mBidQty;
	int mAskQty;
	int mTradeQty;
	int mInsPrecision;
	bool mIsInit;
	QThread mComplexMdThread;
};

#endif