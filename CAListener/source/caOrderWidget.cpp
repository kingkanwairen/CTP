#include "caOrderWidget.h"
#include "caPublicDefine.h"
#include <QtGui/QMessageBox>

caOrderWidget::caOrderWidget(QWidget* parent)
:QTreeWidget(parent)
{	
	mHeaders << ORDER_WIDGET_TAB_HEAD_1 << ORDER_WIDGET_TAB_HEAD_2 << ORDER_WIDGET_TAB_HEAD_3
		<< ORDER_WIDGET_TAB_HEAD_4 << ORDER_WIDGET_TAB_HEAD_5 << ORDER_WIDGET_TAB_HEAD_6
		<< ORDER_WIDGET_TAB_HEAD_7 << ORDER_WIDGET_TAB_HEAD_8 << ORDER_WIDGET_TAB_HEAD_9
		<< ORDER_WIDGET_TAB_HEAD_10;
	setHeaderLabels(mHeaders);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setAlternatingRowColors(true);
	setRootIsDecorated(false);
	setColumnHidden(ORDER_HEAD_LENGTH - 1, true);
	setMinimumHeight(ORDER_WINDOW_MIN_HEIGHT);
}

caOrderWidget::~caOrderWidget()
{

}

void caOrderWidget::doUpdateOrder(const QStringList& orderList)
{	
	int isize = orderList.size();
	if (isize == mHeaders.size()){
		QList<QTreeWidgetItem*> itemList = findItems(orderList[isize - 1], Qt::MatchRecursive, isize - 1);
		if (itemList.size() > 1){
			ShowWarning(tr("列表显示出错."));
			return;
		}

		if (itemList.size() == 0){
			//新添加的
			QTreeWidgetItem *showItem = new QTreeWidgetItem(this);
			for(int i = 0; i < isize; i++){
				showItem->setText(i, orderList[i]);
			}
			insertTopLevelItem(0, showItem);
		}
		else{
			//更新
			QTreeWidgetItem* item = itemList.at(0);
			for(int i = 0; i < isize; i++){
				item->setText(i, orderList[i]);
			}
		}
	}
}


#include "moc_caOrderWidget.cpp"