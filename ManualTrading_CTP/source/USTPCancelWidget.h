#ifndef USTP_CANCEL_WIDGET_H
#define USTP_CANCEL_WIDGET_H

#include <QtGui/QTableWidget>
#include <QtGui/QPushButton>

class USTPCancelWidget : public  QTableWidget
{
	Q_OBJECT
public:

	USTPCancelWidget(QWidget* parent = 0);

	~USTPCancelWidget();

public slots:

	void doUpdateOrderShow(const QString& orderLabel, const QString& requestId, const QString& instrumentId, const char& status, const char& bs, const int&
		orderVolume, const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedge, const double& orderPrice);

	void doOrderFinished(const QString& orderLabel, const QString& instrumentId);
	
	void doBtnDeleteClicked();

	void doBtnCancelClicked();

	void doKeyDownHock(const int& key);
signals:

	void onDelOrder(const QString& orderStyle);
private:
	void addItem(const int& row, const int& colume, const QString& text);
	QPushButton* createButton(const QString &text, const char *member);
private:
	QString getBSText(const char& bs);
	QString getOffsetFlagText(const char& offsetFlag);
private:
	QStringList mHeaders;
};
#endif