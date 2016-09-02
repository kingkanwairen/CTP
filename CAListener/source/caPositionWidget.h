#ifndef caPositionWidget_h
#define caPositionWidget_h

#include <QtGui/QTreeWidget>

class caPositionWidget : public  QTreeWidget
{
	Q_OBJECT
public:

	caPositionWidget(QWidget* parent = 0);

	~caPositionWidget();

public slots:
	void doUpdatePosition(const QStringList& posList);

private:
	QStringList mHeaders;
};
#endif