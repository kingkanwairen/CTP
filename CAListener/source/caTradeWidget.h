#ifndef caTradeWidget_h
#define caTradeWidget_h

#include <QtGui/QTreeWidget>

class caTradeWidget : public  QTreeWidget
{
	Q_OBJECT
public:

	caTradeWidget(QWidget* parent = 0);

	~caTradeWidget();

public slots:
	void doUpdateTrade(const QStringList& tradeList);

private:
	QStringList mHeaders;
};
#endif