#ifndef USTP_MARKKET_WIDGET_H
#define USTP_MARKKET_WIDGET_H

#include <QtGui/QTableWidget>
#include <QtCore/QMap>
#include "USTPComplexMd.h"

class USTPMarketWidget : public  QTableWidget
{
	Q_OBJECT
public:

	USTPMarketWidget(QWidget* parent = 0);

	~USTPMarketWidget();

public slots:

		void doUSTPMdFrontConnected();

		void doSubscribeMd(const QStringList& inss);

		void doUSTPComplexMd(const QString& key, const QString& firstIns, const QString& secIns, const QString& bbPrice, const QString& bsPrice, const QString& sbPrice,
			const QString& ssPrice, const QString& bbQty, const QString& bsQty, const QString& sbQty, const QString& ssQty);
private:
	void initConnect();
	void addItem(const int& row, const int& colume, const QString& text);
	void updateItemPrice(const int& row, const int& colume, const QString& price);
	void updateItemVolume(const int& row, const int& colume, const QString& volume);
private:
	QStringList mHeaders;
	QMap<QString, USTPComplexMd*> mUSTPMdMap;
	QMap<QString, QString> mInss;
};
#endif