#ifndef caOrderWidget_h
#define caOrderWidget_h

#include <QtGui/QTreeWidget>

class caOrderWidget : public  QTreeWidget
{
	Q_OBJECT
public:

	caOrderWidget(QWidget* parent = 0);

	~caOrderWidget();

public slots:
	void doUpdateOrder(const QStringList& orderList);

private:
	QStringList mHeaders;
};
#endif