#ifndef caLogWidget_h
#define caLogWidget_h

#include <QtGui/QTreeWidget>

class caLogWidget : public  QTreeWidget
{
	Q_OBJECT
public:

	caLogWidget(QWidget* parent = 0);

	~caLogWidget();

public slots:

	void doUpdateLog(const QStringList& logList);
	void doDisconnetEngine(const QString& reson);

private:

	QList<QTreeWidgetItem*> mListItem;
	QStringList mHeaders;
};
#endif