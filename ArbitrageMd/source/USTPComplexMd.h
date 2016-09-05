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

	USTPComplexMd(const QString& firstIns, const QString& secondIns, const double& firstFactor, const double& secFactor, const double& constValue,
		USTPMarketWidget* pMarketWidget);

	virtual~USTPComplexMd();

public slots:
	void doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
		const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
		const double& lowestPrice, const int& volume);

signals:
	void onUSTPComplexMd(const QString& key, const QString& firstIns, const QString& secIns, const QString& bbPrice, const QString& bsPrice, const QString& sbPrice,
		const QString& ssPrice, const QString& bbQty, const QString& bsQty, const QString& sbQty, const QString& ssQty);
private:
	void initConnect(USTPMarketWidget* pMarketWidget);

	void calculateComplexMd();

	int getInsPrcision(const double& value);

private:
	QThread mComplexMdThread;
	QString mFirstInstrument;
	QString mSecondInstrument;
	QString mComplexKey;
	double mFirstBidPrice;
	double mFirstAskPrice;
	double mSecondBidPrice;
	double mSecondAskPrice;
	double mFirstFactor;
	double mSecFactor;
	double mConstFactor;
	int mSecondBidQty;
	int mSecondAskQty;
	int mFirstBidQty;
	int mFirstAskQty;
	int mInsPrecision;
};

#endif