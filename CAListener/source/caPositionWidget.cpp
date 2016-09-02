#include "caPositionWidget.h"
#include "caPublicDefine.h"
#include <QtGui/QMessageBox>

caPositionWidget::caPositionWidget(QWidget* parent)
:QTreeWidget(parent)
{	
	mHeaders << POSITION_WIDGET_TAB_HEAD_1 << POSITION_WIDGET_TAB_HEAD_2 << POSITION_WIDGET_TAB_HEAD_3
		<< POSITION_WIDGET_TAB_HEAD_4 << POSITION_WIDGET_TAB_HEAD_5 << POSITION_WIDGET_TAB_HEAD_6;
	setHeaderLabels(mHeaders);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setAlternatingRowColors(true);
	setRootIsDecorated(false);
	setColumnHidden(POSITION_HEAD_LENGTH - 1, true);
	setMinimumHeight(POSITION_WINDOW_MIN_HEIGHT);
}

caPositionWidget::~caPositionWidget()
{

}

void caPositionWidget::doUpdatePosition(const QStringList& posList)
{	
	int isize = posList.size();
	if (isize == mHeaders.size()){
		QList<QTreeWidgetItem*> itemList = findItems(posList[isize - 1], Qt::MatchRecursive, isize - 1);
		if (itemList.size() > 1){
			ShowWarning(tr("列表显示出错."));
			return;
		}

		if (itemList.size() == 0){
			//新添加的
			QTreeWidgetItem *showItem = new QTreeWidgetItem(this);
			for(int i = 0; i < isize; i++){
				showItem->setText(i, posList[i]);
			}
			insertTopLevelItem(0, showItem);
		}
		else{
			//更新
			QTreeWidgetItem* item = itemList.at(0);
			for(int i = 0; i < isize; i++){
				item->setText(i, posList[i]);
			}
		}
	}
}

#include "moc_caPositionWidget.cpp"