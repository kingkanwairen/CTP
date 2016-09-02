#include "caTradeWidget.h"
#include "caPublicDefine.h"
#include <QtGui/QMessageBox>

caTradeWidget::caTradeWidget(QWidget* parent)
:QTreeWidget(parent)
{	
	mHeaders << TRADE_WIDGET_TAB_HEAD_1 << TRADE_WIDGET_TAB_HEAD_2 << TRADE_WIDGET_TAB_HEAD_3
		<< TRADE_WIDGET_TAB_HEAD_4 << TRADE_WIDGET_TAB_HEAD_5 << TRADE_WIDGET_TAB_HEAD_6 
		<< TRADE_WIDGET_TAB_HEAD_7 << TRADE_WIDGET_TAB_HEAD_8;
	setHeaderLabels(mHeaders);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setAlternatingRowColors(true);
	setRootIsDecorated(false);
	setColumnHidden(TRADE_HEAD_LENGTH - 1, true);
	setMinimumHeight(TRADE_WINDOW_MIN_HEIGHT);
}

caTradeWidget::~caTradeWidget()
{

}

void caTradeWidget::doUpdateTrade(const QStringList& tradeList)
{	
	int isize = tradeList.size();
	if (isize == mHeaders.size()){
		QList<QTreeWidgetItem*> itemList = findItems(tradeList[isize - 1], Qt::MatchRecursive, isize - 1);
		if (itemList.size() > 1){
			ShowWarning(tr("列表显示出错."));
			return;
		}

		if (itemList.size() == 0){
			//新添加的
			QTreeWidgetItem *showItem = new QTreeWidgetItem(this);
			for(int i = 0; i < isize; i++){
				showItem->setText(i, tradeList[i]);
			}
			insertTopLevelItem(0, showItem);
		}
		else{
			//更新
			QTreeWidgetItem* item = itemList.at(0);
			for(int i = 0; i < isize; i++){
				item->setText(i, tradeList[i]);
			}
		}
	}
}

#include "moc_caTradeWidget.cpp"