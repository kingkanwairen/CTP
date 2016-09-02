#include "caLogWidget.h"
#include "caPublicDefine.h"
#include <QtCore/QTime>

caLogWidget::caLogWidget(QWidget* parent)
:QTreeWidget(parent)
{	
	mHeaders << TREE_WIDGET_TAB_HEAD_1 << TREE_WIDGET_TAB_HEAD_2;
	setHeaderLabels(mHeaders);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setAlternatingRowColors(true);
	setRootIsDecorated(false);
}

caLogWidget::~caLogWidget()
{
	
}

void caLogWidget::doUpdateLog(const QStringList& logList)
{	
	int isize = logList.size();
	if(isize == mHeaders.size()){
		QTreeWidgetItem *showItem = new QTreeWidgetItem(this);
		for(int i = 0; i < isize; i++){
			showItem->setText(i, logList[i]);
		}
		insertTopLevelItem(0, showItem);
	}	
}

void caLogWidget::doDisconnetEngine(const QString& reson)
{
	if (!reson.isEmpty())
	{
		//ÐÂÌí¼Ó
		QTreeWidgetItem *showItem = new QTreeWidgetItem(this);
		showItem->setText(0, QTime::currentTime().toString("hh:mm:ss.zzz"));
		showItem->setText(1, reson);
		insertTopLevelItem(0, showItem);
	}
}

#include "moc_caLogWidget.cpp"